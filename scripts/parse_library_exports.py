#!/usr/bin/env python3

import os
import re

def parse_exports(library_txt_path):
    with open(library_txt_path) as lib_txt:
        lib_dir = os.path.dirname(library_txt_path)
        components = lib_dir.split('/')
        while components and 'scenery' not in components[0].lower():
            components[:] = components[1:]
        base_path = '/'.join(components)
        base_path = base_path.replace('iphone_scenery', 'Global Scenery')

        whitespace_re = re.compile(r"[ \t]+")

        for line in lib_txt:
            if line.startswith('EXPORT') and '.obj' in line:
                export_kw, lib_path, disk_rel_path = whitespace_re.sub(' ', line).split()
                yield lib_path, base_path + '/' + disk_rel_path


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("library_txt_path", help="Path to the library.txt file whose exports you want to parse")
    args = parser.parse_args()

    for export in parse_exports(args.library_txt_path):
        print("{\"%s\", \"%s\"}," % export)




