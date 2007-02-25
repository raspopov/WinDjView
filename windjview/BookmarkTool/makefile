CPP = cl
LINK = link
RSC = rc

CPP_FLAGS = 
LINK_FLAGS =

!IFDEF UNICODE

INTDIR = .\Release_Unicode
LIBDJVU = libdjvuu.lib

CPP_FLAGS = $(CPP_FLAGS) /D "_UNICODE" /D "UNICODE"
LINK_FLAGS = $(LINK_FLAGS) /entry:"wWinMainCRTStartup"

!ELSE

INTDIR=.\Release
LIBDJVU = libdjvu.lib

!ENDIF

OUT = "$(INTDIR)\BookmarkTool.exe"

PCH = "$(INTDIR)\BookmarkTool.pch"
CPP_PCH = /Yu"stdafx.h"
CPP_PCH_CREATE = /Yc"stdafx.h"

CPP_FLAGS = $(CPP_FLAGS) /nologo /MT /W3 /GX /Ox /Ob2 /Ot /Oi /Og /G6 /GL /GR \
            /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp$(PCH) /Fo"$(INTDIR)/" /c

LINK_FLAGS = $(LINK_FLAGS) "..\libdjvu\$(LIBDJVU)" \
             /nologo /subsystem:windows /incremental:no /machine:x86 \
             /nodefaultlib:"msvcrt.lib" /ltcg /out:$(OUT)

RSC_FLAGS = /l 0x409 /fo"$(INTDIR)\BookmarkTool.res" /d "NDEBUG"

all: $(OUT)

rebuild: clean all

clean:
	@if exist "$(INTDIR)\*.obj" del /q /f "$(INTDIR)\*.obj"
	@if exist "$(INTDIR)\*.sbr" del /q /f "$(INTDIR)\*.sbr"
	@if exist "$(INTDIR)\*.bsc" del /q /f "$(INTDIR)\*.bsc"
	@if exist "$(INTDIR)\*.res" del /q /f "$(INTDIR)\*.res"
	@if exist "$(INTDIR)\*.idb" del /q /f "$(INTDIR)\*.idb"
	@if exist $(PCH) del /q /f $(PCH)
	@if exist $(OUT) del /q /f $(OUT)

"$(INTDIR)" :
	@if not exist "$(INTDIR)" mkdir "$(INTDIR)"

.cpp{$(INTDIR)}.obj::
	@$(CPP) $(CPP_FLAGS) $(CPP_PCH) $<

.rc{$(INTDIR)}.res::
	@$(RSC) $(RSC_FLAGS) $<

LINK_OBJS = \
	"$(INTDIR)\BookmarkDlg.obj" \
	"$(INTDIR)\BookmarkTool.obj" \
	"$(INTDIR)\Global.obj" \
	"$(INTDIR)\MyDropTarget.obj" \
	"$(INTDIR)\MyFileDialog.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\BookmarkTool.res"

$(OUT) : "$(INTDIR)" $(LINK_OBJS)
	@$(LINK) $(LINK_FLAGS) $(LINK_OBJS)

"$(INTDIR)\BookmarkDlg.obj" : BookmarkDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\BookmarkTool.obj" : BookmarkTool.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\Global.obj" : Global.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyDropTarget.obj" : MyDropTarget.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyFileDialog.obj" : MyFileDialog.cpp "$(INTDIR)" $(PCH)

"$(INTDIR)\stdafx.obj" $(PCH) : stdafx.cpp "$(INTDIR)"
	@$(CPP) $(CPP_FLAGS) $(CPP_PCH_CREATE) stdafx.cpp

"$(INTDIR)\BookmarkTool.res" : BookmarkTool.rc "$(INTDIR)"
