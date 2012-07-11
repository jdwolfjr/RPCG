
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "AggregatedDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/
                                
/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL Aggregated_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void Aggregated_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );

/*==========================================================================*/
/*      D i a l o g  P r o c e d u r e  f o r  A g g r e g a t e d D l g    */
/*==========================================================================*/

INT_PTR CALLBACK Aggregated_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(uMsg)
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,       Aggregated_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,          Aggregated_OnCommand );
    }

    return FALSE;
}

static BOOL Aggregated_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) lParam;

    //  Use the actual 'aggregated' name

    TCHAR buffer [ MAX_PATH ];
    _stprintf ( buffer, _T("Genres in '%s'"), generator->_default_aggregate_name );
    ::SetWindowText ( hWnd, buffer );

    IndexListIterator begin = generator->_aggregate_list.begin(), end = generator->_aggregate_list.end();

    for ( ; begin != end; begin++ )
    {
        ::SendDlgItemMessage ( hWnd, IDC_LIST_GENRES, LB_ADDSTRING, 0, (LPARAM) (char *)(*begin)._name.c_str () );
    }

    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) generator );

    return TRUE;
}

static void Aggregated_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

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

// End of File
