import argparse
from pathlib import Path
import struct


def font():
    u16 = lambda values: struct.pack('>' + 'H' * len(values), *(x & 65535 for x in values))
    u32 = lambda value: struct.pack('>I', value)
    instructions = bytes([0x01, 0xB1, 0, 128, 0x48])
    glyf = u16([1, 100, 0, 600, 700, 3, len(instructions)]) + instructions + bytes([1] * 4)
    glyf += u16([100, 0, 500, 0, 0, 700, 0, -700])
    glyf += bytes(len(glyf) % 2)
    tables = {
        'cmap': u16([0, 1, 3, 1]) + u32(12) + u16([4, 32, 0, 4, 4, 1, 0, 65, 65535, 0, 65, 65535, -64, 1, 0, 0]),
        'glyf': glyf,
        'head': u32(65536) + u32(65536) + u32(0) + u32(0x5F0F3CF5) + u16([11, 1000] + [0] * 8 + [0, 0, 700, 800, 0, 8, 2, 0, 0]),
        'hhea': u32(65536) + u16([800, -200, 0, 700, 0, 0, 700, 1, 0, 0, 0, 0, 0, 0, 0, 2]),
        'hmtx': u16([700, 0, 700, 100]),
        'loca': u16([0, 0, len(glyf) // 2]),
        'maxp': u32(65536) + u16([2, 4, 1, 0, 0, 2, 0, 0, 0, 0, 2, len(instructions), 0, 0]),
    }
    offset = 12 + 16 * len(tables)
    output = u32(65536) + u16([len(tables), 64, 2, len(tables) * 16 - 64])
    data = b''
    for tag, body in sorted(tables.items()):
        output += tag.encode() + u32(0) + u32(offset) + u32(len(body))
        padded = body + bytes(-len(body) % 4)
        data += padded
        offset += len(padded)
    return output + data


def generate(source, output):
    output.mkdir(parents=True, exist_ok=True)
    raw = (source / 'src/base/ftobjs.c').read_text()
    start = raw.index('  FT_BASE_DEF( FT_Error )\n  FT_New_GlyphSlot(')
    end = raw.index('\n\n  /* documentation is in ftobjs.h */', start)
    (output / 'ftobjs-slot-import.c').write_text(raw[:start] + raw[end:])
    raw = (source / 'src/base/ftbase.c').read_text()
    assert raw.count('#include "ftobjs.c"') == 1
    (output / 'ftbase-slot-import.c').write_text(raw.replace('#include "ftobjs.c"', '#include "ftobjs-slot-import.c"'))
    (output / 'slot-font.h').write_text('static const unsigned char SlotFont[] = {' + ','.join(map(str, font())) + '};\n')
    (output / 'guest-modules.h').write_text('FT_USE_MODULE( FT_Driver_ClassRec, tt_driver_class )\nFT_USE_MODULE( FT_Module_Class, sfnt_module_class )\nFT_USE_MODULE( FT_Renderer_Class, ft_smooth_renderer_class )\n')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    generate(args.source, args.output)
