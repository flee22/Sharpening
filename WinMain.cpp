#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console") // Using this comment can use Command prompt 
#pragma comment(lib, "ComCtl32.lib") // Using for __imp__InitCommonControlsEx

#include "Resource.h"
#include "stdafx.h" // include Windows.h
#include <shellapi.h> // for Drag and Drop files
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <CommCtrl.h> // for Init Status Bars

using namespace std;

#define IDC_LIST 2
#define iWidth 4608
#define iHeight 3456

#pragma pack(push, 2)	// Avoid Memory Padding
struct BmpFileHeader {
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;

	BmpFileHeader()
	{
		bfType = 'MB';
		bfSize = iWidth * iHeight * 3 + 54; // BMP File Total Size
		bfReserved1 = 0;
		bfReserved2 = 0;
		bfOffBits = 54; // File + Info Header Size
	}
};
struct BmpInfoHeader {
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD bfSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;

	BmpInfoHeader()
	{
		biSize = 40; // Info Header Size
		biWidth = iWidth;
		biHeight = iHeight;
		biPlanes = 1;
		biBitCount = 24;	// RGB full color (24bit)
		biCompression = 0;
		bfSizeImage = 0;
		biXPelsPerMeter = 0;
		biYPelsPerMeter = 0;
		biClrUsed = 0;
		biClrImportant = 1;
	}
};
#pragma pack(pop)

int g_is_clicked = 0;
int g_Width_s, g_Width_e, g_Height_s, g_Height_e;
int IMG_SIZE_Y = iWidth * iHeight;
int IMG_SIZE_YCrCb = IMG_SIZE_Y + (IMG_SIZE_Y >> 1); // Full Size = Y size + CrCb size
int CROP_SIZE = (g_Height_e - g_Height_s) * (g_Width_e - g_Width_s);
int CROP_H = g_Height_e - g_Height_s;
int CROP_W = g_Width_e - g_Width_s;

int g_argc;
char g_argv[MAX_PATH];

