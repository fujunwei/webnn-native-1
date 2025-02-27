// Copyright 2017 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn_native/Texture.h"

#include <algorithm>

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Math.h"
#include "dawn_native/Adapter.h"
#include "dawn_native/ChainUtils_autogen.h"
#include "dawn_native/Device.h"
#include "dawn_native/EnumMaskIterator.h"
#include "dawn_native/ObjectType_autogen.h"
#include "dawn_native/PassResourceUsage.h"
#include "dawn_native/ValidationUtils_autogen.h"

namespace dawn_native {
    namespace {
        // WebGPU currently does not have texture format reinterpretation. If it does, the
        // code to check for it might go here.
        MaybeError ValidateTextureViewFormatCompatibility(const TextureBase* texture,
                                                          const TextureViewDescriptor* descriptor) {
            if (texture->GetFormat().format != descriptor->format) {
                if (descriptor->aspect != wgpu::TextureAspect::All &&
                    texture->GetFormat().GetAspectInfo(descriptor->aspect).format ==
                        descriptor->format) {
                    return {};
                }

                return DAWN_VALIDATION_ERROR(
                    "The format of texture view is not compatible to the original texture");
            }

            return {};
        }

        // TODO(crbug.com/dawn/814): Implement for 1D texture.
        bool IsTextureViewDimensionCompatibleWithTextureDimension(
            wgpu::TextureViewDimension textureViewDimension,
            wgpu::TextureDimension textureDimension) {
            switch (textureViewDimension) {
                case wgpu::TextureViewDimension::e2D:
                case wgpu::TextureViewDimension::e2DArray:
                case wgpu::TextureViewDimension::Cube:
                case wgpu::TextureViewDimension::CubeArray:
                    return textureDimension == wgpu::TextureDimension::e2D;

                case wgpu::TextureViewDimension::e3D:
                    return textureDimension == wgpu::TextureDimension::e3D;

                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::Undefined:
                    break;
            }
            UNREACHABLE();
        }

        // TODO(crbug.com/dawn/814): Implement for 1D texture.
        bool IsArrayLayerValidForTextureViewDimension(
            wgpu::TextureViewDimension textureViewDimension,
            uint32_t textureViewArrayLayer) {
            switch (textureViewDimension) {
                case wgpu::TextureViewDimension::e2D:
                case wgpu::TextureViewDimension::e3D:
                    return textureViewArrayLayer == 1u;
                case wgpu::TextureViewDimension::e2DArray:
                    return true;
                case wgpu::TextureViewDimension::Cube:
                    return textureViewArrayLayer == 6u;
                case wgpu::TextureViewDimension::CubeArray:
                    return textureViewArrayLayer % 6 == 0;

                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::Undefined:
                    break;
            }
            UNREACHABLE();
        }

        MaybeError ValidateSampleCount(const TextureDescriptor* descriptor,
                                       wgpu::TextureUsage usage,
                                       const Format* format) {
            DAWN_INVALID_IF(!IsValidSampleCount(descriptor->sampleCount),
                            "The sample count (%u) of the texture is not supported.",
                            descriptor->sampleCount);

            if (descriptor->sampleCount > 1) {
                DAWN_INVALID_IF(descriptor->mipLevelCount > 1,
                                "The mip level count (%u) of a multisampled texture is not 1.",
                                descriptor->mipLevelCount);

                // Multisampled 1D and 3D textures are not supported in D3D12/Metal/Vulkan.
                // Multisampled 2D array texture is not supported because on Metal it requires the
                // version of macOS be greater than 10.14.
                DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                                "The dimension (%s) of a multisampled texture is not 2D.",
                                descriptor->dimension);

                DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers > 1,
                                "The depthOrArrayLayers (%u) of a multisampled texture is not 1.",
                                descriptor->size.depthOrArrayLayers);

                // If a format can support multisample, it must be renderable. Because Vulkan
                // requires that if the format is not color-renderable or depth/stencil renderable,
                // sampleCount must be 1.
                DAWN_INVALID_IF(!format->isRenderable,
                                "The texture format (%s) does not support multisampling.",
                                format->format);

                // Compressed formats are not renderable. They cannot support multisample.
                ASSERT(!format->isCompressed);

                DAWN_INVALID_IF(usage & wgpu::TextureUsage::StorageBinding,
                                "The sample count (%u) of a storage textures is not 1.",
                                descriptor->sampleCount);
            }

