
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "OptionsDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/
                                
/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL Options_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void Options_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );

/*==========================================================================*/
/*        D i a l o g  P r o c e d u r e  f o r  O p t i o n s D l g        */
/*==========================================================================*/

INT_PTR CALLBACK Options_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(uMsg)
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,   Options_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,      Options_OnCommand );
    }

    return FALSE;
}

static BOOL Options_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    OptionsData * data = (OptionsData *) lParam;

    ::CheckDlgButton ( hWnd, IDC_CHECK_UPDATE_REMINDER, data->reminder );
    ::SetDlgItemInt ( hWnd, IDC_EDIT_DAYS, data->days, FALSE );
    ::CheckDlgButton ( hWnd, IDC_CHECK_PROMPT_FOR_FILENAME, data->prompt_for_playlist );
    ::SetDlgItemText ( hWnd, IDC_EDIT_DEFAULT_PLAYLIST, data->default_playlist_name );
    ::CheckRadioButton ( hWnd, IDC_RADIO_SIZE_IN_BYTES, IDC_RADIO_SIZE_IN_GIGABYTES, data->size_mode );

    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) data );
                                   
    return TRUE;
}

static void Options_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    OptionsData * data = (OptionsData *)::GetWindowLong ( hWnd, GWL_USERDATA );

    switch (codeNotify)
    {
        case BN_CLICKED:

            switch (id)
            {
                case IDOK:

                    {
                        data->reminder              = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_UPDATE_REMINDER );
                        data->days                  = ::GetDlgItemInt ( hWnd, IDC_EDIT_DAYS, NULL, FALSE );
                        data->prompt_for_playlist   = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_PROMPT_FOR_FILENAME );
                        ::GetDlgItemText ( hWnd, IDC_EDIT_DEFAULT_PLAYLIST, data->default_playlist_name, MAX_PATH );

                        if ( BST_CHECKED == ::IsDlgButtonChecked ( hWnd, IDC_RADIO_SIZE_IN_BYTES ) )
                        {
                            data->size_mode = IDC_RADIO_SIZE_IN_BYTES;
                        }
                        else
                        if ( BST_CHECKED == ::IsDlgButtonChecked ( hWnd, IDC_RADIO_SIZE_IN_KILOBYTES ) )
                        {
                            data->size_mode = IDC_RADIO_SIZE_IN_KILOBYTES;
                        }
                        else
                        if ( BST_CHECKED == ::IsDlgButtonChecked ( hWnd, IDC_RADIO_SIZE_IN_MEGABYTES ) )
                        {
                            data->size_mode = IDC_RADIO_SIZE_IN_MEGABYTES;
                        }
                        else
                        {
                            data->size_mode = IDC_RADIO_SIZE_IN_GIGABYTES;                            
                        }
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
