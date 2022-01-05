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

#ifndef WEBNN_NATIVE_ERROR_SCOPE_DAWN_H_
#define WEBNN_NATIVE_ERROR_SCOPE_DAWN_H_

#include "webnn_native/webnn_platform.h"

#include <string>

namespace wgpu {

    // The InternalErrorType will be used in dawn_native namespace.
    enum class ErrorType : uint32_t {
        NoError = 0x00000000,
        Validation = 0x00000001,
        OutOfMemory = 0x00000002,
        Unknown = 0x00000003,
        DeviceLost = 0x00000004,
    };

    enum class ErrorFilter : uint32_t {
        Validation = 0x00000000,
        OutOfMemory = 0x00000001,
    };

}  // namespace dawn_native

#endif  // WEBNN_NATIVE_ERROR_SCOPE_DAWN_H_
