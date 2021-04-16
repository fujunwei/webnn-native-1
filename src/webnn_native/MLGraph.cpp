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

#include "webnn_native/MLGraph.h"

#include <string>

#include "common/Assert.h"
#include "common/RefCounted.h"

namespace webnn_native {

    MLGraphBase::MLGraphBase(MLContextBase* context) : ObjectBase(context) {
    }

    void MLGraphBase::Compute(MLNamedInputsBase* inputs,
                            WebnnComputeCallback callback,
                            void* userdata,
                            MLNamedOutputsBase* outputs) {
        ComputeImpl(inputs, callback, userdata, outputs);
    }

    MaybeError MLGraphBase::AddConstant(const op::Constant* constant) {
        UNREACHABLE();
    }

    MaybeError MLGraphBase::AddInput(const op::Input* input) {
        UNREACHABLE();
    }

    MaybeError MLGraphBase::AddOutput(const std::string& name, const MLOperandBase* output) {
        UNREACHABLE();
    }

    MaybeError MLGraphBase::AddBinary(const op::Binary* binary) {
        UNREACHABLE();
    }

    MaybeError MLGraphBase::AddConv2d(const op::Conv2d* conv2d) {
        UNREACHABLE();
    }

    MaybeError MLGraphBase::AddPool2d(const op::Pool2d* pool2d) {
        UNREACHABLE();
    }

    MaybeError MLGraphBase::AddReshape(const op::Reshape* relu) {
        UNREACHABLE();
    }

    MaybeError MLGraphBase::AddTranspose(const op::Transpose* transpose) {
        UNREACHABLE();
    }

    MaybeError MLGraphBase::AddUnary(const op::Unary* unary) {
        UNREACHABLE();
    }

    MaybeError MLGraphBase::Finish() {
        UNREACHABLE();
    }

}  // namespace webnn_native
