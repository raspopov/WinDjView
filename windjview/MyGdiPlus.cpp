//	WinDjView
//	Copyright (C) 2004-2015 Andrew Zhezherun
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along
//	with this program; if not, write to the Free Software Foundation, Inc.,
//	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//	http://www.gnu.org/copyleft/gpl.html

#include "stdafx.h"
#include "Global.h"

#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		EXTERN_C const GUID name \
				= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#include "MyGdiPlus.h"

namespace Gdip
{

typedef Status (__stdcall *pfGetImageEncodersSize)(UINT* numEncoders, UINT* size);
typedef Status (__stdcall *pfGetImageEncoders)(UINT numEncoders, UINT size, ImageCodecInfo* encoders);
typedef Status (__stdcall *pfLoadImageFromStream)(IStream* stream, Image** image);
typedef Status (__stdcall *pfLoadImageFromFile)(const WCHAR* filename, Image** image);
typedef Status (__stdcall *pfLoadImageFromStreamICM)(IStream* stream, Image** image);
typedef Status (__stdcall *pfLoadImageFromFileICM)(const WCHAR* filename, Image** image);
typedef Status (__stdcall *pfCloneImage)(Image* image, Image** cloneImage);
typedef Status (__stdcall *pfDisposeImage)(Image* image);
typedef Status (__stdcall *pfSaveImageToFile)(Image* image, const WCHAR* filename,
		const CLSID* clsidEncoder, const EncoderParameters* encoderParams);
typedef Status (__stdcall *pfSaveImageToStream)(Image* image, IStream* stream,
		const CLSID* clsidEncoder, const EncoderParameters* encoderParams);
typedef Status (__stdcall *pfGdipSaveAdd)(Image* image,
		const EncoderParameters* encoderParams);
typedef Status (__stdcall *pfGdipSaveAddImage)(Image* image, Image* newImage,
		const EncoderParameters* encoderParams);
typedef Status (__stdcall *pfCreateBitmapFromStream)(IStream* stream, Bitmap** bitmap);
typedef Status (__stdcall *pfCreateBitmapFromFile)(const WCHAR* filename, Bitmap** bitmap);
typedef Status (__stdcall *pfCreateBitmapFromStreamICM)(IStream* stream, Bitmap** bitmap);
typedef Status (__stdcall *pfCreateBitmapFromFileICM)(const WCHAR* filename, Bitmap** bitmap);
typedef Status (__stdcall *pfCreateBitmapFromGdiDib)(const BITMAPINFO* gdiBitmapInfo,
		VOID* gdiBitmapData, Bitmap** bitmap);
typedef Status (__stdcall *pfCreateBitmapFromHBITMAP)(HBITMAP hbm, HPALETTE hpal,
		Bitmap** bitmap);
typedef Status (__stdcall *pfCreateBitmapFromHICON)(HICON hicon, Bitmap** bitmap);
typedef Status (__stdcall *pfCreateBitmapFromResource)(HINSTANCE hInstance,
		const WCHAR* lpBitmapName, Bitmap** bitmap);
typedef Status (__stdcall *pfBitmapSetResolution)(Bitmap* bitmap, float xdpi, float ydpi);

pfGetImageEncodersSize pGetImageEncodersSize = NULL;
pfGetImageEncoders pGetImageEncoders = NULL;
pfCreateBitmapFromGdiDib pCreateBitmapFromGdiDib = NULL;
pfSaveImageToFile pSaveImageToFile = NULL;
pfSaveImageToStream pSaveImageToStream = NULL;
pfCloneImage pCloneImage = NULL;
pfDisposeImage pDisposeImage = NULL;
pfBitmapSetResolution pBitmapSetResolution = NULL;

HMODULE hGdiplusDLL = NULL;
bool bGdiplusAvailable = false, bGdiplusAPILoaded = false, bGdiplusLoadFailed = false;
ULONG token = 0;

//////////////////////////////////////////////////////////////////////
// Startup and shutdown

enum DebugEventLevel
{
    DebugEventLevelFatal,
    DebugEventLevelWarning
};

typedef VOID (__stdcall *DebugEventProc)(DebugEventLevel level, CHAR* message);
typedef Status (__stdcall *NotificationHookProc)(ULONG* token);
typedef VOID (__stdcall *NotificationUnhookProc)(ULONG token);

struct StartupInput
{
    UINT32 GdiplusVersion;  // Must be 1
    DebugEventProc DebugEventCallback;
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
    
