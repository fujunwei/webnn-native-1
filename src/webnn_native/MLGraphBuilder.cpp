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

#include "webnn_native/MLGraphBuilder.h"

#include <stack>
#include <string>
#include <unordered_set>
#include <vector>

#include "common/Assert.h"
#include "common/Log.h"
#include "common/RefCounted.h"
#include "webnn_native/MLContext.h"
#include "webnn_native/MLGraph.h"
#include "webnn_native/MLOperand.h"
#include "webnn_native/ops/Binary.h"
#include "webnn_native/ops/Constant.h"
#include "webnn_native/ops/Conv2d.h"
#include "webnn_native/ops/Input.h"
#include "webnn_native/ops/Pool2d.h"
#include "webnn_native/ops/Reshape.h"
#include "webnn_native/ops/Transpose.h"
#include "webnn_native/ops/Unary.h"

#define DAWN_VALIDATE_AND_INFER_TYPES(ptr)                          \
    Ref<MLOperandBase> op = AcquireRef(ptr);                          \
    if (GetContext()->ConsumedError(op->ValidateAndInferTypes())) { \
        return MLOperandBase::MakeError(this);                        \
    }                                                               \
    return op.Detach();                                             \
    for (;;)                                                        \
    break

#define BUILD_ERROR_AND_CALLBACK(message)                             \
    do {                                                              \
        callback(WebnnBuildStatus_Error, nullptr, message, userdata); \
        return;                                                       \
    } while (0)

namespace webnn_native {

    MLGraphBuilderBase::MLGraphBuilderBase(MLContextBase* context) : ObjectBase(context) {
    }

    MLOperandBase* MLGraphBuilderBase::Constant(MLOperandDescriptor const* desc,
                                            void const* value,
                                            size_t size) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Constant(this, desc, value, size));
    }

    MLOperandBase* MLGraphBuilderBase::Input(char const* name, MLOperandDescriptor const* desc) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Input(this, std::string(name), desc));
    }

    MLOperandBase* MLGraphBuilderBase::Matmul(MLOperandBase* a, MLOperandBase* b) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Binary(this, op::BinaryOpType::kMatMul, a, b));
    }

    MLOperandBase* MLGraphBuilderBase::Add(MLOperandBase* a, MLOperandBase* b) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Binary(this, op::BinaryOpType::kAdd, a, b));
    }

    MLOperandBase* MLGraphBuilderBase::Mul(MLOperandBase* a, MLOperandBase* b) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Binary(this, op::BinaryOpType::kMul, a, b));
    }

    MLOperandBase* MLGraphBuilderBase::Conv2d(MLOperandBase* input,
                                          MLOperandBase* filter,
                                          MLConv2dOptions const* options) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Conv2d(this, input, filter, options));
    }

    MLOperandBase* MLGraphBuilderBase::AveragePool2d(MLOperandBase* input, MLPool2dOptions const* options) {
        DAWN_VALIDATE_AND_INFER_TYPES(
            new op::Pool2d(this, op::Pool2dType::kAveragePool2d, input, options));
    }

    MLOperandBase* MLGraphBuilderBase::MaxPool2d(MLOperandBase* input, MLPool2dOptions const* options) {
        DAWN_VALIDATE_AND_INFER_TYPES(
            new op::Pool2d(this, op::Pool2dType::kMaxPool2d, input, options));
    }

    MLOperandBase* MLGraphBuilderBase::Relu(MLOperandBase* input) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Unary(this, op::UnaryOpType::kRelu, input));
    }

    MLOperandBase* MLGraphBuilderBase::Reshape(MLOperandBase* input,
                                           int32_t const* new_shape,
                                           size_t new_shape_count) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Reshape(this, input, new_shape, new_shape_count));
    }

    MLOperandBase* MLGraphBuilderBase::Softmax(MLOperandBase* input) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Unary(this, op::UnaryOpType::kSoftmax, input));
    }

    MLOperandBase* MLGraphBuilderBase::Transpose(MLOperandBase* input, MLTransposeOptions const* options) {
        DAWN_VALIDATE_AND_INFER_TYPES(new op::Transpose(this, input, options));
    }

    void MLGraphBuilderBase::Build(MLNamedOperandsBase const* namedOperands,
                                 WebnnBuildCallback callback,
                                 void* userdata) {
        if (DAWN_UNLIKELY(this->IsError())) {
            BUILD_ERROR_AND_CALLBACK("This MLGraph object is an error");
        }

        std::vector<const MLOperandBase*> outputs;
        if (namedOperands->GetRecords().empty()) {
            BUILD_ERROR_AND_CALLBACK("The output named operands are empty.");
        }
        for (auto& namedOutput : namedOperands->GetRecords()) {
            outputs.push_back(namedOutput.second);
        }
        std::vector<const MLOperandBase*> sorted_operands = TopologicalSort(outputs);
        Ref<MLGraphBase> graph = AcquireRef(GetContext()->CreateGraph());
        for (auto& op : sorted_operands) {
            if (op->IsError() || GetContext()->ConsumedError(op->AddToGraph(graph.Get()))) {
                BUILD_ERROR_AND_CALLBACK("Failed to add the operand when building graph.");
            }
        }
        for (auto& namedOutput : namedOperands->GetRecords()) {
            if (GetContext()->ConsumedError(
                    graph->AddOutput(namedOutput.first, namedOutput.second))) {
                BUILD_ERROR_AND_CALLBACK("Failed to add output when building graph.");
            }
        }
        if (GetContext()->ConsumedError(graph->Finish())) {
            BUILD_ERROR_AND_CALLBACK("Failed to finish building graph.");
        }
        callback(WebnnBuildStatus_Success, reinterpret_cast<WebnnMLGraph>(graph.Detach()), nullptr,
                 userdata);
    }

    // The implementation derives from nGraph topological_sort in
    // https://github.com/openvinotoolkit/openvino/blob/master/ngraph/core/include/ngraph/graph_util.hpp
    //
    //*****************************************************************************
    // Copyright 2017-2020 Intel Corporation
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
    //*****************************************************************************
    std::vector<const MLOperandBase*> MLGraphBuilderBase::TopologicalSort(
        std::vector<const MLOperandBase*>& rootNodes) {
        std::stack<const MLOperandBase*> nodesToDo;
        std::unordered_set<const MLOperandBase*> nodesDone;
        std::vector<const MLOperandBase*> result;

        for (auto& node : rootNodes) {
            nodesToDo.push(node);
        }
        while (nodesToDo.size() > 0) {
            const MLOperandBase* node = nodesToDo.top();
            if (nodesDone.count(node) == 0) {
                bool can_add = true;
                for (auto& dep : node->Inputs()) {
                    if (nodesDone.count(dep.Get()) == 0) {
                        can_add = false;
                        nodesToDo.push(dep.Get());
                    }
                }
                if (can_add) {
                    result.push_back(node);
                    nodesToDo.pop();
                    nodesDone.insert(node);
                }
            } else {
                nodesToDo.pop();
            }
        }
        return result;
    }

}  // namespace webnn_native
