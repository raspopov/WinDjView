CPP = cl
LINK = link
RSC = rc

CPP_FLAGS = 
LINK_FLAGS =
RSC_FLAGS =

!IFDEF X64
DIR_SUFFIX = _x64
LIB_SUFFIX = 64
MACHINE = X64
CPP_FLAGS = /D "WIN64"
RSC_FLAGS = /d "WIN64"
LINK_FLAGS = /SUBSYSTEM:CONSOLE,5.02
!ELSE
DIR_SUFFIX =
LIB_SUFFIX =
MACHINE = X86
LINK_FLAGS = /SUBSYSTEM:CONSOLE,5.01
!ENDIF

INTDIR = .\Release$(DIR_SUFFIX)
LIBDJVU = libdjvu$(LIB_SUFFIX).lib

OUT = "$(INTDIR)\WinDjView.exe"

PCH = "$(INTDIR)\WinDjView.pch"
CPP_PCH = /Yu"stdafx.h"
CPP_PCH_CREATE = /Yc"stdafx.h"

CPP_FLAGS = $(CPP_FLAGS) /nologo /c /MT /W3 /EHsc /O2 /GL /GR- \
            /D "_UNICODE" /D "UNICODE" /D "_USING_V110_SDK71_" \
            /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp$(PCH) /Fo"$(INTDIR)/"

LINK_FLAGS = $(LINK_FLAGS) /nologo "libdjvu\$(LIBDJVU)" \
             msimg32.lib version.lib shlwapi.lib \
             /entry:"wWinMainCRTStartup" \
             /subsystem:windows /incremental:no /machine:$(MACHINE) \
             /nodefaultlib:"msvcrt.lib" /ltcg /out:$(OUT)

RSC_FLAGS = $(RSC_FLAGS) /l 0x409 /fo"$(INTDIR)\WinDjView.res" \
            /d "_UNICODE" /d "UNICODE" /d "_USING_V110_SDK71_" \
            /d "NDEBUG" /d "WIN32"

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
	"$(INTDIR)\AnnotationDlg.obj" \
	"$(INTDIR)\AppSettings.obj" \
	"$(INTDIR)\BookmarkDlg.obj" \
	"$(INTDIR)\BookmarksView.obj" \
	"$(INTDIR)\DjVuDoc.obj" \
	"$(INTDIR)\DjVuSource.obj" \
	"$(INTDIR)\DjVuView.obj" \
	"$(INTDIR)\DocPropertiesDlg.obj" \
	"$(INTDIR)\Drawing.obj" \
	"$(INTDIR)\FindDlg.obj" \
	"$(INTDIR)\FullscreenWnd.obj" \
	"$(INTDIR)\Global.obj" \
	"$(INTDIR)\GotoPageDlg.obj" \
	"$(INTDIR)\InstallDicDlg.obj" \
	"$(INTDIR)\MagnifyWnd.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MDIChild.obj" \
	"$(INTDIR)\MyBitmapButton.obj" \
	"$(INTDIR)\MyColorPicker.obj" \
	"$(INTDIR)\MyComboBox.obj" \
	"$(INTDIR)\MyDialog.obj" \
	"$(INTDIR)\MyDocManager.obj" \
	"$(INTDIR)\MyDocTemplate.obj" \
	"$(INTDIR)\MyEdit.obj" \
	"$(INTDIR)\MyFileDialog.obj" \
	"$(INTDIR)\MyGdiPlus.obj" \
	"$(INTDIR)\MyScrollView.obj" \
	"$(INTDIR)\MyStatusBar.obj" \
	"$(INTDIR)\MyTheme.obj" \
	"$(INTDIR)\MyToolBar.obj" \
	"$(INTDIR)\MyTreeView.obj" \
	"$(INTDIR)\NavPane.obj" \
	"$(INTDIR)\PageIndexWnd.obj" \
	"$(INTDIR)\PrintDlg.obj" \
	"$(INTDIR)\ProgressDlg.obj" \
	"$(INTDIR)\RenderThread.obj" \
	"$(INTDIR)\Scaling.obj" \
	"$(INTDIR)\SearchResultsView.obj" \
	"$(INTDIR)\SettingsAdvancedPage.obj" \
	"$(INTDIR)\SettingsDictPage.obj" \
	"$(INTDIR)\SettingsDisplayPage.obj" \
	"$(INTDIR)\SettingsDlg.obj" \
	"$(INTDIR)\SettingsGeneralPage.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\TabbedMDIWnd.obj" \
	"$(INTDIR)\ThumbnailsThread.obj" \
	"$(INTDIR)\ThumbnailsView.obj" \
	"$(INTDIR)\UpdateDlg.obj" \
	"$(INTDIR)\WinDjView.obj" \
	"$(INTDIR)\XMLParser.obj" \
	"$(INTDIR)\ZoomDlg.obj" \
	"$(INTDIR)\WinDjView.res"

