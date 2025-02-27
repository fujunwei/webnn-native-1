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

#include "tests/WebnnTest.h"

class MulTests : public WebnnTest {};

TEST_F(MulTests, MulInputAndConstant) {
    ml::GraphBuilder builder = ml::CreateGraphBuilder(GetContext());
    ml::Operand a = utils::BuildInput(builder, "a", {3, 4, 5});
    std::vector<float> dataB = {
        2.0435283,  0.07213961,  -1.1644137,  -1.2209045,  0.8982674,   0.21796915,  0.27658972,
        0.7744382,  -0.52159035, -0.969913,   0.6081186,   -0.04225572, 0.3275312,   -0.06443629,
        -2.257355,  1.7802691,   -1.279233,   -3.1389477,  -1.1663845,  -0.79485595, 0.679013,
        1.0919031,  0.51905185,  1.3186365,   0.6612518,   0.40741763,  0.05208012,  0.16548257,
        -0.4570541, 0.10149371,  0.08249464,  0.3992067,   -0.3945879,  -0.37389037, 1.4760005,
        -0.781274,  -0.49022308, 0.27020553,  -0.2356837,  0.13846985,  0.9767852,   -1.3560135,
        0.78826934, -0.18788454, 0.38178417,  0.9748209,   1.0242884,   0.7939937,   0.24449475,
        -1.3840157, 1.9665064,   0.35833818,  -0.87076694, -0.76727265, 0.6157508,   -0.5558823,
        0.18417479, -0.93904793, -0.00859687, 0.5034271};
    ml::Operand b =
        utils::BuildConstant(builder, {3, 4, 5}, dataB.data(), dataB.size() * sizeof(float));
    ml::Operand c = builder.Mul(a, b);
    ml::Graph graph = utils::Build(builder, {{"c", c}});
    ASSERT_TRUE(graph);
    std::vector<float> dataA = {
        5.6232101e-01,  1.3117781e-01,  -1.4161869e+00, 2.0386910e-02,  9.1077393e-01,
        7.4952751e-01,  -2.8509337e-01, -1.6272701e+00, 1.0271618e+00,  4.2815253e-01,
        -7.7895027e-01, 9.7542489e-01,  3.9352554e-01,  9.7878903e-01,  -6.0965502e-01,
        6.6299748e-01,  -1.1980454e+00, -7.7857232e-01, -9.8175555e-01, -2.8763762e-01,
        -3.2260692e-01, -7.4259090e-01, -1.0055183e+00, -1.4305019e+00, 6.0624069e-01,
        -1.5911928e-01, 4.5598033e-01,  1.0880016e-01,  1.4949993e+00,  6.6210419e-01,
        -5.6889033e-01, -2.0945708e-01, -7.1049523e-01, -2.8507587e-01, 1.1723405e+00,
        -6.3937567e-02, -5.4250038e-01, -1.2398884e+00, -1.0347517e+00, 1.2763804e+00,
        -1.5979607e+00, -5.8152825e-01, -5.0100851e-01, -1.0742084e+00, -1.1273566e+00,
        3.4815140e-04,  -5.6024802e-01, 1.0848801e+00,  -5.1780093e-01, -3.8996863e-01,
        5.3133094e-01,  2.3897937e-01,  -1.3832775e+00, 6.3414145e-01,  1.0691971e+00,
        5.7040757e-01,  3.0711100e-01,  8.8405716e-01,  -2.1583509e+00, 4.3243581e-01};
    std::vector<float> result(utils::SizeOfShape({3, 4, 5}));
    utils::Compute(graph, {{"a", dataA}}, {{"c", result}});
    std::vector<float> expectedData = {
        1.1491189e+00,  9.4631165e-03,  1.6490275e+00,  -2.4890469e-02, 8.1811851e-01,
        1.6337387e-01,  -7.8853898e-02, -1.2602202e+00, -5.3575772e-01, -4.1527072e-01,
        -4.7369415e-01, -4.1217282e-02, 1.2889189e-01,  -6.3069537e-02, 1.3762078e+00,
        1.1803139e+00,  1.5325792e+00,  2.4438977e+00,  1.1451044e+00,  2.2863047e-01,
        -2.1905430e-01, -8.1083733e-01, -5.2191615e-01, -1.8863121e+00, 4.0087774e-01,
        -6.4828001e-02, 2.3747511e-02,  1.8004529e-02,  -6.8329555e-01, 6.7199409e-02,
        -4.6930403e-02, -8.3616674e-02, 2.8035283e-01,  1.0658713e-01,  1.7303753e+00,
        4.9952760e-02,  2.6594621e-01,  -3.3502471e-01, 2.4387409e-01,  1.7674020e-01,
        -1.5608643e+00, 7.8856015e-01,  -3.9492965e-01, 2.0182715e-01,  -4.3040693e-01,
        3.3938527e-04,  -5.7385558e-01, 8.6138797e-01,  -1.2659961e-01, 5.3972268e-01,
        1.0448657e+00,  8.5635431e-02,  1.2045124e+00,  -4.8655939e-01, 6.5835893e-01,
        -3.1707945e-01, 5.6562103e-02,  -8.3017206e-01, 1.8555066e-02,  2.1769990e-01};
    EXPECT_TRUE(utils::CheckValue(result, expectedData));
}

