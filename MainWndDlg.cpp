
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "PlaylistDlg.h"
#include "LibraryOptionsDlg.h"
#include "PlaylistOptionsDlg.h"
#include "AboutDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      G l o b a l   D a t a                               */
/*==========================================================================*/

PlaylistGenerator   _generator;

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/

/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static LRESULT CALLBACK Frame_WndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
static BOOL Frame_OnCreate ( HWND hWnd, LPCREATESTRUCT lpCreateStruct );
static void Frame_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
static void Frame_OnClose ( HWND hWnd );
static void Frame_OnDestroy ( HWND hWnd );
static void Frame_OnGetMinMaxInfo ( HWND hWnd, LPMINMAXINFO lpMinMaxInfo );
static void Frame_OnSize ( HWND hWnd, UINT state, int cx, int cy );
static void Frame_OnMenuSelect ( HWND hWnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags );
static void Frame_OnDropFiles ( HWND hWnd, HDROP hDrop );

static BOOL InitControlClasses ( );
static BOOL InitWndClass ( );
static BOOL CreateAppFrameWindow ( );

/*==========================================================================*/
/*                 P r o g r a m   E n t r y   P o i n t                    */
/*==========================================================================*/

int WINAPI _tWinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    BOOL success;
    MSG msg;

    _generator._instance = hInstance;

    ::LoadString ( _generator._instance, IDR_PLAYLIST, _generator._window_name, MAX_PATH );
    ::LoadString ( _generator._instance, IDC_PLAYLISTCLASS, _generator._class_name, MAX_PATH );

#ifdef _DEBUG

	//  Enable memory checking to avoid buffer overflow and leak detection

	_ASSERTE ( _CrtCheckMemory() == TRUE );
	_CrtSetDbgFlag (  _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );

#endif	// _DEBUG

    //  Initialize all control classes

    /*
        Causes W2k to fail

    if ( !InitControlClasses ( ) )
        return -1;
    */


    //  Need to initialize COM to take advantage of the Shell API calls

    if ( FAILED ( ::CoInitializeEx ( NULL, COINIT_APARTMENTTHREADED ) ) )
        return -2;

    //  Create and register the window class

    if ( !InitWndClass ( ) )
        return -3;

    //  Create the application frame window

    if ( !CreateAppFrameWindow ( ) )
        return -4;

    //  Force update of window

    ::ShowWindow ( _generator._main_hwnd, nCmdShow );
    ::UpdateWindow ( _generator._main_hwnd );

    //  Pump for messages

    while( ( success = ::GetMessage( & msg, NULL, 0, 0 ) ) != 0 )
    { 
        if ( success == -1 )
        {
            // handle the error and possibly exit
        }
        else
        {
            ::TranslateMessage ( & msg ); 
            ::DispatchMessage ( & msg ); 
        }
    }

    ::CoUninitialize ( );
    
    return msg.wParam;
}

static BOOL InitControlClasses ( )
{
    INITCOMMONCONTROLSEX init_controls;

    //  Initialize all control classes

    init_controls.dwSize = sizeof ( init_controls );
    init_controls.dwICC  = ICC_WIN95_CLASSES | ICC_DATE_CLASSES | ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_INTERNET_CLASSES |
                           ICC_PAGESCROLLER_CLASS | ICC_NATIVEFNTCTL_CLASS | ICC_STANDARD_CLASSES | ICC_LINK_CLASS;

    return ::InitCommonControlsEx ( & init_controls );
}

static BOOL InitWndClass ( )
{
    WNDCLASSEX  wnd_class;

    //  Create and register the window class

    wnd_class.cbSize        = sizeof (wnd_class);
    wnd_class.style         = CS_PARENTDC;
    wnd_class.lpfnWndProc   = Frame_WndProc;
    wnd_class.cbClsExtra    = 0;
    wnd_class.cbWndExtra    = 0;
    wnd_class.hInstance     = _generator._instance;
    wnd_class.hIcon         = ::LoadIcon ( _generator._instance, MAKEINTRESOURCE(IDI_PLAYLIST) );
    wnd_class.hCursor       = ::LoadCursor ( NULL, IDC_ARROW );
    wnd_class.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wnd_class.lpszMenuName  = MAKEINTRESOURCE(IDR_PLAYLIST);
    wnd_class.lpszClassName = _generator._class_name;
    wnd_class.hIconSm       = ::LoadIcon ( _generator._instance, MAKEINTRESOURCE(IDI_PLAYLIST) );

    return ::RegisterClassEx ( & wnd_class );
}

