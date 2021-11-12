//* Copyright 2020 The Dawn Authors
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
//* This template itself is part of the Dawn source and generates export header
//* file such as webgpu.h and webnn.h.

{% macro render_c_enum_type() %}
    {% for type in by_category["object"] %}
        typedef struct {{as_cType(type.name)}}Impl* {{as_cType(type.name)}};
    {% endfor %}

    {% for type in by_category["enum"] + by_category["bitmask"] %}
        typedef enum {{as_cType(type.name)}} {
            {% for value in type.values %}
                {{as_cEnum(type.name, value.name)}} = 0x{{format(value.value, "08X")}},
            {% endfor %}
            {{as_cEnum(type.name, Name("force32"))}} = 0x7FFFFFFF
        } {{as_cType(type.name)}};
        {% if type.category == "bitmask" %}
            typedef {{ get_namespace() }}Flags {{as_cType(type.name)}}Flags;
        {% endif %}

    {% endfor -%}
{%- endmacro %}

{% macro render_c_structure_type() %}
    {% set chained_struct = get_namespace() + 'ChainedStruct' %}
    {% set s_type = get_namespace() + 'SType' %}
    {% set chained_struct_out = get_namespace() + 'ChainedStructOut' %}
    typedef struct {{ chained_struct }} {
        struct {{ chained_struct }} const * next;
        {{ s_type }} sType;
    } {{ chained_struct }};

    typedef struct {{ chained_struct_out }} {
        struct {{ chained_struct_out }} * next;
        {{ s_type }} sType;
    } {{ chained_struct_out }};

    {% for type in by_category["structure"] %}
        typedef struct {{as_cType(type.name)}} {
            {% set Out = "Out" if type.output else "" %}
            {% set const = "const " if not type.output else "" %}
                {% if type.extensible %}
                    {{chained_struct}}{{Out}} {{const}}* nextInChain;
                {% endif %}
                {% if type.chained %}
                    {{chained_struct}}{{Out}} chain;
                {% endif %}
            {% for member in type.members %}
                {{as_annotated_cType(member)}};
            {% endfor %}
        } {{as_cType(type.name)}};

    {% endfor %}

    {% for typeDef in by_category["typedef"] %}
        // {{as_cType(typeDef.name)}} is deprecated.
        // Use {{as_cType(typeDef.type.name)}} instead.
        typedef {{as_cType(typeDef.type.name)}} {{as_cType(typeDef.name)}};

    {% endfor %}
{%- endmacro %}

{% macro render_c_callback_type() %}
    {% for type in by_category["callback"] %}
        typedef void (*{{as_cType(type.name)}})(
            {%- for arg in type.arguments -%}
                {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        );
    {% endfor %}
{%- endmacro %}

{% macro render_c_method_for_proc_table() %}
    {% for type in by_category["object"] if len(c_methods(type)) > 0 %}
        // Procs of {{type.name.CamelCase()}}
        {% for method in c_methods(type) %}
            typedef {{as_cType(method.return_type.name)}} (*{{as_cProc(type.name, method.name)}})(
                {{-as_cType(type.name)}} {{as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            );
        {% endfor %}

    {% endfor %}
{%- endmacro %}

{% macro render_c_method_for_exporting() %}
    {% for type in by_category["object"] if len(c_methods(type)) > 0 %}
        // Methods of {{type.name.CamelCase()}}
        {% for method in c_methods(type) %}
            {{ get_namespace() }}_EXPORT {{as_cType(method.return_type.name)}} {{as_cMethod(type.name, method.name)}}(
                {{-as_cType(type.name)}} {{as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            );
        {% endfor %}

    {% endfor %}
{%- endmacro %}

{% block content %}
{% endblock %}

