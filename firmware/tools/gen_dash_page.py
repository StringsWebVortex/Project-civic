#!/usr/bin/env python3
"""Embed web/index.html as a gzip'd C array (served with Content-Encoding: gzip)."""
import gzip, sys
src, dst = sys.argv[1], sys.argv[2]
raw = open(src, 'rb').read()
gz = gzip.compress(raw, compresslevel=9, mtime=0)
with open(dst, 'w') as f:
    f.write('/* generated from %s by tools/gen_dash_page.py - do not edit */\n' % src)
    f.write('#include <stddef.h>\n#include <stdint.h>\n')
    f.write('const uint8_t dash_page_gz[%d] = {\n' % len(gz))
    for i in range(0, len(gz), 20):
        f.write(','.join('0x%02x' % b for b in gz[i:i+20]) + ',\n')
    f.write('};\nconst size_t dash_page_gz_len = %d;\nconst size_t dash_page_raw_len = %d;\n' % (len(gz), len(raw)))
