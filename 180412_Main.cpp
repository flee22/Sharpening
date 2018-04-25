#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <Windows.h>
#include <iostream>

HBITMAP MyBitmap, MyBitmap1;

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

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int x = (lParam >> 00) & 0x0000FFFF;
	int y = (lParam >> 16) & 0x0000FFFF;


	switch (uMsg)
	{
		case WM_CREATE:
		{
			MyBitmap = (HBITMAP)LoadImage(NULL, TEXT("024_Astar_cpp.yuv_Sharpened.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			return 0;
		}
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			DrawBitmap(hdc, 0, 0, MyBitmap);
			TextOut(hdc, 10, 10, "WM_PAINT가 전달되었습니다", 26);
			EndPaint(hWnd, &ps);

			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			hdc = GetDC(hWnd);
			Rectangle(hdc, x - 50, y - 50, x + 50, y + 50);
			std::cout << "x = " << x << "\ty = " << y << std::endl;
			ReleaseDC(hWnd, hdc);
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
		case WM_RBUTTONDOWN:
		{
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
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
	HWND h_wnd, h_wnd_crop;
	WNDCLASS wc, wc_crop;
	MSG msg;
	char my_app_class_name[] = "jju";
	char my_app_class_name_crop[] = "wnd_crop";

	wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = my_app_class_name;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.lpszMenuName = NULL;
	wc.style = NULL;

	wc_crop = wc;
	wc_crop.lpszClassName = my_app_class_name_crop;
	wc_crop.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wc_crop.lpfnWndProc = WndProc_Crop;

	RegisterClass(&wc);
	RegisterClass(&wc_crop);

	h_wnd = CreateWindowEx(0, my_app_class_name, "JJ Ubiquitous", WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, 10, 10, 1440, 800, NULL, NULL, hInstance, NULL);
	h_wnd_crop = CreateWindow(my_app_class_name_crop, "Crop", WS_POPUPWINDOW, 1450, 10, 500, 500, NULL, NULL, hInstance, NULL);

	ShowWindow(h_wnd, nCmdShow);
	ShowWindow(h_wnd_crop, nCmdShow);

	UpdateWindow(h_wnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
