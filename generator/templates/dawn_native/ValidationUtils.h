//* Copyright 2018 The Dawn Authors
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

#ifndef BACKEND_VALIDATIONUTILS_H_
#define BACKEND_VALIDATIONUTILS_H_

{% set api = metadata.api.lower() %}
#include "dawn/{{api}}_cpp.h"

{% set native_namespace = Name(metadata.native_namespace).snake_case() %}

{% set native_dir = native_namespace %}
#include "{{native_dir}}/Error.h"

namespace {{native_namespace}} {

    // Helper functions to check the value of enums and bitmasks
    {% for type in by_category["enum"] + by_category["bitmask"] %}
        {% set namespace = metadata.namespace %}
        MaybeError Validate{{type.name.CamelCase()}}({{namespace}}::{{as_cppType(type.name)}} value);
    {% endfor %}

} // namespace {{native_namespace}}

#endif  // BACKEND_VALIDATIONUTILS_H_
