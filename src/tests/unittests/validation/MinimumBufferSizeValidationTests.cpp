// Copyright 2020 The Dawn Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

namespace {
    // Helper for describing bindings throughout the tests
    struct BindingDescriptor {
        uint32_t group;
        uint32_t binding;
        std::string decl;
        std::string ref_type;
        std::string ref_mem;
        uint64_t size;
        wgpu::BufferBindingType type = wgpu::BufferBindingType::Storage;
        wgpu::ShaderStage visibility = wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment;
    };

    // Runs |func| with a modified version of |originalSizes| as an argument, adding |offset| to
    // each element one at a time This is useful to verify some behavior happens if any element is
    // offset from original
    template <typename F>
    void WithEachSizeOffsetBy(int64_t offset, const std::vector<uint64_t>& originalSizes, F func) {
        std::vector<uint64_t> modifiedSizes = originalSizes;
        for (size_t i = 0; i < originalSizes.size(); ++i) {
            if (offset < 0) {
                ASSERT(originalSizes[i] >= static_cast<uint64_t>(-offset));
            }
            // Run the function with an element offset, and restore element afterwards
            modifiedSizes[i] += offset;
            func(modifiedSizes);
            modifiedSizes[i] -= offset;
        }
    }

    // Runs |func| with |correctSizes|, and an expectation of success and failure
    template <typename F>
    void CheckSizeBounds(const std::vector<uint64_t>& correctSizes, F func) {
        // To validate size:
        // Check invalid with bind group with one less
        // Check valid with bind group with correct size

        // Make sure (every size - 1) produces an error
        WithEachSizeOffsetBy(-1, correctSizes,
                             [&](const std::vector<uint64_t>& sizes) { func(sizes, false); });

        // Make sure correct sizes work
        func(correctSizes, true);

        // Make sure (every size + 1) works
        WithEachSizeOffsetBy(1, correctSizes,
                             [&](const std::vector<uint64_t>& sizes) { func(sizes, true); });
    }

    // Creates a bind group with given bindings for shader text
    std::string GenerateBindingString(const std::vector<BindingDescriptor>& bindings) {
        std::ostringstream ostream;
        size_t index = 0;
        for (const BindingDescriptor& b : bindings) {
            ostream << "struct S" << index << " { " << b.decl << "};\n";
            ostream << "[[group(" << b.group << "), binding(" << b.binding << ")]] ";
            switch (b.type) {
                case wgpu::BufferBindingType::Uniform:
                    ostream << "var<uniform> b" << index << " : S" << index << ";\n";
                    break;
                case wgpu::BufferBindingType::Storage:
                    ostream << "var<storage, read_write> b" << index << " : S" << index << ";\n";
                    break;
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    ostream << "var<storage, read> b" << index << " : S" << index << ";\n";
                    break;
                default:
                    UNREACHABLE();
            }
            index++;
        }
        return ostream.str();
    }

    std::string GenerateReferenceString(const std::vector<BindingDescriptor>& bindings,
                                        wgpu::ShaderStage stage) {
        std::ostringstream ostream;
        size_t index = 0;
        for (const BindingDescriptor& b : bindings) {
            if (b.visibility & stage) {
                if (!b.ref_type.empty() && !b.ref_mem.empty()) {
                    ostream << "var r" << index << " : " << b.ref_type << " = b" << index << "."
                            << b.ref_mem << ";\n";
                }
            }
            index++;
        }
        return ostream.str();
    }

    // Used for adding custom types available throughout the tests
    static const std::string kStructs = "struct ThreeFloats {f1 : f32; f2 : f32; f3 : f32;};\n";

    // Creates a compute shader with given bindings
    std::string CreateComputeShaderWithBindings(const std::vector<BindingDescriptor>& bindings) {
        return kStructs + GenerateBindingString(bindings) +
               "[[stage(compute), workgroup_size(1,1,1)]] fn main() {\n" +
               GenerateReferenceString(bindings, wgpu::ShaderStage::Compute) + "}";
    }

