// Copyright 2018 The Dawn Authors
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

#ifndef DAWNNATIVE_ERROR_H_
#define DAWNNATIVE_ERROR_H_

#include "absl/strings/str_format.h"
#include "common/Result.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/webgpu_absl_format.h"
#include "dawn_native/ErrorCommon.h"

#include <string>

namespace dawn_native {

    enum class InternalErrorType : uint32_t {
        Validation,
        DeviceLost,
        Internal,
        OutOfMemory
    };

    // Assert that errors are device loss so that we can continue with destruction
    void IgnoreErrors(MaybeError maybeError);

    wgpu::ErrorType ToWGPUErrorType(InternalErrorType type);
    InternalErrorType FromWGPUErrorType(wgpu::ErrorType type);

}  // namespace dawn_native

#endif  // DAWNNATIVE_ERROR_H_
