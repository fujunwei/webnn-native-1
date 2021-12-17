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

#include "dawn_native/webnn/ops/Binary.h"

#include "common/Log.h"
#include "dawn_native/webnn/Error.h"

namespace webnn_native { namespace op {

    MaybeError Binary::Validate() {
        MaybeError maybeError = OperatorBase::Validate();
        if (maybeError.IsError()) {
            return maybeError;
        }

        Ref<OperandBase> a = mInputs[0];
        Ref<OperandBase> b = mInputs[1];
        if (a->Type() != b->Type()) {
            return DAWN_VALIDATION_ERROR("Argument types are inconsistent.");
        }
        return {};
    }

}}  // namespace webnn_native::op