    // Creates a vertex shader with given bindings
    std::string CreateVertexShaderWithBindings(const std::vector<BindingDescriptor>& bindings) {
        return kStructs + GenerateBindingString(bindings) +
               "[[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {\n" +
               GenerateReferenceString(bindings, wgpu::ShaderStage::Vertex) +
               "\n   return vec4<f32>(); " + "}";
    }

    // Creates a fragment shader with given bindings
    std::string CreateFragmentShaderWithBindings(const std::vector<BindingDescriptor>& bindings) {
        return kStructs + GenerateBindingString(bindings) + "[[stage(fragment)]] fn main() {\n" +
               GenerateReferenceString(bindings, wgpu::ShaderStage::Fragment) + "}";
    }

    // Concatenates vectors containing BindingDescriptor
    std::vector<BindingDescriptor> CombineBindings(
        std::initializer_list<std::vector<BindingDescriptor>> bindings) {
        std::vector<BindingDescriptor> result;
        for (const std::vector<BindingDescriptor>& b : bindings) {
            result.insert(result.end(), b.begin(), b.end());
        }
        return result;
    }
}  // namespace

class MinBufferSizeTestsBase : public ValidationTest {
  public:
    void SetUp() override {
        ValidationTest::SetUp();
    }

    wgpu::Buffer CreateBuffer(uint64_t bufferSize, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = usage;

        return device.CreateBuffer(&bufferDescriptor);
    }

    // Creates compute pipeline given a layout and shader
    wgpu::ComputePipeline CreateComputePipeline(const std::vector<wgpu::BindGroupLayout>& layouts,
                                                const std::string& shader) {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, shader.c_str());

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = nullptr;
        if (!layouts.empty()) {
            wgpu::PipelineLayoutDescriptor descriptor;
            descriptor.bindGroupLayoutCount = layouts.size();
            descriptor.bindGroupLayouts = layouts.data();
            csDesc.layout = device.CreatePipelineLayout(&descriptor);
        }
        csDesc.compute.module = csModule;
        csDesc.compute.entryPoint = "main";

