#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <psapi.h>
#include<tlhelp32.h>
#include<Shlwapi.h>


BOOL KillProcessByName(const TCHAR* processName);
void DeleteFilesRecursively(const TCHAR* directory);
BOOL DelTargetFile(const TCHAR* command);
BOOL DelMyself();
void FindPathAndDeleteFiles(const TCHAR* szProcessName);


int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <process_name>\n", argv[0]);
        return 1;
    }

    FindPathAndDeleteFiles(argv[1]);
    //删除自身进程
    DelMyself();

    return 0;
}


//自删除
BOOL DelMyself()
{
    TCHAR szFile[MAX_PATH];
    TCHAR szCmd[MAX_PATH];
    if ((GetModuleFileName(0, szFile, MAX_PATH) != 0) && (GetShortPathName(szFile, szFile, MAX_PATH) != 0))
    {
        lstrcpy(szCmd, _T("/c del "));      //拼接命令行参数
        lstrcat(szCmd, szFile);             //添加要删除的文件
        lstrcat(szCmd, _T(" >> NUL"));          //隐藏输出信息

        //获取 cmd.exe 的完整路径，并将命令行参数传递给它来执行删除操作（隐藏窗口）
        GetEnvironmentVariable(_T("ComSpec"), szFile, MAX_PATH);
        ShellExecuteW(0, 0, szFile, szCmd, 0, SW_HIDE);

        //判断是否删除成功
        if ((GetEnvironmentVariable(_T("ComSpec"), szFile, MAX_PATH) != 0) && ((INT)ShellExecute(0, 0, szFile, szCmd, 0, SW_HIDE) > 32))
            return TRUE;
    }
    return FALSE;
}


//kill 指定进程
BOOL KillProcessByName(const TCHAR* processName)
{
    BOOL result = FALSE;

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hProcessSnap, &pe32))
        {
            do
            {
                if (_tcsicmp(pe32.szExeFile, processName) == 0)
                {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                    if (hProcess != NULL)
                    {
                        if (TerminateProcess(hProcess, 0))
                        {
                            result = TRUE;
                        }
                        CloseHandle(hProcess);
                    }
                }
            } while (Process32Next(hProcessSnap, &pe32));
        }
        CloseHandle(hProcessSnap);
    }
    return result;
}

//删除指定文件
BOOL DelTargetFile(const TCHAR* command)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    TCHAR commandLine[MAX_PATH];
    _sntprintf_s(commandLine, MAX_PATH, _TRUNCATE, TEXT("cmd /c del %s"), command);

    if (!CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exitCode == 0;
}

//递归删除指定文件夹及子文件夹
void DeleteFilesRecursively(const TCHAR* directory)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    TCHAR searchPath[MAX_PATH], filePath[MAX_PATH];
    _sntprintf_s(searchPath, MAX_PATH, _TRUNCATE, TEXT("%s\\*"), directory);

    // First, delete all files in the current directory
    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (_tcscmp(findFileData.cFileName, TEXT(".")) == 0 || _tcscmp(findFileData.cFileName, TEXT("..")) == 0)
            {
                continue;
            }

            _sntprintf_s(filePath, MAX_PATH, _TRUNCATE, TEXT("%s\\%s"), directory, findFileData.cFileName);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                DeleteFilesRecursively(filePath);
                RemoveDirectory(filePath);
            }
            else
            {
                DelTargetFile(filePath);
            }

        } while (FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
    }

    // Then, delete the current directory itself (if it's empty)
    RemoveDirectory(directory);
}



void FindPathAndDeleteFiles(const TCHAR* szProcessName)
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    TCHAR szModuleName[MAX_PATH] = TEXT("<unknown>");

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        printf("EnumProcesses failed. Error: %d\n", GetLastError());
        return;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    for (DWORD i = 0; i < cProcesses; i++)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
        if (hProcess)
        {
            HMODULE hMod;
            DWORD cbNeeded;

            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
            {
                GetModuleBaseNameW(hProcess, hMod, szModuleName, sizeof(szModuleName) / sizeof(TCHAR));

                if (_tcsicmp(szModuleName, szProcessName) == 0)
                {
                    TCHAR szFilePath[MAX_PATH];
                    GetModuleFileNameExW(hProcess, hMod, szFilePath, sizeof(szFilePath) / sizeof(TCHAR));

                    TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFile[_MAX_FNAME], szExt[_MAX_EXT];
                    _tsplitpath_s(szFilePath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFile, _MAX_FNAME, szExt, _MAX_EXT);

                    TCHAR szDirPath[MAX_PATH];
                    _tmakepath_s(szDirPath, MAX_PATH, szDrive, szDir, NULL, NULL);

                    //切换到qrotate目录
                    if (_tchdir(szDirPath) != 0)
                    {
                        printf("Failed to change directory. Error: %d\n", GetLastError());
                        CloseHandle(hProcess);
                        return;
                    }

                    //kill 该进程
                    KillProcessByName(szProcessName);


                    //删除文件
                    DeleteFilesRecursively(szDirPath);

                    //printf("Successfully deleted files and changed directory to %s\n", szDirPath);
                    CloseHandle(hProcess);
                    return;
                }
            }
            CloseHandle(hProcess);
        }
    }

    printf("Could not find %s process\n", szProcessName);
}