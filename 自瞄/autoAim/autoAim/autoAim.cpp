// autoAim.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include"windows.h"
#include <TlHelp32.h>
#include<iomanip>
#include <algorithm>
#include"math.h"
using namespace std;
DWORD GetProcessID(wchar_t *FileName)
{
	HANDLE hProcess;
	PROCESSENTRY32 pe;
	BOOL bRet;
	//进行进程快照
	hProcess = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32W);
	//开始进程查找
	bRet = ::Process32First(hProcess, &pe);
	//循环比较，得出ProcessID
	while (bRet)
	{
		if (wcscmp(FileName, pe.szExeFile) == 0)
			return pe.th32ProcessID;
		else
			bRet = ::Process32Next(hProcess, &pe);
	}
	return 0;
}
int num;
HANDLE GameHandle;
DWORD Buffer = 0;
struct TargetList_t
{
	float Distance;//距离
	float AimbotAngle[3];//自瞄角度

	TargetList_t()
	{
	}

	TargetList_t(float aimbotAngle[], float myCoords[], float enemyCoords[])
	{
		//获取敌人到我的距离
		Distance = Get3dDistance(myCoords[0], myCoords[1], myCoords[2],
			enemyCoords[0], enemyCoords[1], enemyCoords[2]);

		//角度的赋值
		AimbotAngle[0] = aimbotAngle[0];
		AimbotAngle[1] = aimbotAngle[1];
		AimbotAngle[2] = aimbotAngle[2];
	}
	//D3D距离获取
	float Get3dDistance(float myCoordsX, float myCoordsZ, float myCoordsY,
		float enX, float enZ, float enY)
	{
		return sqrt(
			pow(double(enX - myCoordsX), 2.0) +
			pow(double(enY - myCoordsY), 2.0) +
			pow(double(enZ - myCoordsZ), 2.0));
	}
};
void CalcAngle(float *src, float *dst, float *angles)
{
	double delta[3] = { (src[0] - dst[0]), (src[1] - dst[1]), (src[2] - dst[2]) };
	double hyp = sqrt(delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2]);
	angles[0] = (float)((asinf(delta[2] / hyp)) * 57.295779513082f);
	angles[1] = (float)((atanf(delta[1] / delta[0])) * 57.295779513082f);
	angles[2] = 0.0f;

	if (delta[0] >= 0.0)
	{
		angles[1] += 180.0f;
	}
}
struct MyPlayer_t
{
	DWORD CLocalPlayer;
	int Team;
	int Health;
	float Position[3];
	void ReadInformation()
	{
		getNum();
		ReadProcessMemory(GameHandle, LPVOID(0x21C2FC0), &Buffer, sizeof(DWORD), NULL);
		DWORD Base = Buffer + (0x324);
		ReadProcessMemory(GameHandle, LPVOID(Base + (0x88)), &Position, sizeof(float[3]), NULL);
		ReadProcessMemory(GameHandle, LPVOID(Base + (0x1E0)), &Health, sizeof(Health), NULL);
		//        cout<<Position[0]<<"  "<<Position[1]<<"  "<<Position[2]<<endl;
	}
	void getNum() {
		cout << "!";
		ReadProcessMemory(GameHandle, LPVOID(0x19F73DC), &Buffer, sizeof(DWORD), NULL);
		DWORD Base = Buffer - 0x704;
		ReadProcessMemory(GameHandle, LPVOID(Base), &Buffer, sizeof(DWORD), NULL);
		Base = Buffer + 0x8;
		ReadProcessMemory(GameHandle, LPVOID(Base), &Buffer, sizeof(DWORD), NULL);
		Base = Buffer + 0xE0;
		ReadProcessMemory(GameHandle, LPVOID(Base), &num, sizeof(int), NULL);
	}
}MyPlayer;

struct PlayerList_t
{
	DWORD CBaseEntity;
	int Team;
	int Health;
	float Position[3];
	float AimbotAngle[3];
	char Name[39];

	void ReadInformation(int Player)
	{
		ReadProcessMemory(GameHandle, LPVOID(0x21C2FC0), &Buffer, sizeof(DWORD), NULL);
		DWORD Base = Buffer + (0x324)*Player;
		ReadProcessMemory(GameHandle, LPVOID(Base + (0x88)), &Position, sizeof(float[3]), NULL);
		ReadProcessMemory(GameHandle, LPVOID(Base + (0x1E0)), &Health, sizeof(Health), NULL);
	}
}PlayerList[32];


struct CompaerTatgetEnArry
{
	bool operator()(TargetList_t & lhs, TargetList_t & rhs)
	{
		return lhs.Distance < rhs.Distance;
	}
};
void Aimbot()
{
	TargetList_t *TargetList = new TargetList_t[num];//因为每个人都要进行距离转换，角度获取，对象数组

	int targetLoop = 0;

	for (int i = 0; i < (num - 1); i++)
	{
		//读出敌人数据
		PlayerList[i].ReadInformation(i + 2);

		//判断阵营-排除队友
//		if (PlayerList[i].Team == MyPlayer.Team)
//			continue;
		//判断是否死亡
		if (PlayerList[i].Health < 2)
			continue;
		//排除一些不应该被瞄准的对象

		CalcAngle(MyPlayer.Position, PlayerList[i].Position, PlayerList[i].AimbotAngle);//通过坐标转换，返回该敌人队友我的自瞄角度

		TargetList[targetLoop] = TargetList_t(PlayerList[i].AimbotAngle, MyPlayer.Position, PlayerList[i].Position);

		targetLoop++;
	}
	//检测到敌人
	if (targetLoop > 0)
	{
		//排序-》相邻敌人对象进行距离的排序
		sort(TargetList, TargetList + targetLoop, CompaerTatgetEnArry());

		//鼠标右键-》不自瞄
		if (!GetAsyncKeyState(VK_RBUTTON))
		{
			cout << "启动";
			//更改鼠标坐标
			WriteProcessMemory(GameHandle, LPVOID(0x2712B04), TargetList[0].AimbotAngle, 12, 0);

		}
	}

	//循环检测是否是敌人和队友
	targetLoop = 0;

	//删除对象指针
	delete[] TargetList;
}
int main() {
	GameHandle = OpenProcess(PROCESS_ALL_ACCESS, 1, GetProcessID(L"cstrike.exe"));
	while (1) {
		MyPlayer.ReadInformation();
		Aimbot();
		cout << "num" << num << endl;
		//        Sleep(100);
	}
}
