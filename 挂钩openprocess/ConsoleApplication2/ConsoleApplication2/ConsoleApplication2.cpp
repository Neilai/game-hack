// ConsoleApplication2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <windows.h>

HANDLE GetThePidOfTargetProcess(HWND hwnd)
{
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	HANDLE hProcee = ::OpenProcess(PROCESS_ALL_ACCESS | PROCESS_CREATE_THREAD, 0, pid);
	return hProcee;
}
//提升权限
void Up()
{
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tp;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
	AdjustTokenPrivileges(hToken, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
}

//进程注入

BOOL DoInjection(const char *DllPath, HANDLE hProcess)
{
	DWORD BufSize = strlen(DllPath) + 1;
	LPVOID AllocAddr = VirtualAllocEx(hProcess, NULL, BufSize, MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, AllocAddr, DllPath, BufSize, NULL);
	PTHREAD_START_ROUTINE pfnStartAddr = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryA");

	HANDLE hRemoteThread;
	hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, pfnStartAddr, AllocAddr, 0, NULL);
	if (hRemoteThread)
	{
		MessageBox(NULL, TEXT("注入成功"), TEXT("提示"), MB_OK);
		return true;
	}
	else
	{
		MessageBox(NULL, TEXT("注入失败"), TEXT("提示"), MB_OK);
		return false;
	}
}


int main()
{
	//这里填写窗口标题
	HWND hwnd = FindWindowExA(NULL, NULL, NULL, "任务管理器");
	Up();
	HANDLE hP = GetThePidOfTargetProcess(hwnd);
	//开始注入
	//这里填写Dll路径
	DoInjection("D:\\VS-project\\F2H1\\x64\\Debug\\F2H1.dll", hP);
}