void Convert_yvu2rgb(int * yvu_array, int width, int height, unsigned char * rgb_array)
{
	int i, j, k;
	int *y, *u, *v, *r, *g, *b, *r_tmp, *g_tmp, *b_tmp;
	int size_y = width * height;
	int size_vu = size_y >> 2;
	int size_yvu = (int)(size_y * 1.5);

	y = new int[size_y];
	v = new int[size_vu];
	u = new int[size_vu];
	r = new int[size_y];
	g = new int[size_y];
	b = new int[size_y];
	r_tmp = new int[size_y];
	g_tmp = new int[size_y];
	b_tmp = new int[size_y];

	// Seperate a YVU array to each Y, V, U arrays
	j = 0;
	for (i = 0; i < size_y; i++)
	{
		y[i] = yvu_array[i];
	}
	for (i = size_y; i < size_yvu; i = i + 2)
	{
		if (yvu_array[i + 0] < 0)	v[j] = yvu_array[i + 0] + 128;
		else						v[j] = yvu_array[i + 0] - 128;
		if (yvu_array[i + 1] < 0)	u[j] = yvu_array[i + 1] + 128;
		else						u[j] = yvu_array[i + 1] - 128;
		j++;
	}

	// YVU to RGB
	k = 0;
	for (j = 0; j < height; j = j + 2)
	{
		for (i = 0; i < width; i = i + 2)
		{
			/* 4 Y pixels pares 1 VU pixels = YUV420 NV21 format
			|-------------------|
			|                   |
			| Y[0000] | Y[0001] |
			|                   |
			|-------------------|
			|                   |
			| Y[4096] | Y[4097] |
			|                   |
			|-------------------| Y Planar
			|-------------------|
			|                   |
			| V[0000] | U[0000] |
			|                   |
			|-------------------|
			|                   |
			| V[0000] | U[0000] |
			|                   |
			|-------------------| VU(CrCb) Interleaved
			*/
			r[((j + 0) * width) + i + 0] = (int)(y[((j + 0) * width) + i + 0] + (1.4065 * v[k]));
			g[((j + 0) * width) + i + 0] = (int)(y[((j + 0) * width) + i + 0] - (0.3455 * u[k]) - (0.7169 * v[k]));
			b[((j + 0) * width) + i + 0] = (int)(y[((j + 0) * width) + i + 0] + (1.7790 * u[k]));

			r[((j + 0) * width) + i + 1] = (int)(y[((j + 0) * width) + i + 1] + (1.4065 * v[k]));
			g[((j + 0) * width) + i + 1] = (int)(y[((j + 0) * width) + i + 1] - (0.3455 * u[k]) - (0.7169 * v[k]));
			b[((j + 0) * width) + i + 1] = (int)(y[((j + 0) * width) + i + 1] + (1.7790 * u[k]));

			r[((j + 1) * width) + i + 0] = (int)(y[((j + 1) * width) + i + 0] + (1.4065 * v[k]));
			g[((j + 1) * width) + i + 0] = (int)(y[((j + 1) * width) + i + 0] - (0.3455 * u[k]) - (0.7169 * v[k]));
			b[((j + 1) * width) + i + 0] = (int)(y[((j + 1) * width) + i + 0] + (1.7790 * u[k]));

			r[((j + 1) * width) + i + 1] = (int)(y[((j + 1) * width) + i + 1] + (1.4065 * v[k]));
			g[((j + 1) * width) + i + 1] = (int)(y[((j + 1) * width) + i + 1] - (0.3455 * u[k]) - (0.7169 * v[k]));
			b[((j + 1) * width) + i + 1] = (int)(y[((j + 1) * width) + i + 1] + (1.7790 * u[k]));

			k++;
		}
	}

	// Prevention color saturation
	for (i = 0; i < size_y; i++)
	{
		r[i] = max(min(r[i], 255), 0);
		g[i] = max(min(g[i], 255), 0);
		b[i] = max(min(b[i], 255), 0);
	}

	// Flip Vertical (Top & Bottom) for BMP format
	k = 0;
	for (j = height - 1; j >= 0; j--)
	{
		for (i = 0; i < width; i++)
		{
			r_tmp[k] = r[(j*width) + i];
			g_tmp[k] = g[(j*width) + i];
			b_tmp[k] = b[(j*width) + i];
			k++;
		}
	}

	// R, G, B -> BGR
	j = 0;
	for (i = 0; i < size_y * 3; i = i + 3)
	{
		rgb_array[i + 0] = b_tmp[j];
		rgb_array[i + 1] = g_tmp[j];
		rgb_array[i + 2] = r_tmp[j];
		j++;
	}

	delete[] y;
	delete[] v;
	delete[] u;
	delete[] r;
	delete[] g;
	delete[] b;
	delete[] r_tmp;
	delete[] g_tmp;
	delete[] b_tmp;
}

