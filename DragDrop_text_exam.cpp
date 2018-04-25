#include <Windows.h>
#include <ShellAPI.h>
#include <string.h>
#include <stdio.h>
#define SIZE 500

#define IDC_EDIT 1
#define IDC_LIST 2

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);


int WINAPI WinMain(HINSTANCE hThisInstance,	HINSTANCE hPrevInstance,	LPSTR lpszArgument,	int nCmdShow)
{

	HWND hwnd;               /* This is the handle for our window */
	MSG messages;            /* Here messages to the application are saved */
	WNDCLASSEX wincl;        /* Data structure for the windowclass */

							 /* The Window structure */
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = "Drag and Drop";
	wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
	wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
	wincl.cbSize = sizeof(WNDCLASSEX);

	/* Use default icon and mouse-pointer */
	wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = "APP_MENU";                 /*menu */
	wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
	wincl.cbWndExtra = 0;                      /* structure or the window instance */
											   /* Use Windows's default colour as the background of the window */
	wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	/* Register the window class, and if it fails quit the program */
	if (!RegisterClassEx(&wincl))
		return 0;

	/* The class is registered, let's create the program*/
	hwnd = CreateWindowEx(
		0,                   /* Extended possibilites for variation */
		"Drag and Drop",         /* Classname */
		"Drag and Drop",       /* Title Text */
		WS_OVERLAPPEDWINDOW | WS_EX_ACCEPTFILES, /* default window */
		CW_USEDEFAULT,       /* Windows decides the position */
		CW_USEDEFAULT,       /* where the window ends up on the screen */
		600,                 /* The programs width */
		600,                 /* and height in pixels */
		HWND_DESKTOP,        /* The window is a child-window to desktop */
		NULL,                /*use class menu */
		hThisInstance,       /* Program Instance handler */
		NULL                 /* No Window Creation data */
	);

	/* Make the window visible on the screen */
	ShowWindow(hwnd, nCmdShow);

	/* Run the message loop. It will run until GetMessage() returns 0 */
	while (GetMessage(&messages, NULL, 0, 0))
	{
		/* Translate virtual-key messages into character messages */
		TranslateMessage(&messages);
		/* Send message to WindowProcedure */
		DispatchMessage(&messages);
	}

	/* The program return-value is 0 - The value that PostQuitMessage() gave */
	return messages.wParam;
}


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	static HWND EditBox;
	static HWND ListBox;
	char *buffer;
	char* outText;
	HDROP hDropInfo = NULL;
	UINT buffsize = SIZE;
	char buf[SIZE];




	switch (message)                  /* handle the messages */
	{
	case WM_CREATE:

		ListBox = CreateWindowEx(0, "LISTBOX", TEXT(""), WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | LBS_HASSTRINGS, 10, 10, 500, 300, hwnd, (HMENU)IDC_LIST, ((LPCREATESTRUCT)lParam)->hInstance, (LPVOID)NULL);
		EditBox = CreateWindowEx(0, "EDIT", TEXT(""), WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_WANTRETURN, 10, 310, 500, 240, hwnd, (HMENU)IDC_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, (LPVOID)NULL);
		DragAcceptFiles(ListBox, TRUE);
		return 0;

	case WM_DROPFILES:
	{
		hDropInfo = (HDROP)wParam;

		DragQueryFile(hDropInfo, 0, buf, buffsize);





		FILE *pFile;
		char* pch = strstr(buf, ".txt");
		if (pch != NULL)
		{
			pFile = fopen(buf, "r");
			if (pFile != NULL)
			{



				// obtain file size:
				fseek(pFile, 0, SEEK_END);
				long lSize = ftell(pFile);
				rewind(pFile);
				// allocate memory to contain the buffers:
				buffer = (char*)malloc(sizeof(char)*lSize);
				outText = (char*)malloc(sizeof(char)*lSize * 2);
				ZeroMemory(buffer, lSize);
				ZeroMemory(outText, lSize);
				// copy the file into the buffers:
				while (fgets(buffer, lSize, pFile) != NULL)
				{
					strcat(outText, buffer);
					strcat(outText, "\r");

				}

				SendMessage(ListBox, LB_ADDSTRING, 0, (LPARAM)buf);
				HFONT hFont = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
					CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Arial Bold"));

				SendMessage(EditBox,             // Handle of edit control
					WM_SETFONT,         // Message to change the font
					(WPARAM)hFont,     // handle of the font
					MAKELPARAM(TRUE, 0) // Redraw text
				);
				SendMessage(EditBox, WM_SETTEXT, 0, (LPARAM)outText);
				free(buffer);
				free(outText);

			}
			fclose(pFile);
		}
		else
			MessageBox(hwnd, "Not a Text File", "Drop Error", MB_OK);








		break;
	}
	case WM_DESTROY:
		free(hDropInfo);
		PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
		break;




	default:                      /* for messages that we don't deal with */
		return DefWindowProc(hwnd, message, wParam, lParam);

	}
	return 0;
}
