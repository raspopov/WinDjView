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

OUT = "$(INTDIR)\IndexTool.exe"

PCH = "$(INTDIR)\IndexTool.pch"
CPP_PCH = /Yu"stdafx.h"
CPP_PCH_CREATE = /Yc"stdafx.h"

CPP_FLAGS = $(CPP_FLAGS) /nologo /MT /W3 /GX /Ox /Ob2 /Ot /Oi /Og /G6 /GL /GR \
            /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp$(PCH) /Fo"$(INTDIR)/" /c

LINK_FLAGS = $(LINK_FLAGS) "..\libdjvu\$(LIBDJVU)" \
             /nologo /subsystem:windows /incremental:no /machine:x86 \
             /nodefaultlib:"msvcrt.lib" /ltcg /out:$(OUT)

RSC_FLAGS = /l 0x409 /fo"$(INTDIR)\IndexTool.res" /d "NDEBUG"

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
	"$(INTDIR)\IndexDlg.obj" \
	"$(INTDIR)\IndexTool.obj" \
	"$(INTDIR)\MyDropTarget.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\IndexTool.res"

$(OUT) : "$(INTDIR)" $(LINK_OBJS)
	@$(LINK) $(LINK_FLAGS) $(LINK_OBJS)

"$(INTDIR)\IndexDlg.obj" : IndexDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\IndexTool.obj" : IndexTool.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyDropTarget.obj" : MyDropTarget.cpp "$(INTDIR)" $(PCH)

"$(INTDIR)\stdafx.obj" $(PCH) : stdafx.cpp "$(INTDIR)"
	@$(CPP) $(CPP_FLAGS) $(CPP_PCH_CREATE) stdafx.cpp

"$(INTDIR)\IndexTool.res" : IndexTool.rc "$(INTDIR)"
