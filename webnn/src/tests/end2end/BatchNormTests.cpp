// Copyright 2021 The WebNN-native Authors

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

#include "tests/WebnnTest.h"

class BatchNormTests : public WebnnTest {
    void SetUp() override {
        builder = ml::CreateGraphBuilder(GetContext());
    }

  protected:
    struct Tensor {
        std::vector<int32_t> shape;
        std::vector<float> value;
    };

    void CheckBatchNorm(const Tensor& input,
                        const Tensor& mean,
                        const Tensor& variance,
                        const std::vector<float>& expectedValue,
                        const Tensor& scale = {},
                        const Tensor& bias = {},
                        ml::BatchNormOptions options = {},
                        utils::FusedActivation activation = utils::FusedActivation::NONE) {
        const ml::Operand x = utils::BuildInput(builder, "input", input.shape);
        const ml::Operand m = utils::BuildConstant(builder, mean.shape, mean.value.data(),
                                                   mean.value.size() * sizeof(float));
        const ml::Operand v = utils::BuildConstant(builder, variance.shape, variance.value.data(),
                                                   variance.value.size() * sizeof(float));
        if (!scale.value.empty()) {
            options.scale = utils::BuildConstant(builder, scale.shape, scale.value.data(),
                                                 scale.value.size() * sizeof(float));
        }
        if (!bias.value.empty()) {
            options.bias = utils::BuildConstant(builder, bias.shape, bias.value.data(),
                                                bias.value.size() * sizeof(float));
        }
        if (activation != utils::FusedActivation::NONE) {
            options.activation = utils::CreateActivationOperator(builder, activation);
        }
        const ml::Operand output = builder.BatchNorm(x, m, v, &options);
        const ml::Graph graph = utils::Build(builder, {{"output", output}});
        ASSERT_TRUE(graph);
        std::vector<float> result(utils::SizeOfShape(input.shape));
        utils::Compute(graph, {{"input", input.value}}, {{"output", result}});
        EXPECT_TRUE(utils::CheckValue(result, expectedValue));
    }

    ml::GraphBuilder builder;
};

TEST_F(BatchNormTests, BatchNormNchw) {
    Tensor input = {{1, 2, 1, 3}, {-1, 0, 1, 2, 3, 4}};
    Tensor mean = {{2}, {0, 3}};
    Tensor variance = {{2}, {1.0, 1.5}};
    Tensor scale = {{2}, {1.0, 1.5}};
    Tensor bias = {{2}, {0, 1}};

    std::vector<float> expectedValue = {-0.999995, 0., 0.999995, -0.22474074, 1., 2.2247407};
    CheckBatchNorm(input, mean, variance, expectedValue, scale, bias);

    expectedValue = {0., 0., 0.999995, 0., 1., 2.2247407};
    CheckBatchNorm(input, mean, variance, expectedValue, scale, bias, {},
                   utils::FusedActivation::RELU);

    expectedValue = {-0.999995, 0., 0.999995, -1.22474, 0., 1.22474};
    CheckBatchNorm(input, mean, variance, expectedValue, scale);

    expectedValue = {0., 0., 0.999995, 0., 0., 1.22474};
    CheckBatchNorm(input, mean, variance, expectedValue, scale, {}, {},
                   utils::FusedActivation::RELU);

    expectedValue = {-0.999995, 0., 0.999995, 0.183506, 1., 1.816494};
    CheckBatchNorm(input, mean, variance, expectedValue, {}, bias);

    expectedValue = {0., 0., 0.999995, 0.183506, 1., 1.816494};
    CheckBatchNorm(input, mean, variance, expectedValue, {}, bias, {},
                   utils::FusedActivation::RELU);
}