static BOOL CreateAppFrameWindow ( )
{
    //  Get the last known position of the window

    int x, y, cx, cy;

    cx  = 900;
    cy  = 700;
    x   = ( ::GetSystemMetrics ( SM_CXSCREEN ) - cx) / 2;
    y   = ( ::GetSystemMetrics ( SM_CYSCREEN ) - cy) / 2;

    _generator.ReadPosition ( x, y, cx, cy );

    //  Create the main application window

    _generator._main_hwnd = ::CreateWindowEx (  WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,
                                                _generator._class_name,
                                                _generator._window_name,
                                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                                x, y, cx, cy,
                                                NULL,
                                                NULL,
                                                _generator._instance,
                                              & _generator );

    return ((_generator._main_hwnd) ? TRUE : FALSE);
}

/*==========================================================================*/
/*        W i n d o w  P r o c e d u r e  f o r  P l a y l i s t D l g      */
/*==========================================================================*/

static LRESULT CALLBACK Frame_WndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( uMsg )
    {
        HANDLE_MSG ( hWnd, WM_CREATE,               Frame_OnCreate );
        HANDLE_MSG ( hWnd, WM_COMMAND,              Frame_OnCommand );
        HANDLE_MSG ( hWnd, WM_CLOSE,                Frame_OnClose );
        HANDLE_MSG ( hWnd, WM_DESTROY,              Frame_OnDestroy );
        HANDLE_MSG ( hWnd, WM_GETMINMAXINFO,        Frame_OnGetMinMaxInfo );
        HANDLE_MSG ( hWnd, WM_SIZE,                 Frame_OnSize );
        HANDLE_MSG ( hWnd, WM_MENUSELECT,           Frame_OnMenuSelect );
        HANDLE_MSG ( hWnd, WM_DROPFILES,            Frame_OnDropFiles );
    }

    return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}

static BOOL Frame_OnCreate ( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
{
    _generator._main_hwnd = hWnd;

    //  Create and place the status bar

    _generator._status_bar._status_hwnd = ::CreateWindowEx ( 0,
                                                             STATUSCLASSNAME,
                                                             NULL,
                                                             WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                                                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                                             hWnd, NULL, NULL, NULL );

    //  Set initial size of status bar parts

    int parts [ 4 ];

    parts [ 0 ] = lpCreateStruct->cx - 525;
    parts [ 1 ] = lpCreateStruct->cx - 350;
    parts [ 2 ] = lpCreateStruct->cx - 175;
    parts [ 3 ] = -1;

    ::SendMessage ( _generator._status_bar._status_hwnd, SB_SETPARTS, 4, (LPARAM) & parts );

    _generator._status_bar.SetPaneText ( 0, _T("Ready") );

    //  Create the client area window

    _generator._form_hwnd = ::CreateDialogParam (   _generator._instance,
                                                    MAKEINTRESOURCE(IDD_PLAYLIST),
                                                    hWnd,
                                                    Playlist_DlgProc,
                                                    (LPARAM) lpCreateStruct->lpCreateParams );

    return TRUE;
}

static void Frame_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    switch ( id )
    {
        case ID_VIEWPLAYLIST:

            {
                ::SendMessage ( _generator._form_hwnd, WM_COMMAND, MAKEWPARAM ( id, BN_CLICKED ), 0 );
            }

            break;
            
        case ID_EXIT:
            
			//	Don't allow app to close until the load thread has terminated

            if (  ::WaitForSingleObject ( _generator._worker_thread, 0 ) == WAIT_TIMEOUT )
            {
                ::MessageBox ( hWnd, _T("You must first CANCEL the current library or playlist action."), _T("Playlist Creator"), MB_ICONINFORMATION );
            }
			else
			{
				::ShowWindow ( hWnd, SW_HIDE );

				_generator.Uninitialize ( );

				::DestroyWindow ( hWnd );
			}

            break;

        case ID_OPTIONS:
            {
                PROPSHEETHEADER         psh;
                PROPSHEETPAGE           pages [ 2 ];
                RPGOptions              library_options ( & _generator );
                RPGOptions              playlist_options ( & _generator );

                ::ZeroMemory ( & psh, sizeof(psh) );
                ::ZeroMemory ( & pages, sizeof(pages) );

                psh.dwSize              = sizeof(psh);
                psh.dwFlags             = PSH_DEFAULT | PSH_NOCONTEXTHELP | PSH_PROPSHEETPAGE;
                psh.hwndParent          = hWnd;
                psh.hInstance           = _generator._instance;
                psh.pszCaption          = _T("Random Playlist Creator Options");
                psh.nPages              = 2;
                psh.nStartPage          = 0;
                psh.ppsp                = pages;

                pages [ 0 ].dwSize      = sizeof ( PROPSHEETPAGE );
                pages [ 0 ].dwFlags     = PSP_DEFAULT;
                pages [ 0 ].hInstance   = _generator._instance;
                pages [ 0 ].pszTemplate = MAKEINTRESOURCE(IDD_LIBRARY_OPTIONS);
                pages [ 0 ].pfnDlgProc  = LibraryOptions_DlgProc;
                pages [ 0 ].lParam      = (LPARAM) & library_options;

                pages [ 1 ].dwSize      = sizeof ( PROPSHEETPAGE );
                pages [ 1 ].dwFlags     = PSP_DEFAULT;
                pages [ 1 ].hInstance   = _generator._instance;
                pages [ 1 ].pszTemplate = MAKEINTRESOURCE(IDD_PLAYLIST_OPTIONS);
                pages [ 1 ].pfnDlgProc  = PlaylistOptions_DlgProc;
                pages [ 1 ].lParam      = (LPARAM) & playlist_options;

                ::PropertySheet ( & psh );
            }

            break;


        case ID_UPDATEDVERSION:

            {
                ::SendMessage ( _generator._form_hwnd, WM_LATESTVERSION, FALSE, 0 );
            }

            break;

        case ID_ABOUT:
            {
                AboutData data;

                data.generator  = & _generator;
                data.background = NULL;

                ::DialogBoxParam ( _generator._instance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, About_DlgProc, (LPARAM) & data );
            }

            break;

    }
}

