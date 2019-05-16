// esp.cpp : 定义应用程序的入口点。
//
#include "stdafx.h"
#include "esp.h"
#include <d3dx9.h>
#include <tchar.h>
#include <TLHelp32.h>
#include <d3d9.h>
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib,"d3d9.lib")
#define TOP_HEIGHT 25


int num;
HANDLE GameHandle;
DWORD Buffer = 0;
RECT gameRect;
HWND game_hWnd = FindWindow(NULL, (L"Counter-Strike"));
HWND transparent_hWnd; //创建用来接收透明窗口的句柄

//定义测试变量
float x_mouse = 0;
float y_mouse = 0;
float x_pos = 0;
float y_pos = 0;

DWORD X_Address = 0x02712B08;
DWORD Y_Address = 0x02712B04;
float distance1 = 0;
float X = 0;
float Y = 0;
float width = 0;
float height = 0;
float value = 0;

IDirect3D9Ex* g_pD3D = NULL; //d3d对象
IDirect3DDevice9Ex* g_pd3dDevice = NULL; //d3d指针
ID3DXLine* g_pLine = NULL;  //d3d画线对象


struct MyPlayer_t
{
	DWORD CLocalPlayer;
	int Team;
	float Health;
	float Position[3];
	float x, y, z;
	void ReadInformation()
	{
		getNum();
		ReadProcessMemory(GameHandle, LPVOID(0x21C2FC0), &Buffer, sizeof(DWORD), NULL);
		DWORD Base = Buffer + (0x324);
		ReadProcessMemory(GameHandle, LPVOID(Base + (0x88)), &Position, sizeof(float[3]), NULL);
		ReadProcessMemory(GameHandle, LPVOID(Base + (0x1E0)), &Health, sizeof(Health), NULL);
		x = Position[0];
		y = Position[1];
		z = Position[2];
	}
	void getNum() {
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
	float Health;
	float Position[3];
	float AimbotAngle[3];
	char Name[39];
	float x, y, z;
	void ReadInformation(int Player)
	{
		ReadProcessMemory(GameHandle, LPVOID(0x21C2FC0), &Buffer, sizeof(DWORD), NULL);
		DWORD Base = Buffer + (0x324)*Player;
		ReadProcessMemory(GameHandle, LPVOID(Base + (0x88)), &Position, sizeof(float[3]), NULL);
		ReadProcessMemory(GameHandle, LPVOID(Base + (0x1E0)), &Health, sizeof(Health), NULL);
		x = Position[0];
		y = Position[1];
		z = Position[2];
	}
}PlayerList[32];
bool D3D_Create(HWND hWnd) //传入透明窗口句柄用来创建和绑定d3d设备
{
	Direct3DCreate9Ex(D3D_SDK_VERSION, &g_pD3D); //创建d3d对象
	D3DPRESENT_PARAMETERS d3d; //设置d3d属性的结构体
	ZeroMemory(&d3d, sizeof(D3DPRESENT_PARAMETERS));

	d3d.Windowed = TRUE;
	d3d.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3d.BackBufferFormat = D3DFMT_UNKNOWN;
	d3d.BackBufferCount = 3;
	d3d.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	//创建d3d设备对象
	if (FAILED(g_pD3D->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, transparent_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3d, 0, &g_pd3dDevice)))
		exit(1);

	//创建d3d线对象
	if (FAILED(D3DXCreateLine(g_pd3dDevice, &g_pLine)))
		exit(1);

	return TRUE;
}

DWORD GetProcessID(const wchar_t *FileName)
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

void draw_Line(float x1, float y1, float x2, float y2, D3DCOLOR color)
{
	D3DXVECTOR2 dLine[2];
	dLine[0].x = x1;
	dLine[0].y = y1;
	dLine[1].x = x2;
	dLine[1].y = y2;

	g_pLine->Draw(dLine, 2, color);
}

void drawEsp(float Rect_x, float Rect_y, float Rect_w, float Rect_h, D3DCOLOR color)
{
	Rect_x = Rect_x - value;
	Rect_y = Rect_y - value;
	draw_Line(Rect_x, Rect_y, Rect_x + Rect_w, Rect_y, color);
	draw_Line(Rect_x + Rect_w, Rect_y, Rect_x + Rect_w, Rect_y +Rect_h , color);
	draw_Line(Rect_x + Rect_w, Rect_y+Rect_h, Rect_x, Rect_y + Rect_h, color);
	draw_Line(Rect_x , Rect_y + Rect_h, Rect_x, Rect_y, color);
}



