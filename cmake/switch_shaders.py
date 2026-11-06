#!/usr/bin/env python3

import argparse
from pathlib import Path
import re
import subprocess
import struct

PROGRAMS = ('screen', 'dysh', 'glob', 'celborder', 'geom', 'blot', 'blip', 'imgui')
UNIFORM = re.compile(r'\buniform\s+(\w+)\s+(\w+)\s*(?:\[\s*(\w+)\s*\])?\s*;')
STRUCT = re.compile(r'\bstruct\s+(\w+)\s*\{([^}]+)\}\s*;', re.S)
FIELD = re.compile(r'\b(\w+)\s+(\w+)\s*(?:\[\s*(\w+)\s*\])?\s*;')

def program_sources(root, name):
    native = root / 'platform/switch/deko/shaders'
    game = root / 'Sly1-Release'
    directory = native if name == 'imgui' else game
    paths = tuple(directory / f'{name}.{stage}' for stage in ('vert', 'frag'))
    return [path.read_text(encoding='utf-8-sig') for path in paths]

def align(value, alignment):
    return (value + alignment - 1) // alignment * alignment

def strip_comments(source):
    return re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)

def native_source(source):
    output, stack = [], []
    active = True
    for line in source.splitlines(keepends=True):
        directive = re.match(r'\s*#\s*(ifdef|ifndef|if|else|endif)\b(.*)', line)
        if directive:
            command, value = directive.groups()
            if command in ('ifdef', 'ifndef', 'if'):
                native = command != 'if' and value.strip() == 'DEKO3D'
                stack.append((active, native))
                if native:
                    active = active and command == 'ifdef'
                    continue
            elif command == 'else' and stack:
                parent, native = stack[-1]
                if native:
                    active = parent and not active
                    continue
            elif command == 'endif' and stack:
                active, native = stack.pop()
                if native:
                    continue
        if active:
            output.append(line)
    return ''.join(output)

def resource_masks(source):
    uniform = storage = 0
    blocks = re.compile(r'layout\s*\(([^)]*)\)\s*'
                        r'(?:(?:readonly|writeonly|coherent|volatile|restrict)\s+)*'
                        r'(uniform|buffer)\s+\w+\s*\{', re.S)
    for qualifiers, kind in blocks.findall(strip_comments(source)):
        binding = re.search(r'\bbinding\s*=\s*(\d+)\b', qualifiers)
        if not binding:
            raise ValueError('Buffer block without an explicit binding')
        slot = int(binding.group(1))
        if slot >= 16:
            raise ValueError(f'Buffer block exceeds native slot limit: {slot}')
        if kind == 'uniform': uniform |= 1 << slot
        else: storage |= 1 << slot
    return dict(uniform_mask=uniform, storage_mask=storage)

def layout_type(kind, structs):
    if kind in ('float', 'int', 'uint', 'bool'):
        return 4, 4, 4
    if re.fullmatch(r'[iub]?vec[234]', kind):
        n = int(kind[-1])
        return (8 if n == 2 else 16), n * 4, n * 4
    if kind in ('mat3', 'mat4'):
        return 16, int(kind[-1]) * 16, 16
    if kind in structs:
        fields = struct_layout(structs[kind], structs)
        return 16, align(max(f['offset'] + f['size'] for f in fields), 16), 0
    raise ValueError(f'Unsupported uniform type: {kind}')

def struct_layout(fields, structs):
    offset = 0
    result = []
    for kind, name, count in fields:
        alignment, size, stride = layout_type(kind, structs)
        if count > 1:
            alignment = align(alignment, 16)
            stride = align(size, 16)
            size = stride * count
        offset = align(offset, alignment)
        result.append(dict(type=kind, name=name, count=count, offset=offset,
                           size=size, stride=stride))
        offset += size
    return result

def program_layout(sources):
    combined = strip_comments('\n'.join(native_source(source) for source in sources))
    defines = dict(re.findall(r'^\s*#define\s+(\w+)\s+(\d+)\s*$', combined, re.M))
    count_of = lambda token: int(defines.get(token, token)) if token else 1
    structs = {name: [(k, n, count_of(c)) for k, n, c in FIELD.findall(body)]
               for name, body in STRUCT.findall(combined)}
    fields, samplers = {}, {}
    for kind, name, count in UNIFORM.findall(combined):
        if kind.startswith('sampler'):
            if name not in samplers:
                samplers[name] = len(samplers)
        else:
            field = (kind, name, count_of(count))
            if name in fields and fields[name] != field:
                raise ValueError(f'Uniform differs between stages: {name}')
            fields[name] = field
    layout = struct_layout(list(fields.values()), structs)
    members = []
    for field in layout:
        if field['type'] in structs:
            for child in struct_layout(structs[field['type']], structs):
                members.append(dict(child, name=field['name'] + '.' + child['name'],
                                    offset=field['offset'] + child['offset']))
        else:
            members.append(field)
    for name, binding in samplers.items():
        kind = next(k for k, n, c in UNIFORM.findall(combined) if n == name and k.startswith('sampler'))
        members.append(dict(name=name, type=kind, offset=binding, size=4,
                            count=1, stride=4))
    size = align(max((f['offset'] + f['size'] for f in layout), default=0), 16)
    return layout, members, samplers, size

