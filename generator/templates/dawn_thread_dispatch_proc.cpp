{% set Prefix = metadata.proc_table_prefix %}
{% set prefix = Prefix.lower() %}
#include "{{prefix}}/{{prefix}}_thread_dispatch_proc.h"

#include <thread>

static {{Prefix}}ProcTable nullProcs;
thread_local {{Prefix}}ProcTable perThreadProcs;

void {{prefix}}ProcSetPerThreadProcs(const {{Prefix}}ProcTable* procs) {
    if (procs) {
        perThreadProcs = *procs;
    } else {
        perThreadProcs = nullProcs;
    }
}

{% for function in by_category["function"] %}
    static {{as_cType(function.return_type.name)}} ThreadDispatch{{as_cppType(function.name)}}(
        {%- for arg in function.arguments -%}
            {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
        {%- endfor -%}
    ) {
        {% if function.return_type.name.canonical_case() != "void" %}return {% endif %}
        perThreadProcs.{{as_varName(function.name)}}(
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif %}{{as_varName(arg.name)}}
            {%- endfor -%}
        );
    }
{% endfor %}

{% for type in by_category["object"] %}
    {% for method in c_methods(type) %}
        static {{as_cType(method.return_type.name)}} ThreadDispatch{{as_MethodSuffix(type.name, method.name)}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            {% if method.return_type.name.canonical_case() != "void" %}return {% endif %}
            perThreadProcs.{{as_varName(type.name, method.name)}}({{as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_varName(arg.name)}}
                {%- endfor -%}
            );
        }
    {% endfor %}
{% endfor %}

extern "C" {
    {{Prefix}}ProcTable {{prefix}}ThreadDispatchProcTable = {
        {% for function in by_category["function"] %}
            ThreadDispatch{{as_cppType(function.name)}},
        {% endfor %}
        {% for type in by_category["object"] %}
            {% for method in c_methods(type) %}
                ThreadDispatch{{as_MethodSuffix(type.name, method.name)}},
            {% endfor %}
        {% endfor %}
    };
}
