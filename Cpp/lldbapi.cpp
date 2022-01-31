#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <lldb/API/SBThread.h>
using namespace std;
using namespace lldb;

#include "lldbapi.hpp"

lldbapi::lldbapi()
{
    lldbapi::exepath = "sample/bin/sample";
    lldbapi::filepath = "sample/deadlock.c";
    lldbapi::exe = "sample";
}

lldbapi::lldbapi(string fpath)
{
    lldbapi::filepath = fpath.c_str();
    string com;
    com = "gcc " + fpath + " -g -o bin/target";
    lldbapi::exepath = "bin/target";
    lldbapi::exe = "target";

    system(com.c_str());
}

lldbapi::lldbapi(string fpath, string lib)
{
    lldbapi::filepath = fpath.c_str();
    string com;
    com = "gcc " + fpath + " -l" + lib + " -g -o bin/target";
    lldbapi::exepath = "bin/target";
    lldbapi::exe = "target";

    system(com.c_str());
}

lldbapi::~lldbapi()
{
    lldbapi::exit = false;
}

void lldbapi::Input(string in)
{
    in += "\n";
    lldbapi::process.PutSTDIN(in.c_str(), in.length());
}

string lldbapi::Output()
{
    char out[256];
    int size = lldbapi::process.GetSTDOUT(out, 256);
    if (size > 0)
        return string(out);
    return string("");
}

void lldbapi::Launch()
{
    SBDebugger::Initialize();
    lldbapi::debugger = SBDebugger::Create();
    debugger.SetAsync(false);
    lldbapi::arch = "x86_64";
    lldbapi::target = debugger.CreateTargetWithFileAndArch(lldbapi::exepath, lldbapi::arch);
    target = CreateBPAtAllLine(target, filepath);
    lldbapi::process = target.LaunchSimple(NULL, NULL, ".");
    lldbapi::procState = process.GetState();
}

void lldbapi::next()
{
    lldbapi::process.Continue();
    lldbapi::procState = process.GetState();
    lldbapi::thread_num = process.GetNumThreads();

    lldbapi::thread = (THREAD *)malloc(lldbapi::thread_num * sizeof(THREAD));
    for (int idx = 0; idx < process.GetNumThreads(); idx++)
    {
        SBThread thread = process.GetThreadAtIndex(idx);
        lldbapi::thread[idx].name = thread.GetName();
        if (!StoreValiableInfo(thread, idx))
            return;
    }
}

void lldbapi::printValiable()
{
    for (int idx = 0; idx < lldbapi::thread_num; idx++)
    {
        printf("=========[No.%d]Thread Name : %s  | Line:%d |===========\n", idx, lldbapi::thread[idx].name, lldbapi::line);
        for (int id = 0; id < lldbapi::thread[idx].valiable_num; id++)
        {
            printf("(Addr:0x%lx)Valiable Name : %s %s = %s\n", lldbapi::thread[idx].vlist[id].addr, lldbapi::thread[idx].vlist[id].typeName, lldbapi::thread[idx].vlist[id].name, lldbapi::thread[idx].vlist[id].value);
        }
    }
}
void lldbapi::vlist_free()
{

    for (int idx = 0; idx < lldbapi::thread_num; idx++)
        if (lldbapi::thread[idx].valiable_num > 0)
            free(lldbapi::thread[idx].vlist);

    free(lldbapi::thread);
}

SBTarget lldbapi::CreateBPAtAllLine(SBTarget target, const char *filepath)
{
    ifstream myfile(filepath);
    int lineNum = 0;
    string line;
    while (getline(myfile, line))
        ++lineNum;
    printf("line number : %d\n", lineNum);
    for (int line = 1; line < lineNum; line++)
    {
        SBBreakpoint bp = target.BreakpointCreateByLocation(filepath, line);
        if (!(bp.IsValid()))
            printf("Can't Create BP(line:%d)\n", line);
    }
    return target;
}

bool lldbapi::StoreValiableInfo(SBThread thread, int thread_id)
{
    SBFrame frame;
    int line;
    lldbapi::thread[thread_id].valiable_num = 0;
    for (int idx = 0; idx < thread.GetNumFrames(); idx++)
    {

        frame = thread.GetFrameAtIndex(idx);
        if (!frame.IsValid())
            return false;
        //printf("frame display name : %s\n", frame.GetModule().GetFileSpec().GetFilename());
        if (lldbapi::exe == string(frame.GetModule().GetFileSpec().GetFilename()))
        {
            line = frame.GetLineEntry().GetLine();
            if (line > 0)
                lldbapi::line = line;
            SBValueList valiable_list = frame.GetVariables(true, true, true, true);

            if (valiable_list.GetSize() > 0)
            {
                lldbapi::thread[thread_id].vlist = (VALIABLE *)malloc(valiable_list.GetSize() * sizeof(VALIABLE));
                lldbapi::thread[thread_id].valiable_num = valiable_list.GetSize();
                for (int id = 0; id < lldbapi::thread[thread_id].valiable_num; id++)
                {
                    SBValue valiable = valiable_list.GetValueAtIndex(id);
                    lldbapi::thread[thread_id].vlist[id].addr = valiable.GetLoadAddress();
                    lldbapi::thread[thread_id].vlist[id].typeName = valiable.GetTypeName();
                    lldbapi::thread[thread_id].vlist[id].name = valiable.GetName();
                    lldbapi::thread[thread_id].vlist[id].value = valiable.GetValue();
                    //printf("(Addr:0x%lx)Valiable Name : %s %s = %s\n", valiable.GetAddress(), valiable.GetTypeName(), valiable.GetName(), valiable.GetValue());
                }
            }
        }
    }
    return true;
}

/*
bool PrintMemoryRegionsAtProcess(SBProcess process, const char *path)
{
    SBMemoryRegionInfoList memory_List = process.GetMemoryRegions();
    SBMemoryRegionInfo region_info;
    const char *region_name;

    for (int idx = 0; idx < memory_List.GetSize(); idx++)
    {
        memory_List.GetMemoryRegionAtIndex(idx, region_info);
        region_name = region_info.GetName();
        if (region_name != NULL && strcmp(region_name, path) == 0)
        {
            printf("Memory Region( %d ) Name : %s\n", idx, region_name);
            printf("(addr Base: 0x%lx End : 0x%lx)\n", region_info.GetRegionBase(), region_info.GetRegionEnd());
        }
    }
    return true;
}
*/