def transform(source, stage, layout, samplers):
    source = native_source(source.lstrip('\ufeff'))
    source = re.sub(r'#version[^\n]*', '#version 450 core', source, count=1)

    def uniform(match):
        kind, name, _ = match.groups()
        return (f'layout(binding = {samplers[name]}) uniform {kind} {name};'
                if kind.startswith('sampler') else '')
    source = UNIFORM.sub(uniform, source)
    if layout:
        fields = []
        for f in layout:
            suffix = f"[{f['count']}]" if f['count'] > 1 else ''
            fields.append(f"    layout(offset = {f['offset']}) {f['type']} {f['name']}{suffix};")
        block = 'layout(std140, binding = 15) uniform CaneConstants\n{\n' + '\n'.join(fields) + '\n};\n\n'

        function = re.search(r'^\s*(?:void|bool|float|int|vec[234]|mat[34]|MATERIAL)\s+\w+\s*\(', source, re.M)
        if not function:
            raise ValueError('No shader entry point')
        source = source[:function.start()] + block + source[function.start():]

    varying = re.compile(r'^(?:layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*)?'
                         r'(flat\s+)?(in|out)\s+(\w+)\s+(\w+)(\[[^;]+\])?;', re.M)
    occupied = {'in': set(), 'out': set()}
    defines = dict(re.findall(r'#define\s+(\w+)\s+(\d+)', source))

    def slots_for(kind, array):
        slots = 3 if kind == 'MATERIAL' else int(kind[-1]) if kind in ('mat3', 'mat4') else 1
        if array:
            token = array[1:-1].strip()
            slots *= int(defines.get(token, token))
        return slots

    for match in varying.finditer(source):
        at, _, direction, kind, name, array = match.groups()
        if at is not None:
            locations = set(range(int(at), int(at) + slots_for(kind, array)))
            if occupied[direction] & locations:
                raise ValueError(f'Overlapping stage interface location: {name}')
            occupied[direction].update(locations)

    def interface(match):
        explicit, flat, direction, kind, name, array = match.groups()
        if explicit is not None:
            at = int(explicit)
        else:
            if stage == 'vert' and direction == 'in':
                raise ValueError(f'Vertex input missing a location: {name}')
            slots = slots_for(kind, array)
            at = 0
            while occupied[direction] & set(range(at, at + slots)):
                at += 1
            occupied[direction].update(range(at, at + slots))
        return f'layout(location = {at}) {flat or ""}{direction} {kind} {name}{array or ""};'
    return varying.sub(interface, source)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--uam', type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    header = ['#pragma once', '#include <cstddef>', '#include <cstdint>',
              'namespace CaneDeko {',
              'struct UniformInfo { const char* name; unsigned offset, size, count, stride; const char* type; };',
              'struct ProgramInfo { const char* name; const uint8_t* vertex; size_t vertexSize; const uint8_t* fragment; size_t fragmentSize; const UniformInfo* uniforms; size_t uniformCount; unsigned constantSize, samplerCount; uint16_t uniformMask[2], storageMask[2]; };']
    programs = []
    for name in PROGRAMS:
        sources = program_sources(args.source, name)
        layout, members, samplers, size = program_layout(sources)
        resources = []
        for stage, source in zip(('vert', 'frag'), sources):
            text = transform(source, stage, layout, samplers)
            resources.append(resource_masks(text))
            glsl = args.output / f'{name}.{stage}'
            binary = args.output / f'{name}.{stage}.dksh'
            glsl.write_text(text)
            subprocess.run([str(args.uam), '-s', stage, '-o', str(binary), str(glsl)], check=True)
            data = binary.read_bytes()
            if len(data) < 64 or data[:4] != b'DKSH':
                raise ValueError(f'Invalid DKSH output: {binary}')
            _,header_size,control_size,code_size,program_offset,program_count=struct.unpack_from('<6I',data)
            if header_size<24 or control_size+code_size>len(data) or program_count!=1 or program_offset+64>control_size:
                raise ValueError(f'Invalid DKSH section bounds: {binary}')
            scratch=struct.unpack_from('<I',data,program_offset+20)[0]
            if scratch>65536:
                raise ValueError(f'Shader exceeds the native queue scratch limit: {binary}')
            header.append(f'alignas(256) inline const uint8_t {name}_{stage}[] = {{')
            header.extend(','.join(f'0x{x:02x}' for x in data[i:i+24]) + ',' for i in range(0, len(data), 24))
            header.append('};')

        header.append(f'inline const UniformInfo {name}_uniforms[] = {{')
        for f in members:
            header.append(f'{{"{f["name"]}", {f["offset"]}, {f["size"]}, {f["count"]}, {f["stride"]}, "{f["type"]}"}},')
        if not members:
            header.append('{nullptr,0,0,0,0,nullptr},')
        header.append('};')
        programs.append(dict(name=name, constant_size=size, members=members,
                           sampler_count=len(samplers), resources=resources))
    header.append('inline const ProgramInfo programs[] = {')
    for p in programs:
        name = p['name']
        vertex, fragment = p['resources']
        masks = (f'{{{vertex["uniform_mask"]},{fragment["uniform_mask"]}}},'
                 f'{{{vertex["storage_mask"]},{fragment["storage_mask"]}}}')
        header.append(f'{{"{name}", {name}_vert, sizeof({name}_vert), {name}_frag, sizeof({name}_frag), {name}_uniforms, {len(p["members"])}, {p["constant_size"]}, {p["sampler_count"]}, {masks}}},')
    header.extend(['};', '}'])
    (args.output / 'deko_shaders.h').write_text('\n'.join(header) + '\n')

if __name__ == '__main__':
    main()
