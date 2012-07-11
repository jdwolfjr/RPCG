
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "SelectedDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/
                                
/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL Selected_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void Selected_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
static int Selected_OnVkeyToItem ( HWND hWnd, UINT vk, HWND hwndListbox, int iCaret );

/*==========================================================================*/
/*        D i a l o g  P r o c e d u r e  f o r  S e l e c t e d D l g      */
/*==========================================================================*/

INT_PTR CALLBACK Selected_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    _RPT1 ( _CRT_WARN, "Message %x\r\n", uMsg );

    switch(uMsg)
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,       Selected_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,          Selected_OnCommand );
        HANDLE_MSG ( hWnd, WM_VKEYTOITEM,       Selected_OnVkeyToItem );
    }

    return FALSE;
}

static BOOL Selected_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) lParam;

    AudioPtrListIterator begin = generator->_selected_list.begin(), end = generator->_selected_list.end();

    for ( ; begin != end; begin++ )
    {
        int index = ::SendDlgItemMessage ( hWnd, IDC_LIST_TUNES, LB_ADDSTRING, 0, (LPARAM) (TCHAR *)(*begin)->GetTitle () );
        if ( index != -1 )
        {
            ::SendDlgItemMessage ( hWnd, IDC_LIST_TUNES, LB_SETITEMDATA, index, (LPARAM) (*begin) );
        }
    }

    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) generator );

    return TRUE;
}

static void Selected_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
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

static int Selected_OnVkeyToItem ( HWND hWnd, UINT vk, HWND hwndListbox, int iCaret )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    if ( vk == VK_DELETE )
    {
        int count, items [ 1 ];

        while ( (count = ::SendMessage ( hwndListbox, LB_GETSELITEMS, 1, (LPARAM) & items )) != 0 )
        {
            AudioFile * audio_file = (AudioFile *) ::SendMessage ( hwndListbox, LB_GETITEMDATA, items [ 0 ], 0 );
            audio_file->_state = false;

            ::SendMessage ( hwndListbox, LB_DELETESTRING, items [ 0 ], 0 );

            //  Find the audio file in the tunes list box and un-selected it!

            LVFINDINFO item;
            ::ZeroMemory ( & item, sizeof( item ) );

            item.flags  = LVFI_PARAM;
            item.lParam = (LPARAM) audio_file;

            HWND list_view = ::GetDlgItem ( generator->_form_hwnd, IDC_LIST_TUNES );

            int index = ListView_FindItem ( list_view, -1, & item );

            if ( index != -1 )
            {
                ListView_SetCheckState ( list_view, index, audio_file->_state );
            }
        }
    }

    return 0;
}

// End of File
