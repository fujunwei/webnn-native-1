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

#include "webnn_native/null/ContextNull.h"
#include "common/RefCounted.h"

namespace webnn_native { namespace null {

    // MLContext
    MLContextBase* Create(WebnnMLContextOptions const* options) {
        return new MLContext(reinterpret_cast<MLContextOptions const*>(options));
    }

    MLContext::MLContext(MLContextOptions const* options) {
    }

    MLGraphBase* MLContext::CreateGraphImpl() {
        return new MLGraph(this);
    }

    // MLGraphBuilder
    MLGraphBuilder::MLGraphBuilder(MLContextBase* context) : MLGraphBuilderBase(context) {
    }

    // MLGraph
    MLGraph::MLGraph(MLContext* context) : MLGraphBase(context) {
    }

    void MLGraph::ComputeImpl(MLNamedInputsBase* inputs,
                              WebnnComputeCallback callback,
                              void* userdata,
                              MLNamedOutputsBase* outputs) {
    }

    MaybeError MLGraph::AddConstant(const op::Constant* constant) {
        return {};
    }

    MaybeError MLGraph::AddInput(const op::Input* input) {
        return {};
    }

    MaybeError MLGraph::AddOutput(const std::string& name, const MLOperandBase* output) {
        return {};
    }

    MaybeError MLGraph::AddBinary(const op::Binary* binary) {
        return {};
    }

    MaybeError MLGraph::AddConv2d(const op::Conv2d* conv2d) {
        return {};
    }

    MaybeError MLGraph::AddPool2d(const op::Pool2d* pool2d) {
        return {};
    }

    MaybeError MLGraph::AddReshape(const op::Reshape* relu) {
        return {};
    }

    MaybeError MLGraph::AddTranspose(const op::Transpose* transpose) {
        return {};
    }

    MaybeError MLGraph::AddUnary(const op::Unary* unary) {
        return {};
    }

    MaybeError MLGraph::Finish() {
        return {};
    }

}}  // namespace webnn_native::null
