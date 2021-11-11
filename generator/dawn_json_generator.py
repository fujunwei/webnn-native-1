#!/usr/bin/env python3
# Copyright 2017 The Dawn Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import json, os, sys
from collections import namedtuple

from generator_lib import Generator, run_generator, FileRender
from generator_utils import get_render_params, parse_json, \
                            has_callback_arguments, annotated, as_frontendType,\
                            compute_wire_params, as_wireType

class MultiGeneratorFromDawnJSON(Generator):
    def get_description(self):
        return 'Generates code for various target from Dawn.json.'

    def add_commandline_arguments(self, parser):
        allowed_targets = [
            'dawn_headers', 'dawncpp_headers', 'dawncpp', 'dawn_proc',
            'mock_webgpu', 'dawn_wire', "dawn_native_utils"
        ]

        parser.add_argument('--dawn-json',
                            required=True,
                            type=str,
                            help='The DAWN JSON definition to use.')
        parser.add_argument('--wire-json',
                            default=None,
                            type=str,
                            help='The DAWN WIRE JSON definition to use.')
        parser.add_argument(
            '--targets',
            required=True,
            type=str,
            help=
            'Comma-separated subset of targets to output. Available targets: '
            + ', '.join(allowed_targets))

    def get_file_renders(self, args):
        with open(args.dawn_json) as f:
            loaded_json = json.loads(f.read())

        targets = args.targets.split(',')

        wire_json = None
        if args.wire_json:
            with open(args.wire_json) as f:
                wire_json = json.loads(f.read())

        renders = []

        RENDER_PARAMS_BASE = get_render_params('WGPU')

        params_dawn = parse_json(loaded_json,
                                 enabled_tags=['dawn', 'native', 'deprecated'])

        if 'dawn_headers' in targets:
            renders.append(
                FileRender('webgpu.h', 'src/include/dawn/webgpu.h',
                           [RENDER_PARAMS_BASE, params_dawn]))
            renders.append(
                FileRender('dawn_proc_table.h',
                           'src/include/dawn/dawn_proc_table.h',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'dawncpp_headers' in targets:
            renders.append(
                FileRender('webgpu_cpp.h', 'src/include/dawn/webgpu_cpp.h',
                           [RENDER_PARAMS_BASE, params_dawn]))

            renders.append(
                FileRender('webgpu_cpp_print.h',
                           'src/include/dawn/webgpu_cpp_print.h',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'dawn_proc' in targets:
            renders.append(
                FileRender('dawn_proc.c', 'src/dawn/dawn_proc.c',
                           [RENDER_PARAMS_BASE, params_dawn]))
            renders.append(
                FileRender('dawn_thread_dispatch_proc.cpp',
                           'src/dawn/dawn_thread_dispatch_proc.cpp',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'dawncpp' in targets:
            renders.append(
                FileRender('webgpu_cpp.cpp', 'src/dawn/webgpu_cpp.cpp',
                           [RENDER_PARAMS_BASE, params_dawn]))

        if 'webgpu_headers' in targets:
            params_upstream = parse_json(loaded_json,
                                         enabled_tags=['upstream', 'native'])
            renders.append(
                FileRender('webgpu.h', 'webgpu-headers/webgpu.h',
                           [RENDER_PARAMS_BASE, params_upstream]))

        if 'emscripten_bits' in targets:
            params_emscripten = parse_json(
                loaded_json, enabled_tags=['upstream', 'emscripten'])
            renders.append(
                FileRender('webgpu.h', 'emscripten-bits/webgpu.h',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('webgpu_cpp.h', 'emscripten-bits/webgpu_cpp.h',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('webgpu_cpp.cpp', 'emscripten-bits/webgpu_cpp.cpp',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('webgpu_struct_info.json',
                           'emscripten-bits/webgpu_struct_info.json',
                           [RENDER_PARAMS_BASE, params_emscripten]))
            renders.append(
                FileRender('library_webgpu_enum_tables.js',
                           'emscripten-bits/library_webgpu_enum_tables.js',
                           [RENDER_PARAMS_BASE, params_emscripten]))

        if 'mock_webgpu' in targets:
            mock_params = [
                RENDER_PARAMS_BASE, params_dawn, {
                    'has_callback_arguments': has_callback_arguments
                }
            ]
            renders.append(
                FileRender('mock_webgpu.h', 'src/dawn/mock_webgpu.h',
                           mock_params))
            renders.append(
                FileRender('mock_webgpu.cpp', 'src/dawn/mock_webgpu.cpp',
                           mock_params))

        if 'dawn_native_utils' in targets:
            frontend_params = [
                RENDER_PARAMS_BASE,
                params_dawn,
                {
                    # TODO: as_frontendType and co. take a Type, not a Name :(
                    'as_frontendType': lambda typ: as_frontendType('WGPU', typ),
                    'as_annotated_frontendType': \
                        lambda arg: annotated(as_frontendType('WGPU', arg.type), arg),
                }
            ]

            renders.append(
                FileRender('dawn_native/ValidationUtils.h',
                           'src/dawn_native/ValidationUtils_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/ValidationUtils.cpp',
                           'src/dawn_native/ValidationUtils_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/dawn_platform.h',
                           'src/dawn_native/dawn_platform_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/wgpu_structs.h',
                           'src/dawn_native/wgpu_structs_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/wgpu_structs.cpp',
                           'src/dawn_native/wgpu_structs_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/ProcTable.cpp',
                           'src/dawn_native/ProcTable.cpp', frontend_params))
            renders.append(
                FileRender('dawn_native/ChainUtils.h',
                           'src/dawn_native/ChainUtils_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/ChainUtils.cpp',
                           'src/dawn_native/ChainUtils_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/webgpu_absl_format.h',
                           'src/dawn_native/webgpu_absl_format_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/webgpu_absl_format.cpp',
                           'src/dawn_native/webgpu_absl_format_autogen.cpp',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/ObjectType.h',
                           'src/dawn_native/ObjectType_autogen.h',
                           frontend_params))
            renders.append(
                FileRender('dawn_native/ObjectType.cpp',
                           'src/dawn_native/ObjectType_autogen.cpp',
                           frontend_params))

        if 'dawn_wire' in targets:
            additional_params = compute_wire_params(params_dawn, wire_json)

            wire_params = [
                RENDER_PARAMS_BASE, params_dawn, {
                    'as_wireType': lambda type : as_wireType('WGPU', type),
                    'as_annotated_wireType': \
                        lambda arg: annotated(as_wireType('WGPU', arg.type), arg),
                }, additional_params
            ]
            renders.append(
                FileRender('dawn_wire/ObjectType.h',
                           'src/dawn_wire/ObjectType_autogen.h', wire_params))
            renders.append(
                FileRender('dawn_wire/WireCmd.h',
                           'src/dawn_wire/WireCmd_autogen.h', wire_params))
            renders.append(
                FileRender('dawn_wire/WireCmd.cpp',
                           'src/dawn_wire/WireCmd_autogen.cpp', wire_params))
            renders.append(
                FileRender('dawn_wire/client/ApiObjects.h',
                           'src/dawn_wire/client/ApiObjects_autogen.h',
                           wire_params))
            renders.append(
                FileRender('dawn_wire/client/ApiProcs.cpp',
                           'src/dawn_wire/client/ApiProcs_autogen.cpp',
                           wire_params))
            renders.append(
                FileRender('dawn_wire/client/ClientBase.h',
                           'src/dawn_wire/client/ClientBase_autogen.h',
                           wire_params))
            renders.append(
                FileRender('dawn_wire/client/ClientHandlers.cpp',
                           'src/dawn_wire/client/ClientHandlers_autogen.cpp',
                           wire_params))
            renders.append(
                FileRender(
                    'dawn_wire/client/ClientPrototypes.inc',
                    'src/dawn_wire/client/ClientPrototypes_autogen.inc',
                    wire_params))
            renders.append(
                FileRender('dawn_wire/server/ServerBase.h',
                           'src/dawn_wire/server/ServerBase_autogen.h',
                           wire_params))
            renders.append(
                FileRender('dawn_wire/server/ServerDoers.cpp',
                           'src/dawn_wire/server/ServerDoers_autogen.cpp',
                           wire_params))
            renders.append(
                FileRender('dawn_wire/server/ServerHandlers.cpp',
                           'src/dawn_wire/server/ServerHandlers_autogen.cpp',
                           wire_params))
            renders.append(
                FileRender(
                    'dawn_wire/server/ServerPrototypes.inc',
                    'src/dawn_wire/server/ServerPrototypes_autogen.inc',
                    wire_params))

        return renders

    def get_dependencies(self, args):
        deps = [os.path.abspath(args.dawn_json)]
        if args.wire_json != None:
            deps += [os.path.abspath(args.wire_json)]
        return deps


if __name__ == '__main__':
    sys.exit(run_generator(MultiGeneratorFromDawnJSON()))
