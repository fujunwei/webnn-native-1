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

#ifndef WEBNN_NATIVE_OPS_CONV2D_H_
#define WEBNN_NATIVE_OPS_CONV2D_H_

#include "webnn_native/MLGraph.h"
#include "webnn_native/MLOperand.h"

namespace webnn_native { namespace op {

    class Conv2d final : public MLOperandBase {
      public:
        Conv2d(MLGraphBuilderBase* builder,
               MLOperandBase* input,
               MLOperandBase* filter,
               MLConv2dOptions const* options);
        ~Conv2d() override = default;

        MaybeError AddToGraph(MLGraphBase* model) const override;
        MaybeError ValidateAndInferTypes() override;

        MLConv2dOptions const* GetOptions() const;

      private:
        MLConv2dOptions mOptions;
        std::vector<int32_t> mPadding;
        std::vector<int32_t> mStride;
        std::vector<int32_t> mDilations;
    };

}}  // namespace webnn_native::op

#endif  // WEBNN_NATIVE_OPS_CONV2D_H_
