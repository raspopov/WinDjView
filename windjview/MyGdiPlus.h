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

//////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation. All rights reserved.
// GDI+ API

#pragma once

namespace Gdip
{

bool IsLoaded();

//////////////////////////////////////////////////////////////////////
// Status return values from GDI+ methods
enum Status
{
	Ok = 0,
	GenericError = 1,
	InvalidParameter = 2,
	OutOfMemory = 3,
	ObjectBusy = 4,
	InsufficientBuffer = 5,
	NotImplemented = 6,
	Win32Error = 7,
	WrongState = 8,
	Aborted = 9,
	FileNotFound = 10,
	ValueOverflow = 11,
	AccessDenied = 12,
	UnknownImageFormat = 13,
	FontFamilyNotFound = 14,
	FontStyleNotFound = 15,
	NotTrueTypeFont = 16,
	UnsupportedGdiplusVersion = 17,
	GdiplusNotInitialized = 18,
	PropertyNotFound = 19,
	PropertyNotSupported = 20
};


//////////////////////////////////////////////////////////////////////
// Encoder parameters

class EncoderParameter
{
public:
	GUID Guid;  // GUID of the parameter
	ULONG NumberOfValues;  // Number of the parameter values
	ULONG Type;  // Value type, like ValueTypeLONG etc.
	VOID* Value;  // A pointer to the parameter values
};

class EncoderParameters
{
public:
	UINT Count;  // number of parameters in this structure
	EncoderParameter Parameter[1];  // parameter values
};

enum EncoderParameterValueType
{
	// 8-bit unsigned int
	EncoderParameterValueTypeByte = 1,
	// 8-bit byte containing one 7-bit ASCII code. NULL terminated.
	EncoderParameterValueTypeASCII = 2,
	// 16-bit unsigned int
	EncoderParameterValueTypeShort = 3,
	// 32-bit unsigned int
	EncoderParameterValueTypeLong = 4,
	// Two Longs. The first Long is the numerator, the second Long expresses the denomintor.
	EncoderParameterValueTypeRational = 5,
	// Two longs which specify a range of integer values. The first Long specifies
	// the lower end and the second one specifies the higher end. All values
	// are inclusive at both ends
	EncoderParameterValueTypeLongRange = 6,
	// 8-bit byte that can take any value depending on field definition
	EncoderParameterValueTypeUndefined = 7,
	// Two Rationals. The first Rational specifies the lower end and the second
	// specifies the higher end. All values are inclusive at both ends
	EncoderParameterValueTypeRationalRange = 8
};

enum EncoderValue
{
	EncoderValueColorTypeCMYK,
	EncoderValueColorTypeYCCK,
	EncoderValueCompressionLZW,
	EncoderValueCompressionCCITT3,
	EncoderValueCompressionCCITT4,
	EncoderValueCompressionRle,
	EncoderValueCompressionNone,
	EncoderValueScanMethodInterlaced,
	EncoderValueScanMethodNonInterlaced,
	EncoderValueVersionGif87,
	EncoderValueVersionGif89,
	EncoderValueRenderProgressive,
	EncoderValueRenderNonProgressive,
	EncoderValueTransformRotate90,
	EncoderValueTransformRotate180,
	EncoderValueTransformRotate270,
	EncoderValueTransformFlipHorizontal,
	EncoderValueTransformFlipVertical,
	EncoderValueMultiFrame,
	EncoderValueLastFrame,
	EncoderValueFlush,
	EncoderValueFrameDimensionTime,
	EncoderValueFrameDimensionResolution,
	EncoderValueFrameDimensionPage
};


class ImageCodecInfo
{
public:
	CLSID Clsid;
	GUID FormatID;
	const WCHAR* CodecName;
	const WCHAR* DllName;
	const WCHAR* FormatDescription;
	const WCHAR* FilenameExtension;
	const WCHAR* MimeType;
	DWORD Flags;
	DWORD Version;
	DWORD SigCount;
	DWORD SigSize;
	const BYTE* SigPattern;
	const BYTE* SigMask;
};


//////////////////////////////////////////////////////////////////////
// Image GUIDs

DEFINE_GUID(EncoderCompression, 0xe09d739d,0xccd4,0x44ee,0x8e,0xba,0x3f,0xbf,0x8b,0xe4,0xfc,0x58);
DEFINE_GUID(EncoderColorDepth, 0x66087055,0xad66,0x4c7c,0x9a,0x18,0x38,0xa2,0x31,0x0b,0x83,0x37);
DEFINE_GUID(EncoderScanMethod, 0x3a4e2661,0x3109,0x4e56,0x85,0x36,0x42,0xc1,0x56,0xe7,0xdc,0xfa);
DEFINE_GUID(EncoderVersion, 0x24d18c76,0x814a,0x41a4,0xbf,0x53,0x1c,0x21,0x9c,0xcc,0xf7,0x97);
DEFINE_GUID(EncoderRenderMethod, 0x6d42c53a,0x229a,0x4825,0x8b,0xb7,0x5c,0x99,0xe2,0xb9,0xa8,0xb8);
DEFINE_GUID(EncoderQuality, 0x1d5be4b5,0xfa4a,0x452d,0x9c,0xdd,0x5d,0xb3,0x51,0x05,0xe7,0xeb);
DEFINE_GUID(EncoderTransformation,0x8d0eb2d1,0xa58e,0x4ea8,0xaa,0x14,0x10,0x80,0x74,0xb7,0xb6,0xf9);
DEFINE_GUID(EncoderLuminanceTable,0xedb33bce,0x0266,0x4a77,0xb9,0x04,0x27,0x21,0x60,0x99,0xe7,0x17);
DEFINE_GUID(EncoderChrominanceTable,0xf2e455dc,0x09b3,0x4316,0x82,0x60,0x67,0x6a,0xda,0x32,0x48,0x1c);
DEFINE_GUID(EncoderSaveFlag,0x292266fc,0xac40,0x47bf,0x8c, 0xfc, 0xa8, 0x5b, 0x89, 0xa6, 0x55, 0xde);

Status GetImageEncodersSize(UINT* numEncoders, UINT* size);
Status GetImageEncoders(UINT numEncoders, UINT size, ImageCodecInfo* encoders);
bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid);


