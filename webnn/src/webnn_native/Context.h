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

#ifndef DAWN_NATIVE_WEBNN_CONTEXT_H_
#define DAWN_NATIVE_WEBNN_CONTEXT_H_

#include "common/RefCounted.h"
#include "webnn_native/Error.h"
#include "webnn_native/ErrorScope.h"
#include "webnn_native/Graph.h"
#include "webnn_native/webnn_platform.h"

namespace webnn_native {

    class ContextBase : public RefCounted {
      public:
        explicit ContextBase(ContextOptions const* options = nullptr);
        virtual ~ContextBase() = default;

        bool ConsumedError(MaybeError maybeError) {
            if (DAWN_UNLIKELY(maybeError.IsError())) {
                ConsumeError(maybeError.AcquireError());
                return true;
            }
            return false;
        }

        GraphBase* CreateGraph();

        // Dawn API
        void APIPushErrorScope(ml::ErrorFilter filter);
        bool APIPopErrorScope(ml::ErrorCallback callback, void* userdata);
        void APISetUncapturedErrorCallback(ml::ErrorCallback callback, void* userdata);
        ContextOptions GetContextOptions() {
            return mContextOptions;
        }

      private:
        // Create concrete model.
        virtual GraphBase* CreateGraphImpl() = 0;

        void ConsumeError(std::unique_ptr<ErrorData> error) {
            ASSERT(error != nullptr);
            HandleError(error->GetType(), error->GetFormattedMessage().c_str());
        }
        void HandleError(InternalErrorType type, const char* message);

        ml::ErrorCallback mUncapturedErrorCallback = nullptr;
        void* mUncapturedErrorUserdata = nullptr;

        std::unique_ptr<ErrorScopeStack> mErrorScopeStack;

        ContextOptions mContextOptions;
    };

}  // namespace webnn_native

#endif  // DAWN_NATIVE_WEBNN_CONTEXT_H_
