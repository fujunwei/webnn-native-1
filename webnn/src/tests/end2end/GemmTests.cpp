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

class GemmTests : public WebnnTest {
  public:
    struct Options {
        float alpha = 1.0;
        float beta = 1.0;
        bool aTranspose = false;
        bool bTranspose = false;
        std::vector<int32_t> cShape = {};
        std::vector<float> cData = {};
    };

    void TestGemm(const std::vector<int32_t>& aShape,
                  const std::vector<float>& aData,
                  const std::vector<int32_t>& bShape,
                  const std::vector<float>& bData,
                  const std::vector<int32_t>& expectedShape,
                  const std::vector<float>& expectedValue,
                  const Options* options = nullptr) {
        const ml::GraphBuilder builder = ml::CreateGraphBuilder(GetContext());
        const ml::Operand a = utils::BuildInput(builder, "a", aShape);
        const ml::Operand b = utils::BuildInput(builder, "b", bShape);
        ml::GemmOptions gemmOptions = {};
        if (options != nullptr) {
            if (!options->cData.empty()) {
                gemmOptions.c =
                    utils::BuildConstant(builder, options->cShape, options->cData.data(),
                                         options->cData.size() * sizeof(float));
            }
            gemmOptions.alpha = options->alpha;
            gemmOptions.beta = options->beta;
            gemmOptions.aTranspose = options->aTranspose;
            gemmOptions.bTranspose = options->bTranspose;
        }

        const ml::Operand gemm = builder.Gemm(a, b, &gemmOptions);
        const ml::Graph graph = utils::Build(builder, {{"c", gemm}});
        ASSERT_TRUE(graph);
        std::vector<float> result(utils::SizeOfShape(expectedShape));
        utils::Compute(graph, {{"a", aData}, {"b", bData}}, {{"c", result}});
        EXPECT_TRUE(utils::CheckValue(result, expectedValue));
    }
};

TEST_F(GemmTests, AllAttributes) {
    const std::vector<int32_t> inputAShape = {4, 3};
    const std::vector<float> inputAData = {
        0.16255096, 0.36969426, 0.86049074, 0.52583617, 0.40931734, 0.5978712,
        0.02344302, 0.22055508, 0.45481342, 0.70314133, 0.7205724,  0.3864092,
    };
    const std::vector<int32_t> inputBShape = {5, 4};
    const std::vector<float> inputBData = {
        0.51755,    0.9102891,  0.28687012, 0.88691926, 0.8710911,  0.70967263, 0.7630267,
        0.5303827,  0.19783545, 0.03771181, 0.9599521,  0.9786202,  0.64332396, 0.05995055,
        0.28637242, 0.27146435, 0.89837885, 0.25250614, 0.71064854, 0.6231573,
    };
    const std::vector<int32_t> expectedShape = {3, 5};
    const std::vector<float> expectedValue = {
        0.52433187, 0.5573979,  0.52811706, 0.27080578, 0.48881707,
        0.5426185,  0.62174726, 0.58883274, 0.31766936, 0.56571984,
        0.59173757, 0.76246,    0.58934915, 0.39352357, 0.6774126,
    };

    Options options;
    options.alpha = 0.25;
    options.beta = 0.35;
    options.aTranspose = true;
    options.bTranspose = true;
    options.cShape = {1, 5};
    options.cData = {
        0.645844, 0.94571555, 0.9641909, 0.53538203, 0.87259406,
    };

    TestGemm(inputAShape, inputAData, inputBShape, inputBData, expectedShape, expectedValue,
             &options);
}

TEST_F(GemmTests, alpha) {
    const std::vector<int32_t> inputAShape = {3, 5};
    const std::vector<float> inputAData = {
        0.6454119,  0.3664521, 0.9339664,  0.80538565, 0.95667505,
        0.88069373, 0.9214904, 0.36922687, 0.14497812, 0.9269842,
        0.35244322, 0.8430814, 0.8758174,  0.72640723, 0.54108346,
    };
    const std::vector<int32_t> inputBShape = {5, 4};
    const std::vector<float> inputBData = {
        0.63330984, 0.35902178, 0.66004074, 0.55209464, 0.61517054, 0.9752662,  0.7728582,
        0.6539411,  0.7348231,  0.16676156, 0.02067508, 0.8232157,  0.8965447,  0.13730305,
        0.84798694, 0.5877349,  0.59452033, 0.77880573, 0.74310994, 0.30480564,
    };

    const std::vector<int32_t> expectedShape = {3, 4};
    const std::vector<float> expectedValue = {
        1.3056517, 0.8002505,  1.0611974, 1.0648878,  1.0385162,  1.009153,
        1.0564499, 0.88026935, 1.1791785, 0.80797654, 0.96019256, 1.0293772,
    };

    Options options;
    options.alpha = 0.5;
    TestGemm(inputAShape, inputAData, inputBShape, inputBData, expectedShape, expectedValue,
             &options);
}

