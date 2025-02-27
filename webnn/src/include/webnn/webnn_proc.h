// Copyright 2019 The Dawn Authors
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

#ifndef WEBNN_WEBNN_PROC_H_
#define WEBNN_WEBNN_PROC_H_

#include "webnn/webnn.h"
#include "dawn/webnn_proc_table.h"

#ifdef __cplusplus
extern "C" {
#endif

// Sets the static proctable used by libdawn_proc to implement the Dawn entrypoints. Passing NULL
// for `procs` sets up the null proctable that contains only null function pointers. It is the
// default value of the proctable. Setting the proctable back to null is good practice when you
// are done using libdawn_proc since further usage will cause a segfault instead of calling an
// unexpected function.
ML_EXPORT void webnnProcSetProcs(const WebnnProcTable* procs);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // WEBNN_WEBNN_PROC_H_
