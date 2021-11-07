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

#ifndef WEBNN_NATIVE_OPS_SPLIT_H_
#define WEBNN_NATIVE_OPS_SPLIT_H_

#include <vector>

#include "dawn_native/webnn/GraphBuilder.h"
#include "dawn_native/webnn/Operand.h"

namespace webnn_native { namespace op {

    class Split final : public OperatorBase {
      public:
        Split(GraphBuilderBase* builder,
              OperandBase* input,
              uint32_t const* splits,
              uint32_t splitsCount,
              SplitOptions const* options)
            : OperatorBase(builder, {input}, splitsCount == 1 ? splits[0] : splitsCount) {
            mAxis = options ? options->axis : 0;
            mSplits.assign(splits, splits + splitsCount);
        }
        ~Split() override = default;

        MaybeError AddToGraph(GraphBase* graph) const override {
            return graph->AddSplit(this);
        }

        MaybeError Validate() override {
            return mSplits.empty() ? DAWN_VALIDATION_ERROR("Argument splits is invalid.")
                                   : MaybeError();
        }

        std::vector<uint32_t> GetSplits() const {
            return mSplits;
        }

        int32_t GetAxis() const {
            return mAxis;
        }

      private:
        std::vector<uint32_t> mSplits;
        int32_t mAxis;
    };

}}  // namespace webnn_native::op

#endif  // WEBNN_NATIVE_OPS_SPLIT_H_