TEST_F(GemmTests, beta) {
    const std::vector<int32_t> inputAShape = {2, 7};
    const std::vector<float> inputAData = {
        0.49310637, 0.90337706, 0.48412615, 0.12574232, 0.46357006, 0.19181924, 0.35951078,
        0.6291139,  0.6532259,  0.5328194,  0.34455,    0.30500737, 0.42374912, 0.5333988,
    };
    const std::vector<int32_t> inputBShape = {7, 4};
    const std::vector<float> inputBData = {
        0.09745993, 0.15800808, 0.8296135,  0.32482183, 0.17396742, 0.8128901,  0.57767284,
        0.5426502,  0.3885252,  0.14400595, 0.4086032,  0.07829365, 0.02829663, 0.68851817,
        0.08823209, 0.99632514, 0.5235559,  0.01747696, 0.13607074, 0.93223673, 0.6470155,
        0.9115019,  0.8971734,  0.8677906,  0.31736255, 0.3187993,  0.5536664,  0.7935204,
    };
    const std::vector<int32_t> expectedShape = {2, 4};
    const std::vector<float> expectedValue = {
        1.0496742, 1.3693886, 1.7876273, 2.0921178, 1.1667527, 1.6092675, 2.0779393, 2.4137995,
    };
    Options options;
    options.beta = 0.5;
    options.cShape = {1, 4};
    options.cData = {0.34378892, 0.20655482, 0.4271018, 0.78929764};
    TestGemm(inputAShape, inputAData, inputBShape, inputBData, expectedShape, expectedValue,
             &options);
}

TEST_F(GemmTests, bias) {
    const std::vector<int32_t> inputAShape = {3, 6};
    const std::vector<float> inputAData = {
        0.26634723, 0.6047771, 0.55068576, 0.9991724,  0.67497027, 0.92930806,
        0.2860512,  0.6988867, 0.89526093, 0.3717174,  0.19019075, 0.4499846,
        0.51885146, 0.4031741, 0.6269008,  0.27176815, 0.8668971,  0.5799844,
    };
    const std::vector<int32_t> inputBShape = {6, 4};
    const std::vector<float> inputBData = {
        0.9380275, 0.12814452, 0.6522671,  0.6646124, 0.17909846, 0.901151,  0.3000952,  0.326868,
        0.3915249, 0.45130828, 0.581687,   0.9738232, 0.38067418, 0.9359868, 0.4762245,  0.07865977,
        0.9511043, 0.7688367,  0.88563424, 0.7623637, 0.04992711, 0.2871258, 0.09989859, 0.78412026,
    };
    const std::vector<int32_t> expectedShape = {3, 4};
    const std::vector<float> expectedValue = {
        2.1861424, 2.830544, 1.9655125, 2.7772129, 1.7395668, 1.8645036,
        1.835017,  2.620344, 2.358861,  2.0097456, 2.0699792, 3.089901,
    };
    Options options;
    options.cShape = {3, 4};
    options.cData = {
        0.5436559,  0.2819061,  0.1235218, 0.5443856,  0.6506955,  0.1706562,
        0.52752787, 0.80288535, 0.5975874, 0.20960917, 0.29078278, 0.8657452,
    };

    TestGemm(inputAShape, inputAData, inputBShape, inputBData, expectedShape, expectedValue,
             &options);
}

TEST_F(GemmTests, NoBias) {
    const std::vector<int32_t> inputAShape = {2, 10};
    const std::vector<float> inputAData = {
        0.97596496, 0.47531518, 0.7147315, 0.14236908, 0.06151228, 0.05889508, 0.3534669,
        0.31915423, 0.61336106, 0.5946216, 0.21969128, 0.7347848,  0.4087221,  0.00412959,
        0.77303815, 0.6495765,  0.3174799, 0.62841094, 0.7002717,  0.63384914,
    };
    const std::vector<int32_t> inputBShape = {10, 3};
    const std::vector<float> inputBData = {
        0.51739925, 0.25108355, 0.31373033, 0.6488124,  0.9777175,  0.13308926,
        0.47903556, 0.23692878, 0.0822504,  0.3080891,  0.51966125, 0.969734,
        0.6691261,  0.59346807, 0.7651862,  0.48655444, 0.48373327, 0.2799068,
        0.35760838, 0.19906454, 0.3612888,  0.11448191, 0.19188708, 0.00769753,
        0.3161914,  0.323555,   0.17573832, 0.79587144, 0.91238266, 0.5517277,
    };
    const std::vector<int32_t> expectedShape = {2, 3};
    const std::vector<float> expectedValue = {
        2.0995352, 1.8906747, 1.1958704, 2.5321422, 2.6342242, 1.5699927,
    };
    TestGemm(inputAShape, inputAData, inputBShape, inputBData, expectedShape, expectedValue);
}