void DropYUVFile(char * argv)
{
	int i, j, k, l, alpha;
	int F1, F2, F3, F4, F5, F6;

	char * image;
	char * image_temp_char;
	int * image_temp_int;
	int * Pixel_Y;
	int * Pixel_Y_Croped;
	unsigned char * rgb;

	string str1 = "_Sharpened.yuv";
	string str2 = "_Sharpened.txt";
	string str3 = "_Sharpened.bmp";

	BmpFileHeader bfHeader;
	BmpInfoHeader biHeader;

	ifstream file1(argv, ios::in | ios::binary | ios::ate);
	ofstream file2(argv + str1, ios::out | ios::binary);
	ofstream file3(argv + str2, ios::out);
	ofstream file4(argv + str3, ios::out | ios::binary);

	if (file1.is_open() & file2.is_open() & file3.is_open() & file4.is_open())
	{
		image = new char[IMG_SIZE_YCrCb];
		image_temp_char = new char[IMG_SIZE_YCrCb];
		image_temp_int = new int[IMG_SIZE_YCrCb];
		Pixel_Y = new int[IMG_SIZE_Y];
		Pixel_Y_Croped = new int[CROP_SIZE];
		rgb = new unsigned char[IMG_SIZE_Y * 3];

		file1.seekg(0, ios::beg);
		file1.read(image, IMG_SIZE_YCrCb);

		// HPF design
		//		F1 = -32;
		//		F2 = -64;
		//		F3 = -128;
		//		F4 = 32;
		//		F5 = 32;
		//		F6 = 896;


		//F1 = 0;
		//F2 = 0;
		//F3 = 0;
		//F4 = 0;
		//F5 = 0;
		//F6 = 0;

//		F1 = -32;
//		F2 = -64;
//		F3 = -128;
//		F4 = -256;
//		F5 = 128;
//		F6 = 1664;

		F1 = 0;
		F2 = 0;
		F3 = 0;
		F4 = -511;
		F5 = 0;
		F6 = 2044;


		/***************************/
		/*       Sharpeness        */
		/***************************/

		// Negative Char pixels change to Positive
		for (i = 0; i < IMG_SIZE_Y; i++)
		{
			if (image[i] < 0)
			{
				image_temp_int[i] = image[i] + 255;
			}
			else
			{
				image_temp_int[i] = image[i];
			}
		}
		for (i = IMG_SIZE_Y; i < IMG_SIZE_YCrCb; i++)
		{
			image_temp_int[i] = image[i];
		}

		// Coeficient for Sharpening strength
		alpha = 1;

		// Sharpening
		for (j = 0; j < iHeight; j++)
		{
			for (i = 0; i < iWidth; i++)
			{
				if (j == 0 || j == 1 || j == iHeight - 1 || j == iHeight - 2 || i == 0 || i == 1 || i == iWidth - 1 || i == iWidth - 2) // Corner progress
				{
					Pixel_Y[(j * iWidth) + i] = 0;
				}
				else
				{
					Pixel_Y[(j * iWidth) + i] =
						(
							image_temp_int[((j - 2) * iWidth) + (i - 2)] * F1 + image_temp_int[((j - 2) * iWidth) + (i - 1)] * F2 + image_temp_int[((j - 2) * iWidth) + i] * F3 + image_temp_int[((j - 2) * iWidth) + (i + 1)] * F2 + image_temp_int[((j - 2) * iWidth) + (i + 2)] * F1 +
							image_temp_int[((j - 1) * iWidth) + (i - 2)] * F2 + image_temp_int[((j - 1) * iWidth) + (i - 1)] * F4 + image_temp_int[((j - 1) * iWidth) + i] * F5 + image_temp_int[((j - 1) * iWidth) + (i + 1)] * F4 + image_temp_int[((j - 1) * iWidth) + (i + 2)] * F2 +
							image_temp_int[((j + 0) * iWidth) + (i - 2)] * F3 + image_temp_int[((j + 0) * iWidth) + (i - 1)] * F5 + image_temp_int[((j + 0) * iWidth) + i] * F6 + image_temp_int[((j + 0) * iWidth) + (i + 1)] * F5 + image_temp_int[((j + 0) * iWidth) + (i + 2)] * F3 +
							image_temp_int[((j + 1) * iWidth) + (i - 2)] * F2 + image_temp_int[((j + 1) * iWidth) + (i - 1)] * F4 + image_temp_int[((j + 1) * iWidth) + i] * F5 + image_temp_int[((j + 1) * iWidth) + (i + 1)] * F4 + image_temp_int[((j + 1) * iWidth) + (i + 2)] * F2 +
							image_temp_int[((j + 2) * iWidth) + (i - 2)] * F1 + image_temp_int[((j + 2) * iWidth) + (i - 1)] * F2 + image_temp_int[((j + 2) * iWidth) + i] * F3 + image_temp_int[((j + 2) * iWidth) + (i + 1)] * F2 + image_temp_int[((j + 2) * iWidth) + (i + 2)] * F1
						) / 1024 * alpha;
				}
			}
		}

		// Original Y Pixels + Sharpened Y Pixels
		for (i = 0; i < IMG_SIZE_Y; i++)
		{
			image_temp_int[i] = max(min(image_temp_int[i] + Pixel_Y[i], 255), 0);
		}

		// Set YCrCb pixels after sharpened 
		for (i = 0; i < IMG_SIZE_YCrCb; i++)
		{
			image_temp_char[i] = image_temp_int[i];
		}

		// Save YUV file
		file2.seekp(0, ios::beg);
		file2.write(image_temp_char, IMG_SIZE_YCrCb);


		/***************************/
		/*    Create Cropped TXT   */
		/***************************/

		// Log for Croped Y
		i = j = k = l = 0;
		for (j = 0; j < CROP_H; j++)
		{
			for (i = 0; i < CROP_W; i++)
			{
				Pixel_Y_Croped[k++] = Pixel_Y[((g_Height_s + j) * iWidth) + g_Width_s + i];
			}
		}

		// Save txt file as Integer type
		file3.seekp(0, ios::beg);
		for (i = 0; i < CROP_SIZE; i++)
		{
			file3 << Pixel_Y_Croped[i] << " ";
		}


		/***************************/
		/*       Create BMP        */
		/***************************/
		Convert_yvu2rgb(image_temp_int, iWidth, iHeight, rgb);

		// Set BMP Header
		file4.seekp(0, ios::beg);
		file4.write((char*)&bfHeader, sizeof(bfHeader));
		file4.seekp(sizeof(bfHeader));
		file4.write((char*)&biHeader, sizeof(biHeader));
		// Set BMP Pixels
		file4.seekp(sizeof(bfHeader) + sizeof(biHeader));
		file4.write((const char*)rgb, IMG_SIZE_Y * 3);

		cout << argv << " Sharpening has been adjusted" << endl;

		file1.close();
		file2.close();
		file3.close();
		file4.close();

		delete[] image;
		delete[] image_temp_int;
		delete[] image_temp_char;
		delete[] Pixel_Y;
		delete[] Pixel_Y_Croped;
		delete[] rgb;
	}
	else
	{
		cout << "Unable to open file" << endl;
	}
}

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

