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

    static_assert(sizeof(ChainedStruct) == sizeof({{ get_namespace() }}ChainedStruct),
            "sizeof mismatch for ChainedStruct");
    static_assert(alignof(ChainedStruct) == alignof({{ get_namespace() }}ChainedStruct),
            "alignof mismatch for ChainedStruct");
    static_assert(offsetof(ChainedStruct, nextInChain) == offsetof({{ get_namespace() }}ChainedStruct, next),
            "offsetof mismatch for ChainedStruct::nextInChain");
    static_assert(offsetof(ChainedStruct, sType) == offsetof({{ get_namespace() }}ChainedStruct, sType),
            "offsetof mismatch for ChainedStruct::sType");

    {% for type in by_category["structure"] %}
        {% set CppType = as_cppType(type.name) %}
        {% set CType = as_cType(type.name) %}

        static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
        static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

        {% if type.extensible %}
            static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, nextInChain),
                    "offsetof mismatch for {{CppType}}::nextInChain");
        {% endif %}
        {% for member in type.members %}
            {% set memberName = member.name.camelCase() %}
            static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                    "offsetof mismatch for {{CppType}}::{{memberName}}");
        {% endfor %}

        bool {{CppType}}::operator==(const {{as_cppType(type.name)}}& rhs) const {
            return {% if type.extensible or type.chained -%}
                (nextInChain == rhs.nextInChain) &&
            {%- endif %} std::tie(
                {% for member in type.members %}
                    {{member.name.camelCase()-}}
                    {{ "," if not loop.last else "" }}
                {% endfor %}
            ) == std::tie(
                {% for member in type.members %}
                    rhs.{{member.name.camelCase()-}}
                    {{ "," if not loop.last else "" }}
                {% endfor %}
            );
        }

    {% endfor %}

{% block content %}
{% endblock %}
