
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#ifndef PLAYLIST_OPTIONS_H
#define PLAYLIST_OPTIONS_H

#define _CRTDBG_MAP_ALLOC

#define WINVER       0x0600
#define _WIN32_WINNT 0x0600

#pragma warning(disable : 4786)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winerror.h>
#include <windowsx.h>
#include <Commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <crtdbg.h>
#include <math.h>
#include <vector>
#include <list>
#include <algorithm>
#include <string>
#include <map>
#include <tchar.h>
#include "Resource.h"

#ifndef UNICODE
    typedef std::string BasicString;
#else
    typedef std::wstring BasicString;
#endif

#include "Utility.h"

//  Private Messages

#define WM_SORTPLAYLIST     (WM_USER + 100)
#define HANDLE_WM_SORTPLAYLIST(hwnd, wParam, lParam, fn) \
        ((fn)((hwnd)), 0L)

#define WM_LASTUPDATED      (WM_USER + 101)
#define HANDLE_WM_LASTUPDATED(hwnd, wParam, lParam, fn) \
        ((fn)((hwnd)), 0L)

#define WM_UPDATEFILESIZE   (WM_USER + 102)
#define HANDLE_WM_UPDATEFILESIZE(hwnd, wParam, lParam, fn) \
        ((fn)((hwnd), (int)wParam), 0L)

#define WM_NOTUNES          (WM_USER + 103)
#define HANDLE_WM_NOTUNES(hwnd, wParam, lParam, fn) \
        ((fn)((hwnd)), 0L)

#define WM_UPDATETUNECOUNT  (WM_USER + 104)
#define HANDLE_WM_UPDATETUNECOUNT(hwnd, wParam, lParam, fn) \
        ((fn)((hwnd)), 0L)

#define WM_PROCESSMULTIPLE  (WM_USER + 105)
#define HANDLE_WM_PROCESSMULTIPLE(hwnd, wParam, lParam, fn) \
        ((fn)((hwnd), (int)wParam, (HWND)lParam), 0L)

#define WM_LATESTVERSION    (WM_USER + 106)
#define HANDLE_WM_LATESTVERSION(hwnd, wParam, lParam, fn) \
        ((fn)((hwnd), (BOOL)wParam), 0L)

#define WM_LOADLIBRARY      (WM_USER + 107)
#define HANDLE_WM_LOADLIBRARY(hwnd, wParam, lParam, fn) \
        ((fn)((hwnd)), 0L)

//
// Always track memory allocation under debug builds (_DEBUG defined)
//

#ifdef _DEBUG
    #define DEBUG_NEW   new( _NORMAL_BLOCK, __FILE__, __LINE__)
#else
    #define DEBUG_NEW
#endif

#endif  //  PLAYLIST_OPTIONS_H