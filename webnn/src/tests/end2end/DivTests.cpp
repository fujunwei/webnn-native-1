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

class DivTests : public WebnnTest {};

TEST_F(DivTests, Div) {
    const ml::GraphBuilder builder = ml::CreateGraphBuilder(GetContext());
    const ml::Operand a = utils::BuildInput(builder, "a", {3, 4, 5});
    const ml::Operand b = utils::BuildInput(builder, "b", {3, 4, 5});
    const ml::Operand c = builder.Div(a, b);
    const ml::Graph graph = utils::Build(builder, {{"c", c}});
    ASSERT_TRUE(graph);
    const std::vector<float> dataA = {
        0.5270042,   0.4537819,  -1.8297404,  0.03700572,  0.76790243,  0.5898798,   -0.36385882,
        -0.8056265,  -1.1183119, -0.13105401, 1.1330799,   -1.9518042,  -0.6598917,  -1.1398025,
        0.7849575,   -0.5543096, -0.47063765, -0.21694957, 0.44539326,  -0.392389,   -3.046143,
        0.5433119,   0.43904296, -0.21954103, -1.0840366,  0.35178012,  0.37923554,  -0.47003287,
        -0.21673147, -0.9301565, -0.17858909, -1.5504293,  0.41731882,  -0.9443685,  0.23810315,
        -1.405963,   -0.5900577, -0.11048941, -1.6606998,  0.11514787,  -0.37914756, -1.7423562,
        -1.3032428,  0.60512006, 0.895556,    -0.13190864, 0.40476182,  0.22384356,  0.32962298,
        1.285984,    -1.5069984, 0.67646074,  -0.38200897, -0.22425893, -0.30224973, -0.3751471,
        -1.2261962,  0.1833392,  1.670943,    -0.05613302};
    const std::vector<float> dataB = {
        0.99861497,  0.312701,    0.88252544,  1.4661665,  0.6297575,   0.546196,   1.4032645,
        0.08199525,  1.2524966,   1.8203218,   2.3599486,  0.909618,    2.367597,   2.03441,
        0.00378734,  -0.21793854, 0.69503635,  2.0289354,  0.927713,    0.39934242, 2.5522432,
        1.2869045,   -1.3205943,  1.3171606,   1.5200406,  1.2256087,   1.449712,   0.9327244,
        -0.31839585, 0.629296,    0.05438423,  0.06725907, -0.26306832, 1.4524891,  1.0978961,
        0.55183464,  0.35066205,  0.9765769,   2.0791948,  -1.0042157,  1.3768766,  0.454288,
        -0.88458586, -0.945703,   0.0872165,   1.2195096,  1.393063,    0.06101841, 2.017021,
        2.4229836,   1.3960866,   0.40859735,  2.1244192,  1.7553957,   1.8674074,  0.34353632,
        -1.8345544,  3.116791,    -0.61087835, 0.9642319};
    std::vector<float> result(utils::SizeOfShape({3, 4, 5}));
    utils::Compute(graph, {{"a", dataA}, {"b", dataB}}, {{"c", result}});
    const std::vector<float> expectedValue(
        {5.2773511e-01,  1.4511688e+00,  -2.0733004e+00, 2.5239782e-02,  1.2193620e+00,
         1.0799783e+00,  -2.5929454e-01, -9.8252831e+00, -8.9286619e-01, -7.1994968e-02,
         4.8012903e-01,  -2.1457405e+00, -2.7871791e-01, -5.6026191e-01, 2.0725833e+02,
         2.5434217e+00,  -6.7714107e-01, -1.0692778e-01, 4.8009813e-01,  -9.8258781e-01,
         -1.1935160e+00, 4.2218509e-01,  -3.3245862e-01, -1.6667749e-01, -7.1316290e-01,
         2.8702483e-01,  2.6159370e-01,  -5.0393540e-01, 6.8069816e-01,  -1.4780906e+00,
         -3.2838395e+00, -2.3051601e+01, -1.5863515e+00, -6.5017247e-01, 2.1687222e-01,
         -2.5477974e+00, -1.6826961e+00, -1.1313948e-01, -7.9872257e-01, -1.1466448e-01,
         -2.7536786e-01, -3.8353560e+00, 1.4732802e+00,  -6.3986266e-01, 1.0268195e+01,
         -1.0816532e-01, 2.9055530e-01,  3.6684597e+00,  1.6342071e-01,  5.3074402e-01,
         -1.0794448e+00, 1.6555681e+00,  -1.7981808e-01, -1.2775406e-01, -1.6185527e-01,
         -1.0920159e+00, 6.6838908e-01,  5.8823064e-02,  -2.7353122e+00, -5.8215268e-02});
    EXPECT_TRUE(utils::CheckValue(result, expectedValue));
}