$(OUT) : "$(INTDIR)" $(LINK_OBJS)
	@$(LINK) $(LINK_FLAGS) $(LINK_OBJS)

"$(INTDIR)\AnnotationDlg.obj" : AnnotationDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\AppSettings.obj" : AppSettings.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\BookmarkDlg.obj" : BookmarkDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\BookmarksView.obj" : BookmarksView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\DjVuDoc.obj" : DjVuDoc.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\DjVuSource.obj" : DjVuSource.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\DjVuView.obj" : DjVuView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\DocPropertiesDlg.obj" : DocPropertiesDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\Drawing.obj" : Drawing.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\FindDlg.obj" : FindDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\FullscreenWnd.obj" : FullscreenWnd.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\Global.obj" : Global.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\GotoPageDlg.obj" : GotoPageDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\InstallDicDlg.obj" : InstallDicDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MagnifyWnd.obj" : MagnifyWnd.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MainFrm.obj" : MainFrm.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MDIChild.obj" : MDIChild.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyBitmapButton.obj" : MyBitmapButton.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyColorPicker.obj" : MyColorPicker.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyComboBox.obj" : MyComboBox.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyDialog.obj" : MyDialog.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyDocManager.obj" : MyDocManager.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyDocTemplate.obj" : MyDocTemplate.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyEdit.obj" : MyEdit.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyFileDialog.obj" : MyFileDialog.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyGdiPlus.obj" : MyGdiPlus.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyScrollView.obj" : MyScrollView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyStatusBar.obj" : MyStatusBar.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyTheme.obj" : MyTheme.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyToolBar.obj" : MyToolBar.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\MyTreeView.obj" : MyTreeView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\NavPane.obj" : NavPane.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\PageIndexWnd.obj" : PageIndexWnd.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\PrintDlg.obj" : PrintDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\ProgressDlg.obj"	: ProgressDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\RenderThread.obj" : RenderThread.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\Scaling.obj" : Scaling.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\SearchResultsView.obj" : SearchResultsView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\SettingsAdvancedPage.obj" : SettingsAdvancedPage.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\SettingsDictPage.obj" : SettingsDictPage.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\SettingsDisplayPage.obj" : SettingsDisplayPage.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\SettingsDlg.obj" : SettingsDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\SettingsGeneralPage.obj" : SettingsGeneralPage.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\TabbedMDIWnd.obj" : TabbedMDIWnd.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\ThumbnailsThread.obj" : ThumbnailsThread.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\ThumbnailsView.obj" : ThumbnailsView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\UpdateDlg.obj" : UpdateDlg.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\WinDjView.obj" : WinDjView.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\XMLParser.obj" : XMLParser.cpp "$(INTDIR)" $(PCH)
"$(INTDIR)\ZoomDlg.obj" : ZoomDlg.cpp "$(INTDIR)" $(PCH)

"$(INTDIR)\stdafx.obj" $(PCH) : stdafx.cpp "$(INTDIR)"
	@$(CPP) $(CPP_FLAGS) $(CPP_PCH_CREATE) stdafx.cpp

"$(INTDIR)\WinDjView.res" : WinDjView.rc "$(INTDIR)"
	@$(RSC) $(RSC_FLAGS) WinDjView.rc
