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

#include "tests/unittests/validation/ValidationTest.h"

#include <memory>

using namespace testing;

class BinaryValidationTest : public ValidationTest {};

TEST_F(BinaryValidationTest, InputsType) {
    std::vector<int32_t> shape = {2, 2};
    webnn::MLOperandDescriptor inputDesc = {webnn::MLOperandType::Float32, shape.data(),
                                            (uint32_t)shape.size()};
    webnn::MLOperand a = mBuilder.Input("input", &inputDesc);
    // success
    {
        std::vector<float> data(4, 1);
        webnn::MLOperand b =
            mBuilder.Constant(&inputDesc, data.data(), data.size() * sizeof(float));
        webnn::MLOperand add = mBuilder.Add(a, b);
        webnn::MLOperand mul = mBuilder.Mul(a, b);
        webnn::MLOperand matmul = mBuilder.Matmul(a, b);
    }
    // inputs types are inconsistent
    {
        std::vector<int32_t> data(4, 1);
        inputDesc = {webnn::MLOperandType::Int32, shape.data(), (uint32_t)shape.size()};
        webnn::MLOperand b =
            mBuilder.Constant(&inputDesc, data.data(), data.size() * sizeof(int32_t));
        ASSERT_CONTEXT_ERROR(mBuilder.Add(a, b));
        ASSERT_CONTEXT_ERROR(mBuilder.Mul(a, b));
        ASSERT_CONTEXT_ERROR(mBuilder.Matmul(a, b));
    }
}
