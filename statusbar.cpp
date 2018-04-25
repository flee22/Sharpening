#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#define IDI_ICON      101
#define IDC_STATUSBAR 1000
const char ClassName[] = "MainWindowClass";
HWND hWndStatusBar;

LRESULT CALLBACK WndProc(HWND hWnd,	UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_CREATE:
	{
		hWndStatusBar = CreateWindowEx(
			0,
			STATUSCLASSNAME,
			NULL,
			WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
			0,
			0,
			0,
			0,
			hWnd,
			(HMENU)IDC_STATUSBAR,
			(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
			NULL);

		if (!hWndStatusBar)
		{
			MessageBox(NULL, "Failed To Create The Status Bar", "Error", MB_OK | MB_ICONERROR);
			return 0;
		}

		int iStatusWidths[] = { 100, 200, -1 };
		char text[256];

		SendMessage(hWndStatusBar, SB_SETPARTS, 3, (LPARAM)iStatusWidths);
		SendMessage(hWndStatusBar, SB_SETTEXT, 0, (LPARAM)"Status Bar");
		SendMessage(hWndStatusBar, SB_SETTEXT, 1, (LPARAM)"Cells");
		sprintf(text, "%d", 3);
		SendMessage(hWndStatusBar, SB_SETTEXT, 2, (LPARAM)text);
		ShowWindow(hWndStatusBar, SW_SHOW);
	}
	break;

	case WM_SIZE:
		SendMessage(hWndStatusBar, WM_SIZE, 0, 0);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return (DefWindowProc(hWnd, Msg, wParam, lParam));
	}

	return 0;
}

INT WINAPI WinMain(HINSTANCE  hInstance, HINSTANCE  hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
//	InitCommonControls();

	WNDCLASSEX    wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = ClassName;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	HWND    hWnd;

	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		ClassName,
		"Status Bar",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		240,
		120,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hWnd)
	{
		MessageBox(NULL, "Window Creation Failed.", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG    Msg;

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}