        return device.CreateComputePipeline(&csDesc);
    }

    // Creates compute pipeline with default layout
    wgpu::ComputePipeline CreateComputePipelineWithDefaultLayout(const std::string& shader) {
        return CreateComputePipeline({}, shader);
    }

    // Creates render pipeline give na layout and shaders
    wgpu::RenderPipeline CreateRenderPipeline(const std::vector<wgpu::BindGroupLayout>& layouts,
                                              const std::string& vertexShader,
                                              const std::string& fragShader) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragShader.c_str());

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        pipelineDescriptor.layout = nullptr;
        if (!layouts.empty()) {
            wgpu::PipelineLayoutDescriptor descriptor;
            descriptor.bindGroupLayoutCount = layouts.size();
            descriptor.bindGroupLayouts = layouts.data();
            pipelineDescriptor.layout = device.CreatePipelineLayout(&descriptor);
        }

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    // Creates render pipeline with default layout
    wgpu::RenderPipeline CreateRenderPipelineWithDefaultLayout(const std::string& vertexShader,
                                                               const std::string& fragShader) {
        return CreateRenderPipeline({}, vertexShader, fragShader);
    }

    // Creates bind group layout with given minimum sizes for each binding
    wgpu::BindGroupLayout CreateBindGroupLayout(const std::vector<BindingDescriptor>& bindings,
                                                const std::vector<uint64_t>& minimumSizes) {
        ASSERT(bindings.size() == minimumSizes.size());
        std::vector<wgpu::BindGroupLayoutEntry> entries;

        for (size_t i = 0; i < bindings.size(); ++i) {
            const BindingDescriptor& b = bindings[i];
            wgpu::BindGroupLayoutEntry e = {};
            e.binding = b.binding;
            e.visibility = b.visibility;
            e.buffer.type = b.type;
            e.buffer.minBindingSize = minimumSizes[i];
            entries.push_back(e);
        }

        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = static_cast<uint32_t>(entries.size());
        descriptor.entries = entries.data();
        return device.CreateBindGroupLayout(&descriptor);
    }

    // Extract the first bind group from a compute shader
    wgpu::BindGroupLayout GetBGLFromComputeShader(const std::string& shader, uint32_t index) {
        wgpu::ComputePipeline pipeline = CreateComputePipelineWithDefaultLayout(shader);
        return pipeline.GetBindGroupLayout(index);
    }

    // Extract the first bind group from a render pass
    wgpu::BindGroupLayout GetBGLFromRenderShaders(const std::string& vertexShader,
                                                  const std::string& fragShader,
                                                  uint32_t index) {
        wgpu::RenderPipeline pipeline =
            CreateRenderPipelineWithDefaultLayout(vertexShader, fragShader);
        return pipeline.GetBindGroupLayout(index);
    }

    // Create a bind group with given binding sizes for each entry (backed by the same buffer)
    wgpu::BindGroup CreateBindGroup(wgpu::BindGroupLayout layout,
                                    const std::vector<BindingDescriptor>& bindings,
                                    const std::vector<uint64_t>& bindingSizes) {
        ASSERT(bindings.size() == bindingSizes.size());
        wgpu::Buffer buffer =
            CreateBuffer(1024, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage);

        std::vector<wgpu::BindGroupEntry> entries;
        entries.reserve(bindingSizes.size());

        for (uint32_t i = 0; i < bindingSizes.size(); ++i) {
            wgpu::BindGroupEntry entry = {};
            entry.binding = bindings[i].binding;
            entry.buffer = buffer;
            ASSERT(bindingSizes[i] < 1024);
            entry.size = bindingSizes[i];
            entries.push_back(entry);
        }

        wgpu::BindGroupDescriptor descriptor;
        descriptor.layout = layout;
        descriptor.entryCount = entries.size();
        descriptor.entries = entries.data();

        return device.CreateBindGroup(&descriptor);
    }

    // Runs a single dispatch with given pipeline and bind group (to test lazy validation during
    // dispatch)
    void TestDispatch(const wgpu::ComputePipeline& computePipeline,
                      const std::vector<wgpu::BindGroup>& bindGroups,
                      bool expectation) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);
        for (size_t i = 0; i < bindGroups.size(); ++i) {
            computePassEncoder.SetBindGroup(i, bindGroups[i]);
        }
        computePassEncoder.Dispatch(1);
        computePassEncoder.EndPass();
        if (!expectation) {
            ASSERT_DEVICE_ERROR(commandEncoder.Finish());
        } else {
            commandEncoder.Finish();
        }
    }

    // Runs a single draw with given pipeline and bind group (to test lazy validation during draw)
    void TestDraw(const wgpu::RenderPipeline& renderPipeline,
                  const std::vector<wgpu::BindGroup>& bindGroups,
                  bool expectation) {
        DummyRenderPass renderPass(device);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);
        for (size_t i = 0; i < bindGroups.size(); ++i) {
            renderPassEncoder.SetBindGroup(i, bindGroups[i]);
        }
        renderPassEncoder.Draw(3);
        renderPassEncoder.EndPass();
        if (!expectation) {
            ASSERT_DEVICE_ERROR(commandEncoder.Finish());
        } else {
            commandEncoder.Finish();
        }
    }
};

// The check between BGL and pipeline at pipeline creation time
class MinBufferSizePipelineCreationTests : public MinBufferSizeTestsBase {};