TEST_F(MulTests, MulTwoInputs) {
    ml::GraphBuilder builder = ml::CreateGraphBuilder(GetContext());
    ml::Operand a = utils::BuildInput(builder, "a", {3, 4, 5});
    ml::Operand b = utils::BuildInput(builder, "b", {3, 4, 5});
    ml::Operand c = builder.Mul(a, b);
    ml::Graph graph = utils::Build(builder, {{"c", c}});
    ASSERT_TRUE(graph);
    std::vector<float> dataA = {
        5.6232101e-01,  1.3117781e-01,  -1.4161869e+00, 2.0386910e-02,  9.1077393e-01,
        7.4952751e-01,  -2.8509337e-01, -1.6272701e+00, 1.0271618e+00,  4.2815253e-01,
        -7.7895027e-01, 9.7542489e-01,  3.9352554e-01,  9.7878903e-01,  -6.0965502e-01,
        6.6299748e-01,  -1.1980454e+00, -7.7857232e-01, -9.8175555e-01, -2.8763762e-01,
        -3.2260692e-01, -7.4259090e-01, -1.0055183e+00, -1.4305019e+00, 6.0624069e-01,
        -1.5911928e-01, 4.5598033e-01,  1.0880016e-01,  1.4949993e+00,  6.6210419e-01,
        -5.6889033e-01, -2.0945708e-01, -7.1049523e-01, -2.8507587e-01, 1.1723405e+00,
        -6.3937567e-02, -5.4250038e-01, -1.2398884e+00, -1.0347517e+00, 1.2763804e+00,
        -1.5979607e+00, -5.8152825e-01, -5.0100851e-01, -1.0742084e+00, -1.1273566e+00,
        3.4815140e-04,  -5.6024802e-01, 1.0848801e+00,  -5.1780093e-01, -3.8996863e-01,
        5.3133094e-01,  2.3897937e-01,  -1.3832775e+00, 6.3414145e-01,  1.0691971e+00,
        5.7040757e-01,  3.0711100e-01,  8.8405716e-01,  -2.1583509e+00, 4.3243581e-01};
    std::vector<float> dataB = {
        2.0435283,  0.07213961,  -1.1644137,  -1.2209045,  0.8982674,   0.21796915,  0.27658972,
        0.7744382,  -0.52159035, -0.969913,   0.6081186,   -0.04225572, 0.3275312,   -0.06443629,
        -2.257355,  1.7802691,   -1.279233,   -3.1389477,  -1.1663845,  -0.79485595, 0.679013,
        1.0919031,  0.51905185,  1.3186365,   0.6612518,   0.40741763,  0.05208012,  0.16548257,
        -0.4570541, 0.10149371,  0.08249464,  0.3992067,   -0.3945879,  -0.37389037, 1.4760005,
        -0.781274,  -0.49022308, 0.27020553,  -0.2356837,  0.13846985,  0.9767852,   -1.3560135,
        0.78826934, -0.18788454, 0.38178417,  0.9748209,   1.0242884,   0.7939937,   0.24449475,
        -1.3840157, 1.9665064,   0.35833818,  -0.87076694, -0.76727265, 0.6157508,   -0.5558823,
        0.18417479, -0.93904793, -0.00859687, 0.5034271};
    std::vector<float> result(utils::SizeOfShape({3, 4, 5}));
    utils::Compute(graph, {{"a", dataA}, {"b", dataB}}, {{"c", result}});
    std::vector<float> expectedData = {
        1.1491189e+00,  9.4631165e-03,  1.6490275e+00,  -2.4890469e-02, 8.1811851e-01,
        1.6337387e-01,  -7.8853898e-02, -1.2602202e+00, -5.3575772e-01, -4.1527072e-01,
        -4.7369415e-01, -4.1217282e-02, 1.2889189e-01,  -6.3069537e-02, 1.3762078e+00,
        1.1803139e+00,  1.5325792e+00,  2.4438977e+00,  1.1451044e+00,  2.2863047e-01,
        -2.1905430e-01, -8.1083733e-01, -5.2191615e-01, -1.8863121e+00, 4.0087774e-01,
        -6.4828001e-02, 2.3747511e-02,  1.8004529e-02,  -6.8329555e-01, 6.7199409e-02,
        -4.6930403e-02, -8.3616674e-02, 2.8035283e-01,  1.0658713e-01,  1.7303753e+00,
        4.9952760e-02,  2.6594621e-01,  -3.3502471e-01, 2.4387409e-01,  1.7674020e-01,
        -1.5608643e+00, 7.8856015e-01,  -3.9492965e-01, 2.0182715e-01,  -4.3040693e-01,
        3.3938527e-04,  -5.7385558e-01, 8.6138797e-01,  -1.2659961e-01, 5.3972268e-01,
        1.0448657e+00,  8.5635431e-02,  1.2045124e+00,  -4.8655939e-01, 6.5835893e-01,
        -3.1707945e-01, 5.6562103e-02,  -8.3017206e-01, 1.8555066e-02,  2.1769990e-01};
    EXPECT_TRUE(utils::CheckValue(result, expectedData));
}

