// Can use Command prompt 
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console") 

#include <Windows.h>
#include <iostream>
#include "Resource.h"
#include "stdafx.h"

HDC hdc_buf;
HBITMAP MyBitmap;

int g_is_clicked = 0;
int g_x, g_y;

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

LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	switch (uMsg)
	{
		case WM_CREATE: // Excute just once when init
		{
			MyBitmap = (HBITMAP)LoadImage(NULL, TEXT("024_Astar_cpp.yuv_Sharpened.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			return 0;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;

			hdc = BeginPaint(hWnd, &ps);
			DrawBitmap(hdc, 0, 0, MyBitmap);
			EndPaint(hWnd, &ps);

			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			g_is_clicked = 1;
			g_x = (lParam >> 00) & 0x0000FFFF;
			g_y = (lParam >> 16) & 0x0000FFFF;
			return 0;
		}
		case WM_LBUTTONUP:
		{
			g_is_clicked = 0;
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			if (g_is_clicked == 1)
			{
				hdc = GetDC(hWnd);
				
				MoveToEx(hdc, g_x, g_y, NULL);
				int x = (lParam >> 00) & 0x0000FFFF;
				int y = (lParam >> 16) & 0x0000FFFF;
				LineTo(hdc, x, y);

				g_x = x;
				g_y = y;

				std::cout << "x = " << x << "\ty = " << y << std::endl;
				ReleaseDC(hWnd, hdc);
			}
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
	HWND wnd_main, wnd_crop;
	WNDCLASS wc_main, wc_crop;
	MSG msg;
	char class_name_main[] = "JJ_wnd_main";
	char class_name_crop[] = "JJ_wnd_crop";

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

	wc_crop = wc_main; // Copy wc_main -> wc_crop
	wc_crop.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wc_crop.lpfnWndProc = WndProc_Crop;
	wc_crop.lpszClassName = class_name_crop;

	RegisterClass(&wc_main);
	RegisterClass(&wc_crop);

	wnd_main = CreateWindow(class_name_main, "Sharpen - Main", WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, 10, 10, 1440, 800, NULL, NULL, hInstance, NULL);
	wnd_crop = CreateWindow(class_name_crop, "Sharpen - Crop", WS_POPUPWINDOW | WS_CAPTION, 1450, 40, 500, 500, NULL, NULL, hInstance, NULL);

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
