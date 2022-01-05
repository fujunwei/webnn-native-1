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

#ifndef WEBNN_NATIVE_ERROR_H_
#define WEBNN_NATIVE_ERROR_H_

#include "common/Result.h"
#include "webnn_native/ErrorDawn.h"
#include "webnn_native/ErrorScopeDawn.h"
#include "dawn_native/ErrorCommon.h"

#include <string>

namespace ml {
    enum class ErrorType : uint32_t;
}

namespace webnn_native {

    using dawn_native::MaybeError;
    using dawn_native::ErrorData;
    using dawn_native::InternalErrorType;

    // Assert that errors are context loss so that we can continue with destruction
    void IgnoreErrors(MaybeError maybeError);

    ml::ErrorType ToMLErrorType(InternalErrorType type);
    InternalErrorType FromMLErrorType(ml::ErrorType type);
    wgpu::ErrorType ToDawnErrorType(InternalErrorType type);
    wgpu::ErrorFilter ToDawnErrorFilter(ml::ErrorFilter filter);

} // namespace webnn_native

#endif  // WEBNN_NATIVE_ERROR_H_
