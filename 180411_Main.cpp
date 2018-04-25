#include <Windows.h>

HDC hdc;
PAINTSTRUCT ps;
HBITMAP MyBitmap;

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
	switch (uMsg)
	{
		case WM_CREATE:
		{
			MyBitmap = (HBITMAP)LoadImage(NULL, TEXT("024_Astar_cpp.yuv_Sharpened.bmp"), IMAGE_BITMAP, 0, 0,
				LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			return 0;
		}
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);

			DrawBitmap(hdc, 0, 0, MyBitmap);
			Rectangle(hdc, 50, 50, 220, 150);

			EndPaint(hWnd, &ps);

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


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND h_wnd;
	WNDCLASS wc;
	MSG msg;
	char my_app_class_name[] = "jju";

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

	RegisterClass(&wc);

	h_wnd = CreateWindow(my_app_class_name, "JJ Ubiquitous", WS_OVERLAPPEDWINDOW, 10, 10, 1920, 1080, NULL, NULL, hInstance, NULL);

	ShowWindow(h_wnd, nCmdShow);
	UpdateWindow(h_wnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