TEST_F(BatchNormTests, BatchNormNhwc) {
    Tensor input = {{1, 1, 3, 2}, {-1, 2, 0, 3, 1, 4}};
    Tensor mean = {{2}, {0, 3}};
    Tensor variance = {{2}, {1.0, 1.5}};
    Tensor scale = {{2}, {1.0, 1.5}};
    Tensor bias = {{2}, {0, 1}};
    ml::BatchNormOptions options;

    options.axis = 3;
    std::vector<float> expectedValue = {-0.999995, -0.22474074, 0., 1., 0.999995, 2.2247407};
    CheckBatchNorm(input, mean, variance, expectedValue, scale, bias, options);

    expectedValue = {0., 0., 0., 1., 0.999995, 2.2247407};
    CheckBatchNorm(input, mean, variance, expectedValue, scale, bias, options,
                   utils::FusedActivation::RELU);

    expectedValue = {-0.999995, -1.22474, 0., 0., 0.999995, 1.22474};
    CheckBatchNorm(input, mean, variance, expectedValue, scale, {}, options);

    expectedValue = {0., 0., 0., 0., 0.999995, 1.22474};
    CheckBatchNorm(input, mean, variance, expectedValue, scale, {}, options,
                   utils::FusedActivation::RELU);

    expectedValue = {-0.999995, 0.183506, 0., 1., 0.999995, 1.816494};
    CheckBatchNorm(input, mean, variance, expectedValue, {}, bias, options);

    expectedValue = {0., 0.183506, 0., 1., 0.999995, 1.816494};
    CheckBatchNorm(input, mean, variance, expectedValue, {}, bias, options,
                   utils::FusedActivation::RELU);
}

TEST_F(BatchNormTests, BatchNormWithoutOptions) {
    Tensor input = {{1, 2, 1, 3}, {-1, 0, 1, 2, 3, 4}};
    Tensor mean = {{2}, {0, 3}};
    Tensor variance = {{2}, {1.0, 1.5}};

    const std::vector<float> expectedValue = {-0.999995, 0, 0.999995, -0.816494, 0, 0.816494};
    CheckBatchNorm(input, mean, variance, expectedValue);
}

