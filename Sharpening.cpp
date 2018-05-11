#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console") // Using this comment can use Command prompt 

#include "Resource.h"	// for using resource ICON
#include "stdafx.h"		// include Windows.h
#include <shellapi.h>	// for Drag and Drop files
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <crtdbg.h>		// for Debug Memory Leak

using namespace std;

#define iWidth 4608
#define iHeight 3456

// for Debug Memory Leak
#ifndef _DEBUG
#define new new(_CLIENT_BLOCK,__FILE__,__LINE__)
#endif

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

HINSTANCE g_hInst;
HWND g_hDlgWnd, g_hMainWnd, g_hImgWnd, g_hCropWnd;
HBITMAP MyBitmap, CropBitmap;
HDC hdc;

int g_is_clicked_LBUTTON, g_is_clicked_RBUTTON;
int g_RectW_s, g_RectW_e, g_RectH_s, g_RectH_e;
int g_cur_x, g_cur_y;
int g_argc;
char g_argv[MAX_PATH];
char g_argv_bmp[MAX_PATH];
char g_argv_dbg[MAX_PATH];
int g_hpf[15];
int g_hpf_coef;
int g_img_zoomScale = -2;
int g_crop_zoomScale = 2;

int IMG_SIZE_Y = iWidth * iHeight;
int IMG_SIZE_YCrCb = IMG_SIZE_Y + (IMG_SIZE_Y >> 1); // Full Size = Y size + CrCb size


void Yvu2rgb(int * yvu_array, int width, int height, unsigned char * rgb_array)
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

