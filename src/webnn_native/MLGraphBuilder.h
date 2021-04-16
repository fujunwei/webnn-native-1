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

#ifndef WEBNN_NATIVE_MODEL_BUILDER_H_
#define WEBNN_NATIVE_MODEL_BUILDER_H_

#include "common/RefCounted.h"
#include "webnn_native/Forward.h"
#include "webnn_native/MLNamedOperands.h"
#include "webnn_native/ObjectBase.h"
#include "webnn_native/webnn_platform.h"

#include <vector>

namespace webnn_native {

    class MLGraphBuilderBase : public ObjectBase {
      public:
        MLGraphBuilderBase(MLContextBase* context);
        virtual ~MLGraphBuilderBase() = default;

        // WebNN API
        MLOperandBase* Constant(MLOperandDescriptor const* desc, void const* value, size_t size);
        MLOperandBase* Input(char const* name, MLOperandDescriptor const* desc);
        MLOperandBase* Matmul(MLOperandBase* a, MLOperandBase* b);
        MLOperandBase* Add(MLOperandBase*, MLOperandBase*);
        MLOperandBase* Mul(MLOperandBase*, MLOperandBase*);
        MLOperandBase* Conv2d(MLOperandBase*, MLOperandBase*, MLConv2dOptions const* options);
        MLOperandBase* AveragePool2d(MLOperandBase*, MLPool2dOptions const* options);
        MLOperandBase* MaxPool2d(MLOperandBase*, MLPool2dOptions const* options);
        MLOperandBase* Relu(MLOperandBase*);
        MLOperandBase* Reshape(MLOperandBase*, int32_t const*, size_t);
        MLOperandBase* Softmax(MLOperandBase*);
        MLOperandBase* Transpose(MLOperandBase*, MLTransposeOptions const* options);
        void Build(MLNamedOperandsBase const* named_operands,
                   WebnnBuildCallback callback,
                   void* userdata);

      private:
        // Topological sort of nodes needed to compute rootNodes
        std::vector<const MLOperandBase*> TopologicalSort(std::vector<const MLOperandBase*>& rootNodes);
    };

}  // namespace webnn_native

#endif  // WEBNN_NATIVE_MODEL_BUILDER_H_
