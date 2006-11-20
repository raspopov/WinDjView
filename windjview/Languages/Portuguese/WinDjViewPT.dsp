# Microsoft Developer Studio Project File - Name="WinDjViewPT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=WINDJVIEWPT - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinDjViewPT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinDjViewPT.mak" CFG="WINDJVIEWPT - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinDjViewPT - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WINDJVIEWRU_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WINDJVIEWPT_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1809 /d "NDEBUG"
# ADD RSC /l 0x816 /d "NDEBUG" /d "AFX_RESOURCE_DLL" /d "AFX_TARG_PTG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /noentry
# Begin Target

# Name "WinDjViewPT - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\WinDjViewPT.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\res\bookmarks.bmp
# End Source File
# Begin Source File

SOURCE=..\..\res\close.bmp
# End Source File
# Begin Source File

SOURCE=..\..\res\DjVuDoc.ico
# End Source File
# Begin Source File

SOURCE=..\..\res\donate.bmp
# End Source File
# Begin Source File

SOURCE=..\..\res\drag.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\hand.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\link.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\magnify.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_all.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_anchor.bmp
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_d.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_dl.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_dr.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_l.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_lr.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_r.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_u.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_ud.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_ul.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\pan_ur.cur
# End Source File
# Begin Source File

SOURCE=..\..\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=..\..\res\WinDjView.ico
# End Source File
# Begin Source File

SOURCE=..\..\res\WinDjView.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\res\WinDjView.manifest
# End Source File
# End Target
# End Project
