#include <stdio.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include "lldbapi.hpp"
using namespace std;

lldbapi debugger;

void mainThread()
{
    while (debugger.procState == lldb::eStateStopped)
    {
        printf("----------Process Continue----------\n");

        debugger.next();
        debugger.printValiable();
        debugger.vlist_free();
    }
}

void inputThread()
{
    while (debugger.exit)
    {
        string input;
        cin >> input;
        debugger.Input(input);
    }
}

void outputThread()
{
    string out;
    while (debugger.exit)
        cout << "\033[33m" + debugger.Output() + "\033[m";
}

int main(void)
{
    debugger.Launch();

    std::thread main(mainThread);
    std::thread input(inputThread);
    std::thread output(outputThread);

    main.join();
    input.detach();
    output.detach();

    printf("Process State : %d   Quiet Continue Loop\n", debugger.procState);

    return 0;
}