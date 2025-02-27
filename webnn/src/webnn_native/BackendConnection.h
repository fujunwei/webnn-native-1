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

#ifndef WEBNNNATIVE_BACKENDCONNECTION_H_
#define WEBNNNATIVE_BACKENDCONNECTION_H_

#include "webnn_native/Context.h"
#include "webnn_native/WebnnNative.h"
#include "webnn_native/webnn_platform.h"

#include <memory>

namespace webnn_native {

    // An common interface for all backends. Mostly used to create adapters for a particular
    // backend.
    class BackendConnection {
      public:
        BackendConnection(InstanceBase* instance, ml::BackendType type);
        virtual ~BackendConnection() = default;

        ml::BackendType GetType() const;
        InstanceBase* GetInstance() const;

        virtual ContextBase* CreateContext(ContextOptions const* options = nullptr) = 0;

      private:
        InstanceBase* mInstance = nullptr;
        ml::BackendType mType;
    };

}  // namespace webnn_native

#endif  // WEBNNNATIVE_BACKENDCONNECTION_H_
