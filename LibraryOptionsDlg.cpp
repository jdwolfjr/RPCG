
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "LibraryOptionsDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/
                                
/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL LibraryOptions_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void LibraryOptions_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
static int LibraryOptions_OnNotify ( HWND hWnd, int id, LPNMHDR lpnmhdr );

/*============================================================================*/
/*  D i a l o g  P r o c e d u r e  f o r  L i b r a r y O p t i o n s D l g  */
/*============================================================================*/

INT_PTR CALLBACK LibraryOptions_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch(uMsg)
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,       LibraryOptions_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,          LibraryOptions_OnCommand );
        HANDLE_MSG ( hWnd, WM_NOTIFY,           LibraryOptions_OnNotify );
    }

    return FALSE;
}

static BOOL LibraryOptions_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    PROPSHEETPAGE * sheet = (PROPSHEETPAGE *) lParam;

    RPGOptions * options = (RPGOptions *) sheet->lParam;    
    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) options );

    options->SetLoading ( );

    PlaylistGenerator * generator = options->GetGenerator();

    ::CheckDlgButton ( hWnd, IDC_CHECK_UPDATE_REMINDER, generator->_reminder );
    ::SetDlgItemInt ( hWnd, IDC_EDIT_DAYS, generator->_days, FALSE );

    ::CheckDlgButton ( hWnd, IDC_CHECK_DIAGNOSTIC_LOG, generator->_log_library );

    ::CheckDlgButton ( hWnd, IDC_CHECK_AGGREGATE_GENRES, generator->_aggregate );
    ::SetDlgItemInt ( hWnd, IDC_EDIT_GENRE_MINIMUM, generator->_aggregate_size, FALSE );
    ::SetDlgItemText ( hWnd, IDC_EDIT_GENRE_MINIMUM_NAME, generator->_default_aggregate_name );

    ::CheckDlgButton ( hWnd, IDC_CHECK_MAINTAIN_SELECTIONS, generator->_maintain_selections );

    ::CheckDlgButton ( hWnd, IDC_CHECK_UPDATED_VERSION, generator->_updated_version );    
    ::SetDlgItemInt ( hWnd, IDC_EDIT_UPDATED_DAYS, generator->_updated_days, FALSE );
    
    ::SendDlgItemMessage ( hWnd, IDC_EDIT_GENRE_MINIMUM_NAME, EM_SETLIMITTEXT, 255, 0 );

    options->SetLoading ( false );

    //  Center the property sheet on the main frame

    HWND main = generator->_main_hwnd;
    HWND prop = ::GetParent ( hWnd );

    RECT rc_main, rc_prop;
    ::GetWindowRect ( main, & rc_main );
    ::GetWindowRect ( prop, & rc_prop );

    int x = rc_main.left + (rc_main.right - rc_main.left) / 2 - (rc_prop.right - rc_prop.left) / 2;
    int y = rc_main.top + (rc_main.bottom - rc_main.top) / 2 - (rc_prop.bottom - rc_prop.top) / 2;
        
    ::MoveWindow ( prop, x, y, (rc_prop.right - rc_prop.left), (rc_prop.bottom - rc_prop.top), TRUE );
                                      
    return TRUE;
}

static void LibraryOptions_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    RPGOptions * options = (RPGOptions *)::GetWindowLong ( hWnd, GWL_USERDATA );

    switch ( codeNotify )
    {
        case BN_CLICKED:

            {
                switch ( id )
                {
                    case IDC_CHECK_UPDATE_REMINDER:
                    case IDC_CHECK_AGGREGATE_GENRES:
                    case IDC_CHECK_MAINTAIN_SELECTIONS:
                    case IDC_CHECK_DIAGNOSTIC_LOG:
                    case IDC_CHECK_UPDATED_VERSION:

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

        case EN_CHANGE:

            {
                switch ( id )
                {
                    case IDC_EDIT_DAYS:
                    case IDC_EDIT_GENRE_MINIMUM:
                    case IDC_EDIT_GENRE_MINIMUM_NAME:
                    case IDC_EDIT_UPDATED_DAYS:

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

static int LibraryOptions_OnNotify ( HWND hWnd, int id, LPNMHDR lpnmhdr )
{
    RPGOptions * options = (RPGOptions *)::GetWindowLong ( hWnd, GWL_USERDATA );

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
                    unsigned aggregate, size;
                    TCHAR buffer [ MAX_PATH ];                    

                    generator->_reminder = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_UPDATE_REMINDER );
                    generator->_days     = ::GetDlgItemInt ( hWnd, IDC_EDIT_DAYS, NULL, FALSE );

                    generator->_log_library = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_DIAGNOSTIC_LOG );
                    
                    aggregate = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_AGGREGATE_GENRES );
                    size = ::GetDlgItemInt ( hWnd, IDC_EDIT_GENRE_MINIMUM, NULL, FALSE );
                    ::GetDlgItemText ( hWnd, IDC_EDIT_GENRE_MINIMUM_NAME, buffer, MAX_PATH );

                    //  If aggregate has changed, all 3 members will get updated so
                    //  SetAggregateSize and SetAggregateName will not get called

                    if ( aggregate != generator->_aggregate )
                    {
                        generator->EnableAggregation ( aggregate, size, buffer );

                        //  Don't forget to update the status bar

                        ::SendMessage ( generator->_form_hwnd, WM_UPDATETUNECOUNT, 0, 0 );
                    }

                    if ( size != generator->_aggregate_size )
                    {
                        generator->SetAggregateSize ( size );

                        //  Don't forget to update the status bar

                        ::SendMessage ( generator->_form_hwnd, WM_UPDATETUNECOUNT, 0, 0 );

                    }

                    if ( _tcscmp ( buffer, generator->_default_aggregate_name) != 0 )
                    {
                        generator->SetAggregateName ( buffer );
                    }                        

                    generator->_maintain_selections = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_MAINTAIN_SELECTIONS );
                    generator->_updated_version     = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_UPDATED_VERSION );
                    generator->_updated_days        = ::GetDlgItemInt ( hWnd, IDC_EDIT_UPDATED_DAYS, NULL, FALSE );

                    options->SetDirty ( false );
                }
    
                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) PSNRET_NOERROR );
            }
            break;
    }

    return rv;
}

// End of File
