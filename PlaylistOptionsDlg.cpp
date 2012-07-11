
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "PlaylistOptionsDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/
                                
/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL PlaylistOptions_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void PlaylistOptions_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
static int PlaylistOptions_OnNotify ( HWND hWnd, int id, LPNMHDR lpnmhdr );

/*============================================================================*/
/* D i a l o g  P r o c e d u r e  f o r  P l a y l i s t O p t i o n s D l g */
/*============================================================================*/

INT_PTR CALLBACK PlaylistOptions_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(uMsg)
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,       PlaylistOptions_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,          PlaylistOptions_OnCommand );
        HANDLE_MSG ( hWnd, WM_NOTIFY,           PlaylistOptions_OnNotify );
    }

    return FALSE;
}

static BOOL PlaylistOptions_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    PROPSHEETPAGE * sheet = (PROPSHEETPAGE *) lParam;

    RPGOptions * options = (RPGOptions *) sheet->lParam;
    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) options );

    options->SetLoading ( );

    PlaylistGenerator * generator = options->GetGenerator();

    ::CheckDlgButton ( hWnd, IDC_CHECK_PROMPT_FOR_FILENAME, generator->_prompt_for_playlist );
    ::SetDlgItemText ( hWnd, IDC_EDIT_DEFAULT_PLAYLIST, generator->_default_playlist_name );

    ::CheckRadioButton ( hWnd, IDC_RADIO_SIZE_IN_BYTES, IDC_RADIO_SIZE_IN_GIGABYTES, generator->_size_mode );

    ::CheckRadioButton ( hWnd, IDC_RADIO_LIMIT_REPEATS, IDC_RADIO_ALLOW_DUPLICATES, IDC_RADIO_LIMIT_REPEATS + generator->_duplicate_mode );

    ::SetDlgItemInt ( hWnd, IDC_EDIT_REPEAT_COUNT, generator->_repeat_count, FALSE );

    if ( ::IsDlgButtonChecked ( hWnd, IDC_RADIO_LIMIT_REPEATS ) == BST_UNCHECKED )
    {
        ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_EDIT_REPEAT_COUNT ) , FALSE );
    }

    ::CheckDlgButton ( hWnd, IDC_CHECK_LIMIT_ARTIST, generator->_limit_artist );

    ::SetDlgItemInt ( hWnd, IDC_EDIT_LIMIT_ARTIST_COUNT, generator->_limit_artist_count, FALSE );

    if ( ::IsDlgButtonChecked ( hWnd, IDC_CHECK_LIMIT_ARTIST ) == BST_UNCHECKED )
    {
        ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_EDIT_LIMIT_ARTIST_COUNT ) , FALSE );
    }

    ::CheckDlgButton ( hWnd, IDC_CHECK_RANDOMIZE_SELECTED, generator->_randomize_manual_selections );    

    ::SendDlgItemMessage ( hWnd, IDC_EDIT_DEFAULT_PLAYLIST, EM_SETLIMITTEXT, 255, 0 );

    options->SetLoading ( false );
                                   
    return TRUE;
}

static void PlaylistOptions_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    RPGOptions * options = (RPGOptions *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    switch ( codeNotify )
    {
        case BN_CLICKED:

            {
                switch ( id )
                {
                    case IDC_CHECK_PROMPT_FOR_FILENAME:
                    case IDC_RADIO_SIZE_IN_BYTES:
                    case IDC_RADIO_SIZE_IN_KILOBYTES:
                    case IDC_RADIO_SIZE_IN_MEGABYTES:
                    case IDC_RADIO_SIZE_IN_GIGABYTES:
                    case IDC_CHECK_RANDOMIZE_SELECTED:
                    case IDC_RADIO_LIMIT_REPEATS:
                    case IDC_RADIO_ALLOW_DUPLICATES:
                    case IDC_CHECK_LIMIT_ARTIST:

                        {
                            if ( !options->IsLoading ( ) )
                            {
                                //  Set the changed flag so the 'Apply' button becomes enabled

                                HWND parent = ::GetParent ( hWnd );
                                ::SendMessage ( parent, PSM_CHANGED, (WPARAM) hWnd, 0 );

                                options->SetDirty ( );
                            }

                            if ( id == IDC_RADIO_ALLOW_DUPLICATES )
                                ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_EDIT_REPEAT_COUNT ) , FALSE );
                            else
                            if ( id == IDC_RADIO_LIMIT_REPEATS )
                                ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_EDIT_REPEAT_COUNT ) , TRUE );
                            else
                            if ( id == IDC_CHECK_LIMIT_ARTIST )
                            {
                                BOOL enable = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_LIMIT_ARTIST ) ? TRUE : FALSE;
                                ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_EDIT_LIMIT_ARTIST_COUNT ) , enable );
                            }
                        }

                        break;

                }

            }

            break;


        case EN_CHANGE:

            {
                switch ( id )
                {
                    case IDC_EDIT_DEFAULT_PLAYLIST:

                        {
                            if ( !options->IsLoading ( ) )
                            {
                                //  Set the changed flag so the 'Apply' button becomes enabled

                                HWND parent = ::GetParent ( hWnd );
                                ::SendMessage ( parent, PSM_CHANGED, (WPARAM) hWnd, 0 );

                                options->SetDirty ( );
                            }
                        }

                        break;
                }               
            }

            break;
    }
}

