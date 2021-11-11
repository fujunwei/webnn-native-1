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

############################################################
# OBJECT MODEL
############################################################

class Name:
    def __init__(self, name, native=False):
        self.native = native
        self.name = name
        if native:
            self.chunks = [name]
        else:
            self.chunks = name.split(' ')

    def get(self):
        return self.name

    def CamelChunk(self, chunk):
        return chunk[0].upper() + chunk[1:]

    def canonical_case(self):
        return (' '.join(self.chunks)).lower()

    def concatcase(self):
        return ''.join(self.chunks)

    def camelCase(self):
        return self.chunks[0] + ''.join(
            [self.CamelChunk(chunk) for chunk in self.chunks[1:]])

    def CamelCase(self):
        return ''.join([self.CamelChunk(chunk) for chunk in self.chunks])

    def SNAKE_CASE(self):
        return '_'.join([chunk.upper() for chunk in self.chunks])

    def snake_case(self):
        return '_'.join(self.chunks)

    def js_enum_case(self):
        result = self.chunks[0].lower()
        for chunk in self.chunks[1:]:
            if not result[-1].isdigit():
                result += '-'
            result += chunk.lower()
        return result


def concat_names(*names):
    return ' '.join([name.canonical_case() for name in names])


class Type:
    def __init__(self, name, json_data, native=False):
        self.json_data = json_data
        self.dict_name = name
        self.name = Name(name, native=native)
        self.category = json_data['category']


EnumValue = namedtuple('EnumValue', ['name', 'value', 'valid', 'json_data'])


class EnumType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)

        self.values = []
        self.contiguousFromZero = True
        lastValue = -1
        for m in self.json_data['values']:
            if not is_enabled(m):
                continue
            value = m['value']
            if value != lastValue + 1:
                self.contiguousFromZero = False
            lastValue = value
            self.values.append(
                EnumValue(Name(m['name']), value, m.get('valid', True), m))

        # Assert that all values are unique in enums
        all_values = set()
        for value in self.values:
            if value.value in all_values:
                raise Exception("Duplicate value {} in enum {}".format(
                    value.value, name))
            all_values.add(value.value)


BitmaskValue = namedtuple('BitmaskValue', ['name', 'value', 'json_data'])


class BitmaskType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)
        self.values = [
            BitmaskValue(Name(m['name']), m['value'], m)
            for m in self.json_data['values'] if is_enabled(m)
        ]
        self.full_mask = 0
        for value in self.values:
            self.full_mask = self.full_mask | value.value


class CallbackType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)
        self.arguments = []


class TypedefType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data)
        self.type = None


class NativeType(Type):
    def __init__(self, is_enabled, name, json_data):
        Type.__init__(self, name, json_data, native=True)


# Methods and structures are both "records", so record members correspond to
# method arguments or structure members.
class RecordMember:
    def __init__(self,
                 name,
                 typ,
                 annotation,
                 json_data,
                 optional=False,
                 is_return_value=False,
                 default_value=None,
                 skip_serialize=False):
        self.name = name
        self.type = typ
        self.annotation = annotation
        self.json_data = json_data
        self.length = None
        self.optional = optional
        self.is_return_value = is_return_value
        self.handle_type = None
        self.default_value = default_value
        self.skip_serialize = skip_serialize

    def set_handle_type(self, handle_type):
        assert self.type.dict_name == "ObjectHandle"
        self.handle_type = handle_type


Method = namedtuple('Method',
                    ['name', 'return_type', 'arguments', 'json_data'])


class ObjectType(Type):
    def __init__(self, is_enabled, name, json_data):
        json_data_override = {'methods': []}
        if 'methods' in json_data:
            json_data_override['methods'] = [
                m for m in json_data['methods'] if is_enabled(m)
            ]
        Type.__init__(self, name, dict(json_data, **json_data_override))


class Record:
    def __init__(self, name):
        self.name = Name(name)
        self.members = []
        self.may_have_dawn_object = False

    def update_metadata(self):
        def may_have_dawn_object(member):
            if isinstance(member.type, ObjectType):
                return True
            elif isinstance(member.type, StructureType):
                return member.type.may_have_dawn_object
            else:
                return False

        self.may_have_dawn_object = any(
            may_have_dawn_object(member) for member in self.members)

        # Set may_have_dawn_object to true if the type is chained or
        # extensible. Chained structs may contain a Dawn object.
        if isinstance(self, StructureType):
            self.may_have_dawn_object = (self.may_have_dawn_object
                                         or self.chained or self.extensible)


