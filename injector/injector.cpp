
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
using namespace std;

HANDLE GetProcessByName(LPCWSTR name)
{
    DWORD pid = 0;

    // Create toolhelp snapshot.
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);

    // Walkthrough all processes.
    if (Process32First(snapshot, &process))
    {
        do
        {
            // Compare process.szExeFile based on format of name, i.e., trim file path
            // trim .exe if necessary, etc.
            if (lstrcmp(process.szExeFile, name) == 0)
            {
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);

    if (pid != 0)
    {
        return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    }

    // Not found
    return NULL;
}

std::string GetExecutableDir()
{
    char result[MAX_PATH] = { 0 };
    string executableLocation = string(result, GetModuleFileNameA(NULL, result, MAX_PATH));
    size_t lastIndex = executableLocation.find_last_of("/\\");
    return executableLocation.substr(0, lastIndex + 1);
}

int main()
{
    // path to our dll
    string executableDir = GetExecutableDir();
    executableDir.append("client.dll");

    LPCSTR DllPath = executableDir.c_str();
    printf("Target DLL identified at: %s\n", DllPath);

    // Open a handle to target process
    HANDLE hProcess = GetProcessByName(L"WowClassic.exe");

    if (hProcess == NULL) {
        printf("Failed to find target process\n");
        cin.get();
        exit(-1);
    }

    // Allocate memory for the dllpath in the target process
    // length of the path string + null terminator
    LPVOID pDllPath = VirtualAllocEx(hProcess, 0, strlen(DllPath) + 1,
        MEM_COMMIT, PAGE_READWRITE);

    if (!pDllPath)
    {
        printf("Failed to allocate memory for Dll path in target process\n");
        cin.get();
        exit(-1);
    }

    // Write the path to the address of the memory we just allocated
    // in the target process
    if (!WriteProcessMemory(hProcess, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, 0))
    {
        printf("Failed to write Dll Path to newly allocated memory\n");
        cin.get();
        exit(-1);
    }


    HMODULE kernelModule = GetModuleHandleA("Kernel32.dll");
    if (!kernelModule)
    {
        printf("Failed to find Kernel32.dll module\n");
        exit(-1);
    }

    LPTHREAD_START_ROUTINE loadLibraryA = (LPTHREAD_START_ROUTINE)GetProcAddress(kernelModule, "LoadLibraryA");
    if (!loadLibraryA)
    {
        printf("Failed to get proc address of LoadLibraryA from Kernel32.dll module\n");
        exit(-1);
    }

    // Create a Remote Thread in the target process which
    // calls LoadLibraryA as our dllpath as an argument -> program loads our dll
    HANDLE hLoadThread = CreateRemoteThread(hProcess, 0, 0, loadLibraryA, pDllPath, 0, 0);
    if (!hLoadThread)
    {
        printf("Failed to create remote thread in target process\n");
        cin.get();
        exit(-1);
    }

    // Wait for the execution of our loader thread to finish
    WaitForSingleObject(hLoadThread, INFINITE);

    std::cout << "Dll path allocated at: " << std::hex << pDllPath << std::endl;

    // Free the memory allocated for our dll path
    if (!VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE))
    {
        printf("Failed to free memory allocated for dll path\n");
        cin.get();
        exit(-1);
    }

    return 0;
}