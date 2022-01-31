# -*- coding:utf-8 -*-
import sys
import pprint
import clang.cindex
from clang.cindex import Index
from clang.cindex import Config


class CParse:
    def __init__(self):
        self.filepath = ""
        self.parse_data = {"func": [], "pthread": []}
        self.index = Index.create()

    def SetFilePath(self, path):
        self.filepath = path

    def Parse(self):
        self.tree = self.index.parse(self.filepath)
        self.RecursivePerse(self.tree.cursor)

    def RecursivePerse(self, node):
        if str(node.location.file) == self.filepath:
            if node.kind.name == "FUNCTION_DECL":
                if(node.displayname.split('(')[0] not in self.parse_data["func"]):
                    self.parse_data["func"].append(node.displayname.split('(')[0])
            if node.kind.name == "CALL_EXPR":
                if "pthread" in node.displayname:
                    pthread = {"line": node.extent.start.line,
                               "name": node.displayname}
                    self.parse_data["pthread"].append(pthread)

        for child in node.get_children():
            self.RecursivePerse(child)


def main():
    parser = CParse()
    if sys.argv[1]:
        parser.SetFilePath(sys.argv[1])
        parser.Parse()
        pprint.pprint(parser.parse_data)


if __name__ == "__main__":
    main()
