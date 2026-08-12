#pragma once

#include <iostream>
#include <Windows.h>

#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>

#include <thread>
#include <chrono>
#include <string>
#include <filesystem>

class Process
{

#if 1

#define BUFSIZE 4096 
public:
    Process(std::wstring processNamepath, std::wstring cmdarg /*HANDLE ChildStd_OUT_Rd = nullptr, HANDLE ChildStd_OUT_Wr = nullptr*/)
    {
        SECURITY_ATTRIBUTES saAttr;

        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        HANDLE ChildStd_OUT_Wr = NULL;
        HANDLE ChildStd_IN_Rd = NULL;

        // if (ChildStd_OUT_Rd)
        //     m_ChildStd_OUT_Rd = ChildStd_OUT_Rd;
        if (!CreatePipe(&m_ChildStd_OUT_Rd, &ChildStd_OUT_Wr, &saAttr, 0))
            printf("StdoutRd CreatePipe");

        if (!SetHandleInformation(m_ChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
            printf("Stdout SetHandleInformation");


        if (!CreatePipe(&ChildStd_IN_Rd, &m_ChildStd_IN_Wr, &saAttr, 0))
            printf("Stdin CreatePipe");

        if (!SetHandleInformation(m_ChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
            printf("Stdin SetHandleInformation");

        TCHAR szCmdline[] = L"";
        PROCESS_INFORMATION piProcInfo;
        STARTUPINFO siStartInfo;
        BOOL bSuccess = FALSE;

        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = ChildStd_OUT_Wr;
        siStartInfo.hStdOutput = ChildStd_OUT_Wr;
        siStartInfo.hStdInput = ChildStd_IN_Rd;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process.

        bSuccess = CreateProcess(processNamepath.c_str(),
            (wchar_t*)(std::filesystem::path(std::filesystem::current_path() / processNamepath).wstring() +
                L" " +
                cmdarg).c_str(),     // command line 
            NULL,          // process security attributes 
            NULL,          // primary thread security attributes 
            TRUE,          // handles are inherited 
            CREATE_NO_WINDOW,// creation flags 
            NULL,          // use parent's environment 
            NULL,          // use parent's current directory 
            &siStartInfo,  // STARTUPINFO pointer 
            &piProcInfo);  // receives PROCESS_INFORMATION 

        if (!bSuccess)
            printf("CreateProcess");
        else
        {
            CloseHandle(ChildStd_OUT_Wr);
            CloseHandle(ChildStd_IN_Rd);
            CloseHandle(piProcInfo.hThread);

            m_Process = piProcInfo.hProcess;
        }
    }

    bool IsProcessActive()
    {
        if (!m_Process)
            return false;

        DWORD exitCode = 0;

        if (GetExitCodeProcess(m_Process, &exitCode))
        {
            if (exitCode == STILL_ACTIVE)
                return true;
            else
                return false;
        }

        return false;
    }

    void EndProcess()
    {
        CloseHandle(m_ChildStd_IN_Wr);
        CloseHandle(m_ChildStd_OUT_Rd);

        if (IsProcessActive())
            TerminateProcess(m_Process, 0);

        CloseHandle(m_Process);
    }

    ~Process()
    {
        if (IsProcessActive())
            EndProcess();
    }

    void Write(std::string message)
    {
        DWORD dwWritten;
        BOOL bSuccess = FALSE;

        message += '\n';

        bSuccess = WriteFile(m_ChildStd_IN_Wr, message.c_str(), message.size(), &dwWritten, NULL);
        if (!bSuccess)
            printf("Write to Process");
    }

    std::string Read()
    {
        DWORD dwRead, dwAvail;
        CHAR chBuf[BUFSIZE];
        BOOL bSuccess = FALSE;
        std::string output;

        if (!PeekNamedPipe(m_ChildStd_OUT_Rd, NULL, 0, &dwRead, &dwAvail, NULL))
            printf("Check read Process");

        if (dwAvail == 0)
            return output;

        do
        {
            dwRead = 0;
            bSuccess = ReadFile(m_ChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
            if (!bSuccess || dwRead == 0)
                break;
            if (!bSuccess)//will not be hitted 
                printf("Read Process");
            output += std::string(chBuf, dwRead);
        } while (dwRead == BUFSIZE);

        return output;
    }

    HANDLE GetHandle()
    {
        return m_Process;
    }

private:

    HANDLE m_ChildStd_IN_Wr = NULL;
    HANDLE m_ChildStd_OUT_Rd = NULL;
    HANDLE m_Process = NULL;

#endif // PLATFORM_WINDOWS_64
};