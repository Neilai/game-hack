#include "pch.h"

#include <iostream>
#include<Windows.h>
#pragma comment(linker, "/SECTION:.text,ERW") 

//the original function 
void func() {
	MessageBox(0, L"not hook", L"info", MB_OK);
}
void hookproc() {
	MessageBox(0, L"hook success", L"info", MB_OK);
}
void hook_code() {
	BYTE *lpFunc1 = (BYTE*)func;
	lpFunc1[0] = 0x68;//machine code of push
	*(ULONG_PTR *)&lpFunc1[1] = (ULONG_PTR)hookproc;//hook function
	lpFunc1[5] = 0xC3;//machine code of retn
}
int main()
{
	hook_code();
	func();
	return 0;
}