    StartupInput(
        DebugEventProc debugEventCallback = NULL,
        BOOL suppressBackgroundThread = FALSE,
        BOOL suppressExternalCodecs = FALSE)
    {
        GdiplusVersion = 1;
        DebugEventCallback = debugEventCallback;
        SuppressBackgroundThread = suppressBackgroundThread;
        SuppressExternalCodecs = suppressExternalCodecs;
    }
};

struct StartupOutput
{
    NotificationHookProc NotificationHook;
    NotificationUnhookProc NotificationUnhook;
};

typedef Status (__stdcall *pfStartup)(ULONG* token, const StartupInput* input,
		StartupOutput* output);
typedef void (__stdcall *pfShutdown)(ULONG token);
pfStartup pStartup = NULL;
pfShutdown pShutdown = NULL;

void LoadGdiplusAPI()
{
	hGdiplusDLL = ::LoadLibrary(_T("gdiplus.dll"));
	if (hGdiplusDLL)
	{
		pStartup = (pfStartup)::GetProcAddress(hGdiplusDLL, "GdiplusStartup");
		pShutdown = (pfShutdown)::GetProcAddress(hGdiplusDLL, "GdiplusShutdown");
		pGetImageEncodersSize = (pfGetImageEncodersSize)::GetProcAddress(hGdiplusDLL, "GdipGetImageEncodersSize");
		pGetImageEncoders = (pfGetImageEncoders)::GetProcAddress(hGdiplusDLL, "GdipGetImageEncoders");
		pCreateBitmapFromGdiDib = (pfCreateBitmapFromGdiDib)::GetProcAddress(hGdiplusDLL, "GdipCreateBitmapFromGdiDib");
		pSaveImageToFile = (pfSaveImageToFile)::GetProcAddress(hGdiplusDLL, "GdipSaveImageToFile");
		pSaveImageToStream = (pfSaveImageToStream)::GetProcAddress(hGdiplusDLL, "GdipSaveImageToStream");
		pCloneImage = (pfCloneImage)::GetProcAddress(hGdiplusDLL, "GdipCloneImage");
		pDisposeImage = (pfDisposeImage)::GetProcAddress(hGdiplusDLL, "GdipDisposeImage");
		pBitmapSetResolution = (pfBitmapSetResolution)::GetProcAddress(hGdiplusDLL, "GdipBitmapSetResolution");
	}

	if (pStartup != NULL
		&& pShutdown != NULL
		&& pGetImageEncodersSize != NULL
		&& pGetImageEncoders != NULL
		&& pCreateBitmapFromGdiDib != NULL
		&& pSaveImageToFile != NULL
		&& pSaveImageToStream != NULL
		&& pCloneImage != NULL
		&& pDisposeImage != NULL
		&& pBitmapSetResolution != NULL)
	{
		bGdiplusAvailable = true;
	}

	if (!bGdiplusAvailable && hGdiplusDLL != NULL)
	{
		::FreeLibrary(hGdiplusDLL);
		hGdiplusDLL = NULL;
	}
}

void UnloadGdiplusAPI()
{
	if (bGdiplusAPILoaded)
		pShutdown(token);

	if (bGdiplusAvailable)
		::FreeLibrary(hGdiplusDLL);

	hGdiplusDLL = NULL;
	bGdiplusAPILoaded = false;
	bGdiplusAvailable = false;
	bGdiplusLoadFailed = false;
}

bool IsLoaded()
{
	if (bGdiplusAvailable && !bGdiplusAPILoaded && !bGdiplusLoadFailed)
	{
		StartupInput input;
		StartupOutput output;
		bGdiplusAPILoaded = (pStartup(&token, &input, &output) == Ok);
		if (!bGdiplusAPILoaded)
			bGdiplusLoadFailed = true;
	}

	return bGdiplusAPILoaded;
}

Status GetImageEncodersSize(UINT* numEncoders, UINT* size)
{
	if (!bGdiplusAPILoaded)
		return GdiplusNotInitialized;

	return pGetImageEncodersSize(numEncoders, size);
}

Status GetImageEncoders(UINT numEncoders, UINT size, ImageCodecInfo* encoders)
{
	if (!bGdiplusAPILoaded)
		return GdiplusNotInitialized;

	return pGetImageEncoders(numEncoders, size, encoders);
}

Status CreateBitmapFromGdiDib(const BITMAPINFO* gdiBitmapInfo,
		VOID* gdiBitmapData, Bitmap** bitmap)
{
	if (!bGdiplusAPILoaded)
		return GdiplusNotInitialized;

	return pCreateBitmapFromGdiDib(gdiBitmapInfo, gdiBitmapData, bitmap);
}

Status SaveImageToFile(Image* image, const WCHAR* filename,
		const CLSID* clsidEncoder, const EncoderParameters* encoderParams)
{
	if (!bGdiplusAPILoaded)
		return GdiplusNotInitialized;

	return pSaveImageToFile(image, filename, clsidEncoder, encoderParams);
}

Status SaveImageToStream(Image* image, IStream* stream,
		const CLSID* clsidEncoder, const EncoderParameters* encoderParams)
{
	if (!bGdiplusAPILoaded)
		return GdiplusNotInitialized;

	return pSaveImageToStream(image, stream, clsidEncoder, encoderParams);
}

Status CloneImage(Image* image, Image** cloneImage)
{
	if (!bGdiplusAPILoaded)
		return GdiplusNotInitialized;

	return pCloneImage(image, cloneImage);
}

Status DisposeImage(Image* image)
{
	if (!bGdiplusAPILoaded)
		return GdiplusNotInitialized;

	return pDisposeImage(image);
}

Status BitmapSetResolution(Bitmap* bitmap, float xdpi, float ydpi)
{
	if (!bGdiplusAPILoaded)
		return GdiplusNotInitialized;

	return pBitmapSetResolution(bitmap, xdpi, ydpi);
}

bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;  // number of image encoders
	UINT size = 0;  // size of the image encoder array in bytes

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return false;

	vector<char> buf(size);
	ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*) &buf[0];

	GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			return true;
		}    
	}

	return false;
}

struct GdiplusAPI
{
	GdiplusAPI() { LoadGdiplusAPI(); }
	~GdiplusAPI() { UnloadGdiplusAPI(); }
	static GdiplusAPI theAPI;
};

GdiplusAPI GdiplusAPI::theAPI;

}  // namespace Gdip
