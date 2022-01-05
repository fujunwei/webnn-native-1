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

#include "webnn_native/Context.h"

#include <sstream>

#include "webnn_native/ValidationUtils_autogen.h"
#include "webnn_native/webnn_platform.h"

namespace webnn_native {

    ContextBase::ContextBase(ContextOptions const* options) {
        if (options != nullptr) {
            mContextOptions = *options;
        }
        mErrorScopeStack = std::make_unique<ErrorScopeStack>();
    }

    GraphBase* ContextBase::CreateGraph() {
        return CreateGraphImpl();
    }

    void ContextBase::APIPushErrorScope(ml::ErrorFilter filter) {
        if (ConsumedError(ValidateErrorFilter(filter))) {
            return;
        }

        mErrorScopeStack->Push(ToDawnErrorFilter(filter));
    }

    bool ContextBase::APIPopErrorScope(ml::ErrorCallback callback, void* userdata) {
        if (mErrorScopeStack->Empty()) {
            return false;
        }
        ErrorScope scope = mErrorScopeStack->Pop();
        if (callback != nullptr) {
            callback(static_cast<MLErrorType>(scope.GetErrorType()), scope.GetErrorMessage(),
                     userdata);
        }

        return true;
    }

    void ContextBase::APISetUncapturedErrorCallback(ml::ErrorCallback callback, void* userdata) {
        // TODO: Flush all deferred callback tasks to guarantee we are never going to use the
        // previous callback after this call.
        mUncapturedErrorCallback = callback;
        mUncapturedErrorUserdata = userdata;
    }

    void ContextBase::HandleError(InternalErrorType type, const char* message) {
        if (type == InternalErrorType::ContextLost) {
            // TODO: Set the state to disconnected as the context cannot be used.
        } else if (type == InternalErrorType::Internal) {
            // TODO: If we receive an internal error, assume the backend can't recover and proceed
            // with device destruction. We first wait for all previous commands to be completed so
            // that backend objects can be freed immediately, before handling the loss.
            type = InternalErrorType::ContextLost;
        }

        if (type == InternalErrorType::ContextLost) {
            // TODO: The device was lost, call the application callback.

            // Still forward device loss errors to the error scopes so they all reject.
            mErrorScopeStack->HandleError(ToDawnErrorType(type), message);
        } else {
            // Pass the error to the error scope stack and call the uncaptured error callback
            // if it isn't handled. ContextLost is not handled here because it should be
            // handled by the lost callback.
            bool captured = mErrorScopeStack->HandleError(ToDawnErrorType(type), message);
            if (!captured && mUncapturedErrorCallback != nullptr) {
                mUncapturedErrorCallback(static_cast<MLErrorType>(ToMLErrorType(type)), message,
                                         mUncapturedErrorUserdata);
            }
        }
    }

}  // namespace webnn_native
