//* Copyright 2020 The Dawn Authors
//* Copyright 2021 The WebNN-native Authors
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at
//*
//*     http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.
//*
//*
//* This template itself is part of the Dawn source and follows Dawn's license
//* but the generated file is used for "WebGPU native". The template comments
//* using //* at the top of the file are removed during generation such that
//* the resulting file starts with the BSD 3-Clause comment.
//*
//*
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

#ifndef WEBNN_H_
#define WEBNN_H_

#if defined(WEBNN_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(WEBNN_IMPLEMENTATION)
#            define ML_EXPORT __declspec(dllexport)
#        else
#            define ML_EXPORT __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined(WEBNN_IMPLEMENTATION)
#            define ML_EXPORT __attribute__((visibility("default")))
#        else
#            define ML_EXPORT
#        endif
#    endif  // defined(_WIN32)
#else       // defined(WEBNN_SHARED_LIBRARY)
#    define ML_EXPORT
#endif  // defined(WEBNN_SHARED_LIBRARY)

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint32_t MLFlags;

typedef enum MLSType {
  MLSType_Force32 = 0x7FFFFFFF
} MLSType;

{% extends '../../templates/base/dawn.h' %}

{% block content %}
{{ render_c_enum_type() }}

{{ render_c_structure_type() }}

#ifdef __cplusplus
extern "C" {
#endif

{{ render_c_callback_type() }}

typedef void (*WebnnProc)(void);

#if !defined(WEBNN_SKIP_PROCS)

typedef MLGraphBuilder (*WebnnProcCreateGraphBuilder)(MLContext context);
typedef MLNamedInputs (*WebnnProcCreateNamedInputs)();
typedef MLNamedOperands (*WebnnProcCreateNamedOperands)();
typedef MLNamedOutputs (*WebnnProcCreateNamedOutputs)();
typedef MLOperatorArray (*WebnnProcCreateOperatorArray)();

{{ render_c_method_for_proc_table() }}

#endif  // !defined(WEBNN_SKIP_PROCS)

#if !defined(WEBNN_SKIP_DECLARATIONS)

ML_EXPORT MLGraphBuilder webnnCreateGraphBuilder(MLContext context);
ML_EXPORT MLNamedInputs webnnCreateNamedInputs();
ML_EXPORT MLNamedOperands webnnCreateNamedOperands();
ML_EXPORT MLNamedOutputs webnnCreateNamedOutputs();
ML_EXPORT MLOperatorArray webnnCreateOperatorArray();

{{ render_c_method_for_exporting() }}
#endif  // !defined(WEBNN_SKIP_DECLARATIONS)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WEBNN_H_

{% endblock %}