// Pipeline can be created if minimum buffer size in layout is specified as 0
TEST_F(MinBufferSizePipelineCreationTests, ZeroMinBufferSize) {
    std::vector<BindingDescriptor> bindings = {{0, 0, "a : f32; b : f32;", "f32", "a", 8},
                                               {0, 1, "c : f32;", "f32", "c", 4}};

    std::string computeShader = CreateComputeShaderWithBindings(bindings);
    std::string vertexShader = CreateVertexShaderWithBindings({});
    std::string fragShader = CreateFragmentShaderWithBindings(bindings);

    wgpu::BindGroupLayout layout = CreateBindGroupLayout(bindings, {0, 0});
    CreateRenderPipeline({layout}, vertexShader, fragShader);
    CreateComputePipeline({layout}, computeShader);
}

// Fail if layout given has non-zero minimum sizes smaller than shader requirements
TEST_F(MinBufferSizePipelineCreationTests, LayoutSizesTooSmall) {
    std::vector<BindingDescriptor> bindings = {{0, 0, "a : f32; b : f32;", "f32", "a", 8},
                                               {0, 1, "c : f32;", "f32", "c", 4}};

    std::string computeShader = CreateComputeShaderWithBindings(bindings);
    std::string vertexShader = CreateVertexShaderWithBindings({});
    std::string fragShader = CreateFragmentShaderWithBindings(bindings);

    CheckSizeBounds({8, 4}, [&](const std::vector<uint64_t>& sizes, bool expectation) {
        wgpu::BindGroupLayout layout = CreateBindGroupLayout(bindings, sizes);
        if (expectation) {
            CreateRenderPipeline({layout}, vertexShader, fragShader);
            CreateComputePipeline({layout}, computeShader);
        } else {
            ASSERT_DEVICE_ERROR(CreateRenderPipeline({layout}, vertexShader, fragShader));
            ASSERT_DEVICE_ERROR(CreateComputePipeline({layout}, computeShader));
        }
    });
}

// Fail if layout given has non-zero minimum sizes smaller than shader requirements
TEST_F(MinBufferSizePipelineCreationTests, LayoutSizesTooSmallMultipleGroups) {
    std::vector<BindingDescriptor> bg0Bindings = {{0, 0, "a : f32; b : f32;", "f32", "a", 8},
                                                  {0, 1, "c : f32;", "f32", "c", 4}};
    std::vector<BindingDescriptor> bg1Bindings = {
        {1, 0, "d : f32; e : f32; f : f32;", "f32", "e", 12},
        {1, 1, "g : mat2x2<f32>;", "mat2x2<f32>", "g", 16}};
    std::vector<BindingDescriptor> bindings = CombineBindings({bg0Bindings, bg1Bindings});

    std::string computeShader = CreateComputeShaderWithBindings(bindings);
    std::string vertexShader = CreateVertexShaderWithBindings({});
    std::string fragShader = CreateFragmentShaderWithBindings(bindings);

    CheckSizeBounds({8, 4, 12, 16}, [&](const std::vector<uint64_t>& sizes, bool expectation) {
        wgpu::BindGroupLayout layout0 = CreateBindGroupLayout(bg0Bindings, {sizes[0], sizes[1]});
        wgpu::BindGroupLayout layout1 = CreateBindGroupLayout(bg1Bindings, {sizes[2], sizes[3]});
        if (expectation) {
            CreateRenderPipeline({layout0, layout1}, vertexShader, fragShader);
            CreateComputePipeline({layout0, layout1}, computeShader);
        } else {
            ASSERT_DEVICE_ERROR(CreateRenderPipeline({layout0, layout1}, vertexShader, fragShader));
            ASSERT_DEVICE_ERROR(CreateComputePipeline({layout0, layout1}, computeShader));
        }
    });
}

// The check between the BGL and the bindings at bindgroup creation time
class MinBufferSizeBindGroupCreationTests : public MinBufferSizeTestsBase {};

