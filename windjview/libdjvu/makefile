CPP = cl
GENLIB = lib
DEFOPT = /D


!IFNDEF DEBUG
INTDIR = .\Objs
OUT = ..\Release\libdjvu.lib
OPTS = /nologo /MT /W3 /GX /Ox /Ob2 /Ot /Og /G6
OUTOPT = /nologo /nodefaultlib:"msvcrt.lib" /out:
!ELSE
INTDIR = .\Objs.dbg
OUT = ..\Debug\libdjvud.lib
OPTS = /nologo /MTd /W3 /GX /Ox /Ob2 /Ot /Og /G6
OUTOPT = /nologo /nodefaultlib:"msvcrtd.lib" /out:
!ENDIF

COMPOPT = /c /Fo"$(INTDIR)\\" /D"HAS_WCTYPE" /D"BUILD_LIB" /D"DJVU_STATIC_LIBRARY" /D"WIN32" /D"_WINDOWS" /D"_CONSOLE" /D"NDEBUG"

all: $(OUT)

rebuild: clean all

clean:
	@if exist "$(INTDIR)\*.*" del /q "$(INTDIR)\*.*"
	@if exist "$(OUT)" del /q "$(OUT)"

OBJS = "$(INTDIR)\Arrays.obj" \
       "$(INTDIR)\BSByteStream.obj" \
       "$(INTDIR)\BSEncodeByteStream.obj" \
       "$(INTDIR)\ByteStream.obj" \
       "$(INTDIR)\DataPool.obj" \
       "$(INTDIR)\debug.obj" \
       "$(INTDIR)\DjVmDir.obj" \
       "$(INTDIR)\DjVmDir0.obj" \
       "$(INTDIR)\DjVmDoc.obj" \
       "$(INTDIR)\DjVuAnno.obj" \
       "$(INTDIR)\DjVuDocEditor.obj" \
       "$(INTDIR)\DjVuDocument.obj" \
       "$(INTDIR)\DjVuDumpHelper.obj" \
       "$(INTDIR)\DjVuErrorList.obj" \
       "$(INTDIR)\DjVuFile.obj" \
       "$(INTDIR)\DjVuFileCache.obj" \
       "$(INTDIR)\DjVuGlobal.obj" \
       "$(INTDIR)\DjVuGlobalMemory.obj" \
       "$(INTDIR)\DjVuImage.obj" \
       "$(INTDIR)\DjVuInfo.obj" \
       "$(INTDIR)\DjVuMessage.obj" \
       "$(INTDIR)\DjVuMessageLite.obj" \
       "$(INTDIR)\DjVuNavDir.obj" \
       "$(INTDIR)\DjVuPalette.obj" \
       "$(INTDIR)\DjVuPort.obj" \
       "$(INTDIR)\DjVuText.obj" \
       "$(INTDIR)\DjVuToPS.obj" \
       "$(INTDIR)\GBitmap.obj" \
       "$(INTDIR)\GContainer.obj" \
       "$(INTDIR)\GException.obj" \
       "$(INTDIR)\GIFFManager.obj" \
       "$(INTDIR)\GMapAreas.obj" \
       "$(INTDIR)\GOS.obj" \
       "$(INTDIR)\GPixmap.obj" \
       "$(INTDIR)\GRect.obj" \
       "$(INTDIR)\GScaler.obj" \
       "$(INTDIR)\GSmartPointer.obj" \
       "$(INTDIR)\GString.obj" \
       "$(INTDIR)\GThreads.obj" \
       "$(INTDIR)\GUnicode.obj" \
       "$(INTDIR)\GURL.obj" \
       "$(INTDIR)\IFFByteStream.obj" \
       "$(INTDIR)\IW44EncodeCodec.obj" \
       "$(INTDIR)\IW44Image.obj" \
       "$(INTDIR)\JB2EncodeCodec.obj" \
       "$(INTDIR)\JB2Image.obj" \
       "$(INTDIR)\JPEGDecoder.obj" \
       "$(INTDIR)\MMRDecoder.obj" \
       "$(INTDIR)\MMX.obj" \
       "$(INTDIR)\UnicodeByteStream.obj" \
       "$(INTDIR)\XMLParser.obj" \
       "$(INTDIR)\XMLTags.obj" \
       "$(INTDIR)\ZPCodec.obj"

$(OUT): "$(INTDIR)" $(OBJS)
	@$(GENLIB) $(OUTOPT)$(OUT) $(OBJS)

.cpp{$(INTDIR)}.obj::
	@$(CPP) $(COMPOPT) $(OPTS) $<

"$(INTDIR)":
	@if not exist "$(INTDIR)" mkdir "$(INTDIR)"