static int PlaylistOptions_OnNotify ( HWND hWnd, int id, LPNMHDR lpnmhdr )
{
    RPGOptions * options = (RPGOptions *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    LPPSHNOTIFY notify = (LPPSHNOTIFY) lpnmhdr;
    int rv = PSNRET_NOERROR;

    switch ( notify->hdr.code )
    {
        case PSN_KILLACTIVE:

            {
                //  We validate changes here (if invalid, SetWindowLong should set 'TRUE' and function should return PSNRET_INVALID)

                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) TRUE );  
            }
            break;

        case PSN_APPLY:

            {
                //  We apply the changes here

                if ( options->IsDirty ( ) )
                {
                    PlaylistGenerator * generator = options->GetGenerator();
                    unsigned size_mode;

                    generator->_prompt_for_playlist = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_PROMPT_FOR_FILENAME );
                    ::GetDlgItemText ( hWnd, IDC_EDIT_DEFAULT_PLAYLIST, generator->_default_playlist_name, MAX_PATH );

                    if ( BST_CHECKED == ::IsDlgButtonChecked ( hWnd, IDC_RADIO_SIZE_IN_BYTES ) )
                    {
                        size_mode = IDC_RADIO_SIZE_IN_BYTES;
                    }
                    else
                    if ( BST_CHECKED == ::IsDlgButtonChecked ( hWnd, IDC_RADIO_SIZE_IN_KILOBYTES ) )
                    {
                        size_mode = IDC_RADIO_SIZE_IN_KILOBYTES;
                    }
                    else
                    if ( BST_CHECKED == ::IsDlgButtonChecked ( hWnd, IDC_RADIO_SIZE_IN_MEGABYTES ) )
                    {
                        size_mode = IDC_RADIO_SIZE_IN_MEGABYTES;
                    }
                    else
                    {
                        size_mode = IDC_RADIO_SIZE_IN_GIGABYTES;                            
                    }

                    if ( size_mode != generator->_size_mode )
                    {
                        if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_SIZE )
                        {
                            ::SendMessage ( generator->_form_hwnd, WM_UPDATEFILESIZE, size_mode, 0 );
                        }

                        generator->_size_mode = size_mode;
                    }

                    generator->_randomize_manual_selections = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_RANDOMIZE_SELECTED );
                    
                    if ( ::IsDlgButtonChecked ( hWnd, IDC_RADIO_LIMIT_REPEATS ) )
                    {
                        generator->_duplicate_mode = LIMIT_REPEATS;
                        generator->_repeat_count = ::GetDlgItemInt ( hWnd, IDC_EDIT_REPEAT_COUNT, NULL, FALSE );
                    }
                    else
                    if ( ::IsDlgButtonChecked ( hWnd, IDC_RADIO_ALLOW_DUPLICATES ) )
                    {
                        generator->_duplicate_mode = ALLOW_DUPLICATES;
                        std::for_each ( generator->_audio_list.begin(), generator->_audio_list.end(), ClearRepeatCount );
                    }

                    generator->_limit_artist = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_LIMIT_ARTIST );
                    if ( generator->_limit_artist == BST_CHECKED )
                    {
                        generator->_limit_artist_count = ::GetDlgItemInt ( hWnd, IDC_EDIT_LIMIT_ARTIST_COUNT, NULL, FALSE );
                    }

                    options->SetDirty ( false );
                }

                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) PSNRET_NOERROR );
            }
            break;
    }

    return rv;
}

// End of File
