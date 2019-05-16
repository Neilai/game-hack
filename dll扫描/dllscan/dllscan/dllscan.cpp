#include "pch.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <map>


#include <windows.h>
#include <TlHelp32.h>

using namespace std;

bool traverseProcesses(std::map<std::string, int>& _nameID)
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);

	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		std::cout << "CreateToolhelp32Snapshot Error!" << std::endl;;
		return false;
	}

	BOOL bResult = Process32First(hProcessSnap, &pe32);

	int num(0);

	while (bResult)
	{
		std::string name = pe32.szExeFile;
		int id = pe32.th32ProcessID;
		_nameID.insert(std::pair<string, int>(name, id)); //字典存储
		bResult = Process32Next(hProcessSnap, &pe32);
	}

	CloseHandle(hProcessSnap);

	return true;
}

bool traverseModels(const std::string _name)
{
	DWORD dwId;

	/*printf("Please enter the name of process to traverse processmodels:");
	std::string name;
	cin >> name;*/

	std::map<std::string, int> nameID;

	if (!traverseProcesses(nameID)) { //变量进程
		cout << "Print Processes Error!" << endl;
	}

	dwId = nameID[_name];

	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwId);
	if (hModuleSnap == INVALID_HANDLE_VALUE) {
		printf("CreateToolhelp32SnapshotError! \n");
		return false;
	}

	MODULEENTRY32 module32;
	module32.dwSize = sizeof(module32);
	BOOL bResult = Module32First(hModuleSnap, &module32);

	int num(0);

	while (bResult) {
		/*std::wcout << "[" << num++ << "] : " << "Module:" << std::left << std::setw(25)
			<< module32.szModule << "  " << endl << "Path:" << module32.szExePath << std::endl;*/
		std::wcout << "[" << num++ << "] : " << "Module:" << module32.szModule << "  " << endl;
		bResult = Module32Next(hModuleSnap, &module32);
	}

	CloseHandle(hModuleSnap);

	return true;

}

int main()
{
	const std::string program("cstrike.exe");

	if (!traverseModels(program)) {
		cout << "Traverse Models Error!" << endl;
	}
	system("pause");
	return 0;
}