TEST_F(MulTests, MulBroadcast) {
    ml::GraphBuilder builder = ml::CreateGraphBuilder(GetContext());
    ml::Operand a = utils::BuildInput(builder, "a", {3, 4, 5});
    std::vector<float> dataB = {
        0.6338172, 1.630534, -1.3819867, -1.0427561, 1.058136,
    };
    ml::Operand b = utils::BuildConstant(builder, {5}, dataB.data(), dataB.size() * sizeof(float));
    ml::Operand c = builder.Mul(a, b);
    ml::Graph graph = utils::Build(builder, {{"c", c}});
    ASSERT_TRUE(graph);
    std::vector<float> dataA = {
        -0.08539673, 0.11800674,  -1.2358714,  0.30089188,  -0.73443925, 1.4894297,   0.16823359,
        -2.2034893,  1.0740992,   -0.35457978, 0.61524934,  0.462153,    0.5992003,   -0.81047946,
        -2.2757835,  -0.21841764, 1.1650828,   -0.56927145, 1.9960726,   0.62048405,  0.10586528,
        -1.0386543,  -1.9402571,  -2.0906122,  -0.4305259,  -1.2730165,  1.5639576,   0.53357494,
        -0.8079486,  -0.06450062, -0.7841324,  -0.24135855, 1.9275267,   0.4476717,   0.15467685,
        -1.2363592,  -0.50745815, 0.03250425,  0.86344534,  -0.7938714,  1.1835734,   1.515135,
        0.3092435,   -1.311751,   -0.6659017,  0.8815683,   -0.31157655, 0.57511795,  -1.1924151,
        -1.8408557,  -0.85080767, -1.3341717,  0.54687303,  -0.14426671, -0.15728855, 0.323939,
        1.167636,    0.03020451,  0.91373825,  1.0675793,
    };
    std::vector<float> result(utils::SizeOfShape({3, 4, 5}));
    utils::Compute(graph, {{"a", dataA}}, {{"c", result}});
    std::vector<float> expectedData = {
        -0.05412592, 0.192414,    1.707958,    -0.31375682, -0.7771366,  0.9440262,   0.2743106,
        3.045193,    -1.1200235,  -0.37519363, 0.3899556,   0.7535562,   -0.82808685, 0.8451324,
        -2.4080884,  -0.13843685, 1.8997072,   0.7867256,   -2.0814168,  0.6565565,   0.06709924,
        -1.6935612,  2.6814096,   2.1799986,   -0.45555493, -0.80685973, 2.550086,    -0.7373935,
        0.8424933,   -0.06825043, -0.4969966,  -0.39354333, -2.6638165,  -0.4668124,  0.16366914,
        -0.7836257,  -0.8274278,  -0.04492044, -0.9003629,  -0.8400239,  0.75016916,  2.4704792,
        -0.42737043, 1.3678364,   -0.7046146,  0.55875313,  -0.5080362,  -0.79480535, 1.2433981,
        -1.9478757,  -0.5392565,  -2.1754124,  -0.7557713,  0.15043499,  -0.16643268, 0.20531811,
        1.9038703,   -0.04174223, -0.9528061,  1.129644,
    };
    EXPECT_TRUE(utils::CheckValue(result, expectedData));
}