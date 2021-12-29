// Copyright 2018 The Dawn Authors
// Copyright 2021 The WebNN-native Authors
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

#include "webnn/webnn_native/Error.h"

#include "webnn/webnn_native/webnn_platform.h"

namespace webnn_native {

    void IgnoreErrors(MaybeError maybeError) {
        if (maybeError.IsError()) {
            std::unique_ptr<ErrorData> errorData = maybeError.AcquireError();
            // During shutdown and destruction, device lost errors can be ignored.
            // We can also ignore other unexpected internal errors on shut down and treat it as
            // device lost so that we can continue with destruction.
            ASSERT(errorData->GetType() == InternalErrorType::ContextLost ||
                   errorData->GetType() == InternalErrorType::Internal);
        }
    }

    wgpu::ErrorType ToDawnErrorType(InternalErrorType type) {
        switch (type) {
            case InternalErrorType::Validation:
                return wgpu::ErrorType::Validation;
            case InternalErrorType::OutOfMemory:
                return wgpu::ErrorType::OutOfMemory;

            // There is no equivalent of Internal errors in the WebGPU API. Internal
            // errors cause the device at the API level to be lost, so treat it like a
            // DeviceLost error.
            case InternalErrorType::Internal:
            case InternalErrorType::ContextLost:
                return wgpu::ErrorType::DeviceLost;

            default:
                return wgpu::ErrorType::Unknown;
        }
    }

    ml::ErrorType ToMLErrorType(InternalErrorType type) {
        switch (type) {
            case InternalErrorType::Validation:
                return ml::ErrorType::Validation;
            case InternalErrorType::OutOfMemory:
                return ml::ErrorType::OutOfMemory;

            // There is no equivalent of Internal errors in the WebGPU API. Internal
            // errors cause the device at the API level to be lost, so treat it like a
            // DeviceLost error.
            case InternalErrorType::Internal:
            case InternalErrorType::ContextLost:
                return ml::ErrorType::ContextLost;

            default:
                return ml::ErrorType::Unknown;
        }
    }

    InternalErrorType FromMLErrorType(ml::ErrorType type) {
        switch (type) {
            case ml::ErrorType::Validation:
                return InternalErrorType::Validation;
            case ml::ErrorType::OutOfMemory:
                return InternalErrorType::OutOfMemory;
            case ml::ErrorType::ContextLost:
                return InternalErrorType::ContextLost;
            default:
                return InternalErrorType::Internal;
        }
    }

    wgpu::ErrorFilter ToDawnErrorFilter(ml::ErrorFilter filter) {
        switch (filter) {
            case ml::ErrorFilter::Validation:
                return wgpu::ErrorFilter::Validation;
            case ml::ErrorFilter::OutOfMemory:
                return wgpu::ErrorFilter::OutOfMemory;
            default:
                UNREACHABLE();
                break;
        }
    }

}  // namespace webnn_native
