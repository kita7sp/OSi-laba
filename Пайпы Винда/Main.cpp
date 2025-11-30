#include <iostream>
#include <windows.h>
#include <string>
#include <vector>

using namespace std;

// Main -> pInput -> M -> p1 -> A -> p2 -> P -> p3 -> S -> stdout

vector<HANDLE> processes;

void doProc(const char* prog, HANDLE inputHandle, HANDLE outputHandle) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = inputHandle;
    si.hStdOutput = outputHandle;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    
    char cmdLine[256];
    strcpy(cmdLine, prog);
    
    if (!CreateProcess(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        return;
    }
    
    processes.push_back(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main() {
    HANDLE p1_read, p1_write, p2_read, p2_write, p3_read, p3_write, pInput_read, pInput_write;
    SECURITY_ATTRIBUTES sa;
    
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    
    CreatePipe(&pInput_read, &pInput_write, &sa, 0);
    CreatePipe(&p1_read, &p1_write, &sa, 0);
    CreatePipe(&p2_read, &p2_write, &sa, 0);
    CreatePipe(&p3_read, &p3_write, &sa, 0);
    
    // только этот конец не наследуется а то виснет
    SetHandleInformation(pInput_write, HANDLE_FLAG_INHERIT, 0);
    
    cout << "Input: 1 2 3 4\n";
    cout << "Result: ";
    cout.flush();

    // S
    doProc("S.exe", p3_read, GetStdHandle(STD_OUTPUT_HANDLE));
    CloseHandle(p3_read);

    // P
    doProc("P.exe", p2_read, p3_write);
    CloseHandle(p2_read);
    CloseHandle(p3_write);

    // A
    doProc("A.exe", p1_read, p2_write);
    CloseHandle(p1_read);
    CloseHandle(p2_write);

    // M
    doProc("M.exe", pInput_read, p1_write);
    CloseHandle(pInput_read);
    CloseHandle(p1_write);

    // используемая строка
    string input = "1 2 3 4";
    DWORD written;
    WriteFile(pInput_write, input.c_str(), input.length(), &written, NULL);
    WriteFile(pInput_write, "\n", 1, &written, NULL);
    CloseHandle(pInput_write);

    WaitForMultipleObjects(processes.size(), processes.data(), TRUE, INFINITE);
    
    for (HANDLE h : processes) {
        CloseHandle(h);
    }

    return 0;
}