class StructureType(Record, Type):
    def __init__(self, is_enabled, name, json_data):
        Record.__init__(self, name)
        json_data_override = {}
        if 'members' in json_data:
            json_data_override['members'] = [
                m for m in json_data['members'] if is_enabled(m)
            ]
        Type.__init__(self, name, dict(json_data, **json_data_override))
        self.chained = json_data.get("chained", None)
        self.extensible = json_data.get("extensible", None)
        if self.chained:
            assert (self.chained == "in" or self.chained == "out")
        if self.extensible:
            assert (self.extensible == "in" or self.extensible == "out")
        # Chained structs inherit from wgpu::ChainedStruct, which has
        # nextInChain, so setting both extensible and chained would result in
        # two nextInChain members.
        assert not (self.extensible and self.chained)

    @property
    def output(self):
        return self.chained == "out" or self.extensible == "out"


class Command(Record):
    def __init__(self, name, members=None):
        Record.__init__(self, name)
        self.members = members or []
        self.derived_object = None
        self.derived_method = None


def linked_record_members(json_data, types):
    members = []
    members_by_name = {}
    for m in json_data:
        member = RecordMember(Name(m['name']),
                              types[m['type']],
                              m.get('annotation', 'value'),
                              m,
                              optional=m.get('optional', False),
                              is_return_value=m.get('is_return_value', False),
                              default_value=m.get('default', None),
                              skip_serialize=m.get('skip_serialize', False))
        handle_type = m.get('handle_type')
        if handle_type:
            member.set_handle_type(types[handle_type])
        members.append(member)
        members_by_name[member.name.canonical_case()] = member

    for (member, m) in zip(members, json_data):
        if member.annotation != 'value':
            if not 'length' in m:
                if member.type.category != 'object':
                    member.length = "constant"
                    member.constant_length = 1
                else:
                    assert False
            elif m['length'] == 'strlen':
                member.length = 'strlen'
            else:
                member.length = members_by_name[m['length']]

    return members


############################################################
# PARSE
############################################################


def link_object(obj, types):
    def make_method(json_data):
        arguments = linked_record_members(json_data.get('args', []), types)
        return Method(Name(json_data['name']),
                      types[json_data.get('returns',
                                          'void')], arguments, json_data)

    obj.methods = [make_method(m) for m in obj.json_data.get('methods', [])]
    obj.methods.sort(key=lambda method: method.name.canonical_case())


def link_structure(struct, types):
    struct.members = linked_record_members(struct.json_data['members'], types)


def link_callback(callback, types):
    callback.arguments = linked_record_members(callback.json_data['args'],
                                               types)


def link_typedef(typedef, types):
    typedef.type = types[typedef.json_data['type']]


# Sort structures so that if struct A has struct B as a member, then B is
# listed before A.
#
# This is a form of topological sort where we try to keep the order reasonably
# similar to the original order (though the sort isn't technically stable).
#
# It works by computing for each struct type what is the depth of its DAG of
# dependents, then re-sorting based on that depth using Python's stable sort.
# This makes a toposort because if A depends on B then its depth will be bigger
# than B's. It is also nice because all nodes with the same depth are kept in
# the input order.
def topo_sort_structure(structs):
    for struct in structs:
        struct.visited = False
        struct.subdag_depth = 0

    def compute_depth(struct):
        if struct.visited:
            return struct.subdag_depth

        max_dependent_depth = 0
        for member in struct.members:
            if member.type.category == 'structure':
                max_dependent_depth = max(max_dependent_depth,
                                          compute_depth(member.type) + 1)

        struct.subdag_depth = max_dependent_depth
        struct.visited = True
        return struct.subdag_depth

    for struct in structs:
        compute_depth(struct)

    result = sorted(structs, key=lambda struct: struct.subdag_depth)

    for struct in structs:
        del struct.visited
        del struct.subdag_depth

    return result


def parse_json(json, enabled_tags):
    is_enabled = lambda json_data: item_is_enabled(enabled_tags, json_data)
    category_to_parser = {
        'bitmask': BitmaskType,
        'enum': EnumType,
        'native': NativeType,
        'callback': CallbackType,
        'object': ObjectType,
        'structure': StructureType,
        'typedef': TypedefType,
    }

    types = {}

    by_category = {}
    for name in category_to_parser.keys():
        by_category[name] = []

    for (name, json_data) in json.items():
        if name[0] == '_' or not item_is_enabled(enabled_tags, json_data):
            continue
        category = json_data['category']
        parsed = category_to_parser[category](is_enabled, name, json_data)
        by_category[category].append(parsed)
        types[name] = parsed

    for obj in by_category['object']:
        link_object(obj, types)

    for struct in by_category['structure']:
        link_structure(struct, types)

    for callback in by_category['callback']:
        link_callback(callback, types)

    for typedef in by_category['typedef']:
        link_typedef(typedef, types)

    for category in by_category.keys():
        by_category[category] = sorted(
            by_category[category], key=lambda typ: typ.name.canonical_case())

    by_category['structure'] = topo_sort_structure(by_category['structure'])

    for struct in by_category['structure']:
        struct.update_metadata()

    api_params = {
        'types': types,
        'by_category': by_category,
        'enabled_tags': enabled_tags,
    }
    return {
        'types': types,
        'by_category': by_category,
        'enabled_tags': enabled_tags,
        'c_methods': lambda typ: c_methods(api_params, typ),
        'c_methods_sorted_by_name': get_c_methods_sorted_by_name(api_params),
    }