//	HBITMAP MyBitmap;
HBITMAP MyBitmap2;
HDC hdc;
PAINTSTRUCT ps;

LRESULT CALLBACK WndProc_Img(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
		case WM_CREATE: // Excute just once when init
		{
			strcat(g_argv, "_Sharpened.bmp");
			cout << g_argv << endl;
			//MyBitmap = (HBITMAP)LoadImage(NULL, TEXT("024_Astar_cpp.yuv_Sharpened.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			MyBitmap2 = (HBITMAP)LoadImage(NULL, g_argv, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			return 0;
		}
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			//DrawBitmap(hdc, 0, 0, MyBitmap);
			DrawBitmap(hdc, 0, 0, MyBitmap2);
			EndPaint(hWnd, &ps);
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			InvalidateRect(hWnd, NULL, TRUE); // Clear img

			g_is_clicked = 1;
			g_Width_s = (lParam >> 00) & 0x0000FFFF;
			g_Height_s = (lParam >> 16) & 0x0000FFFF;
			return 0;
		}
		case WM_LBUTTONUP:
		{
			g_is_clicked = 0;
			g_Width_e = (lParam >> 00) & 0x0000FFFF;
			g_Height_e = (lParam >> 16) & 0x0000FFFF;
			hdc = GetDC(hWnd);
			SelectObject(hdc, GetStockObject(NULL_BRUSH));
			Rectangle(hdc, g_Width_s, g_Height_s, g_Width_e, g_Height_e);
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
		case WM_DESTROY:
		{
			//DeleteObject(MyBitmap);
			DeleteObject(MyBitmap2);
			DeleteDC(hdc);
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TBBUTTON ToolBtn[5] =
	{
		{ 0,10,TBSTATE_ENABLED, TBSTYLE_BUTTON,0,0,0,0 },
		{ 1,11,TBSTATE_ENABLED, TBSTYLE_BUTTON,0,0,0,0 },
		{ 5,0,0,TBSTYLE_SEP,0,0,0,0 },
		{ 2,12,TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECKGROUP,0,0,0,0 },
		{ 3,13,TBSTATE_ENABLED, TBSTYLE_BUTTON,0,0,0,0 }
	};

	switch (uMsg)
	{
		case WM_CREATE:
		{
			InitCommonControls();
			CreateToolbarEx(hWnd, WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 4, ((LPCREATESTRUCT)lParam)->hInstance, 0,ToolBtn, 5, 16, 16, 16, 16, sizeof(TBBUTTON));
		//	CreateWindowEx(0, "Button", "Test", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 100, 25, hWnd, (HMENU)0, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
			return 0;
		}
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == 0)
			{
				MessageBox(hWnd, "Test Button Clicked", "Button", MB_OK);
			}
			return 0;
		}
		case WM_SIZE:
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
		case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)wParam;

			// Count dropped files number
			g_argc = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); 
			string total_path;

			for (int i = 0; i < g_argc; i++)
			{
				DragQueryFile(hDrop, i, g_argv, MAX_PATH);
				total_path += g_argv;
				total_path += "\n";
			}

