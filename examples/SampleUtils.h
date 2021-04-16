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

#ifndef WEBNN_NATIVE_EXAMPLES_SAMPLE_UTILS_H_
#define WEBNN_NATIVE_EXAMPLES_SAMPLE_UTILS_H_

#include <webnn/webnn.h>
#include <webnn/webnn_cpp.h>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "common/Log.h"
#include "common/RefCounted.h"

uint32_t product(const std::vector<int32_t>& dims);

webnn::MLContext CreateCppContext();

void DumpMemoryLeaks();

bool Expected(float output, float expected);

namespace utils {

    webnn::MLOperand BuildInput(const webnn::MLGraphBuilder& builder,
                              std::string name,
                              const std::vector<int32_t>& dimensions,
                              webnn::MLOperandType type = webnn::MLOperandType::Float32);

    webnn::MLOperand BuildConstant(const webnn::MLGraphBuilder& builder,
                                 const std::vector<int32_t>& dimensions,
                                 const void* value,
                                 size_t size,
                                 webnn::MLOperandType type = webnn::MLOperandType::Float32);

    struct MLConv2dOptions {
      public:
        std::vector<int32_t> padding;
        std::vector<int32_t> strides;
        std::vector<int32_t> dilations;
        int32_t groups = 1;
        webnn::MLInputOperandLayout layout = webnn::MLInputOperandLayout::Nchw;

        const webnn::MLConv2dOptions* AsPtr() {
            if (!padding.empty()) {
                mOptions.paddingCount = padding.size();
                mOptions.padding = padding.data();
            }
            if (!strides.empty()) {
                mOptions.stridesCount = strides.size();
                mOptions.strides = strides.data();
            }
            if (!dilations.empty()) {
                mOptions.dilationsCount = dilations.size();
                mOptions.dilations = dilations.data();
            }
            mOptions.groups = groups;
            mOptions.layout = layout;
            return &mOptions;
        }

      private:
        webnn::MLConv2dOptions mOptions;
    };

    struct MLPool2dOptions {
      public:
        std::vector<int32_t> windowDimensions;
        std::vector<int32_t> padding;
        std::vector<int32_t> strides;
        std::vector<int32_t> dilations;
        webnn::MLInputOperandLayout layout = webnn::MLInputOperandLayout::Nchw;

        const webnn::MLPool2dOptions* AsPtr() {
            if (!windowDimensions.empty()) {
                mOptions.windowDimensionsCount = windowDimensions.size();
                mOptions.windowDimensions = windowDimensions.data();
            }
            if (!padding.empty()) {
                mOptions.paddingCount = padding.size();
                mOptions.padding = padding.data();
            }
            if (!strides.empty()) {
                mOptions.stridesCount = strides.size();
                mOptions.strides = strides.data();
            }
            if (!dilations.empty()) {
                mOptions.dilationsCount = dilations.size();
                mOptions.dilations = dilations.data();
            }
            mOptions.layout = layout;
            return &mOptions;
        }

      private:
        webnn::MLPool2dOptions mOptions;
    };

    typedef struct {
        const std::string& name;
        const webnn::MLOperand& operand;
    } NamedOutput;

    webnn::MLGraph AwaitBuild(const webnn::MLGraphBuilder& builder,
                             const std::vector<NamedOutput>& outputs);

    typedef struct {
        const std::string& name;
        const webnn::MLInput& input;
    } NamedInput;

    webnn::MLNamedResults AwaitCompute(const webnn::MLGraph& compilation,
                                     const std::vector<NamedInput>& inputs);

    bool CheckShape(const webnn::MLResult& result, const std::vector<int32_t>& expectedShape);

    template <class T>
    bool CheckValue(const webnn::MLResult& result, const std::vector<T>& expectedValue) {
        size_t size = result.BufferSize() / sizeof(T);
        if (size != expectedValue.size()) {
            dawn::ErrorLog() << "The size of output data is expected as " << expectedValue.size()
                             << ", but got " << size;
            return false;
        }
        for (size_t i = 0; i < result.BufferSize() / sizeof(T); ++i) {
            T value = static_cast<const T*>(result.Buffer())[i];
            if (!Expected(value, expectedValue[i])) {
                dawn::ErrorLog() << "The output value at index " << i << " is expected as "
                                 << expectedValue[i] << ", but got " << value;
                return false;
            }
        }
        return true;
    }

    class Async {
      public:
        Async() : mDone(false) {
        }
        ~Async() = default;
        void Wait();
        void Finish();

      private:
        std::condition_variable mCondVar;
        std::mutex mMutex;
        bool mDone;
    };
}  // namespace utils

#endif  // WEBNN_NATIVE_EXAMPLES_SAMPLE_UTILS_H_
