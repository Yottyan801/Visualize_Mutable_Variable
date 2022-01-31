#ifndef LLDBAPI_H
#define LLDBAPI_H

#include <lldb/API/SBDefines.h>
#include <lldb/API/SBDebugger.h>
#include <lldb/API/SBTarget.h>
#include <lldb/API/SBProcess.h>
#include <lldb/API/SBBreakpoint.h>
#include <lldb/API/SBCommandInterpreter.h>
#include <lldb/API/SBPlatform.h>
#include <lldb/API/SBThread.h>
#include <lldb/API/SBMemoryRegionInfoList.h>
#include <lldb/API/SBMemoryRegionInfo.h>
#include <lldb/API/SBValue.h>
#include <lldb/API/SBValueList.h>
#include <lldb/API/SBStream.h>
#include <string>
#include <functional>

typedef struct _VALIABLE
{
    long unsigned int addr;
    const char *typeName;
    const char *name;
    const char *value;
} VALIABLE;

typedef struct _THREAD
{
    VALIABLE *vlist;
    int valiable_num;
    const char *name;
} THREAD;

class lldbapi
{
public:
    lldbapi();
    lldbapi(std::string fpath);
    lldbapi(std::string fpath, std::string lib);
    void next();
    void Launch();
    void printValiable();
    void vlist_free();
    void Input(std::string in);
    std::string Output();
    //void IsValid();
    ~lldbapi();

    char input[1024];
    char out[512];
    int procState = -1;
    int thread_num = -1;
    THREAD *thread;
    int line = -1;
    lldb::SBProcess process;
    bool exit = true;

private:
    lldb::SBTarget CreateBPAtAllLine(lldb::SBTarget target, const char *filepath);
    bool PrintMemoryRegionsAtProcess(lldb::SBProcess process, const char *path, lldb::addr_t *addr_list);
    bool StoreValiableInfo(lldb::SBThread thread, int thread_id);

    const char *exepath;
    const char *filepath;
    const char *arch;
    std::string exe;

    lldb::SBDebugger debugger;
    lldb::SBTarget target;
};

#endif