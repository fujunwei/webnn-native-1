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

#include "webnn_native/ObjectBase.h"
#include "webnn_native/Context.h"

namespace webnn_native {
    static constexpr uint64_t kErrorPayload = 0;
    static constexpr uint64_t kNotErrorPayload = 1;

    ObjectBase::ObjectBase(ContextBase* context) : RefCounted(kNotErrorPayload), mContext(context) {
    }

    ObjectBase::ObjectBase(ContextBase* context, ErrorTag)
        : RefCounted(kErrorPayload), mContext(context) {
    }

    ContextBase* ObjectBase::GetContext() const {
        return mContext.Get();
    }

    bool ObjectBase::IsError() const {
        return GetRefCountPayload() == kErrorPayload;
    }

}  // namespace webnn_native
