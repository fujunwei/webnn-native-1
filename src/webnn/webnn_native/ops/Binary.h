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

#ifndef WEBNN_NATIVE_OPS_BINARY_H_
#define WEBNN_NATIVE_OPS_BINARY_H_

#include "webnn/webnn_native/Graph.h"
#include "webnn/webnn_native/Operand.h"

namespace webnn_native { namespace op {

    enum BinaryOpType {
        kAdd = 0,
        kSub,
        kMul,
        kDiv,
        kMax,
        kMin,
        kMatMul,
        kPower,
    };

    class Binary final : public OperatorBase {
      public:
        Binary(GraphBuilderBase* builder, BinaryOpType opType, OperandBase* a, OperandBase* b)
            : OperatorBase(builder, {a, b}), mOpType(opType) {
            // For element-wise binary ops, The rank of the output tensor
            // is the maximum rank of the input tensors.
            // According to
            // [numpy-broadcasting-rule](https://webmachinelearning.github.io/webnn/#biblio-numpy-broadcasting-rule)
            // For matmul
            // 1. if a->Rank() == 2 && b->Rank() == 2, rank_ = 2;
            // 2. if a->Rank() > 2 || b->Rank() > 2, rank_ = std::max(a->Rank(), b->Rank());
            // 3. if a->Rank() == 1 && b->Rank() == 1, rank_ = 0;
            // 4. if a->Rank() == 1 && b->Rank() == 2, rank_ = 2;
            // 5. if a->Rank() == 2 && b->Rank() == 1, rank_ = 2;
            uint32_t rank = 0;
            if (mOpType == kMatMul && a->Rank() == 1 && b->Rank() == 1) {
                rank = 0;
            } else {
                rank = std::max(a->Rank(), b->Rank());
            }
            mOutputs[0]->SetRank(rank);
        }
        ~Binary() override = default;

        MaybeError AddToGraph(GraphBase* graph) const override {
            return graph->AddBinary(this);
        }
        BinaryOpType GetType() const {
            return mOpType;
        }
        MaybeError Validate() override;

      private:
        BinaryOpType mOpType;
    };

}}  // namespace webnn_native::op

#endif  // WEBNN_NATIVE_OPS_BINARY_H_
