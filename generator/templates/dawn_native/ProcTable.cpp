//* Copyright 2017 The Dawn Authors
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

{% set Prefix = metadata.proc_table_prefix %}
{% set prefix = Prefix.lower() %}
{% set native_namespace = Name(metadata.native_namespace).snake_case() %}
{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set native_dir = impl_dir + native_namespace %}
#include "{{native_dir}}/{{prefix}}_platform.h"
#include "{{native_dir}}/{{Prefix}}Native.h"

#include <algorithm>
#include <vector>

{% for type in by_category["object"] %}
    {% if type.name.canonical_case() not in ["texture view"] %}
        #include "{{native_dir}}/{{type.name.CamelCase()}}.h"
    {% endif %}
{% endfor %}

namespace {{native_namespace}} {

    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            {% set suffix = as_MethodSuffix(type.name, method.name) %}

            {{as_cType(method.return_type.name)}} Native{{suffix}}(
                {{-as_cType(type.name)}} cSelf
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                //* Perform conversion between C types and frontend types
                auto self = FromAPI(cSelf);

                {% for arg in method.arguments %}
                    {% set varName = as_varName(arg.name) %}
                    {% if arg.type.category in ["enum", "bitmask"] and arg.annotation == "value" %}
                        auto {{varName}}_ = static_cast<{{as_frontendType(arg.type)}}>({{varName}});
                    {% elif arg.annotation != "value" or arg.type.category == "object" %}
                        auto {{varName}}_ = reinterpret_cast<{{decorate("", as_frontendType(arg.type), arg)}}>({{varName}});
                    {% else %}
                        auto {{varName}}_ = {{as_varName(arg.name)}};
                    {% endif %}
                {%- endfor-%}

                {% if method.return_type.name.canonical_case() != "void" %}
                    auto result =
                {%- endif %}
                self->API{{method.name.CamelCase()}}(
                    {%- for arg in method.arguments -%}
                        {%- if not loop.first %}, {% endif -%}
                        {{as_varName(arg.name)}}_
                    {%- endfor -%}
                );
                {% if method.return_type.name.canonical_case() != "void" %}
                    {% if method.return_type.category == "object" %}
                        return ToAPI(result);
                    {% else %}
                        return result;
                    {% endif %}
                {% endif %}
            }
        {% endfor %}
    {% endfor %}

    namespace {

{% set c_prefix = metadata.c_prefix %}
        struct ProcEntry {
            {{c_prefix}}Proc proc;
            const char* name;
        };
        static const ProcEntry sProcMap[] = {
            {% for (type, method) in c_methods_sorted_by_name %}
                { reinterpret_cast<{{c_prefix}}Proc>(Native{{as_MethodSuffix(type.name, method.name)}}), "{{as_cMethod(type.name, method.name)}}" },
            {% endfor %}
        };
        static constexpr size_t sProcMapSize = sizeof(sProcMap) / sizeof(sProcMap[0]);

    }  // anonymous namespace

    {% for function in by_category["function"] %}
        {{as_cType(function.return_type.name)}} Native{{as_cppType(function.name)}}(
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            return ToAPI({{as_cppType(function.return_type.name)}}Base::Create(
                {%- for arg in function.arguments -%}
                FromAPI({% if not loop.first %}, {% endif %}{{as_varName(arg.name)}})
                {%- endfor -%}
            ));
        }
    {% endfor %}

    {{c_prefix}}Proc NativeGetProcAddress(const char* procName) {
        if (procName == nullptr) {
            return nullptr;
        }

        const ProcEntry* entry = std::lower_bound(&sProcMap[0], &sProcMap[sProcMapSize], procName,
            [](const ProcEntry &a, const char *b) -> bool {
                return strcmp(a.name, b) < 0;
            }
        );

        if (entry != &sProcMap[sProcMapSize] && strcmp(entry->name, procName) == 0) {
            return entry->proc;
        }

        // Special case the free-standing functions of the API.
        if (strcmp(procName, "{{as_cMethod(None, Name("GetProcAddress"))}}") == 0) {
            return reinterpret_cast<{{c_prefix}}Proc>(NativeGetProcAddress);
        }

        {% for function in by_category["function"] %}
            if (strcmp(procName, "{{metadata.namespace}}{{as_cppType(function.name)}}") == 0) {
                return reinterpret_cast<{{c_prefix}}Proc>(Native{{as_cppType(function.name)}});
            }

        {% endfor %}

        return nullptr;
    }

    std::vector<const char*> GetProcMapNamesForTestingInternal() {
        std::vector<const char*> result;
        result.reserve(sProcMapSize);
        for (const ProcEntry& entry : sProcMap) {
            result.push_back(entry.name);
        }
        return result;
    }

    static {{Prefix}}ProcTable gProcTable = {
        NativeGetProcAddress,
        {% for function in by_category["function"] %}
            Native{{as_cppType(function.name)}},
        {% endfor %}
        {% for type in by_category["object"] %}
            {% for method in c_methods(type) %}
                Native{{as_MethodSuffix(type.name, method.name)}},
            {% endfor %}
        {% endfor %}
    };

    const {{Prefix}}ProcTable& GetProcsAutogen() {
        return gProcTable;
    }
}
