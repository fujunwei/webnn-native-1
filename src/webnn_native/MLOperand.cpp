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

#include "webnn_native/MLOperand.h"

#include "common/Assert.h"
#include "common/Log.h"
#include "webnn_native/MLGraphBuilder.h"

namespace webnn_native {

    MLOperandBase::MLOperandBase(MLGraphBuilderBase* graphBuilder, std::vector<Ref<MLOperandBase>> inputs)
        : ObjectBase(graphBuilder->GetContext()), mInputs(std::move(inputs)) {
    }

    MLOperandBase::MLOperandBase(MLGraphBuilderBase* graphBuilder, ObjectBase::ErrorTag tag)
        : ObjectBase(graphBuilder->GetContext(), tag) {
    }

    // static
    MLOperandBase* MLOperandBase::MakeError(MLGraphBuilderBase* MLGraphBuilder) {
        return new MLOperandBase(MLGraphBuilder, ObjectBase::kError);
    }

    MaybeError MLOperandBase::AddToGraph(MLGraphBase* model) const {
        DAWN_UNREACHABLE();
    }

    const std::vector<Ref<MLOperandBase>>& MLOperandBase::Inputs() const {
        return mInputs;
    }

}  // namespace webnn_native