void D3D_Paint()
{
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
	if (SUCCEEDED(g_pd3dDevice->BeginScene())){
		g_pLine->SetWidth(3.f);
		g_pLine->SetAntialias(TRUE);
		for (int i = 0; i < num-1; i++){
			if (PlayerList[i].Health > 2){
				distance1 = sqrt((PlayerList[i].y - MyPlayer.y)*(PlayerList[i].y - MyPlayer.y) + (PlayerList[i].x - MyPlayer.x)*(PlayerList[i].x - MyPlayer.x) + (PlayerList[i].z - MyPlayer.z)*(PlayerList[i].z - MyPlayer.z));
				if ((PlayerList[i].x - MyPlayer.x) >= 0){
					x_pos = atanf((PlayerList[i].y - MyPlayer.y) / (PlayerList[i].x - MyPlayer.x)) * 180.0f / 3.1415926;
				}
				else{
					x_pos = atanf((PlayerList[i].y - MyPlayer.y) / (PlayerList[i].x - MyPlayer.x)) * 180.0f / 3.1415926 + 180.0f;
				}
				y_pos = -asinf((PlayerList[i].z - MyPlayer.z) / distance1) * 180.0f / 3.1415926;
				ReadProcessMemory(GameHandle, LPVOID(X_Address), &x_mouse, sizeof(DWORD), NULL);
				ReadProcessMemory(GameHandle, LPVOID(Y_Address), &y_mouse, sizeof(DWORD), NULL);
				//x_mouse判断
				X = tan(((x_mouse - x_pos) *3.1415926) / 180) * 323 + 323;
				Y = (tan((-y_mouse * 3.1415926) / 180) + tan((y_pos* 3.1415926) / 180)) * 323 + 254;

				width = 12200 / distance1;
				height = 22000 / distance1;
			    value = 4213 / distance1;
				if (x_mouse > x_pos - 45 && x_mouse < x_pos + 45 && y_mouse <45 && y_mouse >-30)
				{
					drawEsp(X, Y, width, height, D3DCOLOR_XRGB(55, 255, 155));
				}
			}
			else
			{
				continue;
			}
		}
		//draw_Line(10, 10, 300, 300);
		//结束在后台缓冲区渲染图形
		g_pd3dDevice->EndScene();
	}
	//将在后台绘制的图形提交到前台缓冲区显示
	g_pd3dDevice->PresentEx(NULL, NULL, NULL, NULL, NULL);

}


void D3D_Release()
{
	if (g_pLine != NULL)
		g_pLine->Release();
	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();
	if (g_pD3D != NULL)
		g_pD3D->Release();
}



void readInformation() {
	GameHandle = OpenProcess(PROCESS_ALL_ACCESS, 1, GetProcessID(L"cstrike.exe"));
	MyPlayer.ReadInformation();
	for (int i = 0; i < (num - 1); i++) 	PlayerList[i].ReadInformation(i + 2);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		readInformation();
		D3D_Paint();
		ValidateRect(transparent_hWnd, NULL);
		break;
	case WM_CREATE:
		break;
	case WM_DESTROY:
		D3D_Release(); //释放D3D
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);

}

BOOL isForeGround()
{
	HWND hWnd = GetForegroundWindow();
	if (hWnd == game_hWnd)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX win;
	ZeroMemory(&win, sizeof(WNDCLASSEX));
	win.cbSize = sizeof(WNDCLASSEX);
	win.lpfnWndProc = (WNDPROC)WndProc;
	win.style = CS_VREDRAW | CS_HREDRAW;
	win.hInstance = hInstance;
	win.hCursor = LoadCursor(NULL, IDC_ARROW);
	win.hbrBackground = RGB(0, 0, 0);
	win.lpszClassName = L" ";
	win.hIcon = win.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&win);

	if (GetWindowRect(game_hWnd, &gameRect) == FALSE)
	{
		MessageBox(NULL, L"游戏未开启", L"NO", MB_OK);
		exit(1);
	}


	transparent_hWnd = CreateWindowEx(WS_EX_TRANSPARENT | WS_EX_LAYERED,L" ",L" ", WS_POPUP, gameRect.left, gameRect.top, (gameRect.right - gameRect.left), (gameRect.bottom - gameRect.top), NULL, NULL, hInstance, NULL);

    SetLayeredWindowAttributes(transparent_hWnd, RGB(0, 0, 0),0, ULW_COLORKEY);
	//初始化D3D
	if (SUCCEEDED(D3D_Create(transparent_hWnd)))
	{
		ShowWindow(transparent_hWnd, nCmdShow);
		UpdateWindow(transparent_hWnd);
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));



		while (msg.message != WM_QUIT)
		{
			readInformation();
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (isForeGround() == TRUE)
			{
				SetWindowPos(transparent_hWnd, HWND_TOPMOST, gameRect.left, gameRect.top,
					(gameRect.right - gameRect.left), (gameRect.bottom - gameRect.top), SWP_SHOWWINDOW);
			}
			else
			{
				SetWindowPos(transparent_hWnd, HWND_BOTTOM, gameRect.left, gameRect.top,
					(gameRect.right - gameRect.left), (gameRect.bottom - gameRect.top), SWP_SHOWWINDOW);
			}
		
			D3D_Paint();
			ValidateRect(transparent_hWnd, NULL);
			GetWindowRect(game_hWnd, &gameRect);
			gameRect.top += TOP_HEIGHT;
			MoveWindow(transparent_hWnd, gameRect.left, gameRect.top, (gameRect.right - gameRect.left),
				(gameRect.bottom - gameRect.top), TRUE);
		}
	}



	return 0;
}