############################################################
# WIRE STUFF
############################################################


# Create wire commands from api methods
def compute_wire_params(api_params, wire_json):
    wire_params = api_params.copy()
    types = wire_params['types']

    commands = []
    return_commands = []

    wire_json['special items']['client_handwritten_commands'] += wire_json[
        'special items']['client_side_commands']

    # Generate commands from object methods
    for api_object in wire_params['by_category']['object']:
        for method in api_object.methods:
            command_name = concat_names(api_object.name, method.name)
            command_suffix = Name(command_name).CamelCase()

            # Only object return values or void are supported.
            # Other methods must be handwritten.
            is_object = method.return_type.category == 'object'
            is_void = method.return_type.name.canonical_case() == 'void'
            if not (is_object or is_void):
                assert command_suffix in (
                    wire_json['special items']['client_handwritten_commands'])
                continue

            if command_suffix in (
                    wire_json['special items']['client_side_commands']):
                continue

            # Create object method commands by prepending "self"
            members = [
                RecordMember(Name('self'), types[api_object.dict_name],
                             'value', {})
            ]
            members += method.arguments

            # Client->Server commands that return an object return the
            # result object handle
            if method.return_type.category == 'object':
                result = RecordMember(Name('result'),
                                      types['ObjectHandle'],
                                      'value', {},
                                      is_return_value=True)
                result.set_handle_type(method.return_type)
                members.append(result)

            command = Command(command_name, members)
            command.derived_object = api_object
            command.derived_method = method
            commands.append(command)

    for (name, json_data) in wire_json['commands'].items():
        commands.append(Command(name, linked_record_members(json_data, types)))

    for (name, json_data) in wire_json['return commands'].items():
        return_commands.append(
            Command(name, linked_record_members(json_data, types)))

    wire_params['cmd_records'] = {
        'command': commands,
        'return command': return_commands
    }

    for commands in wire_params['cmd_records'].values():
        for command in commands:
            command.update_metadata()
        commands.sort(key=lambda c: c.name.canonical_case())

    wire_params.update(wire_json.get('special items', {}))

    return wire_params


#############################################################
# Generator
#############################################################


def as_varName(*names):
    varName = names[0].camelCase() + ''.join(
        [name.CamelCase() for name in names[1:]])
    # Avoid to use C++ keyword 'operator', probably check for others on demand.
    if varName == 'operator':
        varName = 'op'
    return varName


def as_cType(prefix, name):
    if name.native:
        return name.concatcase()
    else:
        return prefix + name.CamelCase()


def as_cTypeDawn(name):
    if name.native:
        return name.concatcase()
    else:
        return 'Dawn' + name.CamelCase()


def as_cTypeEnumSpecialCase(prefix, typ):
    if typ.category == 'bitmask':
        return as_cType(prefix, typ.name) + 'Flags'
    return as_cType(prefix, typ.name)


def as_cppType(name):
    if name.native:
        return name.concatcase()
    else:
        return name.CamelCase()


def as_jsEnumValue(value):
    if 'jsrepr' in value.json_data: return value.json_data['jsrepr']
    return "'" + value.name.js_enum_case() + "'"


def convert_cType_to_cppType(typ, annotation, arg, indent=0):
    if typ.category == 'native':
        return arg
    if annotation == 'value':
        if typ.category == 'object':
            return '{}::Acquire({})'.format(as_cppType(typ.name), arg)
        elif typ.category == 'structure':
            converted_members = [
                convert_cType_to_cppType(
                    member.type, member.annotation,
                    '{}.{}'.format(arg, as_varName(member.name)), indent + 1)
                for member in typ.members
            ]

            converted_members = [(' ' * 4) + m for m in converted_members]
            converted_members = ',\n'.join(converted_members)

            return as_cppType(typ.name) + ' {\n' + converted_members + '\n}'
        else:
            return 'static_cast<{}>({})'.format(as_cppType(typ.name), arg)
    else:
        return 'reinterpret_cast<{} {}>({})'.format(as_cppType(typ.name),
                                                    annotation, arg)


def decorate(name, typ, arg):
    if arg.annotation == 'value':
        return typ + ' ' + name
    elif arg.annotation == '*':
        return typ + ' * ' + name
    elif arg.annotation == 'const*':
        return typ + ' const * ' + name
    elif arg.annotation == 'const*const*':
        return 'const ' + typ + '* const * ' + name
    else:
        assert False