// Fail if a binding is smaller than minimum buffer size
TEST_F(MinBufferSizeBindGroupCreationTests, BindingTooSmall) {
    std::vector<BindingDescriptor> bindings = {{0, 0, "a : f32; b : f32;", "f32", "a", 8},
                                               {0, 1, "c : f32;", "f32", "c", 4}};
    wgpu::BindGroupLayout layout = CreateBindGroupLayout(bindings, {8, 4});

    CheckSizeBounds({8, 4}, [&](const std::vector<uint64_t>& sizes, bool expectation) {
        if (expectation) {
            CreateBindGroup(layout, bindings, sizes);
        } else {
            ASSERT_DEVICE_ERROR(CreateBindGroup(layout, bindings, sizes));
        }
    });
}

// Check two layouts with different minimum size are unequal
TEST_F(MinBufferSizeBindGroupCreationTests, LayoutEquality) {
    // Returning the same pointer is an implementation detail of Dawn Native.
    // It is not the same semantic with the Wire.
    DAWN_SKIP_TEST_IF(UsesWire());

    auto MakeLayout = [&](uint64_t size) {
        return utils::MakeBindGroupLayout(
            device,
            {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform, false, size}});
    };

    EXPECT_EQ(MakeLayout(0).Get(), MakeLayout(0).Get());
    EXPECT_NE(MakeLayout(0).Get(), MakeLayout(4).Get());
}

// The check between the bindgroup binding sizes and the required pipeline sizes at draw time
class MinBufferSizeDrawTimeValidationTests : public MinBufferSizeTestsBase {};

// Fail if binding sizes are too small at draw time
TEST_F(MinBufferSizeDrawTimeValidationTests, ZeroMinSizeAndTooSmallBinding) {
    std::vector<BindingDescriptor> bindings = {{0, 0, "a : f32; b : f32;", "f32", "a", 8},
                                               {0, 1, "c : f32;", "f32", "c", 4}};

    std::string computeShader = CreateComputeShaderWithBindings(bindings);
    std::string vertexShader = CreateVertexShaderWithBindings({});
    std::string fragShader = CreateFragmentShaderWithBindings(bindings);

    wgpu::BindGroupLayout layout = CreateBindGroupLayout(bindings, {0, 0});

    wgpu::ComputePipeline computePipeline = CreateComputePipeline({layout}, computeShader);
    wgpu::RenderPipeline renderPipeline = CreateRenderPipeline({layout}, vertexShader, fragShader);

    CheckSizeBounds({8, 4}, [&](const std::vector<uint64_t>& sizes, bool expectation) {
        wgpu::BindGroup bindGroup = CreateBindGroup(layout, bindings, sizes);
        TestDispatch(computePipeline, {bindGroup}, expectation);
        TestDraw(renderPipeline, {bindGroup}, expectation);
    });
}

// Draw time validation works for non-contiguous bindings
TEST_F(MinBufferSizeDrawTimeValidationTests, UnorderedBindings) {
    std::vector<BindingDescriptor> bindings = {
        {0, 2, "a : f32; b : f32;", "f32", "a", 8},
        {0, 0, "c : f32;", "f32", "c", 4},
        {0, 4, "d : f32; e : f32; f : f32;", "f32", "e", 12}};

    std::string computeShader = CreateComputeShaderWithBindings(bindings);
    std::string vertexShader = CreateVertexShaderWithBindings({});
    std::string fragShader = CreateFragmentShaderWithBindings(bindings);

    wgpu::BindGroupLayout layout = CreateBindGroupLayout(bindings, {0, 0, 0});

    wgpu::ComputePipeline computePipeline = CreateComputePipeline({layout}, computeShader);
    wgpu::RenderPipeline renderPipeline = CreateRenderPipeline({layout}, vertexShader, fragShader);

    CheckSizeBounds({8, 4, 12}, [&](const std::vector<uint64_t>& sizes, bool expectation) {
        wgpu::BindGroup bindGroup = CreateBindGroup(layout, bindings, sizes);
        TestDispatch(computePipeline, {bindGroup}, expectation);
        TestDraw(renderPipeline, {bindGroup}, expectation);
    });
}