TEST_F(GemmTests, ScalarBias) {
    const std::vector<int32_t> inputAShape = {2, 3};
    const std::vector<float> inputAData = {
        0.41595492, 0.7063231, 0.3784654, 0.3524597, 0.41936764, 0.08190536,
    };
    const std::vector<int32_t> inputBShape = {3, 4};
    const std::vector<float> inputBData = {
        0.38356313, 0.92939967, 0.06164686, 0.09034675, 0.34704673, 0.9492532,
        0.7738587,  0.93576515, 0.49937814, 0.38543963, 0.02364575, 0.80216527,
    };
    const std::vector<int32_t> expectedShape = {2, 4};
    const std::vector<float> expectedValue = {
        3.7336695, 4.3429437, 3.7211857, 4.1421247, 3.4616325, 3.8972316, 3.4881961, 3.6299748,
    };
    Options options;
    options.cShape = {};
    options.cData = {3.14};
    TestGemm(inputAShape, inputAData, inputBShape, inputBData, expectedShape, expectedValue,
             &options);
}

TEST_F(GemmTests, BroadcastingBias) {
    const std::vector<int32_t> inputAShape = {3, 7};
    const std::vector<float> inputAData = {
        0.96122783, 0.7414551,  0.22178489, 0.23116009, 0.19249596, 0.860125,   0.24145897,
        0.43657154, 0.20278022, 0.01261093, 0.526355,   0.94473153, 0.59416693, 0.5121616,
        0.93981737, 0.9942615,  0.46400633, 0.40644044, 0.43731472, 0.22579351, 0.6787937,
    };
    const std::vector<int32_t> inputBShape = {7, 3};
    const std::vector<float> inputBData = {
        0.1004637,  0.31921694, 0.7323029,  0.05150159, 0.9162225,  0.89180815, 0.00931315,
        0.3568885,  0.9506084,  0.04976705, 0.6065987,  0.99300903, 0.29279497, 0.29296732,
        0.38377914, 0.80959237, 0.5812153,  0.34052548, 0.2931774,  0.12963536, 0.58294684,
    };
    const std::vector<int32_t> expectedShape = {3, 3};
    const std::vector<float> expectedValue = {
        1.7498734, 2.5712128, 3.0910947, 1.7664618, 2.1154947,
        2.6767135, 1.4580698, 2.7485106, 3.8380768,
    };
    Options options;
    options.cShape = {1};
    options.cData = {0.7780463};
    TestGemm(inputAShape, inputAData, inputBShape, inputBData, expectedShape, expectedValue,
             &options);
}

TEST_F(GemmTests, aTranspose) {
    const std::vector<int32_t> inputAShape = {6, 3};
    const std::vector<float> inputAData = {
        0.2714853, 0.0877158,  0.31404206, 0.2387523,  0.25758955, 0.37354097,
        0.6452827, 0.8840964,  0.14744024, 0.65488476, 0.35878596, 0.3690042,
        0.4229308, 0.40953776, 0.85461134, 0.7056307,  0.17941293, 0.4431382,
    };
    const std::vector<int32_t> inputBShape = {6, 4};
    const std::vector<float> inputBData = {
        0.09183464, 0.8638833,  0.9302645,  0.06964016, 0.43232033, 0.7631357,
        0.5690705,  0.57837325, 0.17691018, 0.77424425, 0.8207884,  0.429646,
        0.31379876, 0.29592493, 0.10828935, 0.00203794, 0.9140249,  0.31878716,
        0.11819135, 0.6350557,  0.7605846,  0.40116578, 0.32365775, 0.07651691,
    };
    const std::vector<int32_t> expectedShape = {3, 4};
    const std::vector<float> expectedValue = {
        1.3710694, 1.5280349, 1.2673472, 0.75814927, 0.8991952,  1.2655619,
        1.0991665, 0.8094785, 1.4503862, 1.2299215,  0.91012263, 0.8786486,
    };
    Options options;
    options.aTranspose = true;
    TestGemm(inputAShape, inputAData, inputBShape, inputBData, expectedShape, expectedValue,
             &options);
}

TEST_F(GemmTests, bTranspose) {
    const std::vector<int32_t> inputAShape = {3, 6};
    const std::vector<float> inputAData = {
        0.4520783,  0.25709572, 0.28996432, 0.03766193, 0.0546827,  0.46305302,
        0.91171485, 0.48380807, 0.09058774, 0.6646215,  0.35773644, 0.03604647,
        0.21229707, 0.18758385, 0.01589681, 0.9606218,  0.08803706, 0.18099776,
    };
    const std::vector<int32_t> inputBShape = {4, 6};
    const std::vector<float> inputBData = {
        0.1482661,  0.27676222, 0.10893039, 0.8347901, 0.7146212, 0.7316929,
        0.97991717, 0.97123116, 0.69798464, 0.8436566, 0.9630883, 0.23252074,
        0.09898344, 0.08882044, 0.90780985, 0.7116153, 0.5819304, 0.6742051,
        0.5233705,  0.5594687,  0.963364,   0.1351259, 0.8119938, 0.13756031,
    };
    const std::vector<int32_t> expectedShape = {3, 4};
    const std::vector<float> expectedValue = {
        0.57909805, 1.0871967, 0.7016311, 0.77297145, 1.1157843, 2.340149,
        0.92088836, 1.2203549, 1.0823897, 1.3386247,  0.9089607, 0.45756027,
    };
    Options options;
    options.bTranspose = true;
    TestGemm(inputAShape, inputAData, inputBShape, inputBData, expectedShape, expectedValue,
             &options);
}
