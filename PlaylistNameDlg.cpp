
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "PlaylistNameDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/
                                
/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL PlaylistName_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void PlaylistName_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );

/*==========================================================================*/
/*        D i a l o g  P r o c e d u r e  f o r  O p t i o n s D l g        */
/*==========================================================================*/

INT_PTR CALLBACK PlaylistName_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(uMsg)
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,   PlaylistName_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,      PlaylistName_OnCommand );
    }

    return FALSE;
}

static BOOL PlaylistName_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    PlaylistNameData * data = (PlaylistNameData *) lParam;

    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) data );

    ::SetDlgItemText ( hWnd, IDC_EDIT_PLAYLIST_NAME, data->playlist_name );
                                  
    return TRUE;
}

static void PlaylistName_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    PlaylistNameData * data = (PlaylistNameData *)::GetWindowLong ( hWnd, GWL_USERDATA );

    switch (codeNotify)
    {
        case BN_CLICKED:

            switch (id)
            {
                case IDOK:

                    {
                        ::GetDlgItemText ( hWnd, IDC_EDIT_PLAYLIST_NAME, data->playlist_name, MAX_PATH );
                    }

                case IDCANCEL:

                    {
                        ::EndDialog ( hWnd, id );
                    }

                    break;
            }

            break;
    }
}

// End of File