TEST_F(BatchNormTests, BatchNormWithEpsilon) {
    Tensor input = {
        {2, 3, 4, 5},
        {2.6973534,   -1.1874187,  -0.18637535, -1.7081367,  0.03293341,  1.4802791,   -0.68332213,
         1.618039,    -1.6412221,  -0.52998835, 1.5229957,   -0.92798537, -0.35554567, 0.717948,
         0.50108916,  1.0521007,   -0.68065745, 1.3121722,   0.50907123,  1.5093223,   -0.540522,
         -0.80794656, -0.17974755, -1.8922086,  2.0955374,   0.46592507,  -0.2936382,  -0.43420887,
         -0.11036888, -1.2171484,  -1.9003569,  0.32063156,  0.38756344,  0.4720109,   -0.4177193,
         -0.7655141,  -1.2207903,  0.52860916,  0.22583283,  1.2220219,   -0.0248001,  0.6148501,
         1.0967597,   0.8798244,   -0.6854243,  -0.8442876,  1.6188551,   -0.6460473,  0.76349306,
         2.630077,    -0.85050315, 0.37401453,  0.08842833,  -0.5043717,  -0.7495827,  -0.98900026,
         0.79681706,  -0.3573076,  0.8644746,   1.196009,    0.35148722,  0.39926755,  -0.21630785,
         1.731195,    1.8644739,   -0.60227305, -1.0833911,  -0.6197943,  -0.05721893, -0.23889631,
         -0.24901256, 1.3885167,   -0.67789817, -0.3381054,  0.33224156,  0.79065573,  1.1667213,
         -0.47722074, 0.4234017,   0.2317288,   -0.18525974, -0.17303231, 0.41841915,  0.13230574,
         0.1261528,   1.253214,    1.9984859,   -1.7275336,  0.6593169,   -1.3704892,  0.63530993,
         -0.33128706, -1.2268444,  0.87340677,  1.4801403,   0.09598545,  0.30467814,  -0.15848571,
         -0.16779709, 1.1372787,   0.3292992,   -0.2240395,  0.88280654,  1.3370756,   0.2533313,
         0.84305125,  -1.6560661,  -0.09365056, -1.301057,   -0.1476929,  -1.2850751,  -1.286735,
         -1.9894414,  -0.5574838,  -0.392564,   -0.92764777, -0.79910755, 0.9099533,   0.9825949,
         -0.8327678}};
    Tensor mean = {{3}, {0.3432895, 1.0855169, 1.8725895}};
    Tensor variance = {{3}, {0.601868, 0.86580527, 0.38809904}};
    Tensor scale = {{3}, {0.17215693, -0.7909758, 0.12456307}};
    Tensor bias = {{3}, {0.5280557, -1.4475446, 0.1760742}};
    ml::BatchNormOptions options;
    options.epsilon = 1e-2;

    const std::vector<float> expectedValue = {
        1.0461562e+00,  1.9116578e-01,  4.1148305e-01,  7.6562166e-02,  4.5975018e-01,
        7.7829313e-01,  3.0211121e-01,  8.0861235e-01,  9.1289252e-02,  3.3585808e-01,
        7.8769445e-01,  2.4826387e-01,  3.7425077e-01,  6.1051345e-01,  5.6278551e-01,
        6.8405628e-01,  3.0269766e-01,  7.4129486e-01,  5.6454223e-01,  7.8468513e-01,
        -7.3216558e-02, 1.5281045e-01,  -3.7814319e-01, 1.0692286e+00,  -2.3012137e+00,
        -9.2386562e-01, -2.8188276e-01, -1.6307259e-01, -4.3678200e-01, 4.9866784e-01,
        1.0761156e+00,  -8.0106354e-01, -8.5763437e-01, -9.2900938e-01, -1.7700946e-01,
        1.1694658e-01,  5.0174618e-01,  -9.7684616e-01, -7.2093964e-01, -1.5629185e+00,
        -1.9851068e-01, -7.2230190e-02, 2.2908971e-02,  -1.9918650e-02, -3.2893193e-01,
        -3.6029488e-01, 1.2598167e-01,  -3.2115802e-01, -4.2884916e-02, 3.2561827e-01,
        -3.6152196e-01, -1.1977625e-01, -1.7615700e-01, -2.9318827e-01, -3.4159809e-01,
        -3.8886422e-01, -3.6306053e-02, -2.6415467e-01, -2.2949010e-02, 4.2502895e-02,
        5.2985996e-01,  5.4037583e-01,  4.0489528e-01,  8.3351660e-01,  8.6284959e-01,
        3.1994909e-01,  2.1406096e-01,  3.1609288e-01,  4.3990877e-01,  3.9992383e-01,
        3.9769736e-01,  7.5809729e-01,  3.0330497e-01,  3.7808913e-01,  5.2562422e-01,
        6.2651551e-01,  7.0928288e-01,  3.4747157e-01,  5.4568744e-01,  5.0350261e-01,
        -3.7348437e-01, -3.8381898e-01, -8.8371360e-01, -6.4189059e-01, -6.3669008e-01,
        -1.5892822e+00, -2.2191858e+00, 9.3004560e-01,  -1.0873203e+00, 6.2827158e-01,
        -1.0670297e+00, -2.5006199e-01, 5.0686288e-01,  -1.2682691e+00, -1.7810802e+00,
        -6.1119264e-01, -7.8757972e-01, -3.9611375e-01, -3.8824368e-01, -1.4912935e+00,
        -1.2860399e-01, -2.3784474e-01, -1.9329906e-02, 7.0352428e-02,  -1.4360166e-01,
        -2.7178437e-02, -5.2055711e-01, -2.1210325e-01, -4.5047081e-01, -2.2277233e-01,
        -4.4731563e-01, -4.4764340e-01, -5.8637255e-01, -3.0367374e-01, -2.7111509e-01,
        -3.7675196e-01, -3.5137540e-01, -1.3970569e-02, 3.7042797e-04,  -3.5802066e-01};
    CheckBatchNorm(input, mean, variance, expectedValue, scale, bias, options);
}