def annotated(typ, arg):
    name = as_varName(arg.name)
    return decorate(name, typ, arg)


def item_is_enabled(enabled_tags, json_data):
    tags = json_data.get('tags')
    if tags is None: return True
    return any(tag in enabled_tags for tag in tags)


def as_cEnum(prefix, type_name, value_name):
    assert not type_name.native and not value_name.native
    return prefix + type_name.CamelCase() + '_' + value_name.CamelCase()


def as_cEnumDawn(type_name, value_name):
    assert not type_name.native and not value_name.native
    return ('DAWN' + '_' + type_name.SNAKE_CASE() + '_' +
            value_name.SNAKE_CASE())


def as_cppEnum(value_name):
    assert not value_name.native
    if value_name.concatcase()[0].isdigit():
        return "e" + value_name.CamelCase()
    return value_name.CamelCase()


def as_cMethod(prefix, type_name, method_name):
    assert not type_name.native and not method_name.native
    return prefix + type_name.CamelCase() + method_name.CamelCase()


def as_cMethodDawn(type_name, method_name):
    assert not type_name.native and not method_name.native
    return 'dawn' + type_name.CamelCase() + method_name.CamelCase()


def as_MethodSuffix(type_name, method_name):
    assert not type_name.native and not method_name.native
    return type_name.CamelCase() + method_name.CamelCase()


def as_cProc(prefix, type_name, method_name):
    assert not type_name.native and not method_name.native
    return prefix + 'Proc' + type_name.CamelCase() + method_name.CamelCase()


def as_cProcDawn(type_name, method_name):
    assert not type_name.native and not method_name.native
    return 'Dawn' + 'Proc' + type_name.CamelCase() + method_name.CamelCase()


def as_frontendType(prefix, typ):
    if typ.category == 'object':
        return typ.name.CamelCase() + 'Base*'
    elif typ.category in ['bitmask', 'enum']:
        return prefix.lower() + '::' + typ.name.CamelCase()
    elif typ.category == 'structure':
        return as_cppType(typ.name)
    else:
        return as_cType(prefix.upper(), typ.name)


def as_wireType(prefix, typ):
    if typ.category == 'object':
        return typ.name.CamelCase() + '*'
    elif typ.category in ['bitmask', 'enum', 'structure']:
        return prefix + typ.name.CamelCase()
    else:
        return as_cppType(typ.name)


def c_methods(params, typ):
    return typ.methods + [
        x for x in [
            Method(Name('reference'), params['types']['void'], [],
                   {'tags': ['dawn', 'emscripten']}),
            Method(Name('release'), params['types']['void'], [],
                   {'tags': ['dawn', 'emscripten']}),
        ] if item_is_enabled(params['enabled_tags'], x.json_data)
    ]


def get_c_methods_sorted_by_name(api_params):
    unsorted = [(as_MethodSuffix(typ.name, method.name), typ, method) \
            for typ in api_params['by_category']['object'] \
            for method in c_methods(api_params, typ) ]
    return [(typ, method) for (_, typ, method) in sorted(unsorted)]


def has_callback_arguments(method):
    return any(arg.type.category == 'callback' for arg in method.arguments)

def get_render_params(prefix):
    def as_annotated_cType(arg):
        return annotated(as_cTypeEnumSpecialCase(prefix.upper(), arg.type), arg)
    def as_annotated_cppType(arg):
        return annotated(as_cppType(arg.type.name), arg)
    def as_cPrefixEnum(type_name, value_name):
        return as_cEnum(prefix.upper(), type_name, value_name)
    def as_cPrefixMethod(type_name, method_name):
        return as_cMethod(prefix.lower(), type_name, method_name)
    def as_cPrefixProc(type_name, method_name):
        return as_cProc(prefix.upper(), type_name, method_name)
    def as_cPrefixType(name):
        return as_cType(prefix.upper(), name)
    return {
            'Name': lambda name: Name(name),
            'as_annotated_cType': as_annotated_cType,
            'as_annotated_cppType': as_annotated_cppType,
            'as_cEnum': as_cPrefixEnum,
            'as_cEnumDawn': as_cEnumDawn,
            'as_cppEnum': as_cppEnum,
            'as_cMethod': as_cPrefixMethod,
            'as_cMethodDawn': as_cMethodDawn,
            'as_MethodSuffix': as_MethodSuffix,
            'as_cProc': as_cPrefixProc,
            'as_cProcDawn': as_cProcDawn,
            'as_cType': as_cPrefixType,
            'as_cTypeDawn': as_cTypeDawn,
            'as_cppType': as_cppType,
            'as_jsEnumValue': as_jsEnumValue,
            'convert_cType_to_cppType': convert_cType_to_cppType,
            'as_varName': as_varName,
            'decorate': decorate,
        }
