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

OUT = "$(INTDIR)\WinDjView.exe"

PCH = "$(INTDIR)\WinDjView.pch"
CPP_PCH = /Yu"stdafx.h"
CPP_PCH_CREATE = /Yc"stdafx.h"

CPP_FLAGS = $(CPP_FLAGS) /nologo /MT /W3 /GX /Ox /Ob2 /Ot /Oi /Og /G6 /GR- \
            /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp$(PCH) /Fo"$(INTDIR)/" /c

LINK_FLAGS = $(LINK_FLAGS) "libdjvu\$(LIBDJVU)" msimg32.lib version.lib \
             /nologo /subsystem:windows /incremental:no /machine:x86 \
             /nodefaultlib:"msvcrt.lib" /ltcg /out:$(OUT)

RSC_FLAGS = /l 0x409 /fo"$(INTDIR)\WinDjView.res" /d "NDEBUG"

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

LINK_OBJS = \
	"$(INTDIR)\AppSettings.obj" \
	"$(INTDIR)\BookmarksView.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\DjVuDoc.obj" \
	"$(INTDIR)\DjVuView.obj" \
	"$(INTDIR)\Drawing.obj" \
	"$(INTDIR)\FindDlg.obj" \
	"$(INTDIR)\FullscreenWnd.obj" \
	"$(INTDIR)\GotoPageDlg.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MyBitmapButton.obj" \
	"$(INTDIR)\MyComboBox.obj" \
	"$(INTDIR)\MyEdit.obj" \
	"$(INTDIR)\MyScrollView.obj" \
	"$(INTDIR)\MySplitterWnd.obj" \
	"$(INTDIR)\MyStatusBar.obj" \
	"$(INTDIR)\MyTheme.obj" \
	"$(INTDIR)\MyToolBar.obj" \
	"$(INTDIR)\NavPane.obj" \
	"$(INTDIR)\PrintDlg.obj" \
	"$(INTDIR)\ProgressDlg.obj" \
	"$(INTDIR)\RenderThread.obj" \
	"$(INTDIR)\SearchResultsView.obj" \
	"$(INTDIR)\SettingsDlg.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\ThumbnailsThread.obj" \
	"$(INTDIR)\ThumbnailsView.obj" \
	"$(INTDIR)\UpdateDlg.obj" \
	"$(INTDIR)\WinDjView.obj" \
	"$(INTDIR)\ZoomDlg.obj" \
	"$(INTDIR)\WinDjView.res"

$(OUT) : "$(INTDIR)" $(LINK_OBJS)
	@$(LINK) $(LINK_FLAGS) $(LINK_OBJS)

"$(INTDIR)\AppSettings.obj" : AppSettings.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\BookmarksView.obj" : BookmarksView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\ChildFrm.obj" : ChildFrm.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\DjVuDoc.obj" : DjVuDoc.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\DjVuView.obj" : DjVuView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\Drawing.obj"	: Drawing.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\FindDlg.obj" : FindDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\FullscreenWnd.obj" : FullscreenWnd.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\GotoPageDlg.obj"	: GotoPageDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MainFrm.obj"	: MainFrm.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyBitmapButton.obj" : MyBitmapButton.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyComboBox.obj" : MyComboBox.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyEdit.obj" : MyEdit.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyScrollView.obj" : MyScrollView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MySplitterWnd.obj" : MySplitterWnd.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyStatusBar.obj" : MyStatusBar.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyTheme.obj" : MyTheme.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyToolBar.obj" : MyToolBar.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\NavPane.obj" : NavPane.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\PrintDlg.obj" : PrintDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\ProgressDlg.obj"	: ProgressDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\RenderThread.obj" : RenderThread.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\SearchResultsView.obj" : SearchResultsView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\SettingsDlg.obj" : SettingsDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\ThumbnailsThread.obj" : ThumbnailsThread.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\ThumbnailsView.obj" : ThumbnailsView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\UpdateDlg.obj" : UpdateDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\WinDjView.obj" : WinDjView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\ZoomDlg.obj" : ZoomDlg.cpp "$(INTDIR)" $(PCH)

"$(INTDIR)\stdafx.obj" $(PCH) : stdafx.cpp "$(INTDIR)"
	@$(CPP) $(CPP_FLAGS) $(CPP_PCH_CREATE) stdafx.cpp

"$(INTDIR)\WinDjView.res" : WinDjView.rc "$(INTDIR)"
	@$(RSC) $(RSC_FLAGS) WinDjView.rc