"$(INTDIR)\Arrays.obj" : "$(INTDIR)" Arrays.cpp
"$(INTDIR)\BSByteStream.obj" : "$(INTDIR)" BSByteStream.cpp
"$(INTDIR)\BSEncodeByteStream.obj" : "$(INTDIR)" BSEncodeByteStream.cpp
"$(INTDIR)\ByteStream.obj" : "$(INTDIR)" ByteStream.cpp
"$(INTDIR)\DataPool.obj" : "$(INTDIR)" DataPool.cpp
"$(INTDIR)\debug.obj" : "$(INTDIR)" debug.cpp
"$(INTDIR)\DjVmDir.obj" : "$(INTDIR)" DjVmDir.cpp
"$(INTDIR)\DjVmDir0.obj" : "$(INTDIR)" DjVmDir0.cpp
"$(INTDIR)\DjVmDoc.obj" : "$(INTDIR)" DjVmDoc.cpp
"$(INTDIR)\DjVuAnno.obj" : "$(INTDIR)" DjVuAnno.cpp
"$(INTDIR)\DjVuDocEditor.obj" : "$(INTDIR)" DjVuDocEditor.cpp
"$(INTDIR)\DjVuDocument.obj" : "$(INTDIR)" DjVuDocument.cpp
"$(INTDIR)\DjVuDumpHelper.obj" : "$(INTDIR)" DjVuDumpHelper.cpp
"$(INTDIR)\DjVuErrorList.obj" : "$(INTDIR)" DjVuErrorList.cpp
"$(INTDIR)\DjVuFile.obj" : "$(INTDIR)" DjVuFile.cpp
"$(INTDIR)\DjVuFileCache.obj" : "$(INTDIR)" DjVuFileCache.cpp
"$(INTDIR)\DjVuGlobal.obj" : "$(INTDIR)" DjVuGlobal.cpp
"$(INTDIR)\DjVuGlobalMemory.obj" : "$(INTDIR)" DjVuGlobalMemory.cpp
"$(INTDIR)\DjVuImage.obj" : "$(INTDIR)" DjVuImage.cpp
"$(INTDIR)\DjVuInfo.obj" : "$(INTDIR)" DjVuInfo.cpp
"$(INTDIR)\DjVuMessage.obj" : "$(INTDIR)" DjVuMessage.cpp
"$(INTDIR)\DjVuMessageLite.obj" : "$(INTDIR)" DjVuMessageLite.cpp
"$(INTDIR)\DjVuNavDir.obj" : "$(INTDIR)" DjVuNavDir.cpp
"$(INTDIR)\DjVuPalette.obj" : "$(INTDIR)" DjVuPalette.cpp
"$(INTDIR)\DjVuPort.obj" : "$(INTDIR)" DjVuPort.cpp
"$(INTDIR)\DjVuText.obj" : "$(INTDIR)" DjVuText.cpp
"$(INTDIR)\DjVuToPS.obj" : "$(INTDIR)" DjVuToPS.cpp
"$(INTDIR)\GBitmap.obj" : "$(INTDIR)" GBitmap.cpp
"$(INTDIR)\GContainer.obj" : "$(INTDIR)" GContainer.cpp
"$(INTDIR)\GException.obj" : "$(INTDIR)" GException.cpp
"$(INTDIR)\GIFFManager.obj" : "$(INTDIR)" GIFFManager.cpp
"$(INTDIR)\GMapAreas.obj" : "$(INTDIR)" GMapAreas.cpp
"$(INTDIR)\GOS.obj" : "$(INTDIR)" GOS.cpp
"$(INTDIR)\GPixmap.obj" : "$(INTDIR)" GPixmap.cpp
"$(INTDIR)\GRect.obj" : "$(INTDIR)" GRect.cpp
"$(INTDIR)\GScaler.obj" : "$(INTDIR)" GScaler.cpp
"$(INTDIR)\GSmartPointer.obj" : "$(INTDIR)" GSmartPointer.cpp
"$(INTDIR)\GString.obj" : "$(INTDIR)" GString.cpp
"$(INTDIR)\GThreads.obj" : "$(INTDIR)" GThreads.cpp
"$(INTDIR)\GUnicode.obj" : "$(INTDIR)" GUnicode.cpp
"$(INTDIR)\GURL.obj" : "$(INTDIR)" GURL.cpp
"$(INTDIR)\IFFByteStream.obj" : "$(INTDIR)" IFFByteStream.cpp
"$(INTDIR)\IW44EncodeCodec.obj" : "$(INTDIR)" IW44EncodeCodec.cpp
"$(INTDIR)\IW44Image.obj" : "$(INTDIR)" IW44Image.cpp
"$(INTDIR)\JB2EncodeCodec.obj" : "$(INTDIR)" JB2EncodeCodec.cpp
"$(INTDIR)\JB2Image.obj" : "$(INTDIR)" JB2Image.cpp
"$(INTDIR)\JPEGDecoder.obj" : "$(INTDIR)" JPEGDecoder.cpp
"$(INTDIR)\MMRDecoder.obj" : "$(INTDIR)" MMRDecoder.cpp
"$(INTDIR)\MMX.obj" : "$(INTDIR)" MMX.cpp
"$(INTDIR)\UnicodeByteStream.obj" : "$(INTDIR)" UnicodeByteStream.cpp
"$(INTDIR)\XMLParser.obj" : "$(INTDIR)" XMLParser.cpp
"$(INTDIR)\XMLTags.obj" : "$(INTDIR)" XMLTags.cpp
"$(INTDIR)\ZPCodec.obj" : "$(INTDIR)" ZPCodec.cpp