//			MessageBox(hWnd, total_path.c_str(), "Drop Info", MB_ICONINFORMATION);

			DropYUVFile(g_argv);

//			MessageBox(hWnd, "Sharpening 완료", "알림", MB_OK);

			HWND wnd_img;
			WNDCLASS wc_img;
			char class_name_img[] = "JJ_wnd_img";

			// Init Img Window 
			wc_img.hbrBackground = (HBRUSH)COLOR_BACKGROUND + 3; // +0 = Gray, +1 = Light Gray, +2 = Black, +3 = Dark Gray
			wc_img.hCursor = LoadCursor(NULL, IDC_ARROW);
//			wc_img.hIcon = LoadIcon(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDI_ICON1));
//			wc_img.hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
			wc_img.lpfnWndProc = WndProc_Img;
			wc_img.lpszClassName = class_name_img;
			wc_img.cbClsExtra = NULL;
			wc_img.cbWndExtra = NULL;
			wc_img.lpszMenuName = NULL;
			wc_img.style = NULL;
			RegisterClass(&wc_img);

			wnd_img = CreateWindowEx(0, class_name_img, "Img", WS_OVERLAPPEDWINDOW | WS_CHILD | WS_VSCROLL | WS_HSCROLL, 50, 50, 800, 600, hWnd, NULL, NULL, NULL);
			ShowWindow(wnd_img, SW_SHOW);
			
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND wnd_main;
	WNDCLASS wc_main;
	MSG msg;
	char class_name_main[] = "JJ_wnd_main";

	// Init Main Window 
	wc_main.hbrBackground = (HBRUSH)COLOR_BACKGROUND + 3; // +0 = Gray, +1 = Light Gray, +2 = Black, +3 = Dark Gray
	wc_main.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc_main.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc_main.hInstance = hInstance;
	wc_main.lpfnWndProc = WndProc_Main;
	wc_main.lpszClassName = class_name_main;
	wc_main.cbClsExtra = NULL;
	wc_main.cbWndExtra = NULL;
	wc_main.lpszMenuName = NULL;
	wc_main.style = NULL;

	RegisterClass(&wc_main);

	wnd_main = CreateWindowEx(0, class_name_main, "Sharpen Tool", WS_OVERLAPPEDWINDOW, 10, 10, 1900, 1060, NULL, NULL, hInstance, NULL);

	// Accept Drag & Drop files to window
	DragAcceptFiles(wnd_main, TRUE);

	ShowWindow(wnd_main, nCmdShow);

	UpdateWindow(wnd_main);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
