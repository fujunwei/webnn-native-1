// Copyright 2019 The Dawn Authors
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

#ifndef WEBNN_NATIVE_ERRORSCOPE_H_
#define WEBNN_NATIVE_ERRORSCOPE_H_

#include "webnn_native/webnn_platform.h"
#include "webnn_native/ErrorScopeDawn.h"
#include "dawn_native/ErrorScopeCommon.h"
#include "common/RefCounted.h"

#include <string>

namespace webnn_native {

    using dawn_native::ErrorScope;
    using dawn_native::ErrorScopeStack;

    wgpu::ErrorType ToDawnErrorType(ml::ErrorType type);
    wgpu::ErrorFilter ToDawnErrorFilter(ml::ErrorFilter filter);

}  // namespace webnn_native

#endif  // WEBNN_NATIVE_ERRORSCOPE_H_
