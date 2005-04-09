# Microsoft Developer Studio Project File - Name="WinDjView" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=WinDjView - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinDjView.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinDjView.mak" CFG="WinDjView - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinDjView - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WinDjView - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WinDjView - Win32 Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /Ox /Ot /Og /Oi /Oy- /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 Release/libdjvu.lib msimg32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"msvcrt.lib"

!ELSEIF  "$(CFG)" == "WinDjView - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /Gf /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Debug/libdjvud.lib msimg32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "WinDjView - Win32 Release"
# Name "WinDjView - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AppSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\BookmarksView.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\DjVuView.cpp
# End Source File
# Begin Source File

SOURCE=.\Drawing.cpp
# End Source File
# Begin Source File

SOURCE=.\FindDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FullscreenWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MyBitmapButton.cpp
# End Source File
# Begin Source File

SOURCE=.\MyComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\MyEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\MyScrollView.cpp
# End Source File
# Begin Source File

SOURCE=.\MySplitterWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MyStatusBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MyToolBar.cpp
# End Source File
# Begin Source File

SOURCE=.\NavPane.cpp
# End Source File
# Begin Source File

SOURCE=.\PrintDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RenderThread.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchResultsView.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\ThumbnailsThread.cpp
# End Source File
# Begin Source File

SOURCE=.\ThumbnailsView.cpp
# End Source File
# Begin Source File

SOURCE=.\UpdateDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WinDjView.cpp
# End Source File
# Begin Source File

SOURCE=.\ZoomDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AppSettings.h
# End Source File
# Begin Source File

SOURCE=.\BookmarksView.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\DjVuDoc.h
# End Source File
# Begin Source File

SOURCE=.\DjVuView.h
# End Source File
# Begin Source File

SOURCE=.\Drawing.h
# End Source File
# Begin Source File

SOURCE=.\FindDlg.h
# End Source File
# Begin Source File

SOURCE=.\FullscreenWnd.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MyBitmapButton.h
# End Source File
# Begin Source File

SOURCE=.\MyComboBox.h
# End Source File
# Begin Source File

SOURCE=.\MyEdit.h
# End Source File
# Begin Source File

SOURCE=.\MyScrollView.h
# End Source File
# Begin Source File

SOURCE=.\MySplitterWnd.h
# End Source File
# Begin Source File

SOURCE=.\MyStatusBar.h
# End Source File
# Begin Source File

SOURCE=.\MyToolBar.h
# End Source File
# Begin Source File

SOURCE=.\NavPane.h
# End Source File
# Begin Source File

SOURCE=.\PrintDlg.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\RenderThread.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SearchResultsView.h
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\ThumbnailsThread.h
# End Source File
# Begin Source File

SOURCE=.\ThumbnailsView.h
# End Source File
# Begin Source File

SOURCE=.\UpdateDlg.h
# End Source File
# Begin Source File

SOURCE=.\WinDjView.h
# End Source File
# Begin Source File

SOURCE=.\ZoomDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bookmarks.bmp
# End Source File
# Begin Source File

SOURCE=.\res\close.bmp
# End Source File
# Begin Source File

SOURCE=.\res\DjVuDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\drag.cur
# End Source File
# Begin Source File

SOURCE=.\res\hand.cur
# End Source File
# Begin Source File

SOURCE=.\res\link.cur
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\WinDjView.ico
# End Source File
# Begin Source File

SOURCE=.\res\WinDjView.manifest
# End Source File
# Begin Source File

SOURCE=.\WinDjView.rc
# End Source File
# Begin Source File

SOURCE=.\res\WinDjView.rc2
# End Source File
# End Group
# End Target
# End Project
