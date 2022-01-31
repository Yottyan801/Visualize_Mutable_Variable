import lldbapi
from threading import Thread
import lldb
import sys
import pprint


def inputThread(debugger):
    while debugger.exit:
        inputStr = sys.stdin.read(1)
        print(inputStr)
        debugger.Input(inputStr)


def outputThread(debugger):
    while debugger.exit:
        out = debugger.Output()
        if out:
            print("out:"+out, end="")


def main(debugger):

    while debugger.procState == 5:
        #print('-----------------------------')
        debugger.Continue()
        #pprint.pprint(debugger.proc_info)


if __name__ == "__main__":
    # print(sys.getrecursionlimit())
    debugger = lldbapi.lldbapi()
    debugger.compile(sys.argv[1], "pthread")
    debugger.SetCondition(0, 4, 0)
    debugger.Create()
    #print('-----------------------------')
    debugger.Launch()
    #pprint.pprint(debugger.proc_info)

    main(debugger)

    debugger.__del__()
