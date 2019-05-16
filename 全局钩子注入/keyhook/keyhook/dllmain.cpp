// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include "stdio.h"
#include "windows.h"

#define DEF_PROCESS_NAME		"notepad.exe"

HINSTANCE g_hInstance = NULL;
HHOOK g_hHook = NULL;
HWND g_hWnd = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstance = hinstDLL;
		break;

	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{


	char szPath[MAX_PATH] = { 0, };
	char *p = NULL;

	if (nCode >= 0)
	{
		// bit 31 : 0 => press, 1 => release

		if (!(lParam & 0x80000000))
		{
			GetModuleFileNameA(NULL, szPath, MAX_PATH);
			p = strrchr(szPath, '\\');
			printf("%s\n", p + 1);
			
			if (!_stricmp(p + 1, DEF_PROCESS_NAME)) {
				return 1;
			}
		}
	}

	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) void HookStart()
	{

		printf("start");
		g_hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, g_hInstance, 0);

	}

	__declspec(dllexport) void HookStop()
	{
		if (g_hHook)
		{
			UnhookWindowsHookEx(g_hHook);
			g_hHook = NULL;
		}
	}
#ifdef __cplusplus
}
#endif