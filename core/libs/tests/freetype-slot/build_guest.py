import argparse
from pathlib import Path
import shutil
import subprocess

from prepare import generate


def run(*arguments):
    subprocess.run([str(x) for x in arguments], check=True)


def build(args):
    root = args.output.resolve()
    root.mkdir(parents=True, exist_ok=True)
    generated = root / 'generated'
    generate(args.source, generated)
    imports = root / 'imports'
    imports.mkdir(exist_ok=True)
    for name in ['libSceFreeTypeOt', 'libc']:
        source = imports / (name + '.c')
        source.write_text('int SlotLinkMarker_nid_no_patch;\n')
        run(args.zig, 'cc', '-target', 'x86_64-linux-gnu', '-shared', '-fPIC', '-nostdlib', source,
            '-Wl,-soname,' + name + '.prx', '-o', imports / (name + '.so'))
    run(args.cmake, '-S', args.test_source, '-B', root / 'reference', '-G', 'Ninja',
        '-DCMAKE_SYSTEM_NAME=Linux', '-DCMAKE_C_COMPILER=' + str(args.zig), '-DCMAKE_C_COMPILER_ARG1=cc',
        '-DCMAKE_C_FLAGS=-target x86_64-linux-gnu -fPIC -O2 -ffreestanding -fno-stack-protector',
        '-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY', '-DCMAKE_AR=' + str(args.host_bin / 'ar.exe'),
        '-DCMAKE_RANLIB=' + str(args.host_bin / 'ranlib.exe'), '-DCMAKE_MAKE_PROGRAM=' + str(args.ninja),
        '-DFREETYPE_SLOT_SOURCE=' + str(args.source), '-DFREETYPE_SLOT_GENERATED=' + str(generated),
        '-DFREETYPE_SLOT_IMPORTS=' + str(imports))
    run(args.cmake, '--build', root / 'reference', '--target', 'slotFreeType')
    library = root / 'reference/slotFreeType.prx'
    shutil.copy2(library, root / 'reference/slotFreeType.so')
    run(args.zig, 'cc', '-target', 'x86_64-linux-gnu', '-fPIC', '-O2', '-ffreestanding',
        '-fno-stack-protector', '-I' + str(args.source / 'include'), '-I' + str(generated),
        '-c', args.test_source / 'Ttf.c', '-o', root / 'main.o')
    run(args.zig, 'cc', '-target', 'x86_64-linux-gnu', '-shared', '-nostdlib', '-Wl,-e,main',
        '-Wl,--hash-style=sysv', '-Wl,--no-as-needed', root / 'main.o', root / 'reference/slotFreeType.so',
        imports / 'libc.so', '-o', root / 'test.elf')
    modules = root / 'modules'
    modules.mkdir(exist_ok=True)
    shutil.copy2(library, modules / library.name)
    run(args.relinker, '--windows', '--module-dir', 'modules', '--rpath', '.', root / 'test.elf', root / 'SlotTtf.exe')
    for name in ['libc', 'libkernel', 'libSceFreeType', 'libSceFreeTypeOt']:
        destination = root / (name + '.prx')
        shutil.copy2(args.runtime / destination.name, destination)
        options = [] if name == 'libc' else ['--preserve-export-list', args.exports]
        run(args.patcher, name, *options, destination)
    for name in ['libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll']:
        shutil.copy2(args.host_bin / name, root / name)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    for option in ['output', 'source', 'test-source', 'zig', 'cmake', 'ninja', 'host-bin', 'relinker', 'patcher', 'runtime', 'exports']:
        parser.add_argument('--' + option, type=Path, required=True)
    build(parser.parse_args())
