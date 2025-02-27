// Copyright 2021 The Dawn Authors
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

#ifndef DAWNWIRE_CLIENT_ADAPTER_H_
#define DAWNWIRE_CLIENT_ADAPTER_H_

#include <dawn/webgpu.h>

#include "dawn_wire/WireClient.h"
#include "dawn_wire/WireCmd_autogen.h"
#include "dawn_wire/client/LimitsAndFeatures.h"
#include "dawn_wire/client/ObjectBase.h"
#include "dawn_wire/client/RequestTracker.h"

namespace dawn_wire { namespace client {

    class Adapter final : public ObjectBase {
      public:
        using ObjectBase::ObjectBase;

        ~Adapter();
        void CancelCallbacksForDisconnect() override;

        bool GetLimits(WGPUSupportedLimits* limits) const;
        bool HasFeature(WGPUFeatureName feature) const;
        uint32_t EnumerateFeatures(WGPUFeatureName* features) const;
        void SetLimits(const WGPUSupportedLimits* limits);
        void SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount);
        void SetProperties(const WGPUAdapterProperties* properties);
        void GetProperties(WGPUAdapterProperties* properties) const;
        void RequestDevice(const WGPUDeviceDescriptor* descriptor,
                           WGPURequestDeviceCallback callback,
                           void* userdata);

        bool OnRequestDeviceCallback(uint64_t requestSerial,
                                     WGPURequestDeviceStatus status,
                                     const char* message,
                                     const WGPUSupportedLimits* limits,
                                     uint32_t featuresCount,
                                     const WGPUFeatureName* features);

      private:
        LimitsAndFeatures mLimitsAndFeatures;
        WGPUAdapterProperties mProperties;

        struct RequestDeviceData {
            WGPURequestDeviceCallback callback = nullptr;
            ObjectId deviceObjectId;
            void* userdata = nullptr;
        };
        RequestTracker<RequestDeviceData> mRequestDeviceRequests;
    };

}}  // namespace dawn_wire::client

#endif  // DAWNWIRE_CLIENT_ADAPTER_H_