// Draw time validation works for multiple bind groups
TEST_F(MinBufferSizeDrawTimeValidationTests, MultipleGroups) {
    std::vector<BindingDescriptor> bg0Bindings = {{0, 0, "a : f32; b : f32;", "f32", "a", 8},
                                                  {0, 1, "c : f32;", "f32", "c", 4}};
    std::vector<BindingDescriptor> bg1Bindings = {
        {1, 0, "d : f32; e : f32; f : f32;", "f32", "e", 12},
        {1, 1, "g : mat2x2<f32>;", "mat2x2<f32>", "g", 16}};
    std::vector<BindingDescriptor> bindings = CombineBindings({bg0Bindings, bg1Bindings});

    std::string computeShader = CreateComputeShaderWithBindings(bindings);
    std::string vertexShader = CreateVertexShaderWithBindings({});
    std::string fragShader = CreateFragmentShaderWithBindings(bindings);

    wgpu::BindGroupLayout layout0 = CreateBindGroupLayout(bg0Bindings, {0, 0});
    wgpu::BindGroupLayout layout1 = CreateBindGroupLayout(bg1Bindings, {0, 0});

    wgpu::ComputePipeline computePipeline =
        CreateComputePipeline({layout0, layout1}, computeShader);
    wgpu::RenderPipeline renderPipeline =
        CreateRenderPipeline({layout0, layout1}, vertexShader, fragShader);

    CheckSizeBounds({8, 4, 12, 16}, [&](const std::vector<uint64_t>& sizes, bool expectation) {
        wgpu::BindGroup bindGroup0 = CreateBindGroup(layout0, bg0Bindings, {sizes[0], sizes[1]});
        wgpu::BindGroup bindGroup1 = CreateBindGroup(layout0, bg0Bindings, {sizes[2], sizes[3]});
        TestDispatch(computePipeline, {bindGroup0, bindGroup1}, expectation);
        TestDraw(renderPipeline, {bindGroup0, bindGroup1}, expectation);
    });
}

// The correctness of minimum buffer size for the defaulted layout for a pipeline
class MinBufferSizeDefaultLayoutTests : public MinBufferSizeTestsBase {
  public:
    // Checks BGL |layout| has minimum buffer sizes equal to sizes in |bindings|
    void CheckLayoutBindingSizeValidation(const wgpu::BindGroupLayout& layout,
                                          const std::vector<BindingDescriptor>& bindings) {
        std::vector<uint64_t> correctSizes;
        correctSizes.reserve(bindings.size());
        for (const BindingDescriptor& b : bindings) {
            correctSizes.push_back(b.size);
        }

        CheckSizeBounds(correctSizes, [&](const std::vector<uint64_t>& sizes, bool expectation) {
            if (expectation) {
                CreateBindGroup(layout, bindings, sizes);
            } else {
                ASSERT_DEVICE_ERROR(CreateBindGroup(layout, bindings, sizes));
            }
        });
    }

    // Constructs shaders with given layout type and bindings, checking defaulted sizes match sizes
    // in |bindings|
    void CheckShaderBindingSizeReflection(
        std::initializer_list<std::vector<BindingDescriptor>> bindings) {
        std::vector<BindingDescriptor> combinedBindings = CombineBindings(bindings);
        std::string computeShader = CreateComputeShaderWithBindings(combinedBindings);
        std::string vertexShader = CreateVertexShaderWithBindings({});
        std::string fragShader = CreateFragmentShaderWithBindings(combinedBindings);

        size_t i = 0;
        for (const std::vector<BindingDescriptor>& b : bindings) {
            wgpu::BindGroupLayout computeLayout = GetBGLFromComputeShader(computeShader, i);
            wgpu::BindGroupLayout renderLayout =
                GetBGLFromRenderShaders(vertexShader, fragShader, i);

            CheckLayoutBindingSizeValidation(computeLayout, b);
            CheckLayoutBindingSizeValidation(renderLayout, b);
            ++i;
        }
    }
};