void Sharpening(char * argv)
{
	int i, j, alpha;
	char * image;
	char * image_temp_char;
	int * image_temp_int;
	int * image_temp_int_dbg;
	int * Pixel_Y;
	unsigned char * rgb;
	unsigned char * rgb_dbg;

	string str1 = "_Sharpened.yuv";
	string str2 = "_Sharpened_dbg.bmp";
	string str3 = "_Sharpened.bmp";

	BmpFileHeader bfHeader;
	BmpInfoHeader biHeader;

	ifstream file1(argv, ios::in | ios::binary | ios::ate);
	ofstream file2(argv + str1, ios::out | ios::binary);
	ofstream file3(argv + str2, ios::out | ios::binary);
	ofstream file4(argv + str3, ios::out | ios::binary);

	if (file1.is_open() & file2.is_open() & file3.is_open() & file4.is_open())
	{
		image = new char[IMG_SIZE_YCrCb];
		image_temp_char = new char[IMG_SIZE_YCrCb];
		image_temp_int = new int[IMG_SIZE_YCrCb];
		image_temp_int_dbg = new int[IMG_SIZE_YCrCb];
		Pixel_Y = new int[IMG_SIZE_Y];
		rgb = new unsigned char[IMG_SIZE_Y * 3];
		rgb_dbg = new unsigned char[IMG_SIZE_Y * 3];

		file1.seekg(0, ios::beg);
		file1.read(image, IMG_SIZE_YCrCb);


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
		alpha = g_hpf_coef;

		// Sharpening
		for (j = 0; j < iHeight; j++)
		{
			for (i = 0; i < iWidth; i++)
			{
				if (j == 0				|| j == 1				|| j == 2				|| j == 3				|| 
					j == iHeight - 1	|| j == iHeight - 2		|| j == iHeight - 3		|| j == iHeight - 4		||
					i == 0				|| i == 1				|| i == 2				|| i == 3				||
					i == iWidth - 1		|| i == iWidth - 2		|| i == iWidth - 3		|| i == iWidth - 4) // Ignore corner
				{
					Pixel_Y[(j * iWidth) + i] = 0;
				}
				else
				{
					/*************************************************************************************************/
					/* Diagonal HPF Kernel calculation
					     i |    -4   |   -3    |   -2    |   -1    |   +0    |   +1    |   +2    |   +3    |   +4
					-------------------------------------------------------------------------------------------------
					j -4   | HPF[00] | HPF[01] | HPF[02] | HPF[03] | HPF[04] | HPF[03] | HPF[02] | HPF[01] | HPF[00]
					  -3   | HPF[01] | HPF[05] | HPF[06] | HPF[07] | HPF[08] | HPF[07] | HPF[06] | HPF[05] | HPF[01]
					  -2   | HPF[02] | HPF[06] | HPF[09] | HPF[10] | HPF[11] | HPF[10] | HPF[09] | HPF[06] | HPF[02]
					  -1   | HPF[03] | HPF[07] | HPF[10] | HPF[12] | HPF[13] | HPF[12] | HPF[10] | HPF[07] | HPF[03]
					  +0   | HPF[04] | HPF[08] | HPF[11] | HPF[13] | HPF[14] | HPF[13] | HPF[11] | HPF[08] | HPF[04]
					  +1   | HPF[03] | HPF[07] | HPF[10] | HPF[12] | HPF[13] | HPF[12] | HPF[10] | HPF[07] | HPF[03]
					  +2   | HPF[02] | HPF[06] | HPF[09] | HPF[10] | HPF[11] | HPF[10] | HPF[09] | HPF[06] | HPF[02]
					  +3   | HPF[01] | HPF[05] | HPF[06] | HPF[07] | HPF[08] | HPF[07] | HPF[06] | HPF[05] | HPF[01]
					  +4   | HPF[00] | HPF[01] | HPF[02] | HPF[03] | HPF[04] | HPF[03] | HPF[02] | HPF[01] | HPF[00] */
					/*************************************************************************************************/
					Pixel_Y[(j * iWidth) + i] =
						(
							image_temp_int[((j - 4) * iWidth) + (i - 4)] * g_hpf[0] + 
							image_temp_int[((j - 4) * iWidth) + (i - 3)] * g_hpf[1] + 
							image_temp_int[((j - 4) * iWidth) + (i - 2)] * g_hpf[2] + 
							image_temp_int[((j - 4) * iWidth) + (i - 1)] * g_hpf[3] + 
							image_temp_int[((j - 4) * iWidth) + (i + 0)] * g_hpf[4] + 
							image_temp_int[((j - 4) * iWidth) + (i + 1)] * g_hpf[3] + 
							image_temp_int[((j - 4) * iWidth) + (i + 2)] * g_hpf[2] + 
							image_temp_int[((j - 4) * iWidth) + (i + 3)] * g_hpf[1] + 
							image_temp_int[((j - 4) * iWidth) + (i + 4)] * g_hpf[0] +

							image_temp_int[((j - 3) * iWidth) + (i - 4)] * g_hpf[1] +
							image_temp_int[((j - 3) * iWidth) + (i - 3)] * g_hpf[5] +
							image_temp_int[((j - 3) * iWidth) + (i - 2)] * g_hpf[6] +
							image_temp_int[((j - 3) * iWidth) + (i - 1)] * g_hpf[7] +
							image_temp_int[((j - 3) * iWidth) + (i + 0)] * g_hpf[8] +
							image_temp_int[((j - 3) * iWidth) + (i + 1)] * g_hpf[7] +
							image_temp_int[((j - 3) * iWidth) + (i + 2)] * g_hpf[6] +
							image_temp_int[((j - 3) * iWidth) + (i + 3)] * g_hpf[5] +
							image_temp_int[((j - 3) * iWidth) + (i + 4)] * g_hpf[1] +

							image_temp_int[((j - 2) * iWidth) + (i - 4)] * g_hpf[2] +
							image_temp_int[((j - 2) * iWidth) + (i - 3)] * g_hpf[6] +
							image_temp_int[((j - 2) * iWidth) + (i - 2)] * g_hpf[9] +
							image_temp_int[((j - 2) * iWidth) + (i - 1)] * g_hpf[10] +
							image_temp_int[((j - 2) * iWidth) + (i + 0)] * g_hpf[11] +
							image_temp_int[((j - 2) * iWidth) + (i + 1)] * g_hpf[10] +
							image_temp_int[((j - 2) * iWidth) + (i + 2)] * g_hpf[9] +
							image_temp_int[((j - 2) * iWidth) + (i + 3)] * g_hpf[6] +
							image_temp_int[((j - 2) * iWidth) + (i + 4)] * g_hpf[2] +

							image_temp_int[((j - 1) * iWidth) + (i - 4)] * g_hpf[3] +
							image_temp_int[((j - 1) * iWidth) + (i - 3)] * g_hpf[7] +
							image_temp_int[((j - 1) * iWidth) + (i - 2)] * g_hpf[10] +
							image_temp_int[((j - 1) * iWidth) + (i - 1)] * g_hpf[12] +
							image_temp_int[((j - 1) * iWidth) + (i + 0)] * g_hpf[13] +
							image_temp_int[((j - 1) * iWidth) + (i + 1)] * g_hpf[12] +
							image_temp_int[((j - 1) * iWidth) + (i + 2)] * g_hpf[10] +
							image_temp_int[((j - 1) * iWidth) + (i + 3)] * g_hpf[7] +
							image_temp_int[((j - 1) * iWidth) + (i + 4)] * g_hpf[3] +

							image_temp_int[((j + 0) * iWidth) + (i - 4)] * g_hpf[4] +
							image_temp_int[((j + 0) * iWidth) + (i - 3)] * g_hpf[8] +
							image_temp_int[((j + 0) * iWidth) + (i - 2)] * g_hpf[11] +
							image_temp_int[((j + 0) * iWidth) + (i - 1)] * g_hpf[13] +
							image_temp_int[((j + 0) * iWidth) + (i + 0)] * g_hpf[14] +
							image_temp_int[((j + 0) * iWidth) + (i + 1)] * g_hpf[13] +
							image_temp_int[((j + 0) * iWidth) + (i + 2)] * g_hpf[11] +
							image_temp_int[((j + 0) * iWidth) + (i + 3)] * g_hpf[8] +
							image_temp_int[((j + 0) * iWidth) + (i + 4)] * g_hpf[4] +

							image_temp_int[((j + 1) * iWidth) + (i - 4)] * g_hpf[3] +
							image_temp_int[((j + 1) * iWidth) + (i - 3)] * g_hpf[7] +
							image_temp_int[((j + 1) * iWidth) + (i - 2)] * g_hpf[10] +
							image_temp_int[((j + 1) * iWidth) + (i - 1)] * g_hpf[12] +
							image_temp_int[((j + 1) * iWidth) + (i + 0)] * g_hpf[13] +
							image_temp_int[((j + 1) * iWidth) + (i + 1)] * g_hpf[12] +
							image_temp_int[((j + 1) * iWidth) + (i + 2)] * g_hpf[10] +
							image_temp_int[((j + 1) * iWidth) + (i + 3)] * g_hpf[7] +
							image_temp_int[((j + 1) * iWidth) + (i + 4)] * g_hpf[3] +

							image_temp_int[((j + 2) * iWidth) + (i - 4)] * g_hpf[2] +
							image_temp_int[((j + 2) * iWidth) + (i - 3)] * g_hpf[6] +
							image_temp_int[((j + 2) * iWidth) + (i - 2)] * g_hpf[9] +
							image_temp_int[((j + 2) * iWidth) + (i - 1)] * g_hpf[10] +
							image_temp_int[((j + 2) * iWidth) + (i + 0)] * g_hpf[11] +
							image_temp_int[((j + 2) * iWidth) + (i + 1)] * g_hpf[10] +
							image_temp_int[((j + 2) * iWidth) + (i + 2)] * g_hpf[9] +
							image_temp_int[((j + 2) * iWidth) + (i + 3)] * g_hpf[6] +
							image_temp_int[((j + 2) * iWidth) + (i + 4)] * g_hpf[2] +

							image_temp_int[((j + 3) * iWidth) + (i - 4)] * g_hpf[1] +
							image_temp_int[((j + 3) * iWidth) + (i - 3)] * g_hpf[5] +
							image_temp_int[((j + 3) * iWidth) + (i - 2)] * g_hpf[6] +
							image_temp_int[((j + 3) * iWidth) + (i - 1)] * g_hpf[7] +
							image_temp_int[((j + 3) * iWidth) + (i + 0)] * g_hpf[8] +
							image_temp_int[((j + 3) * iWidth) + (i + 1)] * g_hpf[7] +
							image_temp_int[((j + 3) * iWidth) + (i + 2)] * g_hpf[6] +
							image_temp_int[((j + 3) * iWidth) + (i + 3)] * g_hpf[5] +
							image_temp_int[((j + 3) * iWidth) + (i + 4)] * g_hpf[1] +

							image_temp_int[((j + 4) * iWidth) + (i - 4)] * g_hpf[0] +
							image_temp_int[((j + 4) * iWidth) + (i - 3)] * g_hpf[1] +
							image_temp_int[((j + 4) * iWidth) + (i - 2)] * g_hpf[2] +
							image_temp_int[((j + 4) * iWidth) + (i - 1)] * g_hpf[3] +
							image_temp_int[((j + 4) * iWidth) + (i + 0)] * g_hpf[4] +
							image_temp_int[((j + 4) * iWidth) + (i + 1)] * g_hpf[3] +
							image_temp_int[((j + 4) * iWidth) + (i + 2)] * g_hpf[2] +
							image_temp_int[((j + 4) * iWidth) + (i + 3)] * g_hpf[1] +
							image_temp_int[((j + 4) * iWidth) + (i + 4)] * g_hpf[0]
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


		/********************************/
		/*    Create Debug Sharpening   */
		/********************************/

		// Set Y to Sharpened pixels for debug
		for (i = 0; i < IMG_SIZE_Y; i++)
		{
			image_temp_int_dbg[i] = max(min(Pixel_Y[i] + 128, 255), 0);
		}
		// Set CrCb to 128(none color) for debug
		for (i = IMG_SIZE_Y; i < IMG_SIZE_YCrCb; i++)
		{
			image_temp_int_dbg[i] = 128;
		}

		/***********************************/
		/*       Create BMP for Debug      */
		/***********************************/
		Yvu2rgb(image_temp_int_dbg, iWidth, iHeight, rgb_dbg);

		// Set BMP Headers
		file3.seekp(0, ios::beg);
		file3.write((char*)&bfHeader, sizeof(bfHeader));
		file3.seekp(sizeof(bfHeader));
		file3.write((char*)&biHeader, sizeof(biHeader));
		// Set BMP Pixels
		file3.seekp(sizeof(bfHeader) + sizeof(biHeader));
		file3.write((const char*)rgb_dbg, IMG_SIZE_Y * 3);

		/**********************************/
		/*       Create BMP for Img       */
		/**********************************/
		Yvu2rgb(image_temp_int, iWidth, iHeight, rgb);

		// Set BMP Headers
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
		delete[] image_temp_char;
		delete[] image_temp_int;
		delete[] image_temp_int_dbg;
		delete[] Pixel_Y;
		delete[] rgb;
		delete[] rgb_dbg;
	}
	else
	{
		cout << "Unable to open file" << endl;
	}
}

void DrawBitmap(HDC hdc, int x, int y, int Width, int Height, HBITMAP hBitmap)
{
	int bx, by;
	HDC MemDC;
	BITMAP bit;
	HBITMAP OldBitmap;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBitmap);

	GetObject(hBitmap, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	SetStretchBltMode(hdc, COLORONCOLOR);

	if (g_img_zoomScale >= 0) {
		StretchBlt(hdc, 0, 0, bx << g_img_zoomScale, by << g_img_zoomScale, MemDC, x, y, Width, Height, SRCCOPY);	
	}
	else {
		StretchBlt(hdc, 0, 0, bx >> -g_img_zoomScale, by >> -g_img_zoomScale, MemDC, x, y, Width, Height, SRCCOPY);
	}

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}

void DrawCropBitmap(HDC hdc, int x, int y, int Width, int Height, HBITMAP hBitmap)
{
	HDC mDC;
	BITMAP bit;
	HBITMAP OldBitmap;

	mDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(mDC, hBitmap);

	GetObject(hBitmap, sizeof(BITMAP), &bit);

	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchBlt(hdc, 0, 0, Width << g_crop_zoomScale, Height << g_crop_zoomScale, mDC, x, y, Width, Height, SRCCOPY);

	SelectObject(mDC, OldBitmap);
	DeleteDC(mDC);
}


BOOL CALLBACK WndProc_Dlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG: 
		{
			SetDlgItemText(hDlg, ID_HPF_01, "0");
			SetDlgItemText(hDlg, ID_HPF_02, "0");
			SetDlgItemText(hDlg, ID_HPF_03, "0");
			SetDlgItemText(hDlg, ID_HPF_04, "0");
			SetDlgItemText(hDlg, ID_HPF_05, "0");
			SetDlgItemText(hDlg, ID_HPF_06, "0");
			SetDlgItemText(hDlg, ID_HPF_07, "0");
			SetDlgItemText(hDlg, ID_HPF_08, "0");
			SetDlgItemText(hDlg, ID_HPF_09, "0");
			SetDlgItemText(hDlg, ID_HPF_10, "0");
			SetDlgItemText(hDlg, ID_HPF_11, "0");
			SetDlgItemText(hDlg, ID_HPF_12, "0");
			SetDlgItemText(hDlg, ID_HPF_13, "-511");
			SetDlgItemText(hDlg, ID_HPF_14, "0");
			SetDlgItemText(hDlg, ID_HPF_15, "2044");
			SetDlgItemText(hDlg, ID_HPF_ALPHA, "2");
			  g_hpf[0] = 0;
			  g_hpf[1] = 0;
			  g_hpf[2] = 0;
			  g_hpf[3] = 0;
			  g_hpf[4] = 0;
			  g_hpf[5] = 0;
			  g_hpf[6] = 0;
			  g_hpf[7] = 0;
			  g_hpf[8] = 0;
			  g_hpf[9] = 0;
			 g_hpf[10] = 0;
			 g_hpf[11] = 0;
			 g_hpf[12] = -511;
			 g_hpf[13] = 0;
			 g_hpf[14] = 2044;
			g_hpf_coef = 2;
			return TRUE;
		}
		case WM_COMMAND:
		{
			switch (wParam)
			{
				case ID_UPDATE:
				{
					  g_hpf[0] = GetDlgItemInt(hDlg, ID_HPF_01, NULL, TRUE);
					  g_hpf[1] = GetDlgItemInt(hDlg, ID_HPF_02, NULL, TRUE);
					  g_hpf[2] = GetDlgItemInt(hDlg, ID_HPF_03, NULL, TRUE);
					  g_hpf[3] = GetDlgItemInt(hDlg, ID_HPF_04, NULL, TRUE);
					  g_hpf[4] = GetDlgItemInt(hDlg, ID_HPF_05, NULL, TRUE);
					  g_hpf[5] = GetDlgItemInt(hDlg, ID_HPF_06, NULL, TRUE);
					  g_hpf[6] = GetDlgItemInt(hDlg, ID_HPF_07, NULL, TRUE);
					  g_hpf[7] = GetDlgItemInt(hDlg, ID_HPF_08, NULL, TRUE);
					  g_hpf[8] = GetDlgItemInt(hDlg, ID_HPF_09, NULL, TRUE);
					  g_hpf[9] = GetDlgItemInt(hDlg, ID_HPF_10, NULL, TRUE);
					 g_hpf[10] = GetDlgItemInt(hDlg, ID_HPF_11, NULL, TRUE);
					 g_hpf[11] = GetDlgItemInt(hDlg, ID_HPF_12, NULL, TRUE);
					 g_hpf[12] = GetDlgItemInt(hDlg, ID_HPF_13, NULL, TRUE);
					 g_hpf[13] = GetDlgItemInt(hDlg, ID_HPF_14, NULL, TRUE);
					 g_hpf[14] = GetDlgItemInt(hDlg, ID_HPF_15, NULL, TRUE);
					g_hpf_coef = GetDlgItemInt(hDlg, ID_HPF_ALPHA, NULL, FALSE);

					if (g_hImgWnd)
					{
						Sharpening(g_argv);
						InvalidateRect(g_hImgWnd, NULL, TRUE);
						InvalidateRect(g_hCropWnd, NULL, TRUE);
					}
					return TRUE;
				}
				case ID_COPY:
				{
					return TRUE;
				}
			}
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK WndProc_Crop(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch (uMsg)
	{
		case WM_CREATE:
		{
			return 0;
		}
		case WM_PAINT:
		{
			if (CropBitmap) DeleteObject(CropBitmap);

			CropBitmap = (HBITMAP)LoadImage(NULL, g_argv_dbg, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

			hdc = BeginPaint(hWnd, &ps);
			if (g_RectW_s > g_RectW_e) {
				DrawCropBitmap(hdc, g_RectW_e, g_RectH_e, (g_RectW_s - g_RectW_e), (g_RectH_s - g_RectH_e), CropBitmap);
			}
			else {
				DrawCropBitmap(hdc, g_RectW_s, g_RectH_s, (g_RectW_e - g_RectW_s), (g_RectH_e - g_RectH_s), CropBitmap);
			}
			EndPaint(hWnd, &ps);

			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			if ((SHORT)HIWORD(wParam) > 0) 
			{
				// Zoom In
				if (g_crop_zoomScale > 2) g_crop_zoomScale = 3;
				else g_crop_zoomScale++;
			}
			else 
			{
				// Zoom Out
				if (g_crop_zoomScale < 1) g_crop_zoomScale = 0;
				else g_crop_zoomScale--;
			}

			SetWindowPos(hWnd, NULL, 0, 0, (g_RectW_e - g_RectW_s) << g_crop_zoomScale, (g_RectH_e - g_RectH_s) << g_crop_zoomScale, SWP_NOMOVE);

			InvalidateRect(hWnd, NULL, FALSE); // Clear img

			return 0;
		}
		case WM_DESTROY:
		{
			DeleteObject(CropBitmap);
			return 0;
		}
	}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_Img(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch (uMsg)
	{
		case WM_CREATE:
		{
			strcat_s(g_argv_bmp, "_Sharpened.bmp");
			strcat_s(g_argv_dbg, "_Sharpened_dbg.bmp");

			return 0;
		}
		case WM_PAINT:
		{
			if (MyBitmap) DeleteObject(MyBitmap);

			MyBitmap = (HBITMAP)LoadImage(NULL, g_argv_bmp, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

			hdc = BeginPaint(hWnd, &ps);
			DrawBitmap(hdc, 0, 0, iWidth, iHeight, MyBitmap);
			EndPaint(hWnd, &ps);

			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			if((SHORT)HIWORD(wParam) > 0) 
			{ 
				// Zoom In
				if (g_img_zoomScale > 2) g_img_zoomScale = 3;
				else g_img_zoomScale++;
			}
			else 
			{ 
				// Zoom Out
				if (g_img_zoomScale < -3) g_img_zoomScale = -4;
				else g_img_zoomScale--;
			}

			if (g_img_zoomScale < 0) 
				SetWindowPos(hWnd, NULL, 0, 0, iWidth >> -g_img_zoomScale, iHeight >> -g_img_zoomScale, SWP_NOMOVE);
			else
				SetWindowPos(hWnd, NULL, 0, 0, iWidth << g_img_zoomScale, iHeight << g_img_zoomScale, SWP_NOMOVE);

			InvalidateRect(hWnd, NULL, FALSE);

			return 0;
		}
		case WM_MOUSEMOVE:
		{
			if (g_is_clicked_RBUTTON) 
			{
				// Get current Mouse xy point at Client area
				if (g_img_zoomScale >= 0) {
					g_cur_x = ((lParam >> 00) & 0x0000FFFF) >> g_img_zoomScale;
					g_cur_y = ((lParam >> 16) & 0x0000FFFF) >> g_img_zoomScale;
				}
				else {
					g_cur_x = ((lParam >> 00) & 0x0000FFFF) << -g_img_zoomScale;
					g_cur_y = ((lParam >> 16) & 0x0000FFFF) << -g_img_zoomScale;
				}

				// Refresh if image zoommed
				if (g_img_zoomScale > -1)
				{
					InvalidateRect(hWnd, NULL, FALSE);
				}
			}
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			InvalidateRect(hWnd, NULL, FALSE); // Clear img

			g_is_clicked_LBUTTON = 1;

			if (g_img_zoomScale >= 0) {
				g_RectW_s = ((lParam >> 00) & 0x0000FFFF) >> g_img_zoomScale;
				g_RectH_s = ((lParam >> 16) & 0x0000FFFF) >> g_img_zoomScale;
			}
			else {
				g_RectW_s = ((lParam >> 00) & 0x0000FFFF) << -g_img_zoomScale;
				g_RectH_s = ((lParam >> 16) & 0x0000FFFF) << -g_img_zoomScale;
			}

//			std::cout << "x1 = " << g_RectW_s << "\ty1 = " << g_RectH_s << std::endl;

			return 0;
		}
		case WM_LBUTTONUP:
		{
			g_is_clicked_LBUTTON = 0;

			if (g_img_zoomScale >= 0) {
				g_RectW_e = ((lParam >> 00) & 0x0000FFFF) >> g_img_zoomScale;
				g_RectH_e = ((lParam >> 16) & 0x0000FFFF) >> g_img_zoomScale;
			}
			else {
				g_RectW_e = ((lParam >> 00) & 0x0000FFFF) << -g_img_zoomScale;
				g_RectH_e = ((lParam >> 16) & 0x0000FFFF) << -g_img_zoomScale;
			}

//			std::cout << "x2 = " << g_RectW_e << "\ty2 = " << g_RectH_e << std::endl;

			hdc = GetDC(hWnd);
			SelectObject(hdc, GetStockObject(NULL_BRUSH));

			if (g_img_zoomScale >= 0) {
				Rectangle(hdc, 
					g_RectW_s << g_img_zoomScale, 
					g_RectH_s << g_img_zoomScale, 
					g_RectW_e << g_img_zoomScale, 
					g_RectH_e << g_img_zoomScale);
			}
			else {
				Rectangle(hdc, 
					g_RectW_s >> -g_img_zoomScale, 
					g_RectH_s >> -g_img_zoomScale, 
					g_RectW_e >> -g_img_zoomScale, 
					g_RectH_e >> -g_img_zoomScale);
			}

			ReleaseDC(hWnd, hdc);

			if (g_hCropWnd) DestroyWindow(g_hCropWnd);

			HWND wnd_crop;
			WNDCLASS wc_crop;
			char class_name_crop[] = "JJ_wnd_crop";

			// Init Crop Window 
			wc_crop.hbrBackground = (HBRUSH)COLOR_BACKGROUND + 1; // +0 = Gray, +1 = Light Gray, +2 = Black, +3 = Dark Gray
			wc_crop.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc_crop.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
			wc_crop.hInstance = g_hInst;
			wc_crop.lpfnWndProc = WndProc_Crop;
			wc_crop.lpszClassName = class_name_crop;
			wc_crop.cbClsExtra = NULL;
			wc_crop.cbWndExtra = NULL;
			wc_crop.lpszMenuName = NULL;
			wc_crop.style = NULL;
			RegisterClass(&wc_crop);

			if (g_RectW_s > g_RectW_e) {
				wnd_crop = CreateWindow(class_name_crop, "Crop", WS_POPUP | WS_OVERLAPPEDWINDOW | WS_TABSTOP,
					1200, 80, (g_RectW_s - g_RectW_e) << g_crop_zoomScale, (g_RectH_s - g_RectH_e) << g_crop_zoomScale, hWnd, NULL, NULL, NULL);
			}
			else {
				wnd_crop = CreateWindow(class_name_crop, "Crop", WS_POPUP | WS_OVERLAPPEDWINDOW | WS_TABSTOP,
					1200, 80, (g_RectW_e - g_RectW_s) << g_crop_zoomScale, (g_RectH_e - g_RectH_s) << g_crop_zoomScale, hWnd, NULL, NULL, NULL);
			}

			ShowWindow(wnd_crop, SW_SHOW);
			g_hCropWnd = wnd_crop;

			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			g_is_clicked_RBUTTON = 1;

			return 0;
		}
		case WM_RBUTTONUP:
		{
			g_is_clicked_RBUTTON = 0;

			return 0;
		}
		case WM_DESTROY:
		{
			DeleteObject(MyBitmap);
			DeleteObject(CropBitmap);
			DestroyWindow(g_hDlgWnd);
			DestroyWindow(g_hCropWnd);
			DeleteDC(hdc);
			return 0;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:
		{
			if (!IsWindow(g_hDlgWnd))
			{
				HWND wnd_dlg;
				wnd_dlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, WndProc_Dlg);
				ShowWindow(wnd_dlg , SW_SHOW);
				g_hDlgWnd = wnd_dlg;
			}

			g_is_clicked_LBUTTON = 0;
			g_is_clicked_RBUTTON = 0;
			g_cur_x = 0;
			g_cur_y = 0;

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

			Sharpening(g_argv);

			strcpy_s(g_argv_bmp, g_argv);
			strcpy_s(g_argv_dbg, g_argv);

			HWND wnd_img;
			WNDCLASS wc_img;
			char class_name_img[] = "JJ_wnd_img";

			// Init Img Window 
			wc_img.hbrBackground = (HBRUSH)COLOR_BACKGROUND + 3; // +0 = Gray, +1 = Light Gray, +2 = Black, +3 = Dark Gray
			wc_img.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc_img.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
			wc_img.hInstance = g_hInst;
			wc_img.lpfnWndProc = WndProc_Img;
			wc_img.lpszClassName = class_name_img;
			wc_img.cbClsExtra = NULL;
			wc_img.cbWndExtra = NULL;
			wc_img.lpszMenuName = NULL;
			wc_img.style = NULL;
			RegisterClass(&wc_img);

			if (g_img_zoomScale >= 0) {
				wnd_img = CreateWindowEx(0, class_name_img, "Img", WS_OVERLAPPEDWINDOW | WS_CHILD | WS_TABSTOP,
					10, 10, iWidth << g_img_zoomScale, iHeight << g_img_zoomScale, hWnd, NULL, NULL, NULL);
			}
			else {
				wnd_img = CreateWindowEx(0, class_name_img, "Img", WS_OVERLAPPEDWINDOW | WS_CHILD | WS_TABSTOP,
					10, 10, iWidth >> -g_img_zoomScale, iHeight >> -g_img_zoomScale, hWnd, NULL, NULL, NULL);
			}

			ShowWindow(wnd_img, SW_SHOW);
			g_hImgWnd = wnd_img;

			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	HWND hWnd;
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

	hWnd = CreateWindowEx(0, class_name_main, "Sharpen Tool", WS_OVERLAPPEDWINDOW, 10, 10, 1900, 1060, NULL, NULL, hInstance, NULL);

	// Accept Drag & Drop files to window
	DragAcceptFiles(hWnd, TRUE);
	ShowWindow(hWnd, nCmdShow);

	g_hInst = hInstance;
	g_hMainWnd = hWnd;

	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(g_hDlgWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}
