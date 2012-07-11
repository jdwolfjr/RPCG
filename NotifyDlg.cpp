
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "NotifyDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/
                                
/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL Notify_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void Notify_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );

/*==========================================================================*/
/*          D i a l o g  P r o c e d u r e  f o r  N o t i f y D l g        */
/*==========================================================================*/

INT_PTR CALLBACK Notify_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(uMsg)
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,       Notify_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,          Notify_OnCommand );
    }

    return FALSE;
}

static BOOL Notify_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) lParam;

    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) generator );

    HANDLE world = ::LoadImage ( generator->_instance, MAKEINTRESOURCE(IDB_WORLD), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS | LR_SHARED );
    ::SendMessage ( ::GetDlgItem ( hWnd, IDC_STATIC_WORLD ), STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) world );

    return TRUE;
}

static void Notify_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    PlaylistGenerator * generator = (PlaylistGenerator *)::GetWindowLong ( hWnd, GWL_USERDATA );

    switch (codeNotify)
    {
        case BN_CLICKED:

            switch (id)
            {
                case IDYES:
                case IDNO:
                    
                    {
                        generator->_notify_update   = BST_UNCHECKED;
                        generator->_updated_version = ( id == IDYES ) ? BST_CHECKED : BST_UNCHECKED;

                        ::EndDialog ( hWnd, id );
                    }

                    break;
            }

            break;
    }
}

// End of File