static void Frame_OnClose ( HWND hWnd )
{
    ::PostMessage ( hWnd, WM_COMMAND, MAKEWPARAM ( ID_EXIT, BN_CLICKED ), 0 );
}

static void Frame_OnDestroy ( HWND hWnd )
{
    ::PostQuitMessage ( 0 );
}

static void Frame_OnGetMinMaxInfo ( HWND hWnd, LPMINMAXINFO lpMinMaxInfo )
{
    //  4/06/06 JWW - Don't shrink app past the point of minimal display of logo

    lpMinMaxInfo->ptMinTrackSize.x = 900;
    lpMinMaxInfo->ptMinTrackSize.y = 340;
}

static void Frame_OnSize ( HWND hWnd, UINT state, int cx, int cy )
{
    RECT rc;

    _generator._status_bar.Lock ();

    ::GetWindowRect ( _generator._status_bar._status_hwnd, & rc );

    _RPT1 ( _CRT_WARN, "Status bar width = %d\r\n", rc.right - rc.left );

    int height = (rc.bottom - rc.top);

    HDWP defer = ::BeginDeferWindowPos ( 2 );

    ::DeferWindowPos ( defer, _generator._status_bar._status_hwnd, hWnd, 0, cy - height, cx, height, SWP_NOZORDER );
    ::DeferWindowPos ( defer, _generator._form_hwnd, hWnd, 0, 0, cx, cy - height, SWP_NOZORDER );

    ::EndDeferWindowPos ( defer );

    //  No need to resize the status bar is the width is 0

    if ( cx != 0 && (rc.right - rc.left) != 0 )
    {    
        _generator._status_bar.SetBarWidth ( cx );

        //  Resize status bar panes as required

        int delta = ( cx != 0 ) ? cx - (rc.right - rc.left) : 0;

        _RPT1 ( _CRT_WARN, "Delta change = %d\r\n", delta );

        _generator._status_bar.SetPaneDelta ( delta );
    }

    _generator._status_bar.Unlock ();
}

static void Frame_OnMenuSelect ( HWND hWnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags )
{
    TCHAR buffer [ MAX_PATH ];

    //  System has closed the menu

    if ( flags == 0xFFFFFFFF)
    {
        _tcscpy ( buffer, _T("Ready") );
    }
    else
    if ( ::LoadString ( _generator._instance, item, buffer, MAX_PATH ) == 0 )
    {
        return;
    }

    _generator._status_bar.SetPaneText ( 0, buffer );
}

static void Frame_OnDropFiles ( HWND hWnd, HDROP hDrop )
{
    TCHAR file_name [ MAX_PATH ];
        
    unsigned count = ::DragQueryFile ( hDrop, -1, NULL, 0 );

    _generator._status_bar.SetPaneText ( 0, _T("Adding titles to music image") );

    for ( unsigned i = 0; i < count; i++ )
    {
        if ( ::DragQueryFile ( hDrop, i, file_name, MAX_PATH ) != 0 )
        {
            _generator.AddMusicFile ( file_name );
        }
    }

    _generator._status_bar.SetPaneText ( 0, _T("Sorting genre") );
    _generator._status_bar.SetPaneText ( 3, _T("") );

    _generator.SortGenre ( );

    _generator._status_bar.SetPaneText ( 0, _T("Sorting artists") );

    _generator.SortArtist ( );

    //  If the list view contains any items, enable the generate button

    if ( ListView_GetItemCount ( ::GetDlgItem ( _generator._form_hwnd, IDC_LIST_TUNES ) ) != 0 )
    {
        ::EnableWindow ( ::GetDlgItem ( _generator._form_hwnd, IDC_BUTTON_GENERATE ), TRUE );
    }

    _generator._status_bar.SetPaneText ( 0, _T("Done") );

    //  Refresh the status bar

    ::SendMessage ( _generator._form_hwnd, WM_UPDATEFILESIZE, 0, 0 );

    ::DragFinish ( hDrop );
}

// End of File



