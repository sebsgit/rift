#!/usr/bin/env python
""" Usage: call with <filename>
"""

import sys
import clang.cindex

def find_class_decls(node):
    if node.kind == clang.cindex.CursorKind.CLASS_DECL:
        print(node.spelling)
    # Recurse for children of this node
    for c in node.get_children():
        if c.kind == clang.cindex.CursorKind.CXX_METHOD:
            print("found method: " + c.spelling)
            if c.is_const_method():
                print("and it's const")
            print(c.result_type.spelling)
        find_class_decls(c)

clang.cindex.Config.set_library_path('D:/projects/tests/llvm_build/Release/bin/')
index = clang.cindex.Index.create()
tu = index.parse(sys.argv[1])
print ('Translation unit: {}'.format(tu.spelling))
find_class_decls(tu.cursor)