TEST_F(DivTests, DivBroadcast) {
    const ml::GraphBuilder builder = ml::CreateGraphBuilder(GetContext());
    const ml::Operand a = utils::BuildInput(builder, "a", {3, 4, 5});
    const ml::Operand b = utils::BuildInput(builder, "b", {5});
    const ml::Operand c = builder.Div(a, b);
    const ml::Graph graph = utils::Build(builder, {{"c", c}});
    ASSERT_TRUE(graph);
    const std::vector<float> dataA = {
        2.3807454,   0.33057675,  0.94924647,  -1.5023966,  -1.7776669,  -0.5327028,  1.0907497,
        -0.34624946, -0.7946363,  0.19796729,  1.0819352,   -1.4449402,  -1.210543,   -0.7886692,
        1.0946383,   0.23482153,  2.1321535,   0.9364457,   -0.03509518, 1.2650778,   0.21149701,
        -0.70492136, 0.67997485,  -0.6963267,  -0.2903971,  1.3277828,   -0.10128149, -0.8031414,
        -0.46433768, 1.0217906,   -0.55254066, -0.38687086, -0.51029277, 0.1839255,   -0.38548976,
        -1.6018361,  -0.8871809,  -0.932789,   1.2433194,   0.81267405,  0.58725935,  -0.50535834,
        -0.81579155, -0.5075176,  -1.0518801,  2.4972005,   -2.2453218,  0.56400853,  -1.2845523,
        -0.10434349, -0.98800194, -1.177629,   -1.1401963,  1.7549862,   -0.13298842, -0.7657022,
        0.55578697,  0.01034931,  0.72003376,  -1.8242567};
    const std::vector<float> dataB = {1.3041736, 1.5910654, 1.9217191, 1.8052639, 1.7239413};
    std::vector<float> result(utils::SizeOfShape({3, 4, 5}));
    utils::Compute(graph, {{"a", dataA}, {"b", dataB}}, {{"c", result}});
    const std::vector<float> expectedValue(
        {1.825482,    0.20777069,  0.49395692,  -0.832231,   -1.0311644,  -0.40846005, 0.68554676,
         -0.18017694, -0.44017738, 0.11483412,  0.82959443,  -0.9081589,  -0.62992716, -0.436872,
         0.6349626,   0.18005389,  1.3400791,   0.48729584,  -0.01944047, 0.73382884,  0.16216937,
         -0.4430499,  0.35383677,  -0.38572016, -0.16844954, 1.0181028,   -0.0636564,  -0.41792864,
         -0.25721318, 0.59270614,  -0.4236711,  -0.24315208, -0.26553974, 0.10188289,  -0.22360957,
         -1.2282383,  -0.5576018,  -0.48539302, 0.6887189,   0.4714047,   0.45029232,  -0.3176226,
         -0.42451134, -0.28113207, -0.61016005, 1.9147762,   -1.4112065,  0.29349166,  -0.7115593,
         -0.06052613, -0.7575694,  -0.7401512,  -0.593321,   0.9721494,   -0.07714208, -0.5871168,
         0.3493175,   0.00538545,  0.39885238,  -1.0581895});
    EXPECT_TRUE(utils::CheckValue(result, expectedValue));
}