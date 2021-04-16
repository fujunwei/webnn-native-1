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

#include "webnn_native/WebnnNative.h"

#include <memory>

#include "common/Assert.h"
#include "webnn_native/MLGraphBuilder.h"

// Contains the entry-points into webnn_native
namespace webnn_native {
    const WebnnProcTable& GetProcsAutogen();

    const WebnnProcTable& GetProcs() {
        return GetProcsAutogen();
    }

    namespace null {
        MLContextBase* Create(WebnnMLContextOptions const* options);
    }
    namespace ie {
        MLContextBase* Create(WebnnMLContextOptions const* options);
    }
    namespace dml {
        MLContextBase* Create(WebnnMLContextOptions const* options);
    }

    // Should put the default null backend at the end.
    WebnnMLContext CreateContext(WebnnMLContextOptions const* options) {
#if defined(WEBNN_ENABLE_BACKEND_OPENVINO)
        return reinterpret_cast<WebnnMLContext>(ie::Create(options));
#elif defined(WEBNN_ENABLE_BACKEND_DML)
        return reinterpret_cast<WebnnMLContext>(dml::Create(options));
#elif defined(WEBNN_ENABLE_BACKEND_NULL)
        return reinterpret_cast<WebnnMLContext>(null::Create(options));
#else
        return nullptr;
#endif
    }

}  // namespace webnn_native
