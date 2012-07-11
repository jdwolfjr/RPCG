
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "AboutDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/
                                
/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL About_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void About_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
static HBRUSH About_OnCtlColor ( HWND hWnd, HDC hdc, HWND hwndChild, int type);
static void About_OnLButtonDown ( HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags );
static void About_OnMouseMove ( HWND hWnd, int x, int y, UINT keyFlags );

static void WriteVersion ( HWND hWnd );

/*==========================================================================*/
/*          D i a l o g  P r o c e d u r e  f o r  A b o u t D l g          */
/*==========================================================================*/

INT_PTR CALLBACK About_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(uMsg)
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,       About_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,          About_OnCommand );
        HANDLE_MSG ( hWnd, WM_CTLCOLORDLG,      About_OnCtlColor );
        HANDLE_MSG ( hWnd, WM_CTLCOLORSTATIC,   About_OnCtlColor );
        HANDLE_MSG ( hWnd, WM_LBUTTONDOWN,      About_OnLButtonDown );        
        HANDLE_MSG ( hWnd, WM_MOUSEMOVE,        About_OnMouseMove );
    }

    return FALSE;
}

static BOOL About_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    AboutData * data = (AboutData *) lParam;
    
    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) data );

    data->background = ::CreateSolidBrush ( RGB(255,255,255) );
    if ( !data->background )
    {
        ::DisplaySystemError ( hWnd );
    }

    WriteVersion ( hWnd );

    return TRUE;
}

static void About_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    AboutData * data = (AboutData *)::GetWindowLong ( hWnd, GWL_USERDATA );

    switch (codeNotify)
    {
        case BN_CLICKED:

            switch (id)
            {
                case IDOK:

                    ::DeleteObject ( data->background );
                    
                    ::EndDialog ( hWnd, 0 );

                    break;
            }

            break;
    }
}

static HBRUSH About_OnCtlColor ( HWND hWnd, HDC hdc, HWND hwndChild, int type) 
{
    AboutData * data = (AboutData *)::GetWindowLong ( hWnd, GWL_USERDATA );

    if ( type == CTLCOLOR_DLG || type == CTLCOLOR_STATIC )
    {
        if ( hwndChild == ::GetDlgItem ( hWnd, IDC_STATIC_WEBLINK ) )
        {
            ::SetTextColor ( hdc, RGB(0,0,0) );
        }
        else
        if ( hwndChild == ::GetDlgItem ( hWnd, IDC_STATIC_SUPPORT ) )
        {
            ::SetTextColor ( hdc, RGB(0,0,0) );
        }

        return data->background;
    }

    return NULL;
}

static void About_OnLButtonDown ( HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
    AboutData * data = (AboutData *)::GetWindowLong ( hWnd, GWL_USERDATA );

    //  Did the user double click on the web link?

    RECT rc;
    ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_STATIC_WEBLINK ), & rc );

    ::ScreenToClient ( hWnd, (POINT *) & rc.left );
    ::ScreenToClient ( hWnd, (POINT *) & rc.right );

    if ( x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom )
    {
        //  Launch web browser to WW Software

        ::ShellExecute ( hWnd, _T("open"), _T("http://software.davewolf.net"), NULL, NULL, SW_SHOWNORMAL );
    }

    //  Try the logo

    ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_STATIC_LOGO ), & rc );

    ::ScreenToClient ( hWnd, (POINT *) & rc.left );
    ::ScreenToClient ( hWnd, (POINT *) & rc.right );

    if ( x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom )
    {
        //  Launch web browser to WW Software

        ::ShellExecute ( hWnd, _T("open"), _T("http://software.davewolf.net"), NULL, NULL, SW_SHOWNORMAL );
    }

    //  Try the support address

    ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_STATIC_SUPPORT ), & rc );

    ::ScreenToClient ( hWnd, (POINT *) & rc.left );
    ::ScreenToClient ( hWnd, (POINT *) & rc.right );

    if ( x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom )
    {
        //  Launch web browser to WW Software support page

        ::ShellExecute ( hWnd, _T("open"), _T("http://www.davewolf.net/software/contact.php"), NULL, NULL, SW_SHOWNORMAL );
    }
}

static void About_OnMouseMove ( HWND hWnd, int x, int y, UINT keyFlags )
{
    AboutData * data = (AboutData *)::GetWindowLong ( hWnd, GWL_USERDATA );
    
    //  If mouse is over logo

    RECT rc;

    ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_STATIC_LOGO ), & rc );

    ::ScreenToClient ( hWnd, (POINT *) & rc.left );
    ::ScreenToClient ( hWnd, (POINT *) & rc.right );

    if ( x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom )
    {
        ::SetCursor ( ::LoadCursor ( NULL, MAKEINTRESOURCE( IDC_HAND ) ) );
    }
    else
    {
        //  If mouse is over web link

        ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_STATIC_WEBLINK ), & rc );

        ::ScreenToClient ( hWnd, (POINT *) & rc.left );
        ::ScreenToClient ( hWnd, (POINT *) & rc.right );

        if ( x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom )
        {
            ::SetCursor ( ::LoadCursor ( NULL, MAKEINTRESOURCE( IDC_HAND ) ) );
        }
        else
        {
            //  If mouse is over support

            ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_STATIC_SUPPORT ), & rc );

            ::ScreenToClient ( hWnd, (POINT *) & rc.left );
            ::ScreenToClient ( hWnd, (POINT *) & rc.right );

            if ( x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom )
            {
                ::SetCursor ( ::LoadCursor ( NULL, MAKEINTRESOURCE( IDC_HAND ) ) );
            }
            else
            {
                ::SetCursor ( ::LoadCursor ( NULL, MAKEINTRESOURCE( IDC_ARROW ) ) );
            }
        }
    }
}

void WriteVersion ( HWND hWnd )
{
    AboutData * data = (AboutData *)::GetWindowLong ( hWnd, GWL_USERDATA );

    //  Read the current version from the RC

    DWORD length;
    VS_FIXEDFILEINFO * ver_info;
    void * info;

    HRSRC hr = ::FindResource ( NULL, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION );
    if ( hr )
    {
        HGLOBAL hg = ::LoadResource ( NULL, hr );
        if ( hg )
        {
            info = LockResource ( hg );

            if ( ::VerQueryValue ( info, _T("\\"), (void **) & ver_info, (unsigned *) & length ) )
            {
                TCHAR buffer [ MAX_PATH ];

                _stprintf ( buffer,
                           _T("Version %d.%d.%d.%d"),
                           HIWORD ( ver_info->dwFileVersionMS ),
                           LOWORD ( ver_info->dwFileVersionMS ),
                           HIWORD ( ver_info->dwFileVersionLS ),
                           LOWORD ( ver_info->dwFileVersionLS ) );

                ::SetDlgItemText ( hWnd, IDC_STATIC_VERSION, buffer );
            }
            
            UnlockResource ( hg );
        }
    }
}


// End of File
