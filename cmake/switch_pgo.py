import argparse
import hashlib
import json
from pathlib import Path
import struct
import zipfile


def crc_entry(value):
    seed = value << 24
    for _ in range(8):
        seed = ((seed << 1) ^ (0x04c11db7 if seed & 0x80000000 else 0)) & 0xffffffff
    return seed


crc_table = [crc_entry(value) for value in range(256)]


def checksum(seed, value):
    for byte in value.encode() + b'\0':
        seed = ((seed << 8) ^ crc_table[(seed >> 24) ^ byte]) & 0xffffffff
    return seed


def source_hash(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def header_hash(source):
    digest = hashlib.sha256()
    headers = sorted((path for folder in ('Sly1-Release', 'Libraries/include', 'platform')
                      for path in (source / folder).rglob('*') if path.suffix in ('.h', '.hpp', '.inl')),
                     key=lambda path: path.relative_to(source).as_posix())
    for path in headers:
        digest.update(path.relative_to(source).as_posix().encode() + b'\0')
        digest.update(bytes.fromhex(source_hash(path)))
    return digest.hexdigest()


parser = argparse.ArgumentParser()
parser.add_argument('--source', type=Path, required=True)
parser.add_argument('--devkitpro', type=Path, required=True)
parser.add_argument('--archive', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
with zipfile.ZipFile(args.archive) as archive:
    manifest = json.loads(archive.read('profiles.json'))
    compatible = header_hash(args.source) == manifest['headers']
    compatible = compatible and all((args.devkitpro / path).is_file() and
        source_hash(args.devkitpro / path) == digest for path, digest in manifest['sdk'].items())
    for name, entry in manifest['profiles'].items():
        destination = args.output / name
        source = args.source / entry['source']
        if not compatible or not source.is_file() or source_hash(source) != entry['sha256']:
            destination.unlink(missing_ok=True)
            continue
        data = bytearray(archive.read(name))
        base = 'CMakeFiles/ProjectCane.dir/' + entry['source']
        for offset, symbol, filename, line, internal in entry['functions']:
            if filename.startswith('@source/'):
                filename = (args.source / filename[8:]).as_posix()
            elif filename.startswith('@sdk/'):
                filename = (args.devkitpro / filename[5:]).as_posix()
            ident = checksum(checksum(checksum(0, filename), symbol), base) if internal else checksum(0, symbol)
            ident = (ident & 0x7fffffff) or 1
            struct.pack_into('<II', data, offset, ident, checksum(checksum(line, filename), symbol))
        if not destination.exists() or destination.read_bytes() != data:
            destination.write_bytes(data)
