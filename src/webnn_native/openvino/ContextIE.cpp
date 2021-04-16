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

#include "webnn_native/openvino/ContextIE.h"

#include "common/Log.h"
#include "common/RefCounted.h"
#include "webnn_native/openvino/GraphIE.h"
#include "webnn_native/openvino/ienn_symbol_table/ienn_symbol_table.h"

namespace webnn_native { namespace ie {

    MLContextBase* Create(WebnnMLContextOptions const* options) {
        // Load ienn_c_api library.
        if (!GetIESymbolTable()->Load()) {
            dawn::ErrorLog() << "Failed to load the OpenVINO libraries, please make sure the "
                                "OpenVINO environment variables are set.";
            return nullptr;
        }
        return new MLContext(reinterpret_cast<MLContextOptions const*>(options));
    }

    MLContext::MLContext(MLContextOptions const* options) {
        if (options == nullptr) {
            return;
        }
        mOptions = *options;
    }

    MLGraphBase* MLContext::CreateGraphImpl() {
        return new MLGraph(this);
    }

}}  // namespace webnn_native::ie
