
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "AudioInfoDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/
                                
/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL AudioInfo_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void AudioInfo_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );

static void SetHorizontalExtent ( HWND hWnd );

/*==========================================================================*/
/*          D i a l o g  P r o c e d u r e  f o r  A b o u t D l g          */
/*==========================================================================*/

INT_PTR CALLBACK AudioInfo_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(uMsg)
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,       AudioInfo_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,          AudioInfo_OnCommand );
    }

    return FALSE;
}

static BOOL AudioInfo_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    TCHAR buffer [ MAX_PATH ];

    AudioFile * audio = (AudioFile *) lParam;

    //  Update the dialog title

    _stprintf ( buffer, _T("Audio Information - %s"), audio->GetTitle() );
    ::SetWindowText ( hWnd, buffer );

    //  Set a nice even left margin

    int tab = 60;

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_SETTABSTOPS, 1, (LPARAM) & tab );

    //  Disable redraw until all items are in list

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, WM_SETREDRAW, FALSE, 0 );

    //  Fill in the list box

    audio->PopulateAudioInformation ( hWnd );

    SetHorizontalExtent ( hWnd );
    
    //  Re-enable redraw

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, WM_SETREDRAW, TRUE, 0 );

    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) audio );

    return TRUE;
}

static void AudioInfo_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    AudioFile * data = (AudioFile *)::GetWindowLong ( hWnd, GWL_USERDATA );

    switch (codeNotify)
    {
        case BN_CLICKED:

            switch (id)
            {
                case IDOK:
                   
                    ::EndDialog ( hWnd, 0 );

                    break;
            }

            break;
    }
}

static void SetHorizontalExtent ( HWND hWnd )
{
    TCHAR buffer [ 1024 ];
    int length, max_len = 0;
    RECT rc;

    int count = ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_GETCOUNT, 0, 0 );

    for ( int i = 0; i < count; i++ )
    {
        length = ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_GETTEXTLEN, i, 0 );
        if ( length > max_len )
        {
            max_len = length;

            ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_GETTEXT , i, (LPARAM) & buffer );
            ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_GETITEMRECT, i, (LPARAM) & rc );
        }
    }    
    
    HDC dc = ::GetDC ( ::GetDlgItem ( hWnd, IDC_LIST_INFO ) );

    SIZE sz;
    ::GetTextExtentPoint32 ( dc, buffer, _tcslen(buffer), & sz );

    if ( sz.cx > rc.right )
    {
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_SETHORIZONTALEXTENT, sz.cx, 0 );
    }

    ::ReleaseDC ( ::GetDlgItem ( hWnd, IDC_LIST_INFO ), dc );
}

// End of File