//////////////////////////////////////////////////////////////////////
// Image APIs

class Image {};

Status LoadImageFromStream(IStream* stream, Image** image);
Status LoadImageFromFile(const WCHAR* filename, Image** image);
Status LoadImageFromStreamICM(IStream* stream, Image** image);
Status LoadImageFromFileICM(const WCHAR* filename, Image** image);
Status CloneImage(Image* image, Image** cloneImage);
Status DisposeImage(Image* image);
Status SaveImageToFile(Image* image, const WCHAR* filename,
		const CLSID* clsidEncoder, const EncoderParameters* encoderParams);
Status SaveImageToStream(Image* image, IStream* stream,
		const CLSID* clsidEncoder, const EncoderParameters* encoderParams);
Status SaveAdd(Image* image, const EncoderParameters* encoderParams);
Status SaveAddImage(Image* image, Image* newImage,
		const EncoderParameters* encoderParams);

//////////////////////////////////////////////////////////////////////
// Bitmap APIs

class Bitmap : public Image {};

Status CreateBitmapFromStream(IStream* stream, Bitmap** bitmap);
Status CreateBitmapFromFile(const WCHAR* filename, Bitmap** bitmap);
Status CreateBitmapFromStreamICM(IStream* stream, Bitmap** bitmap);
Status CreateBitmapFromFileICM(const WCHAR* filename, Bitmap** bitmap);
Status CreateBitmapFromGdiDib(const BITMAPINFO* gdiBitmapInfo,
		VOID* gdiBitmapData, Bitmap** bitmap);
Status CreateBitmapFromHBITMAP(HBITMAP hbm, HPALETTE hpal, Bitmap** bitmap);
Status CreateBitmapFromHICON(HICON hicon, Bitmap** bitmap);
Status CreateBitmapFromResource(HINSTANCE hInstance, const WCHAR* lpBitmapName,
		Bitmap** bitmap);
Status BitmapSetResolution(Bitmap* bitmap, float xdpi, float ydpi);

}  // namespace Gdip