// Test the minimum size computations for various WGSL types.
TEST_F(MinBufferSizeDefaultLayoutTests, DefaultLayoutVariousWGSLTypes) {
    CheckShaderBindingSizeReflection({{{0, 0, "a : f32;", "f32", "a", 4},
                                       {0, 1, "b : array<f32>;", "f32", "b[0]", 4},
                                       {0, 2, "c : mat2x2<f32>;", "mat2x2<f32>", "c", 16}}});
    CheckShaderBindingSizeReflection({{{0, 3, "d : u32; e : array<f32>;", "u32", "d", 8},
                                       {0, 4, "f : ThreeFloats;", "f32", "f.f1", 12},
                                       {0, 5, "g : array<ThreeFloats>;", "f32", "g[0].f1", 12}}});
}

// Test the minimum size computations for various buffer binding types.
TEST_F(MinBufferSizeDefaultLayoutTests, DefaultLayoutVariousBindingTypes) {
    CheckShaderBindingSizeReflection(
        {{{0, 0, "a : f32;", "f32", "a", 4, wgpu::BufferBindingType::Uniform},
          {0, 1, "a : f32; b : f32;", "f32", "a", 8, wgpu::BufferBindingType::Storage},
          {0, 2, "a : f32; b : f32; c: f32;", "f32", "a", 12,
           wgpu::BufferBindingType::ReadOnlyStorage}}});
}

// Test the minimum size computations works with multiple bind groups.
TEST_F(MinBufferSizeDefaultLayoutTests, MultipleBindGroups) {
    CheckShaderBindingSizeReflection(
        {{{0, 0, "a : f32;", "f32", "a", 4, wgpu::BufferBindingType::Uniform}},
         {{1, 0, "a : f32; b : f32;", "f32", "a", 8, wgpu::BufferBindingType::Storage}},
         {{2, 0, "a : f32; b : f32; c : f32;", "f32", "a", 12,
           wgpu::BufferBindingType::ReadOnlyStorage}}});
}

// Test the minimum size computations with manual size/align/stride decorations.
TEST_F(MinBufferSizeDefaultLayoutTests, NonDefaultLayout) {
    CheckShaderBindingSizeReflection(
        {{{0, 0, "[[size(256)]] a : u32; b : u32;", "u32", "a", 260},
          {0, 1, "c : u32; [[align(16)]] d : u32;", "u32", "c", 20},
          {0, 2, "d : [[stride(40)]] array<u32, 3>;", "u32", "d[0]", 120},
          {0, 3, "e : [[stride(40)]] array<u32>;", "u32", "e[0]", 40}}});
}

// Minimum size should be the max requirement of both vertex and fragment stages.
TEST_F(MinBufferSizeDefaultLayoutTests, RenderPassConsidersBothStages) {
    std::string vertexShader = CreateVertexShaderWithBindings(
        {{0, 0, "a : f32; b : f32;", "f32", "a", 8, wgpu::BufferBindingType::Uniform,
          wgpu::ShaderStage::Vertex},
         {0, 1, "c : vec4<f32>;", "vec4<f32>", "c", 16, wgpu::BufferBindingType::Uniform,
          wgpu::ShaderStage::Vertex}});
    std::string fragShader = CreateFragmentShaderWithBindings(
        {{0, 0, "a : f32;", "f32", "a", 4, wgpu::BufferBindingType::Uniform,
          wgpu::ShaderStage::Fragment},
         {0, 1, "b : f32; c : f32;", "f32", "b", 8, wgpu::BufferBindingType::Uniform,
          wgpu::ShaderStage::Fragment}});

    wgpu::BindGroupLayout renderLayout = GetBGLFromRenderShaders(vertexShader, fragShader, 0);

    CheckLayoutBindingSizeValidation(renderLayout, {{0, 0, "", "", "", 8}, {0, 1, "", "", "", 16}});
}