            return {};
        }

        MaybeError ValidateTextureViewDimensionCompatibility(
            const TextureBase* texture,
            const TextureViewDescriptor* descriptor) {
            DAWN_INVALID_IF(
                !IsArrayLayerValidForTextureViewDimension(descriptor->dimension,
                                                          descriptor->arrayLayerCount),
                "The dimension (%s) of the texture view is not compatible with the layer count "
                "(%u) of %s.",
                descriptor->dimension, descriptor->arrayLayerCount, texture);

            DAWN_INVALID_IF(
                !IsTextureViewDimensionCompatibleWithTextureDimension(descriptor->dimension,
                                                                      texture->GetDimension()),
                "The dimension (%s) of the texture view is not compatible with the dimension (%s) "
                "of %s.",
                descriptor->dimension, texture->GetDimension(), texture);

            switch (descriptor->dimension) {
                case wgpu::TextureViewDimension::Cube:
                case wgpu::TextureViewDimension::CubeArray:
                    DAWN_INVALID_IF(
                        texture->GetSize().width != texture->GetSize().height,
                        "A %s texture view is not compatible with %s because the texture's width "
                        "(%u) and height (%u) are not equal.",
                        descriptor->dimension, texture, texture->GetSize().width,
                        texture->GetSize().height);
                    break;

                case wgpu::TextureViewDimension::e2D:
                case wgpu::TextureViewDimension::e2DArray:
                case wgpu::TextureViewDimension::e3D:
                    break;

                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::Undefined:
                    UNREACHABLE();
                    break;
            }

            return {};
        }

        MaybeError ValidateTextureSize(const DeviceBase* device,
                                       const TextureDescriptor* descriptor,
                                       const Format* format) {
            ASSERT(descriptor->size.width != 0 && descriptor->size.height != 0 &&
                   descriptor->size.depthOrArrayLayers != 0);
            const CombinedLimits& limits = device->GetLimits();
            Extent3D maxExtent;
            switch (descriptor->dimension) {
                case wgpu::TextureDimension::e2D:
                    maxExtent = {limits.v1.maxTextureDimension2D, limits.v1.maxTextureDimension2D,
                                 limits.v1.maxTextureArrayLayers};
                    break;
                case wgpu::TextureDimension::e3D:
                    maxExtent = {limits.v1.maxTextureDimension3D, limits.v1.maxTextureDimension3D,
                                 limits.v1.maxTextureDimension3D};
                    break;
                case wgpu::TextureDimension::e1D:
                default:
                    UNREACHABLE();
            }
            DAWN_INVALID_IF(descriptor->size.width > maxExtent.width ||
                                descriptor->size.height > maxExtent.height ||
                                descriptor->size.depthOrArrayLayers > maxExtent.depthOrArrayLayers,
                            "Texture size (%s) exceeded maximum texture size (%s).",
                            &descriptor->size, &maxExtent);

            uint32_t maxMippedDimension = descriptor->size.width;
            if (descriptor->dimension != wgpu::TextureDimension::e1D) {
                maxMippedDimension = std::max(maxMippedDimension, descriptor->size.height);
            }
            if (descriptor->dimension == wgpu::TextureDimension::e3D) {
                maxMippedDimension =
                    std::max(maxMippedDimension, descriptor->size.depthOrArrayLayers);
            }
            DAWN_INVALID_IF(
                Log2(maxMippedDimension) + 1 < descriptor->mipLevelCount,
                "Texture mip level count (%u) exceeds the maximum (%u) for its size (%s).",
                descriptor->mipLevelCount, Log2(maxMippedDimension) + 1, &descriptor->size);

            if (format->isCompressed) {
                const TexelBlockInfo& blockInfo =
                    format->GetAspectInfo(wgpu::TextureAspect::All).block;
                DAWN_INVALID_IF(
                    descriptor->size.width % blockInfo.width != 0 ||
                        descriptor->size.height % blockInfo.height != 0,
                    "The size (%s) of the texture is not a multiple of the block width (%u) and "
                    "height (%u) of the texture format (%s).",
                    &descriptor->size, blockInfo.width, blockInfo.height, format->format);
            }

            return {};
        }

        MaybeError ValidateTextureUsage(const TextureDescriptor* descriptor,
                                        wgpu::TextureUsage usage,
                                        const Format* format) {
            DAWN_TRY(dawn_native::ValidateTextureUsage(usage));

            constexpr wgpu::TextureUsage kValidCompressedUsages =
                wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                wgpu::TextureUsage::CopyDst;
            DAWN_INVALID_IF(
                format->isCompressed && !IsSubset(usage, kValidCompressedUsages),
                "The texture usage (%s) is incompatible with the compressed texture format (%s).",
                usage, format->format);

            DAWN_INVALID_IF(
                !format->isRenderable && (usage & wgpu::TextureUsage::RenderAttachment),
                "The texture usage (%s) includes %s, which is incompatible with the non-renderable "
                "format (%s).",
                usage, wgpu::TextureUsage::RenderAttachment, format->format);

            DAWN_INVALID_IF(
                !format->supportsStorageUsage && (usage & wgpu::TextureUsage::StorageBinding),
                "The texture usage (%s) includes %s, which is incompatible with the format (%s).",
                usage, wgpu::TextureUsage::StorageBinding, format->format);

            // Only allows simple readonly texture usages.
            constexpr wgpu::TextureUsage kValidMultiPlanarUsages =
                wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc;
            DAWN_INVALID_IF(
                format->IsMultiPlanar() && !IsSubset(usage, kValidMultiPlanarUsages),
                "The texture usage (%s) is incompatible with the multi-planar format (%s).", usage,
                format->format);

            return {};
        }

    }  // anonymous namespace

    MaybeError ValidateTextureDescriptor(const DeviceBase* device,
                                         const TextureDescriptor* descriptor) {
        DAWN_TRY(ValidateSingleSType(descriptor->nextInChain,
                                     wgpu::SType::DawnTextureInternalUsageDescriptor));

        const DawnTextureInternalUsageDescriptor* internalUsageDesc = nullptr;
        FindInChain(descriptor->nextInChain, &internalUsageDesc);

        DAWN_INVALID_IF(descriptor->dimension == wgpu::TextureDimension::e1D,
                        "1D textures aren't supported (yet).");

        DAWN_INVALID_IF(
            internalUsageDesc != nullptr && !device->IsFeatureEnabled(Feature::DawnInternalUsages),
            "The dawn-internal-usages feature is not enabled");

        const Format* format;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->format));

        wgpu::TextureUsage usage = descriptor->usage;
        if (internalUsageDesc != nullptr) {
            usage |= internalUsageDesc->internalUsage;
        }

        DAWN_TRY(ValidateTextureUsage(descriptor, usage, format));
        DAWN_TRY(ValidateTextureDimension(descriptor->dimension));
        DAWN_TRY(ValidateSampleCount(descriptor, usage, format));

        DAWN_INVALID_IF(descriptor->size.width == 0 || descriptor->size.height == 0 ||
                            descriptor->size.depthOrArrayLayers == 0 ||
                            descriptor->mipLevelCount == 0,
                        "The texture size (%s) or mipLevelCount (%u) is empty.", &descriptor->size,
                        descriptor->mipLevelCount);

        DAWN_INVALID_IF(
            descriptor->dimension != wgpu::TextureDimension::e2D && format->isCompressed,
            "The dimension (%s) of a texture with a compressed format (%s) is not 2D.",
            descriptor->dimension, format->format);

        // Depth/stencil formats are valid for 2D textures only. Metal has this limit. And D3D12
        // doesn't support depth/stencil formats on 3D textures.
        DAWN_INVALID_IF(
            descriptor->dimension != wgpu::TextureDimension::e2D &&
                (format->aspects & (Aspect::Depth | Aspect::Stencil)) != 0,
            "The dimension (%s) of a texture with a depth/stencil format (%s) is not 2D.",
            descriptor->dimension, format->format);

        DAWN_TRY(ValidateTextureSize(device, descriptor, format));

        // TODO(crbug.com/dawn/838): Implement a workaround for this issue.
        // Readbacks from the non-zero mip of a stencil texture may contain garbage data.
        DAWN_INVALID_IF(
            device->IsToggleEnabled(Toggle::DisallowUnsafeAPIs) && format->HasStencil() &&
                descriptor->mipLevelCount > 1 &&
                device->GetAdapter()->GetBackendType() == wgpu::BackendType::Metal,
            "https://crbug.com/dawn/838: Stencil textures with more than one mip level are "
            "disabled on Metal.");

        DAWN_INVALID_IF(
            device->IsToggleEnabled(Toggle::DisableR8RG8Mipmaps) && descriptor->mipLevelCount > 1 &&
                (descriptor->format == wgpu::TextureFormat::R8Unorm ||
                 descriptor->format == wgpu::TextureFormat::RG8Unorm),
            "https://crbug.com/dawn/1071: r8unorm and rg8unorm textures with more than one mip "
            "level are disabled on Metal.");

        return {};
    }

    MaybeError ValidateTextureViewDescriptor(const DeviceBase* device,
                                             const TextureBase* texture,
                                             const TextureViewDescriptor* descriptor) {
        DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

        // Parent texture should have been already validated.
        ASSERT(texture);
        ASSERT(!texture->IsError());

        DAWN_TRY(ValidateTextureViewDimension(descriptor->dimension));
        DAWN_INVALID_IF(descriptor->dimension == wgpu::TextureViewDimension::e1D,
                        "1D texture views aren't supported (yet).");

        DAWN_TRY(ValidateTextureFormat(descriptor->format));

        DAWN_TRY(ValidateTextureAspect(descriptor->aspect));
        DAWN_INVALID_IF(
            SelectFormatAspects(texture->GetFormat(), descriptor->aspect) == Aspect::None,
            "Texture format (%s) does not have the texture view's selected aspect (%s).",
            texture->GetFormat().format, descriptor->aspect);

        DAWN_INVALID_IF(descriptor->arrayLayerCount == 0 || descriptor->mipLevelCount == 0,
                        "The texture view's arrayLayerCount (%u) or mipLevelCount (%u) is zero.",
                        descriptor->arrayLayerCount, descriptor->mipLevelCount);

        DAWN_INVALID_IF(
            uint64_t(descriptor->baseArrayLayer) + uint64_t(descriptor->arrayLayerCount) >
                uint64_t(texture->GetArrayLayers()),
            "Texture view array layer range (baseArrayLayer: %u, arrayLayerCount: %u) exceeds the "
            "texture's array layer count (%u).",
            descriptor->baseArrayLayer, descriptor->arrayLayerCount, texture->GetArrayLayers());

        DAWN_INVALID_IF(
            uint64_t(descriptor->baseMipLevel) + uint64_t(descriptor->mipLevelCount) >
                uint64_t(texture->GetNumMipLevels()),
            "Texture view mip level range (baseMipLevel: %u, mipLevelCount: %u) exceeds the "
            "texture's mip level count (%u).",
            descriptor->baseMipLevel, descriptor->mipLevelCount, texture->GetNumMipLevels());

        DAWN_TRY(ValidateTextureViewFormatCompatibility(texture, descriptor));
        DAWN_TRY(ValidateTextureViewDimensionCompatibility(texture, descriptor));

        return {};
    }

    TextureViewDescriptor GetTextureViewDescriptorWithDefaults(
        const TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        ASSERT(texture);

        TextureViewDescriptor desc = {};
        if (descriptor) {
            desc = *descriptor;
        }

        // The default value for the view dimension depends on the texture's dimension with a
        // special case for 2DArray being chosen automatically if arrayLayerCount is unspecified.
        if (desc.dimension == wgpu::TextureViewDimension::Undefined) {
            switch (texture->GetDimension()) {
                case wgpu::TextureDimension::e1D:
                    desc.dimension = wgpu::TextureViewDimension::e1D;
                    break;

                case wgpu::TextureDimension::e2D:
                    desc.dimension = wgpu::TextureViewDimension::e2D;
                    break;

                case wgpu::TextureDimension::e3D:
                    desc.dimension = wgpu::TextureViewDimension::e3D;
                    break;
            }
        }

        if (desc.format == wgpu::TextureFormat::Undefined) {
            // TODO(dawn:682): Use GetAspectInfo(aspect).
            desc.format = texture->GetFormat().format;
        }
        if (desc.arrayLayerCount == wgpu::kArrayLayerCountUndefined) {
            switch (desc.dimension) {
                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::e2D:
                case wgpu::TextureViewDimension::e3D:
                    desc.arrayLayerCount = 1;
                    break;
                case wgpu::TextureViewDimension::Cube:
                    desc.arrayLayerCount = 6;
                    break;
                case wgpu::TextureViewDimension::e2DArray:
                case wgpu::TextureViewDimension::CubeArray:
                    desc.arrayLayerCount = texture->GetArrayLayers() - desc.baseArrayLayer;
                    break;
                default:
                    // We don't put UNREACHABLE() here because we validate enums only after this
                    // function sets default values. Otherwise, the UNREACHABLE() will be hit.
                    break;
            }
        }

        if (desc.mipLevelCount == wgpu::kMipLevelCountUndefined) {
            desc.mipLevelCount = texture->GetNumMipLevels() - desc.baseMipLevel;
        }
        return desc;
    }

    // WebGPU only supports sample counts of 1 and 4. We could expand to more based on
    // platform support, but it would probably be a feature.
    bool IsValidSampleCount(uint32_t sampleCount) {
        switch (sampleCount) {
            case 1:
            case 4:
                return true;

            default:
                return false;
        }
    }

    // TextureBase

    TextureBase::TextureBase(DeviceBase* device,
                             const TextureDescriptor* descriptor,
                             TextureState state)
        : ApiObjectBase(device, descriptor->label),
          mDimension(descriptor->dimension),
          mFormat(device->GetValidInternalFormat(descriptor->format)),
          mSize(descriptor->size),
          mMipLevelCount(descriptor->mipLevelCount),
          mSampleCount(descriptor->sampleCount),
          mUsage(descriptor->usage),
          mInternalUsage(mUsage),
          mState(state) {
        uint32_t subresourceCount =
            mMipLevelCount * GetArrayLayers() * GetAspectCount(mFormat.aspects);
        mIsSubresourceContentInitializedAtIndex = std::vector<bool>(subresourceCount, false);

        const DawnTextureInternalUsageDescriptor* internalUsageDesc = nullptr;
        FindInChain(descriptor->nextInChain, &internalUsageDesc);
        if (internalUsageDesc != nullptr) {
            mInternalUsage |= internalUsageDesc->internalUsage;
        }
        TrackInDevice();
    }

    static Format kUnusedFormat;

    TextureBase::TextureBase(DeviceBase* device, TextureState state)
        : ApiObjectBase(device, kLabelNotImplemented), mFormat(kUnusedFormat), mState(state) {
        TrackInDevice();
    }

    TextureBase::TextureBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ApiObjectBase(device, tag), mFormat(kUnusedFormat) {
    }

    void TextureBase::DestroyImpl() {
        mState = TextureState::Destroyed;
    }

    // static
    TextureBase* TextureBase::MakeError(DeviceBase* device) {
        return new TextureBase(device, ObjectBase::kError);
    }

    ObjectType TextureBase::GetType() const {
        return ObjectType::Texture;
    }

    wgpu::TextureDimension TextureBase::GetDimension() const {
        ASSERT(!IsError());
        return mDimension;
    }

    const Format& TextureBase::GetFormat() const {
        ASSERT(!IsError());
        return mFormat;
    }
    const Extent3D& TextureBase::GetSize() const {
        ASSERT(!IsError());
        return mSize;
    }
    uint32_t TextureBase::GetWidth() const {
        ASSERT(!IsError());
        return mSize.width;
    }
    uint32_t TextureBase::GetHeight() const {
        ASSERT(!IsError());
        ASSERT(mDimension != wgpu::TextureDimension::e1D);
        return mSize.height;
    }
    uint32_t TextureBase::GetDepth() const {
        ASSERT(!IsError());
        ASSERT(mDimension == wgpu::TextureDimension::e3D);
        return mSize.depthOrArrayLayers;
    }
    uint32_t TextureBase::GetArrayLayers() const {
        ASSERT(!IsError());
        // TODO(crbug.com/dawn/814): Update for 1D textures when they are supported.
        ASSERT(mDimension != wgpu::TextureDimension::e1D);
        if (mDimension == wgpu::TextureDimension::e3D) {
            return 1;
        }
        return mSize.depthOrArrayLayers;
    }
    uint32_t TextureBase::GetNumMipLevels() const {
        ASSERT(!IsError());
        return mMipLevelCount;
    }
    SubresourceRange TextureBase::GetAllSubresources() const {
        ASSERT(!IsError());
        return {mFormat.aspects, {0, GetArrayLayers()}, {0, mMipLevelCount}};
    }
    uint32_t TextureBase::GetSampleCount() const {
        ASSERT(!IsError());
        return mSampleCount;
    }
    uint32_t TextureBase::GetSubresourceCount() const {
        ASSERT(!IsError());
        return static_cast<uint32_t>(mIsSubresourceContentInitializedAtIndex.size());
    }
    wgpu::TextureUsage TextureBase::GetUsage() const {
        ASSERT(!IsError());
        return mUsage;
    }
    wgpu::TextureUsage TextureBase::GetInternalUsage() const {
        ASSERT(!IsError());
        return mInternalUsage;
    }

    TextureBase::TextureState TextureBase::GetTextureState() const {
        ASSERT(!IsError());
        return mState;
    }

    uint32_t TextureBase::GetSubresourceIndex(uint32_t mipLevel,
                                              uint32_t arraySlice,
                                              Aspect aspect) const {
        ASSERT(HasOneBit(aspect));
        return mipLevel +
               GetNumMipLevels() * (arraySlice + GetArrayLayers() * GetAspectIndex(aspect));
    }

    bool TextureBase::IsSubresourceContentInitialized(const SubresourceRange& range) const {
        ASSERT(!IsError());
        for (Aspect aspect : IterateEnumMask(range.aspects)) {
            for (uint32_t arrayLayer = range.baseArrayLayer;
                 arrayLayer < range.baseArrayLayer + range.layerCount; ++arrayLayer) {
                for (uint32_t mipLevel = range.baseMipLevel;
                     mipLevel < range.baseMipLevel + range.levelCount; ++mipLevel) {
                    uint32_t subresourceIndex = GetSubresourceIndex(mipLevel, arrayLayer, aspect);
                    ASSERT(subresourceIndex < mIsSubresourceContentInitializedAtIndex.size());
                    if (!mIsSubresourceContentInitializedAtIndex[subresourceIndex]) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    void TextureBase::SetIsSubresourceContentInitialized(bool isInitialized,
                                                         const SubresourceRange& range) {
        ASSERT(!IsError());
        for (Aspect aspect : IterateEnumMask(range.aspects)) {
            for (uint32_t arrayLayer = range.baseArrayLayer;
                 arrayLayer < range.baseArrayLayer + range.layerCount; ++arrayLayer) {
                for (uint32_t mipLevel = range.baseMipLevel;
                     mipLevel < range.baseMipLevel + range.levelCount; ++mipLevel) {
                    uint32_t subresourceIndex = GetSubresourceIndex(mipLevel, arrayLayer, aspect);
                    ASSERT(subresourceIndex < mIsSubresourceContentInitializedAtIndex.size());
                    mIsSubresourceContentInitializedAtIndex[subresourceIndex] = isInitialized;
                }
            }
        }
    }

    MaybeError TextureBase::ValidateCanUseInSubmitNow() const {
        ASSERT(!IsError());
        DAWN_INVALID_IF(mState == TextureState::Destroyed, "Destroyed texture %s used in a submit.",
                        this);
        return {};
    }

    bool TextureBase::IsMultisampledTexture() const {
        ASSERT(!IsError());
        return mSampleCount > 1;
    }

    Extent3D TextureBase::GetMipLevelVirtualSize(uint32_t level) const {
        Extent3D extent = {std::max(mSize.width >> level, 1u), 1u, 1u};
        if (mDimension == wgpu::TextureDimension::e1D) {
            return extent;
        }

        extent.height = std::max(mSize.height >> level, 1u);
        if (mDimension == wgpu::TextureDimension::e2D) {
            return extent;
        }

        extent.depthOrArrayLayers = std::max(mSize.depthOrArrayLayers >> level, 1u);
        return extent;
    }

    Extent3D TextureBase::GetMipLevelPhysicalSize(uint32_t level) const {
        Extent3D extent = GetMipLevelVirtualSize(level);

        // Compressed Textures will have paddings if their width or height is not a multiple of
        // 4 at non-zero mipmap levels.
        if (mFormat.isCompressed && level != 0) {
            // If |level| is non-zero, then each dimension of |extent| is at most half of
            // the max texture dimension. Computations here which add the block width/height
            // to the extent cannot overflow.
            const TexelBlockInfo& blockInfo = mFormat.GetAspectInfo(wgpu::TextureAspect::All).block;
            extent.width = (extent.width + blockInfo.width - 1) / blockInfo.width * blockInfo.width;
            extent.height =
                (extent.height + blockInfo.height - 1) / blockInfo.height * blockInfo.height;
        }

        return extent;
    }

    Extent3D TextureBase::ClampToMipLevelVirtualSize(uint32_t level,
                                                     const Origin3D& origin,
                                                     const Extent3D& extent) const {
        const Extent3D virtualSizeAtLevel = GetMipLevelVirtualSize(level);
        ASSERT(origin.x <= virtualSizeAtLevel.width);
        ASSERT(origin.y <= virtualSizeAtLevel.height);
        uint32_t clampedCopyExtentWidth = (extent.width > virtualSizeAtLevel.width - origin.x)
                                              ? (virtualSizeAtLevel.width - origin.x)
                                              : extent.width;
        uint32_t clampedCopyExtentHeight = (extent.height > virtualSizeAtLevel.height - origin.y)
                                               ? (virtualSizeAtLevel.height - origin.y)
                                               : extent.height;
        return {clampedCopyExtentWidth, clampedCopyExtentHeight, extent.depthOrArrayLayers};
    }

    TextureViewBase* TextureBase::APICreateView(const TextureViewDescriptor* descriptor) {
        DeviceBase* device = GetDevice();

        Ref<TextureViewBase> result;
        if (device->ConsumedError(device->CreateTextureView(this, descriptor), &result,
                                  "calling %s.CreateView(%s).", this, descriptor)) {
            return TextureViewBase::MakeError(device);
        }
        return result.Detach();
    }

    void TextureBase::APIDestroy() {
        if (GetDevice()->ConsumedError(ValidateDestroy(), "calling %s.Destroy().", this)) {
            return;
        }
        ASSERT(!IsError());
        Destroy();
    }

    MaybeError TextureBase::ValidateDestroy() const {
        DAWN_TRY(GetDevice()->ValidateObject(this));
        return {};
    }

    // TextureViewBase

    TextureViewBase::TextureViewBase(TextureBase* texture, const TextureViewDescriptor* descriptor)
        : ApiObjectBase(texture->GetDevice(), descriptor->label),
          mTexture(texture),
          mFormat(GetDevice()->GetValidInternalFormat(descriptor->format)),
          mDimension(descriptor->dimension),
          mRange({ConvertViewAspect(mFormat, descriptor->aspect),
                  {descriptor->baseArrayLayer, descriptor->arrayLayerCount},
                  {descriptor->baseMipLevel, descriptor->mipLevelCount}}) {
        TrackInDevice();
    }

    TextureViewBase::TextureViewBase(TextureBase* texture)
        : ApiObjectBase(texture->GetDevice(), kLabelNotImplemented),
          mTexture(texture),
          mFormat(kUnusedFormat) {
        TrackInDevice();
    }

    TextureViewBase::TextureViewBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ApiObjectBase(device, tag), mFormat(kUnusedFormat) {
    }

    void TextureViewBase::DestroyImpl() {
    }

    // static
    TextureViewBase* TextureViewBase::MakeError(DeviceBase* device) {
        return new TextureViewBase(device, ObjectBase::kError);
    }

    ObjectType TextureViewBase::GetType() const {
        return ObjectType::TextureView;
    }

    const TextureBase* TextureViewBase::GetTexture() const {
        ASSERT(!IsError());
        return mTexture.Get();
    }

    TextureBase* TextureViewBase::GetTexture() {
        ASSERT(!IsError());
        return mTexture.Get();
    }

    Aspect TextureViewBase::GetAspects() const {
        ASSERT(!IsError());
        return mRange.aspects;
    }

    const Format& TextureViewBase::GetFormat() const {
        ASSERT(!IsError());
        return mFormat;
    }

    wgpu::TextureViewDimension TextureViewBase::GetDimension() const {
        ASSERT(!IsError());
        return mDimension;
    }

    uint32_t TextureViewBase::GetBaseMipLevel() const {
        ASSERT(!IsError());
        return mRange.baseMipLevel;
    }

    uint32_t TextureViewBase::GetLevelCount() const {
        ASSERT(!IsError());
        return mRange.levelCount;
    }

    uint32_t TextureViewBase::GetBaseArrayLayer() const {
        ASSERT(!IsError());
        return mRange.baseArrayLayer;
    }

    uint32_t TextureViewBase::GetLayerCount() const {
        ASSERT(!IsError());
        return mRange.layerCount;
    }

    const SubresourceRange& TextureViewBase::GetSubresourceRange() const {
        ASSERT(!IsError());
        return mRange;
    }

}  // namespace dawn_native
