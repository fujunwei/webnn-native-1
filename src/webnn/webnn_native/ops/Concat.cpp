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

#include "webnn/webnn_native/ops/Concat.h"

#include "webnn/webnn_native/Error.h"

namespace webnn_native { namespace op {

    MaybeError Concat::Validate() {
        MaybeError maybeError = OperatorBase::Validate();
        if (maybeError.IsError()) {
            return maybeError;
        }

        auto inputType = mInputs[0]->Type();
        auto inputRank = mInputs[0]->Rank();
        for (auto& input : mInputs) {
            if (input->Type() != inputType) {
                return DAWN_VALIDATION_ERROR("Argument types are inconsistent.");
            }
            if (input->Rank() != inputRank) {
                return DAWN_VALIDATION_ERROR(
                    "Argument inputs must have same shape except for the size of the dimension to "
                    "concatenate on.");
            }
        }

        if (mAxis >= inputRank) {
            return DAWN_VALIDATION_ERROR("The axis is out of rank range.");
        }
        return {};
    }

}}  // namespace webnn_native::op
