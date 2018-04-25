// Using this comment can use Command prompt 
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console") 

// Using for __imp__InitCommonControlsEx
#pragma comment(lib, "ComCtl32.lib")

#include <Windows.h>
#include <iostream>
#include "Resource.h"
#include "stdafx.h"

#include <CommCtrl.h> // for Init Status Bars

HWND wnd_main, wnd_crop, bar_main;
HBITMAP MyBitmap;

int g_is_clicked = 0;
int g_x_start, g_y_start, g_x_end, g_y_end;

void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	int bx, by;
	HDC MemDC;
	BITMAP bit;
	HBITMAP OldBitmap;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);
	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}

void CropBitmap(HDC hdc, int x_start, int y_start, int x_end, int y_end, HBITMAP hBit)
{

}

LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (uMsg)
	{
		case WM_CREATE: // Excute just once when init
		{
			SendMessage(bar_main, SB_SETPARTS, 3, lParam);
			SendMessage(bar_main, SB_SETTEXT, 0, (LPARAM)"111");
			SendMessage(bar_main, SB_SETTEXT, 1, (LPARAM)"222");

			MyBitmap = (HBITMAP)LoadImage(NULL, TEXT("024_Astar_cpp.yuv_Sharpened.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			return 0;
		}
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			DrawBitmap(hdc, 0, 0, MyBitmap);
			EndPaint(hWnd, &ps);

			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			g_is_clicked = 1;
			g_x_start = (lParam >> 00) & 0x0000FFFF;
			g_y_start = (lParam >> 16) & 0x0000FFFF;
			return 0;
		}
		case WM_LBUTTONUP:
		{
			g_is_clicked = 0;
			g_x_end = (lParam >> 00) & 0x0000FFFF;
			g_y_end = (lParam >> 16) & 0x0000FFFF;

			hdc = GetDC(hWnd);

			SelectObject(hdc, GetStockObject(NULL_BRUSH));
			Rectangle(hdc, g_x_start, g_y_start, g_x_end, g_y_end);

			ReleaseDC(hWnd, hdc);

			return 0;
		}
		case WM_MOUSEMOVE:
		{
			int x = (lParam >> 00) & 0x0000FFFF;
			int y = (lParam >> 16) & 0x0000FFFF;
			std::cout << "x = " << x << "\ty = " << y << std::endl;
			return 0;
		}
		case WM_VSCROLL:
		{
			return 0;
		}
		case WM_HSCROLL:
		{
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			return 0;
		}
		case WM_SIZE:
		{
			SendMessage(bar_main, WM_SIZE, 0, 0);
			return 0;
		}
		case WM_CLOSE:
		{
			int check = MessageBox(hWnd, "프로그램을 종료합니다", "알림", MB_ICONQUESTION | MB_OKCANCEL);
			if (check == IDOK)
			{
				PostQuitMessage(0);
			}
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_Crop(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wc_main, wc_crop;
	MSG msg;
	char class_name_main[] = "JJ_wnd_main";
	char class_name_crop[] = "JJ_wnd_crop";
	char bar_name_main[] = "";

	// Init Status Bar
	InitCommonControls();

	// Init Main Window 
	wc_main.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wc_main.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc_main.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc_main.hInstance = hInstance;
	wc_main.lpfnWndProc = WndProc_Main;
	wc_main.lpszClassName = class_name_main;
	wc_main.cbClsExtra = NULL;
	wc_main.cbWndExtra = NULL;
	wc_main.lpszMenuName = NULL;
	wc_main.style = NULL;

	// Init Crop Window 
	wc_crop = wc_main; // Copy wc_main -> wc_crop
	wc_crop.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wc_crop.lpfnWndProc = WndProc_Crop;
	wc_crop.lpszClassName = class_name_crop;

	RegisterClass(&wc_main);
	RegisterClass(&wc_crop);

	wnd_main = CreateWindow(class_name_main, "Sharpen - Main", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_HSCROLL | WS_VSCROLL, 10, 10, 1440, 800, NULL, NULL, hInstance, NULL);
	wnd_crop = CreateWindow(class_name_crop, "Sharpen - Crop", WS_POPUPWINDOW | WS_CAPTION, 1450, 40, 500, 500, NULL, NULL, hInstance, NULL);

	// StatusBar Window
	bar_main = CreateStatusWindow( WS_CHILD | WS_VISIBLE, bar_name_main, wnd_main, 0);
	
	ShowWindow(wnd_main, nCmdShow);
	ShowWindow(wnd_crop, nCmdShow);

	UpdateWindow(wnd_main);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
