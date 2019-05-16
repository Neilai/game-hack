// CRC.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include<cstdio>
#include<Windows.h>
using namespace std;
#pragma pack(1)
typedef struct __STBINGLEPARAM
{
	HANDLE hEvent;  //用于同步的一个信号量
	DWORD dwStart;  //要校验的代码的起始地址
	DWORD dwEnd;   //要校验的代码的终结地址
}STBINGLEPARAM, *PBINGLEPARAM;
#pragma pack()

DWORD xxx;
DWORD CRC32(BYTE* ptr, DWORD Size)
{

	DWORD crcTable[256], crcTmp1;

	//动态生成CRC-32表  
	for (int i = 0; i < 256; i++)
	{
		crcTmp1 = i;
		for (int j = 8; j > 0; j--)
		{
			if (crcTmp1 & 1) crcTmp1 = (crcTmp1 >> 1) ^ 0xEDB88320L;
			else crcTmp1 >>= 1;
		}

		crcTable[i] = crcTmp1;
	}
	//计算CRC32值  
	DWORD crcTmp2 = 0xFFFFFFFF;
	while (Size--)
	{
		crcTmp2 = ((crcTmp2 >> 8) & 0x00FFFFFF) ^ crcTable[(crcTmp2 ^ (*ptr)) & 0xFF];
		ptr++;
	}

	return(crcTmp2 ^ 0xFFFFFFFF);
}
DWORD WINAPI bingleProc(LPVOID lpParameter)
{
	STBINGLEPARAM *stParam = (STBINGLEPARAM *)lpParameter;

	DWORD dwCodeSize = stParam->dwEnd - stParam->dwStart;
	BYTE *pbyteBuf = NULL;
	pbyteBuf = (BYTE *)stParam->dwStart;

	DWORD dwOldProtect = 0;
	VirtualProtect((LPVOID)stParam->dwStart, 4 * 1024, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	if (CRC32(pbyteBuf, dwCodeSize) != xxx)
	{
		MessageBox(NULL, L"代码被修改了", L"警告", NULL);
		printf("代码被修改了\n");

		SetEvent(stParam->hEvent);
		ExitProcess(0);
	}

	SetEvent(stParam->hEvent); //执行完上边的代码开始传信，让主线程继续运行

	return 0;
}
int main()
{	
ProtectStart:   //要保护的代码的起始地址
	__asm
	{
		inc eax  //花指令
		dec eax
		push eax
		pop eax
	}
	HMODULE hMod = GetModuleHandle(NULL);//同样是花指令
	HMODULE hUser32 = LoadLibrary(L"user32.dll");
ProtectEnd:               //要保护代码的终结地址
	DWORD dwThreadId = 0;

	STBINGLEPARAM stParam = { 0 };
	stParam.hEvent = CreateEvent(NULL, FALSE, FALSE, L"bingle");

	DWORD dwAddr = 0;  //一个缓存空间
	__asm mov eax, offset ProtectStart  //计算代码的起始地址
	__asm mov dwAddr, eax
	stParam.dwStart = dwAddr; //保存在我们自己定义的结构体里

	__asm mov eax, offset ProtectEnd //计算保护代码的结束地址，同样保存在自己定义的结构体里。
	__asm mov dwAddr, eax
	stParam.dwEnd = dwAddr;

	printf("开始了\n");

	DWORD dwCodeSize = stParam.dwEnd - stParam.dwStart;
	BYTE *pbyteBuf = NULL;
	pbyteBuf = (BYTE *)stParam.dwStart;
	DWORD dwOldProtect = 0;
	VirtualProtect((LPVOID)stParam.dwStart, 4 * 1024, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	xxx = CRC32(pbyteBuf, dwCodeSize);
	
	
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)bingleProc, (LPVOID)&stParam, 0, &dwThreadId);
	DWORD dwRet = 0;
	dwRet = WaitForSingleObject(stParam.hEvent, INFINITE);
	while (dwRet == WAIT_OBJECT_0)
	{
		Sleep(5000);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)bingleProc, (LPVOID)&stParam, 0, &dwThreadId);
		dwRet = WaitForSingleObject(stParam.hEvent, INFINITE);
	}

}
