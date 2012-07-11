# Microsoft Developer Studio Project File - Name="PlaylistGen" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PlaylistGen - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PlaylistGen.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PlaylistGen.mak" CFG="PlaylistGen - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PlaylistGen - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PlaylistGen - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "PlaylistGen - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "PlaylistGen - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PlaylistGen - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ws2_32.lib version.lib Msimg32.lib shlwapi.lib shell32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"Release/PlaylistCreator.exe"

!ELSEIF  "$(CFG)" == "PlaylistGen - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib version.lib Msimg32.lib shlwapi.lib shell32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/PlaylistCreator.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "PlaylistGen - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PlaylistGen___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "PlaylistGen___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_Debug_Unicode"
# PROP Intermediate_Dir "Win32_Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ws2_32.lib version.lib Msimg32.lib shlwapi.lib shell32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/PlaylistCreator.exe" /pdbtype:sept
# ADD LINK32 ws2_32.lib version.lib Msimg32.lib shlwapi.lib shell32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /out:"Win32_Debug_Unicode/PlaylistCreator.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "PlaylistGen - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PlaylistGen___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "PlaylistGen___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Win32_Release_Unicode"
# PROP Intermediate_Dir "Win32_Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 ws2_32.lib version.lib Msimg32.lib shlwapi.lib shell32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"Release/PlaylistCreator.exe"
# ADD LINK32 ws2_32.lib version.lib Msimg32.lib shlwapi.lib shell32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"Win32_Release_Unicode/PlaylistCreator.exe"

!ENDIF 

# Begin Target

# Name "PlaylistGen - Win32 Release"
# Name "PlaylistGen - Win32 Debug"
# Name "PlaylistGen - Win32 Debug Unicode"
# Name "PlaylistGen - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AggregatedDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioFile.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FLACParser.cpp
# End Source File
# Begin Source File

SOURCE=.\LibraryOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainWndDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MP4Parser.cpp
# End Source File
# Begin Source File

SOURCE=.\NotifyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OggParser.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistNameDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaylistOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Resource.rc
# End Source File
# Begin Source File

SOURCE=.\RPGStatusBar.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectedDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Utility.cpp
# End Source File
# Begin Source File

SOURCE=.\WMAParser.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\AggregatedDlg.h
# End Source File
# Begin Source File

SOURCE=.\AudioFile.h
# End Source File
# Begin Source File

SOURCE=.\AudioInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\FLACParser.h
# End Source File
# Begin Source File

SOURCE=.\LibraryOptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\MP4Parser.h
# End Source File
# Begin Source File

SOURCE=.\NotifyDlg.h
# End Source File
# Begin Source File

SOURCE=.\OggParser.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistDlg.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistGenerator.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistNameDlg.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistOptions.h
# End Source File
# Begin Source File

SOURCE=.\PlaylistOptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\RPGStatusBar.h
# End Source File
# Begin Source File

SOURCE=.\SelectedDlg.h
# End Source File
# Begin Source File

SOURCE=.\Utility.h
# End Source File
# Begin Source File

SOURCE=.\WMAParser.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\check.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Logo.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\LogoSmall.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\sound.ico
# End Source File
# Begin Source File

SOURCE=.\Res\Sounds.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Text.bmp
# End Source File
# Begin Source File

SOURCE=".\Res\W&W Logo.bmp"
# End Source File
# Begin Source File

SOURCE=.\Res\world.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\PlaylistCreator.exe.manifest
# End Source File
# Begin Source File

SOURCE=.\PlaylistGen.exe.manifest
# End Source File
# End Target
# End Project
