// Copyright 2018 The Dawn Authors
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

#include "webnn_native/WebnnNative.h"

#include <memory>

#include "common/Assert.h"
#include "webnn_native/Context.h"
#include "webnn_native/Instance.h"

#if defined(_WIN32)
#    include <crtdbg.h>
#endif

// Contains the entry-points into webnn_native
namespace webnn_native {

    namespace {

        void DumpMemoryLeaks() {
#if defined(_WIN32) && defined(_DEBUG)
            // Send all reports to STDOUT.
            // _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            // _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
            // _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
            // _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
            // _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
            // _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
            // // Perform automatic leak checking at program exit through a call to _CrtDumpMemoryLeaks
            // // and generate an error report if the application failed to free all the memory it
            // // allocated.
            // _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
        }
    }  // namespace

    // Instance

    Instance::Instance() : mImpl(InstanceBase::Create()) {
    }

    Instance::~Instance() {
        if (mImpl != nullptr) {
            mImpl->Release();
            mImpl = nullptr;
        }
    }

    MLContext Instance::CreateTestContext(const ml::ContextOptions* options) {
        return reinterpret_cast<MLContext>(
            mImpl->CreateTestContext(reinterpret_cast<const ContextOptions*>(options)));
    }

    MLContext Instance::CreateContext(const ml::ContextOptions* options) {
        return reinterpret_cast<MLContext>(
            mImpl->APICreateContext(reinterpret_cast<const ContextOptions*>(options)));
    }

    MLInstance Instance::Get() const {
        return reinterpret_cast<MLInstanceImpl*>(mImpl);
    }

    const WebnnProcTable& GetProcsAutogen();

    const WebnnProcTable& GetProcs() {
        DumpMemoryLeaks();
        return GetProcsAutogen();
    }

}  // namespace webnn_native
