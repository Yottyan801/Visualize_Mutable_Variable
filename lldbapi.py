
import lldb
import threading
import subprocess
import platform
import Cparse


class lldbapi:
    ValueType = ['invalid', 'global', 'static', 'arg', 'local', 0, 0, 0, 0]
    StopReason = ['invalid', 'None', 'Trace',
                  'BP', 'WP', 'Signal', 'Exception', 'Exec', 'PlanComplete', 'ThreadExiting']
    PthreadList = ['pthread_join', 'pthread_create']
    PTHREAD_LIBRARY = 'libpthread.so.0'

    def __init__(self):
        self.procState = -1
        self.proc_info = dict()
        self.WPlist = {}
        self.level = 0
        self.recursion = 20
        self.mode = 0
        self.exit = True
        self.thread = threading.Thread()
        self.func = []
        self.pthread = []
        lldb.SBDebugger.Initialize()
        self.arch = platform.machine()
        self.parser = Cparse.CParse()
        self.stoplist = list()

    def compile(self, fpath="sample/sample.c", lib="", bin_path="bin/target"):

        com = (
            "gcc %s -l%s -g -o %s" % (fpath, lib, bin_path)
            if lib
            else "gcc %s -g -o %s" % (fpath, bin_path)
        )
        print("COM:" + com)

        res = subprocess.call(com, shell=True)
        if res:
            print("Compile Error")
            return False
        else:
            self.pthread = []
            self.parser.__init__()
            self.parser.SetFilePath(fpath)
            self.parser.Parse()
            self.func = self.parser.parse_data["func"]
            print(self.func)
            self.selectPthread()
            self.file = lldb.SBFileSpec(fpath)
            if not self.file.IsValid():
                print("File Error")
            self.module = lldb.SBFileSpec(bin_path)
            if not self.module.IsValid():
                print("Module Error")
            return True

    def selectPthread(self):
        for pthread in self.parser.parse_data["pthread"]:
            if pthread["name"] in self.PthreadList:
                self.pthread.append(pthread)

    def __del__(self):
        self.exit = False

    def SetCondition(self, level, limit_recursion, mode):
        self.level = level
        self.recursion = limit_recursion
        self.mode = mode

    def Create(self):
        self.debugger = lldb.SBDebugger.Create()
        if not self.debugger.IsValid():
            print("Debugger Error")
            return
        self.debugger.SetAsync(False)
        modulepath = self.module.GetDirectory()+'/'+self.module.GetFilename()
        self.target = self.debugger.CreateTargetWithFileAndArch(
            modulepath, self.arch)
        if not self.target.IsValid():
            print("Target Error")
            return
        self.CreateBPAtAllLine()
        #self.CreateBPAtFunc()
        if self.mode == 1:
            for pthread in self.pthread:
                self.CreateBPAtLine(pthread['line'])

    def Launch(self):
        self.process = self.target.LaunchSimple(None, None, ".")
        self.StoreProcessInfo()

    def Continue(self):
        self.process.Continue()
        self.StoreProcessInfo()

    def AllThreadStepOver(self):
        thread = self.process.GetThreadAtIndex(0)
        thread.StepOver(lldb.eAllThreads)
        self.StoreProcessInfo()

    def OnlyThreadStepOver(self, thread_id):
        thread = self.process.GetThreadByID(thread_id)
        thread.StepOver(lldb.eOnlyThisThread)
        self.StoreProcessInfo()

    def Input(self, input):
        input += "\n"
        self.process.PutSTDIN(input)

    def Output(self):
        out = self.process.GetSTDOUT(2048)
        return out if out else ""

    def info_free(self):
        '''
        for t_idx in range(len(self.proc_info)):
            try:
                del self.proc_info
            except IndexError:
                pass
        '''
        self.stoplist = list()
        self.proc_info.__init__()

    def CreateBPAtFunc(self):
        for func in self.func:
            bp = self.target.BreakpointCreateByName(func)
            if not bp.IsValid():
                print("Can not Create BP at main()")

    def CreateBPAtAllLine(self):
        filepath = self.file.GetDirectory()+'/'+self.file.GetFilename()
        lineNum = sum([1 for _ in open(filepath)])
        print("line number :" + str(lineNum))
        for line in range(1, lineNum + 1):
            bp = self.target.BreakpointCreateByLocation(filepath, line)
            if not bp.IsValid():
                print("Can not Create BP(line:%s)" % str(line))

    def CreateBPAtLine(self, line):
        filepath = self.file.GetDirectory()+'/'+self.file.GetFilename()
        bp = self.target.BreakpointCreateByLocation(filepath, line)
        if not bp.IsValid():
            print("Can not Create BP(line:%s)" % str(line))

    def StoreProcessInfo(self):
        if not self.process.IsValid():
            print("Process Error")
            return
        self.info_free()
        self.procState = self.process.GetState()
        self.thread_num = self.process.GetNumThreads()
        for idx in range(self.thread_num):
            thread = self.process.GetThreadAtIndex(idx)
            if thread.GetThreadID() not in self.proc_info:
                self.proc_info[thread.GetThreadID()] = dict()
            self.StoreThreadInfo(thread,self.proc_info[thread.GetThreadID()])

    def StoreThreadInfo(self, thread,t_dict):
        if not thread.IsValid():
            print("Thread Error")
            return
        if thread.GetStopReason() == lldb.eStopReasonWatchpoint:
            for reason_idx in range(thread.GetStopReasonDataCount()):
                wp_id = thread.GetStopReasonDataAtIndex(reason_idx)
                self.stoplist.append(wp_id)

        for idx in range(thread.GetNumFrames()):
            frame = thread.GetFrameAtIndex(idx)
            if frame.GetDisplayFunctionName() not in t_dict:
                if frame.GetDisplayFunctionName() not in self.func:
                    return
                t_dict[frame.GetDisplayFunctionName()] = dict()
            self.StoreFrameInfo(frame,t_dict[frame.GetDisplayFunctionName()])
                
        

    def StoreFrameInfo(self, frame,f_dict):
        if not frame.IsValid():
            print("Frame Error")
            return
        
        f_dict['name'] = frame.GetDisplayFunctionName()
        v_list = frame.GetVariables(True, True, True, False)
        for idx in range(v_list.GetSize()):
            variable = v_list.GetValueAtIndex(idx)
            if variable.GetID() not in f_dict:
                f_dict[variable.GetID()] = dict()
            self.StoreVariableInfo(variable, 0,f_dict)
        if frame.GetLineEntry().IsValid():
            f_dict['line'] = frame.GetLineEntry().GetLine()

        
    def StoreVariableInfo(self, variable, deep,f_dict):
        if(not variable.IsValid()):
            print("variable is not valid")
            return
        v_dict =  f_dict[variable.GetID()]

        if 'wp_id' not in v_dict:
            v_dict['wp_id'] = self.setWP(variable)
        if v_dict['wp_id'] in self.stoplist:
            print('%s行目:%s関数の変数名%sが%sから'%(f_dict['line'],f_dict['name'],v_dict['name'],v_dict['value']),end='')

        v_dict["type"] = variable.GetTypeName()
        v_dict["name"] = variable.GetName()
        v_dict["value"] = variable.GetValue()
        if v_dict['wp_id'] in self.stoplist:
            print('%sに変わりました。'%(v_dict['value']))
        v_dict["attr"] = self.ValueType[variable.GetValueType()]
        v_dict['child'] = []
        if variable.MightHaveChildren():
            for v_idx in range(variable.GetNumChildren()):
                if deep > self.recursion:
                    return v_dict
                child = variable.GetChildAtIndex(v_idx)
                deep += 1
                v_dict["child"].append(self.StoreVariableInfo(child, deep))
        v_dict["value"] = variable.GetValue()
        v_dict["attr"] = self.ValueType[variable.GetValueType()]

        
        
        return v_dict

    def setWP(self, variable):
        #print(variable.__class__)
        error = lldb.SBError()
        WP = variable.Watch(True, False, True, error)
        if not WP.IsValid():
            print('Watch Point Error')
            print(error.GetType())
            return 
        return WP.GetID()
        
