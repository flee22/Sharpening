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

HBITMAP MyBitmap, MyBitmap2;
HDC hdc;
PAINTSTRUCT ps;

int g_is_clicked = 0;
int g_x_start, g_y_start, g_x_end, g_y_end;

////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////
void Convert_yvu2rgb(int * yvu_array, int width, int height, unsigned char * rgb_array)
{
	int i, j, k;
	int *y, *u, *v, *r, *g, *b, *r_tmp, *g_tmp, *b_tmp;
	int size_y = width * height;
	int size_vu = size_y >> 2;
	int size_yvu = size_y * 1.5;

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
			r[((j + 0) * width) + i + 0] = y[((j + 0) * width) + i + 0] + (1.4065 * v[k]);
			g[((j + 0) * width) + i + 0] = y[((j + 0) * width) + i + 0] - (0.3455 * u[k]) - (0.7169 * v[k]);
			b[((j + 0) * width) + i + 0] = y[((j + 0) * width) + i + 0] + (1.7790 * u[k]);

			r[((j + 0) * width) + i + 1] = y[((j + 0) * width) + i + 1] + (1.4065 * v[k]);
			g[((j + 0) * width) + i + 1] = y[((j + 0) * width) + i + 1] - (0.3455 * u[k]) - (0.7169 * v[k]);
			b[((j + 0) * width) + i + 1] = y[((j + 0) * width) + i + 1] + (1.7790 * u[k]);

			r[((j + 1) * width) + i + 0] = y[((j + 1) * width) + i + 0] + (1.4065 * v[k]);
			g[((j + 1) * width) + i + 0] = y[((j + 1) * width) + i + 0] - (0.3455 * u[k]) - (0.7169 * v[k]);
			b[((j + 1) * width) + i + 0] = y[((j + 1) * width) + i + 0] + (1.7790 * u[k]);

			r[((j + 1) * width) + i + 1] = y[((j + 1) * width) + i + 1] + (1.4065 * v[k]);
			g[((j + 1) * width) + i + 1] = y[((j + 1) * width) + i + 1] - (0.3455 * u[k]) - (0.7169 * v[k]);
			b[((j + 1) * width) + i + 1] = y[((j + 1) * width) + i + 1] + (1.7790 * u[k]);

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
}

void DropYUVFile(char * argv)
{
	int i, j, k, l, alpha;
	int F1, F2, F3, F4, F5, F6;
	int IMG_SIZE_Y = iWidth * iHeight;
	int IMG_SIZE_YCrCb = IMG_SIZE_Y + (IMG_SIZE_Y >> 1); // Full Size = Y size + CrCb size
	int CROP_SIZE = (g_y_end - g_y_start) * (g_x_end - g_x_start);
	int CROP_H = g_y_end - g_y_start;
	int CROP_W = g_x_end - g_x_start;

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


		//		F1 = 0;
		//		F2 = 0;
		//		F3 = 0;
		//		F4 = 0;
		//		F5 = 0;
		//		F6 = 0;

		F1 = -32;
		F2 = -64;
		F3 = -128;
		F4 = -256;
		F5 = 128;
		F6 = 1664;

		//		F1 = 0;
		//		F2 = 0;
		//		F3 = 0;
		//		F4 = -511;
		//		F5 = 0;
		//		F6 = 2044;


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
				Pixel_Y_Croped[k++] = Pixel_Y[((g_y_start + j) * iWidth) + g_x_start + i];
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

LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:
		{
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
			char argv[500];
			DragQueryFile(hDrop, 0, argv, MAX_PATH);

			DropYUVFile(argv);

			MessageBox(hWnd, "Sharpening 완료", "알림", MB_OK);

			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_Img1(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE: // Excute just once when init
		{
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

LRESULT CALLBACK WndProc_List(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND ListBox;
	HDROP hDrop;
	
	switch (uMsg)
	{
		case WM_CREATE:
		{
			ListBox = CreateWindowEx(0, "LISTBOX", "", WS_CHILD| WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | LBS_HASSTRINGS, 0, 0, 500, 500, hWnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, (LPVOID)NULL);
			return 0;
		}
		case WM_DROPFILES:
		{
			hDrop = (HDROP)wParam;
			int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); // Count dropped files number
			char temp_path[500];
			string total_path;

			for (int i = 0; i < count; i++)
			{
				DragQueryFile(hDrop, i, temp_path, MAX_PATH);
				total_path += temp_path;
				total_path += "\n";
			}

			MessageBox(hWnd, total_path.c_str(), "Drop Info", MB_ICONINFORMATION);
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND wnd_main, wnd_crop, wnd_img1, wnd_list;
	WNDCLASS wc_main, wc_img1, wc_crop, wc_list;
	MSG msg;
	char class_name_main[] = "JJ_wnd_main";
	char class_name_img1[] = "JJ_wnd_img1";
	char class_name_crop[] = "JJ_wnd_crop";
	char class_name_list[] = "JJ_wnd_list";

	// Init Status Bar
	InitCommonControls();

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

	// Init Img1 Window 
	wc_img1 = wc_main; // Copy wc_main style to wc_img1
	wc_img1.lpfnWndProc = WndProc_Img1;
	wc_img1.lpszClassName = class_name_img1;

	// Init Crop Window 
	wc_crop = wc_main; // Copy wc_main style to wc_crop
	wc_crop.lpfnWndProc = WndProc_Crop;
	wc_crop.lpszClassName = class_name_crop;

	// Init List Window
	wc_list = wc_main; // Copy wc_main style to wc_list
	wc_list.lpfnWndProc = WndProc_List;
	wc_list.lpszClassName = class_name_list;

	RegisterClass(&wc_main);
	RegisterClass(&wc_img1);
	RegisterClass(&wc_crop);
	RegisterClass(&wc_list);

	wnd_main = CreateWindowEx(0, class_name_main, "JJU Sharpen Tool", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 10, 10, 1270, 1000, NULL, NULL, hInstance, NULL);
	wnd_img1 = CreateWindowEx(0, class_name_img1, "Img1", WS_OVERLAPPEDWINDOW | WS_CHILD | WS_VSCROLL | WS_HSCROLL, 10, 10, 800, 600, wnd_main, NULL, hInstance, NULL);
	wnd_crop = CreateWindowEx(0, class_name_crop, "Crop", WS_POPUPWINDOW | WS_CAPTION, 1280, 10, 500, 500, wnd_main, NULL, hInstance, NULL);
	wnd_list = CreateWindowEx(0, class_name_list, "List", WS_POPUPWINDOW | WS_CAPTION, 1280, 510, 500, 500, wnd_main, NULL, hInstance, NULL);

	// Accept Drag & Drop files to window
	DragAcceptFiles(wnd_main, TRUE);

	ShowWindow(wnd_main, nCmdShow);
	ShowWindow(wnd_img1, nCmdShow);
	ShowWindow(wnd_crop, nCmdShow);
	ShowWindow(wnd_list, nCmdShow);

	UpdateWindow(wnd_main);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
