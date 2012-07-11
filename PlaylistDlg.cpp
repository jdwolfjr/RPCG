
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "PlaylistDlg.h"
#include "PlaylistNameDlg.h"
#include "AudioInfoDlg.h"
#include "SelectedDlg.h"
#include "AggregatedDlg.h"
#include "NotifyDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif	//	_DEBUG

/*==========================================================================*/
/*                      G l o b a l   D a t a                               */
/*==========================================================================*/

/*==========================================================================*/
/*                      P r i v a t e   D a t a                             */
/*==========================================================================*/

static TCHAR * tune_column_headers [ ] =
{
    _T(""),
	_T("Title"),
    _T("Artist"),
	_T("Album"),
	_T("Year"),
	_T("Track"),
	_T("Genre"),
    _T("Type")
};

static TCHAR * genre_column_headers [ ] =
{
    _T(""),
	_T("Genre"),
	_T("Count")
};

static TCHAR * artist_column_headers [ ] =
{
    _T(""),
	_T("Artist"),
	_T("Count")
};

static const LIST_VIEW_SEPARATOR            = 5;
static const char WWW_DAVEWOLF_NET [ ]      = "www.davewolf.net";
static const TCHAR MSGBOX_TITLE [ ]         = _T("Random Playlist Creator");

/*==========================================================================*/
/*                    P r i v a t e   P r o t o t y p e s                   */
/*==========================================================================*/

static BOOL Playlist_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam );
static void Playlist_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify );
static void Playlist_OnSize ( HWND hWnd, UINT state, int cx, int cy );
static void Playlist_OnRButtonDown ( HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags );
static void Playlist_OnLButtonDown ( HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags );
static void Playlist_OnLButtonUp ( HWND hWnd, int x, int y, UINT keyFlags );
static void Playlist_OnMouseMove ( HWND hWnd, int x, int y, UINT keyFlags );
static int  Playlist_OnNotify ( HWND hWnd, int id, LPNMHDR lpnmhdr );
static int  Playlist_OnTimer ( HWND hWnd, int id );

static void Playlist_OnSortPlaylist ( HWND hWnd );
static void Playlist_OnLastUpdated ( HWND hWnd );
static void Playlist_OnUpdateFileSize ( HWND hWnd, int new_size_mode );
static void Playlist_OnNoTunes ( HWND hWnd );
static void Playlist_OnUpdateTuneCount ( HWND hWnd );
static void Playlist_OnProcessMultiple ( HWND hWnd, int state, HWND hwndList );
static void Playlist_OnLatestVersion ( HWND hWnd, BOOL bNotify );
static void Playlist_OnLoadLibrary ( HWND hWnd );

static DWORD CreateListViewHeaders ( HWND hWnd, HWND hWndList, TCHAR * headers [ ], int count );
static void UpdateStatusInfo ( PlaylistGenerator * generator, int id );
static BOOL CanGenerate ( HWND hWnd, PlaylistGenerator * generator );
static void LoadListViews ( HWND hWnd, PlaylistGenerator * generator );
static void RestoreListViewPositions ( HWND hWnd, PlaylistGenerator * generator );

static void GetMaxSize ( HWND hWnd, PlaylistGenerator * generator );
static void SetMaxSize ( HWND hWnd, PlaylistGenerator * generator );

static void PaintListViewBackground ( HWND hWnd, int ctrl_id, HDC dc );
static void LoadCheckBoxText ( HWND hWnd );

void DisableUIElements ( PlaylistGenerator * generator )
{
    //  Enable cancel button

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_CANCEL ),      TRUE );
    generator->_cancel = false;

    //  Disable UI elements

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_EDIT_MUSIC_DIRECTORY ),      FALSE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_BROWSE ),             FALSE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_LOAD ),               FALSE );

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_RADIO_PLAYLIST_BY_NUMBER ),  FALSE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_RADIO_PLAYLIST_BY_SIZE ),    FALSE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_RADIO_PLAYLIST_BY_DURATION ),FALSE );

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_EDIT_PLAYLIST_MAXIMUM ),     FALSE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_EDIT_PLAYLIST_DIRECTORY ),   FALSE );

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_CHECK_COPY_FILE ),           FALSE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_CHECK_RELATIVE_PATHS ),      FALSE );

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_BROWSE_2 ),           FALSE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_GENERATE ),           FALSE );

    //  Disable the 'File' and 'Options' menu selections as well

    MENUITEMINFO item;
    
    item.cbSize  = sizeof(item);
    item.fMask   = MIIM_STATE;
    item.fState = MFS_DISABLED;

    HMENU menu = ::GetMenu ( generator->_main_hwnd );

    ::SetMenuItemInfo ( menu, 0, TRUE, & item );
    ::SetMenuItemInfo ( menu, 1, TRUE, & item );

    ::DrawMenuBar ( generator->_main_hwnd );
}

void EnableUIElements ( PlaylistGenerator * generator )
{
    //  Can't cancel now

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_CANCEL ),      FALSE );
    ::SetWindowText ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_CANCEL ), _T("Cancel") );    

    //  Enable UI elements

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_EDIT_MUSIC_DIRECTORY ),      TRUE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_BROWSE ),             TRUE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_LOAD ),               TRUE );

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_RADIO_PLAYLIST_BY_NUMBER ),  TRUE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_RADIO_PLAYLIST_BY_SIZE ),    TRUE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_RADIO_PLAYLIST_BY_DURATION ),TRUE );

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_EDIT_PLAYLIST_MAXIMUM ),     TRUE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_EDIT_PLAYLIST_DIRECTORY ),   TRUE );

    if ( generator->_copy_tunes )
        ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_CHECK_COPY_FILE ),       TRUE );
    else
    if ( generator->_make_paths_relative )
        ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_CHECK_RELATIVE_PATHS ),  TRUE );
    else
    {
        ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_CHECK_COPY_FILE ),       TRUE );
        ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_CHECK_RELATIVE_PATHS ),  TRUE );
    }

    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_BROWSE_2 ),           TRUE );
    ::EnableWindow ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_GENERATE ),           TRUE );
        
    //  Re-enable the 'File' and 'Options' menu selections as well

    MENUITEMINFO item;
    
    item.cbSize  = sizeof(item);
    item.fMask   = MIIM_STATE;
    item.fState = MFS_ENABLED;

    HMENU menu = ::GetMenu ( generator->_main_hwnd );

    ::SetMenuItemInfo ( menu, 0, TRUE, & item );
    ::SetMenuItemInfo ( menu, 1, TRUE, & item );

    ::DrawMenuBar ( generator->_main_hwnd );

}

void FlashIconic ( PlaylistGenerator * generator )
{
    if ( ::IsIconic ( generator->_main_hwnd ) )
    {
        FLASHWINFO fi;
        ::ZeroMemory ( & fi, sizeof(fi) );

        fi.cbSize   = sizeof(fi);
        fi.hwnd     = generator->_main_hwnd;
        fi.dwFlags  = FLASHW_TIMERNOFG | FLASHW_TRAY | FLASHW_CAPTION;

        ::FlashWindowEx ( & fi );
    }
}

DWORD WINAPI InitializeLibraryThread ( void * data )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) data;

    ::SetWindowText ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_CANCEL ), _T("Cancel Update") );

    DisableUIElements ( generator );

    try
    {
        generator->InitializeLibrary ( );
    }
    catch ( ... )
    {
        KillTimer ( generator->_form_hwnd, 1 );
    }

    //  Reset the status bar parts to their 'default' size

    generator->_status_bar.Lock ();

    int width = generator->_status_bar.GetBarWidth ( );

    int parts [ 4 ];

    parts [ 0 ] = width - 525;
    parts [ 1 ] = width - 350;
    parts [ 2 ] = width - 175;
    parts [ 3 ] = -1;

    ::SendMessage ( generator->_status_bar._status_hwnd, SB_SETPARTS, 4, (LPARAM) & parts );

    generator->_status_bar.Unlock ();

    ::SendMessage ( generator->_form_hwnd, WM_UPDATETUNECOUNT, 0, 0 );

    EnableUIElements ( generator );

    //  Can the user generate a playlist?

    BOOL enable = ::CanGenerate ( generator->_form_hwnd, generator );
    ::EnableWindow ( ::GetDlgItem( generator->_form_hwnd, IDC_BUTTON_GENERATE ), enable );

    //  If no tunes found

    if ( generator->_audio_list.empty ( ) )
    {
        TCHAR message [ 1024 ];
        ::LoadString ( generator->_instance, IDS_NOAUDIOFILES, message, 1024 );

        if ( IDYES == ::MessageBox ( generator->_form_hwnd, message, MSGBOX_TITLE, MB_ICONQUESTION | MB_YESNO ) )
        {
            ::PostMessage ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_BROWSE ), BM_CLICK, 0, 0 );
        }
    }

    // If the frame window is minimized, then flash to gain attention

    FlashIconic ( generator );

    //  Close and NULL thread handle

    ::CloseHandle ( generator->_worker_thread );
     generator->_worker_thread  = NULL;

    return 0;
}

DWORD WINAPI GeneratePlaylist ( void * data )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) data;

    ::SetWindowText ( ::GetDlgItem ( generator->_form_hwnd, IDC_BUTTON_CANCEL ), _T("Cancel Playlist") );

    DisableUIElements ( generator );

    try
    {
        generator->GeneratePlaylist ( );
    }
    catch ( ... )
    {
        
    }

    EnableUIElements ( generator );

    // If the frame window is minimized, then flash to gain attention

    FlashIconic ( generator );

    //  Close and NULL thread handle

    ::CloseHandle ( generator->_worker_thread );
     generator->_worker_thread  = NULL;

    return 0;
}

static int CALLBACK BrowseCallbackProc ( HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    if ( uMsg == BFFM_INITIALIZED )
    {
        ::SendMessage ( hWnd, WM_SETTEXT, 0, (LPARAM) _T("Music Library") );

        if ( lpData )
            ::SendMessage ( hWnd, BFFM_SETEXPANDED, 1, lpData );
    }

    return 0;
}

/*==========================================================================*/
/*        W i n d o w  P r o c e d u r e  f o r  P l a y l i s t D l g      */
/*==========================================================================*/

INT_PTR CALLBACK Playlist_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( uMsg )
    {
        HANDLE_MSG ( hWnd, WM_INITDIALOG,       Playlist_OnInitDialog );
        HANDLE_MSG ( hWnd, WM_COMMAND,          Playlist_OnCommand );
        HANDLE_MSG ( hWnd, WM_SIZE,             Playlist_OnSize );
        HANDLE_MSG ( hWnd, WM_RBUTTONDOWN,      Playlist_OnRButtonDown );
        HANDLE_MSG ( hWnd, WM_LBUTTONDOWN,      Playlist_OnLButtonDown );  
        HANDLE_MSG ( hWnd, WM_LBUTTONDBLCLK,    Playlist_OnLButtonDown );  
        HANDLE_MSG ( hWnd, WM_LBUTTONUP,        Playlist_OnLButtonUp );
        HANDLE_MSG ( hWnd, WM_MOUSEMOVE,        Playlist_OnMouseMove );
        HANDLE_MSG ( hWnd, WM_NOTIFY,           Playlist_OnNotify );
        HANDLE_MSG ( hWnd, WM_TIMER,            Playlist_OnTimer );
        HANDLE_MSG ( hWnd, WM_SORTPLAYLIST,     Playlist_OnSortPlaylist );
        HANDLE_MSG ( hWnd, WM_LASTUPDATED,      Playlist_OnLastUpdated );
        HANDLE_MSG ( hWnd, WM_UPDATEFILESIZE,   Playlist_OnUpdateFileSize );
        HANDLE_MSG ( hWnd, WM_NOTUNES,          Playlist_OnNoTunes );
        HANDLE_MSG ( hWnd, WM_UPDATETUNECOUNT,  Playlist_OnUpdateTuneCount );
        HANDLE_MSG ( hWnd, WM_PROCESSMULTIPLE,  Playlist_OnProcessMultiple );
        HANDLE_MSG ( hWnd, WM_LATESTVERSION,    Playlist_OnLatestVersion );
        HANDLE_MSG ( hWnd, WM_LOADLIBRARY,      Playlist_OnLoadLibrary );
    }

    return FALSE;
}

static BOOL Playlist_OnInitDialog ( HWND hWnd, HWND hWndFocus, LPARAM lParam )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) lParam;

    //  Save our pointer to the PlaylistGenerator class

    ::SetWindowLong ( hWnd, GWL_USERDATA, (long) generator );

    LoadCheckBoxText ( hWnd );

    RECT rc;
    ::GetWindowRect ( hWnd, & rc );

	//	Reads the registry and parses in the 'dat' files from the last session

    generator->Initialize ( hWnd );

    generator->_update_column_width = true;

    //  Create the image list for our column header

    HIMAGELIST image_list = ::ImageList_LoadBitmap ( generator->_instance, MAKEINTRESOURCE( IDB_CHECK ), 13, 1, RGB(255, 255, 255) );   

    HDITEM hditem;
    ::ZeroMemory ( & hditem, sizeof(hditem) );

    hditem.mask     = HDI_FORMAT | HDI_IMAGE;
    hditem.fmt      = HDF_IMAGE;
    hditem.iImage   = 0;

    //  Enable selection check boxes in tunes list box

    HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );

    HWND header = ListView_GetHeader ( list_view );
    Header_SetImageList ( header, image_list );

    ListView_SetExtendedListViewStyle ( list_view, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT );

    CreateListViewHeaders ( hWnd, list_view, tune_column_headers, sizeof(tune_column_headers) / sizeof(tune_column_headers [ 0 ]) );

    //  Enable selection check boxes in genre list box

    list_view = ::GetDlgItem ( hWnd, IDC_LIST_GENRES );

    header = ListView_GetHeader ( list_view );
    Header_SetImageList ( header, image_list );

    ListView_SetExtendedListViewStyle ( list_view, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT );

    CreateListViewHeaders ( hWnd, list_view, genre_column_headers, sizeof(genre_column_headers) / sizeof(genre_column_headers [ 0 ]) );

    //  Enable selection check boxes in artist list box

    list_view = ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS );

    header = ListView_GetHeader ( list_view );
    Header_SetImageList ( header, image_list );

    ListView_SetExtendedListViewStyle ( list_view, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT );

    CreateListViewHeaders ( hWnd, list_view, artist_column_headers, sizeof(artist_column_headers) / sizeof(artist_column_headers [ 0 ]) );

 	//	Load the list view controls ( true means to restore the column widths)

	//LoadListViews ( hWnd, generator );

    //  Restore the list views to their last known positions

    RestoreListViewPositions ( hWnd, generator );

    //  Populate the other controls
    
    BOOL enable = FALSE;

    if ( _tcslen (generator->_music_directory) )
    {
        enable = TRUE;
        ::SetDlgItemText ( hWnd, IDC_EDIT_MUSIC_DIRECTORY, generator->_music_directory );
    }

    ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_BUTTON_LOAD ), enable );
    ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_BUTTON_CANCEL ), FALSE );

    ::CheckRadioButton ( hWnd, IDC_RADIO_PLAYLIST_BY_NUMBER, IDC_RADIO_PLAYLIST_BY_DURATION, generator->_playlist_mode );

    if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_NUMBER )
    {
        ::SetDlgItemInt( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, generator->_max_tunes, FALSE );
        ::SetDlgItemText ( hWnd, IDC_STATIC_MAXIMUM, _T("Songs") );
    }
    else
    if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_SIZE )
    {
        SetMaxSize ( hWnd, generator );
    }
    else
    if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_DURATION )
    {
        ::SetDlgItemInt( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, generator->_max_duration, FALSE );
        ::SetDlgItemText ( hWnd, IDC_STATIC_MAXIMUM, _T("Seconds") );
    }
    
    ::CheckDlgButton ( hWnd, IDC_CHECK_COPY_FILE, generator->_copy_tunes );

    if ( _tcslen (generator->_playlist_directory) )
    {
        ::SetDlgItemText ( hWnd, IDC_EDIT_PLAYLIST_DIRECTORY, generator->_playlist_directory );
    }

    ::CheckDlgButton ( hWnd, IDC_CHECK_RELATIVE_PATHS, generator->_make_paths_relative );

    if ( ::IsDlgButtonChecked ( hWnd, IDC_CHECK_COPY_FILE ) == BST_CHECKED )
    {
        ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_CHECK_RELATIVE_PATHS ) , FALSE );
    }
    else
    if ( ::IsDlgButtonChecked ( hWnd, IDC_CHECK_RELATIVE_PATHS ) == BST_CHECKED )
    {
        ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_CHECK_COPY_FILE ) , FALSE );
    }

    /*
    //  Test to see if we are able to generate a playlist

    enable = ::CanGenerate ( hWnd, generator );
    ::EnableWindow ( ::GetDlgItem( hWnd, IDC_BUTTON_GENERATE ), enable );

    generator->_logo = ::LoadImage ( generator->_instance,
                                     MAKEINTRESOURCE ( IDB_PLAYLIST ),
                                     IMAGE_BITMAP,
                                     0, 0,
                                     LR_LOADTRANSPARENT | LR_CREATEDIBSECTION | LR_DEFAULTSIZE );

    //  Do we need to update the library?

    if ( generator->_audio_list.empty ( ) )
    {
        ::PostMessage ( hWnd, WM_NOTUNES, 0, 0 );
    }
    else
    if ( generator->IsTimeToUpdate ( RPC_UPDATE_LIBARAY ) )
    {
        ::PostMessage ( hWnd, WM_LASTUPDATED, 0, 0 );
    }

    //  Make sure to announce that we are going to connect to www.davewolf.net

    if ( generator->_updated_version == BST_CHECKED && generator->IsTimeToUpdate ( RPG_UPDATE_VERSION) )
    {
        BOOL notifiy = (generator->_notify_update == BST_CHECKED) ? TRUE : FALSE;
        ::PostMessage ( hWnd, WM_LATESTVERSION, notifiy, 0 );
    }
    */

    //  Set the last column width to the maximum

    ListView_SetColumnWidth ( ::GetDlgItem ( hWnd, IDC_LIST_TUNES ), 7, LVSCW_AUTOSIZE_USEHEADER );
    ListView_SetColumnWidth ( ::GetDlgItem ( hWnd, IDC_LIST_GENRES ), 2, LVSCW_AUTOSIZE_USEHEADER );
    ListView_SetColumnWidth ( ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS ), 2, LVSCW_AUTOSIZE_USEHEADER );

    generator->_update_column_width = false;

    //Playlist_OnUpdateTuneCount ( hWnd );

    ::PostMessage ( hWnd, WM_LOADLIBRARY, 0, 0 );

    return FALSE;
}

static void Playlist_OnCommand ( HWND hWnd, int id, HWND hWndCtl, UINT codeNotify )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    switch ( codeNotify )
    {
        case BN_CLICKED:

            {
                switch ( id )
                {
		            case ID_VIEWPLAYLIST:

			            {
                            TCHAR filename [ MAX_PATH ];

                            OPENFILENAME ofn;
                            ::ZeroMemory ( & ofn, sizeof ( ofn ) );

                            filename [ 0 ] = '\0';

                            ofn.lStructSize     = sizeof ( ofn );
                            ofn.hwndOwner       = hWnd;
                            ofn.lpstrFilter     = _T("Playlist files (*.m3u)\0*.m3u\0All files (*.*)\0*.*\0\0");
                            ofn.Flags           = OFN_EXPLORER | OFN_FILEMUSTEXIST;
                            ofn.lpstrDefExt     = _T("m3u");
                            ofn.lpstrFile       = (TCHAR *) filename;
                            ofn.nMaxFile        = MAX_PATH;
                            ofn.lpstrInitialDir = generator->_playlist_directory;
                            ofn.lpstrTitle      = _T("Open Playlist");

                            if ( ! ::GetOpenFileName ( & ofn ) )
                            {
                                return;
                            }

                            TCHAR buffer [ MAX_PATH ];

				            STARTUPINFO			si;
				            PROCESS_INFORMATION pi;

				            ::ZeroMemory ( & si, sizeof( si ) );
				            ::ZeroMemory ( & pi, sizeof( pi ) );

				            si.cb = sizeof( si );

                            _stprintf ( buffer, _T("notepad.exe \"%s\""), filename );

				            ::CreateProcess ( NULL,
								              buffer,
								              NULL,
								              NULL,
								              FALSE,
								              0,
								              NULL,
								              NULL,
								              & si,
								              & pi );

				            ::CloseHandle ( pi.hProcess );
				            ::CloseHandle ( pi.hThread );

			            }	

			            break;

                    case ID_DELETE:
            
                        {
                            HWND tunes_list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );
                            HWND genres_list_view = ::GetDlgItem ( hWnd, IDC_LIST_GENRES );
                            HWND artists_list_view = ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS );

                            //  Disable controls

                            ::SendMessage ( tunes_list_view, WM_SETREDRAW, FALSE, 0 );
                            ::SendMessage ( genres_list_view, WM_SETREDRAW, FALSE, 0 );
                            ::SendMessage ( artists_list_view, WM_SETREDRAW, FALSE, 0 );

                            //  If all items are to be deleted, we can use a bit more efficient method

                            if ( ListView_GetItemCount ( tunes_list_view ) == ListView_GetSelectedCount ( tunes_list_view ) )
                            {
                                ListView_DeleteAllItems ( tunes_list_view );
                                ListView_DeleteAllItems ( genres_list_view );
                                ListView_DeleteAllItems ( artists_list_view );

                                generator->CleanUp ( );
                            }
                            else
                            {
                                LVITEM item;
                                ::ZeroMemory ( & item, sizeof( item ) );

                                item.mask  = LVIF_PARAM;

                                while ( (item.iItem = ListView_GetNextItem ( tunes_list_view, -1, LVNI_SELECTED )) != -1 )
                                {
                                    ListView_GetItem ( tunes_list_view, & item );
                                    ListView_DeleteItem ( tunes_list_view, item.iItem );

                                    generator->DeleteTune ( item );
                                }

                                //  Resort our genre and artist

                                generator->SortGenre ( );
                                generator->SortArtist ( );
                            }

                            //  Enable controls

                            ::SendMessage ( tunes_list_view, WM_SETREDRAW, TRUE, 0 );
                            ::SendMessage ( genres_list_view, WM_SETREDRAW, TRUE, 0 );
                            ::SendMessage ( artists_list_view, WM_SETREDRAW, TRUE, 0 );

                            //  Test to see if we are able to generate a playlist

                            BOOL enable = ::CanGenerate ( hWnd, generator );

                            ::EnableWindow ( ::GetDlgItem( hWnd, IDC_BUTTON_GENERATE ), enable );                            

                            Playlist_OnUpdateTuneCount ( hWnd );

                            //  Make sure to the last column to fill up the width of the control

                            ListView_SetColumnWidth ( tunes_list_view, 7, LVSCW_AUTOSIZE_USEHEADER );

                        }

                        break;

                   case ID_SELECTALL:
            
                        {
                            HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );
                            ListView_SetItemState ( list_view, -1, LVIS_SELECTED, LVIS_SELECTED );
                        }

                        break;

                    case ID_CHECKSELECTED:
                    case ID_UNCHECKSELECTED:
            
                        {
				            BOOL state = ( id == ID_CHECKSELECTED ) ? TRUE : FALSE;

                            HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );
                            int index = -1;

                            while ( (index = ListView_GetNextItem ( list_view, index, LVNI_SELECTED )) != -1 )
                            {
					            ListView_SetCheckState ( list_view, index, state );				
                            }
                        }

			            break;

                   case ID_SELECTALLGENRE:
            
                        {
                            HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_GENRES );
                            ListView_SetItemState ( list_view, -1, LVIS_SELECTED, LVIS_SELECTED );
                        }

                        break;

                    case ID_CHECKSELECTEDGENRE:
                    case ID_UNCHECKSELECTEDGENRE:
            
                        {
				            BOOL state = ( id == ID_CHECKSELECTEDGENRE ) ? TRUE : FALSE;

                            HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_GENRES );
                            int index = -1;

                            while ( (index = ListView_GetNextItem ( list_view, index, LVNI_SELECTED )) != -1 )
                            {
					            ListView_SetCheckState ( list_view, index, state );				
                            }
                        }

			            break;

                   case ID_SELECTALLARTISTS:
            
                        {
                            HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS );
                            ListView_SetItemState ( list_view, -1, LVIS_SELECTED, LVIS_SELECTED );
                        }

                        break;

                    case ID_CHECKSELECTEDARTISTS:
                    case ID_UNCHECKSELECTEDARTISTS:
            
                        {
				            BOOL state = ( id == ID_CHECKSELECTEDARTISTS ) ? TRUE : FALSE;

                            HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS );
                            int index = -1;

                            while ( (index = ListView_GetNextItem ( list_view, index, LVNI_SELECTED )) != -1 )
                            {
					            ListView_SetCheckState ( list_view, index, state );				
                            }
                        }

			            break;

                    case IDC_CHECK_COPY_FILE:

                        {
                            generator->_copy_tunes = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_COPY_FILE );

                            if ( generator->_copy_tunes == BST_CHECKED )
                            {
                                ::CheckDlgButton ( hWnd, IDC_CHECK_RELATIVE_PATHS, BST_UNCHECKED );
                                ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_CHECK_RELATIVE_PATHS ) , FALSE );
                            }
                            else
                            {
                                ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_CHECK_RELATIVE_PATHS ) , TRUE );
                            }
                        }

                        break;

                    case IDC_CHECK_RELATIVE_PATHS:

                        {
                            generator->_make_paths_relative = ::IsDlgButtonChecked ( hWnd, IDC_CHECK_RELATIVE_PATHS );

                            if ( generator->_make_paths_relative == BST_CHECKED )
                            {
                                ::CheckDlgButton ( hWnd, IDC_CHECK_COPY_FILE, BST_UNCHECKED );
                                ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_CHECK_COPY_FILE ) , FALSE );
                            }
                            else
                            {
                                ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_CHECK_COPY_FILE ) , TRUE );
                            }
                        }

                        break;
                        

                    case IDC_BUTTON_BROWSE:

                        {
                            BROWSEINFO browse_info;

                            browse_info.hwndOwner       = ::GetParent ( hWnd );
                            browse_info.pidlRoot        = NULL;
                            browse_info.pszDisplayName  = generator->_music_directory;
                            browse_info.lpszTitle       = _T("Select the directory that contains the your music.");
                            browse_info.ulFlags         = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
                            browse_info.lpfn            = BrowseCallbackProc;
                            browse_info.lParam          = 0;
                            browse_info.iImage          = 0;

                            //  Try to open where we are pointing

                            TCHAR buffer [ MAX_PATH ];
                            WCHAR music_path [ MAX_PATH ];

                            unsigned length = ::GetDlgItemText ( hWnd, IDC_EDIT_MUSIC_DIRECTORY, buffer, MAX_PATH );

                            if ( length )
                            {                       
#ifndef _UNICODE
                                ::MultiByteToWideChar ( CP_ACP, 0, buffer, -1, (WCHAR *)music_path, MAX_PATH );
#else
                                _tcscpy ( music_path, buffer );
#endif
                                browse_info.lParam  = (LPARAM) music_path;
                            }
                
                            LPITEMIDLIST item_list = ::SHBrowseForFolder ( & browse_info );

                            if ( !item_list )
                            {
                                generator->_auto_update_image = false;
                                return;
                            }

                            generator->_music_directory [ 0 ] = '\0';

                            SHGetPathFromIDList ( item_list, generator->_music_directory );               

                            IMalloc * imalloc = NULL;

                            HRESULT hr = ::SHGetMalloc ( & imalloc );
                            if ( SUCCEEDED(hr ) )
                            {
                                imalloc->Free ( (void *) item_list );
                                if ( browse_info.pidlRoot )
                                    imalloc->Free ( (void *) browse_info.pidlRoot );
                                imalloc->Release ( );
                            }

                            if ( _tcslen ( generator->_music_directory ) == 0 )
                            {
                                generator->_auto_update_image = false;
                                return;
                            }

                            ::SetDlgItemText ( hWnd, IDC_EDIT_MUSIC_DIRECTORY, generator->_music_directory );

                            if ( generator->_auto_update_image )
                            {
                                generator->_auto_update_image = false;
                                ::PostMessage ( ::GetDlgItem ( hWnd, IDC_BUTTON_LOAD ), BM_CLICK, 0, 0 );                                
                            }
                        }

                        break;

                    case IDC_BUTTON_BROWSE_2:

                        {
                            BROWSEINFO browse_info;

                            browse_info.hwndOwner       = ::GetParent ( hWnd );
                            browse_info.pidlRoot        = NULL;
                            browse_info.pszDisplayName  = generator->_playlist_directory;
                            browse_info.lpszTitle       = _T("Select the destination directory for your playlist.");
                            browse_info.ulFlags         = BIF_NEWDIALOGSTYLE;
                            browse_info.lpfn            = BrowseCallbackProc;
                            browse_info.lParam          = 0;
                            browse_info.iImage          = 0;

                            //  Try to open where we are pointing

                            TCHAR buffer [ MAX_PATH ];
                            WCHAR music_path [ MAX_PATH ];

                            unsigned length = ::GetDlgItemText ( hWnd, IDC_EDIT_PLAYLIST_DIRECTORY, buffer, MAX_PATH );

                            if ( length )
                            {                       
#ifndef _UNICODE
                                ::MultiByteToWideChar ( CP_ACP, 0, buffer, -1, (WCHAR *)music_path, MAX_PATH );
#else
                                _tcscpy ( music_path, buffer );
#endif

                                browse_info.lParam  = (LPARAM) music_path;
                            }
                
                            LPITEMIDLIST item_list = ::SHBrowseForFolder ( & browse_info );

                            if ( !item_list )
                            {
                                return;
                            }

                            generator->_playlist_directory [ 0 ] = '\0';

                            SHGetPathFromIDList ( item_list, generator->_playlist_directory );               

                            IMalloc * imalloc = NULL;

                            HRESULT hr = ::SHGetMalloc ( & imalloc );
                            if ( SUCCEEDED(hr ) )
                            {
                                imalloc->Free ( (void *) item_list );
                                if ( browse_info.pidlRoot )
                                    imalloc->Free ( (void *) browse_info.pidlRoot );
                                imalloc->Release ( );
                            }

                            if ( _tcslen ( generator->_playlist_directory ) == 0 )
                            {
                                return;
                            }

                            ::SetDlgItemText ( hWnd, IDC_EDIT_PLAYLIST_DIRECTORY, generator->_playlist_directory );
                        }

                        break;

                    case IDC_BUTTON_LOAD:

                        {
                            ::GetDlgItemText ( hWnd, IDC_EDIT_MUSIC_DIRECTORY, generator->_music_directory, MAX_PATH );

                            generator->_worker_thread  = ::CreateThread ( NULL, 0, InitializeLibraryThread, (void *)generator, 0, NULL );
                            if ( !generator->_worker_thread )
                            {
                                ::DisplaySystemError ( hWnd );                                 
                            }
                        }

                        break;

                    case IDC_BUTTON_CANCEL:

                        {
                            generator->_cancel = true;
                            ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_BUTTON_CANCEL ), FALSE );
                        }

                        break;

                    case IDC_BUTTON_GENERATE:

                        {

                            generator->_worker_thread  = ::CreateThread ( NULL, 0, GeneratePlaylist, (void *)generator, 0, NULL );
                            if ( !generator->_worker_thread )
                            {
                                ::DisplaySystemError ( hWnd );                                 
                            }

                            /*

                            PlaylistNameData data;

                            _tcscpy ( data.playlist_name, generator->_default_playlist_name );

                            if ( generator->_prompt_for_playlist == BST_CHECKED )
                            {                              
                                if ( IDCANCEL == ::DialogBoxParam ( generator->_instance, MAKEINTRESOURCE(IDD_PLAYLIST_NAME), generator->_main_hwnd, PlaylistName_DlgProc, (LPARAM) & data ) )
                                    return;

                                //  Need to redraw when _copy_tunes is checked because it will take a few minutes for all the
                                //  files to be copied the window will be 'dirty' until then

                                if ( generator->_copy_tunes == BST_CHECKED )
                                {
                                    ::RedrawWindow ( hWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW );
                                }
                            }

                            //::EnableWindow ( ::GetDlgItem ( hWnd, IDC_BUTTON_GENERATE ), FALSE );
                            //::EnableWindow ( ::GetDlgItem ( hWnd, IDC_BUTTON_LOAD ), FALSE );

                            generator->GeneratePlaylist ( hWnd, data.playlist_name );

                            //::EnableWindow ( ::GetDlgItem ( hWnd, IDC_BUTTON_GENERATE ), TRUE );
                            //::EnableWindow ( ::GetDlgItem ( hWnd, IDC_BUTTON_LOAD ), TRUE );
                            */
                        }

                        break;

                    case IDC_RADIO_PLAYLIST_BY_NUMBER:

                        {
                            //  Read the current value

                            if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_SIZE )
                            {
                                GetMaxSize ( hWnd, generator );
                            }
                            else
                            if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_DURATION )
                            {
                                generator->_max_duration = ::GetDlgItemInt ( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, NULL, FALSE );
                            }

                            //  Set the max number of songs

                            ::SetDlgItemInt( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, generator->_max_tunes, FALSE );
                            ::SetDlgItemText ( hWnd, IDC_STATIC_MAXIMUM, _T("Songs") );

                            generator->_playlist_mode = IDC_RADIO_PLAYLIST_BY_NUMBER;

                            //  Test to see if we are able to generate a playlist

                            BOOL enable = ::CanGenerate ( hWnd, generator );

                            ::EnableWindow ( ::GetDlgItem( hWnd, IDC_BUTTON_GENERATE ), enable );                                
                        }

                        break;

                    case IDC_RADIO_PLAYLIST_BY_SIZE:

                        {
                            //  Read the current value

                            if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_NUMBER )
                            {
                                generator->_max_tunes = ::GetDlgItemInt ( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, NULL, FALSE );
                            }
                            else
                            if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_DURATION )
                            {
                                generator->_max_duration = ::GetDlgItemInt ( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, NULL, FALSE );
                            }

                            //  Set the maximum number of bytes

                            SetMaxSize ( hWnd, generator );

                            generator->_playlist_mode = IDC_RADIO_PLAYLIST_BY_SIZE;

                            //  Test to see if we are able to generate a playlist

                            BOOL enable = ::CanGenerate ( hWnd, generator );

                            ::EnableWindow ( ::GetDlgItem( hWnd, IDC_BUTTON_GENERATE ), enable );       
                        }

                        break;

                    case IDC_RADIO_PLAYLIST_BY_DURATION:

                        {
                            //  Read the current value

                            if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_SIZE )
                            {
                                GetMaxSize ( hWnd, generator );
                            }
                            else
                            if ( generator->_playlist_mode == IDC_RADIO_PLAYLIST_BY_NUMBER )
                            {
                                generator->_max_tunes = ::GetDlgItemInt ( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, NULL, FALSE );
                            }

                            //  Set the max number of songs

                            ::SetDlgItemInt( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, generator->_max_duration, FALSE );
                            ::SetDlgItemText ( hWnd, IDC_STATIC_MAXIMUM, _T("Seconds") );

                            generator->_playlist_mode = IDC_RADIO_PLAYLIST_BY_DURATION;

                            //  Test to see if we are able to generate a playlist

                            BOOL enable = ::CanGenerate ( hWnd, generator );

                            ::EnableWindow ( ::GetDlgItem( hWnd, IDC_BUTTON_GENERATE ), enable );                                
                        }

                        break;

                    case ID_AUDIOINFO:
                        
                        {
                            HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );
                            
                            int index = ListView_GetNextItem ( list_view, -1, LVNI_SELECTED );
                            if ( index != -1 )
                            {
                                LVITEM  item;

                                ::ZeroMemory ( & item, sizeof( item ) );

                                item.mask       = LVIF_PARAM;
                                item.iItem      = index;

                                ListView_GetItem ( list_view, & item );

                                ::DialogBoxParam ( generator->_instance, MAKEINTRESOURCE(IDD_AUDIO_INFO), hWnd, AudioInfo_DlgProc, (LPARAM) item.lParam );
                            }                            
                        }

                        break;

                    case ID_PLAYSELECTED:

                        {
                            generator->PlaySelected ( hWnd );
                        }

                        break;


                    case ID_SELECTED:

                        {
                            ::DialogBoxParam ( generator->_instance, MAKEINTRESOURCE(IDD_SELECTED), hWnd, Selected_DlgProc, (LPARAM) generator );

                        }

                        break;

                    case ID_GENRE:

                        {
                            ::DialogBoxParam ( generator->_instance, MAKEINTRESOURCE(IDD_AGGREGATED), hWnd, Aggregated_DlgProc, (LPARAM) generator );

                        }

                        break;
                }
            }

            break;

        case EN_CHANGE:

            {
                switch ( id )
                {
                    case IDC_EDIT_MUSIC_DIRECTORY:

                        {
                            //   Need to validate length first

                            ::GetDlgItemText ( hWnd, IDC_EDIT_MUSIC_DIRECTORY, generator->_music_directory, MAX_PATH );

                            BOOL enable = ( PathIsDirectory ( generator->_music_directory ) ) ? TRUE : FALSE;
                            ::EnableWindow ( ::GetDlgItem ( hWnd, IDC_BUTTON_LOAD ), enable );
                        }
                        
                        break;

                    case IDC_EDIT_PLAYLIST_DIRECTORY:

                        {
                            //   Need to validate length first

                            ::GetDlgItemText ( hWnd, IDC_EDIT_PLAYLIST_DIRECTORY, generator->_playlist_directory, MAX_PATH );
                        }

                        break;

                    case IDC_EDIT_PLAYLIST_MAXIMUM:

                        {
                            if ( BST_CHECKED == ::IsDlgButtonChecked ( hWnd, IDC_RADIO_PLAYLIST_BY_NUMBER ) )
                            {
                                generator->_max_tunes = ::GetDlgItemInt ( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, NULL, FALSE );
                            }
                            else
                            if ( BST_CHECKED == ::IsDlgButtonChecked ( hWnd, IDC_RADIO_PLAYLIST_BY_SIZE ) )
                            {
                                GetMaxSize ( hWnd, generator );
                            }
                            else
                            if ( BST_CHECKED == ::IsDlgButtonChecked ( hWnd, IDC_RADIO_PLAYLIST_BY_DURATION ) )
                            {
                                generator->_max_duration = ::GetDlgItemInt ( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, NULL, FALSE );
                            }

                            //  Test to see if we are able to generate a playlist

                            BOOL enable = ::CanGenerate ( hWnd, generator );

                            ::EnableWindow ( ::GetDlgItem( hWnd, IDC_BUTTON_GENERATE ), enable );
                        }

                        break;
                }                        
            }

            break;

        case EN_SETFOCUS:

            {
                ::UpdateStatusInfo ( generator, id );
            }

            break;

        case EN_KILLFOCUS:

            {
                ::UpdateStatusInfo ( generator, -1 );
            }

            break;
    }
}

static void Playlist_OnSize ( HWND hWnd, UINT state, int cx, int cy )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    _RPT2 ( _CRT_WARN, "CX = %d\tCY = %d\r\n", cx, cy );

    // Resize the tunes list view

    HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );

    RECT rcList;
    ::GetWindowRect ( list_view, & rcList );

    ::ScreenToClient ( hWnd, (POINT *) & rcList.left );
    ::ScreenToClient ( hWnd, (POINT *) & rcList.right );

    if ( !::SetWindowPos ( list_view, NULL, 3, rcList.top, cx - 6, rcList.bottom - rcList.top, SWP_NOZORDER ) )
    {
        ::DisplaySystemError ( hWnd );
    }

    int top     = rcList.bottom + LIST_VIEW_SEPARATOR;
    int width   = ((cx - LIST_VIEW_SEPARATOR) >> 1) - 3;
    
    //  Adjust column width

    ListView_SetColumnWidth ( list_view, 7, LVSCW_AUTOSIZE_USEHEADER );

    // Resize the genre list view

    list_view = ::GetDlgItem ( hWnd, IDC_LIST_GENRES );

    ::GetWindowRect ( list_view, & rcList );

    ::ScreenToClient ( hWnd, (POINT *) & rcList.left );
    ::ScreenToClient ( hWnd, (POINT *) & rcList.right );

    RECT rc;
    ::GetClientRect ( hWnd, & rc );

    int height = rc.bottom - 3 - rcList.top;

    if ( !::SetWindowPos ( list_view, NULL, 3, top, width, height, SWP_NOZORDER ) )
    {
        ::DisplaySystemError ( hWnd );
    }

    ListView_SetColumnWidth ( list_view, 2, LVSCW_AUTOSIZE_USEHEADER );

    // Resize the artist list view

    list_view = ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS );

    int left = 3 + width + LIST_VIEW_SEPARATOR;

    if ( !::SetWindowPos ( list_view, NULL, left, top, width, height, SWP_NOZORDER ) )
    {
        ::DisplaySystemError ( hWnd );
    }

    ListView_SetColumnWidth ( list_view, 2, LVSCW_AUTOSIZE_USEHEADER );

}

static void Playlist_OnRButtonDown ( HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    POINT pt;
    ::GetCursorPos ( &pt );

    HMENU menu = ::CreatePopupMenu ( );

    MENUITEMINFO menu_item;
    ::ZeroMemory ( & menu_item, sizeof( menu_item ) );    

    menu_item.cbSize        = sizeof ( menu_item );
    menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
    menu_item.wID           = ID_VIEWPLAYLIST;
    menu_item.dwTypeData    = _T("View playlist");
    menu_item.fState        = MFS_DISABLED;

    if ( !generator->_worker_thread )
    {
        //  Does a playlist exists?

        TCHAR buffer [ MAX_PATH ];
        _stprintf ( buffer, _T("%s\\playlist.m3u"), generator->_playlist_directory );

        HANDLE file = ::CreateFile (    buffer,
                                        GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL );
        if ( file != INVALID_HANDLE_VALUE )
        {
            ::CloseHandle ( file );
            menu_item.fState = MFS_ENABLED;
        }
    }

    ::InsertMenuItem ( menu, 0, TRUE, & menu_item );

    menu_item.cbSize        = sizeof ( menu_item );
    menu_item.fMask         = MIIM_FTYPE;
	menu_item.fType			= MFT_SEPARATOR;

    ::InsertMenuItem ( menu, 1, TRUE, & menu_item );

    menu_item.cbSize        = sizeof ( menu_item );
    menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
    menu_item.wID           = ID_SELECTALL;
    menu_item.dwTypeData    = _T("Select all");
    menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_TUNES, LVM_GETITEMCOUNT, 0, 0 ) != 0 && !generator->_worker_thread ) ? MFS_ENABLED : MFS_DISABLED;

    ::InsertMenuItem ( menu, 2, TRUE, & menu_item );

    ::TrackPopupMenu ( menu,
                       TPM_LEFTALIGN | TPM_TOPALIGN,
                       pt.x, pt.y,
                       0,
                       hWnd,
                       NULL );
}

static void Playlist_OnLButtonDown ( HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    RECT rcTunes, rcGenre;

    ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_LIST_TUNES ), & rcTunes );
    ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_LIST_GENRES ), & rcGenre );

    ::ScreenToClient ( hWnd, (POINT *) & rcTunes.left );
    ::ScreenToClient ( hWnd, (POINT *) & rcTunes.right );

    ::ScreenToClient ( hWnd, (POINT *) & rcGenre.left );
    ::ScreenToClient ( hWnd, (POINT *) & rcGenre.right );

    _RPT2 ( _CRT_WARN, "WM_LBUTTONDOWN\tX pos = %d\tY pos = %d\r\n", x, y );

    //  If the left button is down

    if ( x > rcTunes.left && x < rcTunes.right && y > rcTunes.bottom && y < rcGenre.top )
    {
        generator->_splitter_position = rcTunes.bottom;

        generator->_sizing_hwnd = ::CreateWindow (  _T("STATIC"),
                                                    NULL,
                                                    WS_CHILDWINDOW | WS_VISIBLE | SS_BLACKRECT,
                                                    rcTunes.left,
                                                    generator->_splitter_position,
                                                    rcTunes.right - rcTunes.left,
                                                    LIST_VIEW_SEPARATOR,
                                                    hWnd,
                                                    NULL,
                                                    NULL,
                                                    NULL );

        //  Capture the mouse while we are sizing the list views else we might loose input

        SetCapture ( hWnd );
    }
    else
    {
        RECT rc;
        ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_STATIC_LOGO ), & rc );

        ::ScreenToClient ( hWnd, (POINT *) & rc.left );
        ::ScreenToClient ( hWnd, (POINT *) & rc.right );

        if ( x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom )
        {
            //  Launch web browser to WW Software

            ::ShellExecute ( hWnd, _T("open"), _T("http://software.davewolf.net"), NULL, NULL, SW_SHOWNORMAL );
        }
        else
        {
            ::SetCursor ( ::LoadCursor ( NULL, MAKEINTRESOURCE( IDC_ARROW ) ) );
        }
    }
}

static void Playlist_OnLButtonUp ( HWND hWnd, int x, int y, UINT keyFlags )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    if ( generator->_sizing_hwnd )
    {
        RECT rcSizing, rcTunes, rcGenre, rcArtist, rcWindow;

        ::GetWindowRect ( generator->_sizing_hwnd, & rcSizing );

        ::ScreenToClient ( hWnd, (POINT *) & rcSizing.left );
        ::ScreenToClient ( hWnd, (POINT *) & rcSizing.right );

        generator->_splitter_position = rcSizing.top;

        //  Destroy the sizing window so when we update the size of the list view controls it won't
        //  interfere with updating them.

        ::DestroyWindow ( generator->_sizing_hwnd );

        generator->_sizing_hwnd = NULL;

        //  Now position the list views

        HWND list_view_tunes    = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );
        HWND list_view_genres   = ::GetDlgItem ( hWnd, IDC_LIST_GENRES );
        HWND list_view_artists  = ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS );

        ::GetWindowRect ( list_view_tunes, & rcTunes );
        ::GetWindowRect ( list_view_genres, & rcGenre );
        ::GetWindowRect ( list_view_artists, & rcArtist );

        ::ScreenToClient ( hWnd, (POINT *) & rcTunes.left );
        ::ScreenToClient ( hWnd, (POINT *) & rcTunes.right );

        ::ScreenToClient ( hWnd, (POINT *) & rcGenre.left );
        ::ScreenToClient ( hWnd, (POINT *) & rcGenre.right );

        ::ScreenToClient ( hWnd, (POINT *) & rcArtist.left );
        ::ScreenToClient ( hWnd, (POINT *) & rcArtist.right );

        ::MoveWindow ( list_view_tunes, rcTunes.left, rcTunes.top, rcTunes.right - rcTunes.left, rcSizing.top - rcTunes.top, TRUE );

        //  We don't want the genre/artist boxes to touch the status bar
        
        ::GetClientRect ( hWnd, & rcWindow );

        rcWindow.bottom -= 3;
        
        ::MoveWindow ( list_view_genres, rcGenre.left, rcSizing.bottom, rcGenre.right - rcGenre.left, rcWindow.bottom - rcSizing.bottom, TRUE );
        ::MoveWindow ( list_view_artists, rcArtist.left, rcSizing.bottom, rcArtist.right - rcArtist.left, rcWindow.bottom - rcSizing.bottom, TRUE );

        ::GetWindowRect ( list_view_tunes, & rcTunes );
        ::GetWindowRect ( list_view_genres, & rcGenre );
        ::GetWindowRect ( list_view_artists, & rcArtist );

        //  Make sure if the scroll bar is not present that the last column is set to auto size

        ::PostMessage ( list_view_genres, LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE_USEHEADER );
        ::PostMessage ( list_view_artists, LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE_USEHEADER );

        ::ReleaseCapture ( );
    }
}

static void Playlist_OnMouseMove ( HWND hWnd, int x, int y, UINT keyFlags )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    RECT rcTunes, rcGenre;

    ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_LIST_TUNES ), & rcTunes );
    ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_LIST_GENRES ), & rcGenre );

    ::ScreenToClient ( hWnd, (POINT *) & rcTunes.left );
    ::ScreenToClient ( hWnd, (POINT *) & rcTunes.right );

    ::ScreenToClient ( hWnd, (POINT *) & rcGenre.left );
    ::ScreenToClient ( hWnd, (POINT *) & rcGenre.right );

    //PT2 ( _CRT_WARN, _T("WM_MOUSEMOVE\tX pos = %d\tY pos = %d\r\n", x, y );

    if ( x > rcTunes.left && x < rcTunes.right && y > rcTunes.bottom && y < rcGenre.top || generator->_sizing_hwnd )
    {
        ::SetCursor ( ::LoadCursor ( NULL, MAKEINTRESOURCE( IDC_SIZENS ) ) );

        if ( generator->_sizing_hwnd )
        {
            RECT rcSizing;
            ::GetWindowRect ( generator->_sizing_hwnd, & rcSizing );

            ::ScreenToClient ( hWnd, (POINT *) & rcSizing.left );
            ::ScreenToClient ( hWnd, (POINT *) & rcSizing.right );

            //  Don't let the sizing window complete obscure the list view controls

            //  Height of the header is 20 pixels and the row height is approximately 20 pixels.
            //  We add in for 1/2 the sizing window height.

            int top_pos = rcTunes.top + 40 + 5;

            //  If the horizontal scroll is visible, we must add it

            HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );

            SCROLLBARINFO sbinfo;
            ::ZeroMemory ( & sbinfo, sizeof ( sbinfo ) );

            sbinfo.cbSize = sizeof ( sbinfo );

            if ( ::GetScrollBarInfo ( list_view, OBJID_HSCROLL, & sbinfo ) && !( sbinfo.rgstate [ 0 ] & STATE_SYSTEM_INVISIBLE ) )
            {
                top_pos += sbinfo.rcScrollBar.bottom - sbinfo.rcScrollBar.top;
            }

            //  The bottom position is determined by the client area

            RECT rc;
            ::GetClientRect ( hWnd, & rc );

            int bottom_pos = rc.bottom - 40 - 5;

            //  If the horizontal scroll is visible, we must subtract it

            list_view = ::GetDlgItem ( hWnd, IDC_LIST_GENRES );

            ::ZeroMemory ( & sbinfo, sizeof ( sbinfo ) );

            sbinfo.cbSize = sizeof ( sbinfo );

            if ( ::GetScrollBarInfo ( list_view, OBJID_HSCROLL, & sbinfo ) && !( sbinfo.rgstate [ 0 ] & STATE_SYSTEM_INVISIBLE ) )
            {
                bottom_pos -= sbinfo.rcScrollBar.bottom - sbinfo.rcScrollBar.top;
            }

            if ( ( y >= top_pos ) && ( y <= bottom_pos ) )
            {
                ::MoveWindow ( generator->_sizing_hwnd, rcTunes.left, y - (LIST_VIEW_SEPARATOR / 2), rcSizing.right - rcSizing.left, LIST_VIEW_SEPARATOR, TRUE );

                _RPT2 ( _CRT_WARN, "Moving sizing window\tX pos = %d\tY pos = %d\r\n", rcTunes.left, y );
            }
        }
    }
    else
    {       
        //  If mouse is over logo

        RECT rcLogo;

        ::GetWindowRect ( ::GetDlgItem ( hWnd, IDC_STATIC_LOGO ), & rcLogo );

        ::ScreenToClient ( hWnd, (POINT *) & rcLogo.left );
        ::ScreenToClient ( hWnd, (POINT *) & rcLogo.right );

        if ( x >= rcLogo.left && x <= rcLogo.right && y >= rcLogo.top && y <= rcLogo.bottom )
        {
            UpdateStatusInfo ( generator, IDC_STATIC_LOGO );
            ::SetCursor ( ::LoadCursor ( NULL, MAKEINTRESOURCE( IDC_HAND ) ) );
        }
        else
        {
            UpdateStatusInfo ( generator, -1 );
            ::SetCursor ( ::LoadCursor ( NULL, MAKEINTRESOURCE( IDC_ARROW ) ) );
        }
    }
}

static int Playlist_OnNotify ( HWND hWnd, int id, LPNMHDR lpnmhdr )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );
    int rv = 0;

    //_RPT1 ( _CRT_WARN, _T("Code %d\r\n", lpnmhdr->code );

    switch ( lpnmhdr->idFrom )
    {
        case IDC_LIST_TUNES:
            {
                switch ( lpnmhdr->code )
                {
                    case NM_KILLFOCUS:

                        {
                            generator->_process_multiple_items = false;
                        }

                        break;

                    case LVN_ITEMCHANGED:
                        {
                            NMLISTVIEW * list_view = (NMLISTVIEW *) lpnmhdr;

                            if ( generator->_loading_list_views != true && list_view->uChanged & LVIF_STATE )
                            {
                                if ( (list_view->uNewState & LVIS_STATEIMAGEMASK) ^ (list_view->uOldState & LVIS_STATEIMAGEMASK) )
                                {
                                    BOOL state = ListView_GetCheckState ( lpnmhdr->hwndFrom, list_view->iItem );

                                    AudioFile * audio_file = static_cast<AudioFile *>( (void *) list_view->lParam );
                                    audio_file->_state = (DWORD) state;

                                    if ( state )
                                    {
                                        generator->AddSelected ( audio_file );
                                    }
                                    else
                                    {
                                        generator->RemoveSelected ( audio_file );
                                    }

                                    if ( !generator->_process_multiple_items && ListView_GetSelectedCount ( lpnmhdr->hwndFrom ) > 1 )
                                    {
                                        generator->_process_multiple_items = true;
                                        ::PostMessage ( hWnd, WM_PROCESSMULTIPLE, (WPARAM)state, (LPARAM)lpnmhdr->hwndFrom );
                                    }
                                }
                            }
                        }

                        break;

                    case NM_RCLICK:
                        {
                            POINT pt;
                            ::GetCursorPos ( &pt );

                            HMENU menu = ::CreatePopupMenu ( );

                            MENUITEMINFO menu_item;
                            ::ZeroMemory ( & menu_item, sizeof( menu_item ) );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_PLAYSELECTED;
                            menu_item.dwTypeData    = _T("Play Selected Songs");
                            menu_item.fState        = ( ListView_GetSelectedCount ( lpnmhdr->hwndFrom ) && !generator->_worker_thread ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 0, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_SELECTED;
                            menu_item.dwTypeData    = _T("View Manually Selected Songs");
                            menu_item.fState        = ( !generator->_selected_list.empty() && !generator->_worker_thread ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 1, TRUE, & menu_item );
                            
                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_AUDIOINFO;
                            menu_item.dwTypeData    = _T("Audio File Information");
                            menu_item.fState        = ( ListView_GetSelectedCount ( lpnmhdr->hwndFrom ) == 1 && !generator->_worker_thread ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 2, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_FTYPE;
					        menu_item.fType			= MFT_SEPARATOR;

                            ::InsertMenuItem ( menu, 3, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_CHECKSELECTED;
                            menu_item.dwTypeData    = _T("Check selected title(s)");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_TUNES, LVM_GETSELECTEDCOUNT, 0, 0 ) != 0 && !generator->_worker_thread  ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 4, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_UNCHECKSELECTED;
                            menu_item.dwTypeData    = _T("Uncheck selected title(s)");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_TUNES, LVM_GETSELECTEDCOUNT, 0, 0 ) != 0 && !generator->_worker_thread  ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 5, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_FTYPE;
					        menu_item.fType			= MFT_SEPARATOR;

                            ::InsertMenuItem ( menu, 6, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_SELECTALL;
                            menu_item.dwTypeData    = _T("Select all");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_TUNES, LVM_GETITEMCOUNT, 0, 0 ) != 0 && !generator->_worker_thread  ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 7, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_DELETE;
                            menu_item.dwTypeData    = _T("Delete selected title(s) from Image");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_TUNES, LVM_GETSELECTEDCOUNT, 0, 0 ) != 0 && !generator->_worker_thread  ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 8, TRUE, & menu_item );

                            ::TrackPopupMenu ( menu,
                                               TPM_LEFTALIGN | TPM_TOPALIGN,
                                               pt.x, pt.y,
                                               0,
                                               hWnd,
                                               NULL );
                        }

                        break;

                    case LVN_COLUMNCLICK:
                        {

                            NMLISTVIEW * list_view = (NMLISTVIEW *) lpnmhdr;

                            if ( ListView_GetItemCount ( lpnmhdr->hwndFrom) != 0 )
                            {
                                if ( list_view->iSubItem == generator->GetSortColumn ( IDC_LIST_TUNES ) )
                                {
                                    SortMode mode = generator->ToggleSortMode ( IDC_LIST_TUNES );

                                    ListView_SortItems ( lpnmhdr->hwndFrom, CompareItems, (LPARAM) generator );

                                    //  Toggle indicator in same column

                                    generator->UpdateColumnHeader ( hWnd, IDC_LIST_TUNES, list_view->iSubItem, mode );
                                }
                                else
                                {
                                    int old_column = generator->SetSortColumn ( list_view->iSubItem, IDC_LIST_TUNES );
                                    generator->SetSortMode ( SORT_ASCENDING, IDC_LIST_TUNES );
                        
                                    ListView_SortItems ( lpnmhdr->hwndFrom, CompareItems, (LPARAM) generator );

                                    //  Remove sort indicator from old column and set ASCENDING order in new column

                                    if ( old_column != -1 )
                                    {
                                        generator->UpdateColumnHeader ( hWnd, IDC_LIST_TUNES, old_column, SORT_NONE );
                                    }
                                    generator->UpdateColumnHeader ( hWnd, IDC_LIST_TUNES, list_view->iSubItem, SORT_ASCENDING );
                                }
                            }
                        }

                        break;

                    case LVN_KEYDOWN:

                        {
                            NMLVKEYDOWN * key_down = (NMLVKEYDOWN *) lpnmhdr;

                            if ( key_down->wVKey == VK_DELETE )
                            {
                                ::PostMessage ( hWnd, WM_COMMAND, MAKEWPARAM(ID_DELETE, BN_CLICKED), 0 );
                            }
                        }

                        break;

                    case NM_CUSTOMDRAW:

                        {
                            NMLVCUSTOMDRAW * custom = (NMLVCUSTOMDRAW *) lpnmhdr;

                            switch ( custom->nmcd.dwDrawStage )
                            {
                                case CDDS_PREPAINT:

                                    PaintListViewBackground ( hWnd, IDC_LIST_TUNES, custom->nmcd.hdc );

                                    SetWindowLong ( hWnd, DWL_MSGRESULT, (long) CDRF_NOTIFYITEMDRAW );
                                    rv = CDRF_NOTIFYITEMDRAW;                                   

                                    break;

                                default:

                                    if ( custom->nmcd.dwDrawStage | CDDS_ITEM )
                                    {
                                       	custom->clrText   = RGB(0,0,0); 
                                        custom->clrTextBk = (custom->nmcd.dwItemSpec % 2) ? RGB(224,224,224) : RGB(255,255,255);
										rv = CDRF_NEWFONT;

                                        SetWindowLong ( hWnd, DWL_MSGRESULT, (long) rv );
                                    }
                                    else
                                    {
                                        rv = CDRF_DODEFAULT;
                                    }

                                    break;
                            }
                        }

                        break;

                    case NM_DBLCLK:

                        {
                            NMITEMACTIVATE * list_view = (NMITEMACTIVATE *) lpnmhdr;

                            if ( ListView_GetSelectedCount ( list_view->hdr.hwndFrom ) == 1 )
                            {
                                //int index = ListView_GetNextItem ( list_view->hdr.hwndFrom, -1, LVNI_SELECTED );
                                ::PostMessage ( hWnd, WM_COMMAND, MAKEWPARAM ( ID_AUDIOINFO, 0 ), NULL );
                            }
                        }

                        break;

                }
            }
            
            break;

        case IDC_LIST_GENRES:

            {
                switch ( lpnmhdr->code )
                {
                    case LVN_ITEMCHANGED:
                        {
                            NMLISTVIEW * list_view = (NMLISTVIEW *) lpnmhdr;

                            if ( generator->_loading_list_views != true && list_view->uChanged & LVIF_STATE )
                            {
                                if ( (list_view->uNewState & LVIS_STATEIMAGEMASK) ^ (list_view->uOldState & LVIS_STATEIMAGEMASK) )
                                {
                                    LVITEM  item;
                                    TCHAR    buffer [ MAX_PATH ];

                                    ::ZeroMemory ( & item, sizeof( item ) );

                                    //  Get the genre name

                                    item.mask       = LVIF_TEXT | LVIF_PARAM;
                                    item.iItem      = list_view->iItem;
                                    item.iSubItem   = 1;
                                    item.pszText    = buffer;
                                    item.cchTextMax = MAX_PATH;

                                    ListView_GetItem ( list_view->hdr.hwndFrom, & item );

                                    AudioIndex * genre = static_cast<AudioIndex *>( (void *) list_view->lParam );
                                    BOOL state = ListView_GetCheckState ( lpnmhdr->hwndFrom, list_view->iItem );
                                    
                                    genre->_state = (DWORD) state;

                                    if ( state )
                                    {
                                        generator->AddGenreToListView ( hWnd, buffer );
                                    }
                                    else
                                    {
                                        generator->RemoveGenreFromListView ( hWnd, buffer );
                                    }

                                    if ( !generator->_process_multiple_items && ListView_GetSelectedCount ( lpnmhdr->hwndFrom ) > 1 )
                                    {
                                        generator->_process_multiple_items = true;
                                        ::PostMessage ( hWnd, WM_PROCESSMULTIPLE, (WPARAM)state, (LPARAM)lpnmhdr->hwndFrom );
                                    }
                                    
                                    ::PostMessage ( hWnd, WM_SORTPLAYLIST, 0, 0 );
                                }
                            }
                        }

                        break;

                    case NM_RCLICK:
                        {
                            POINT pt;
                            ::GetCursorPos ( &pt );

                            HMENU menu = ::CreatePopupMenu ( );

                            MENUITEMINFO menu_item;
                            ::ZeroMemory ( & menu_item, sizeof( menu_item ) );

                            int index = 0;

                            if ( generator->_aggregate == BST_CHECKED )
                            {
                                menu_item.cbSize        = sizeof ( menu_item );
                                menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                                menu_item.wID           = ID_GENRE;
                                menu_item.dwTypeData    = _T("View Genre in Aggregate");
                                menu_item.fState        = ( !generator->_worker_thread  ) ? MFS_ENABLED : MFS_DISABLED;
                            
                                ::InsertMenuItem ( menu, index ++, TRUE, & menu_item );

                                menu_item.cbSize        = sizeof ( menu_item );
                                menu_item.fMask         = MIIM_FTYPE;
					            menu_item.fType			= MFT_SEPARATOR;

                                ::InsertMenuItem ( menu, index ++, TRUE, & menu_item );
                            }

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_CHECKSELECTEDGENRE;
                            menu_item.dwTypeData    = _T("Check selected genre(s)");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_GENRES, LVM_GETSELECTEDCOUNT, 0, 0 ) != 0 && !generator->_worker_thread  ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, index ++, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_UNCHECKSELECTEDGENRE;
                            menu_item.dwTypeData    = _T("Uncheck selected genre(s)");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_GENRES, LVM_GETSELECTEDCOUNT, 0, 0 ) != 0 && !generator->_worker_thread  ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, index ++, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_FTYPE;
					        menu_item.fType			= MFT_SEPARATOR;

                            ::InsertMenuItem ( menu, index ++, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_SELECTALLGENRE;
                            menu_item.dwTypeData    = _T("Select all");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_GENRES, LVM_GETITEMCOUNT, 0, 0 ) != 0 && !generator->_worker_thread ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, index ++, TRUE, & menu_item );

                            ::TrackPopupMenu ( menu,
                                               TPM_LEFTALIGN | TPM_TOPALIGN,
                                               pt.x, pt.y,
                                               0,
                                               hWnd,
                                               NULL );
                        }

                        break;

                    case LVN_COLUMNCLICK:
                        {

                            NMLISTVIEW * list_view = (NMLISTVIEW *) lpnmhdr;

                            if ( ListView_GetItemCount ( lpnmhdr->hwndFrom) != 0 )
                            {
                                if ( list_view->iSubItem == generator->GetSortColumn ( IDC_LIST_GENRES) )
                                {
                                    SortMode mode = generator->ToggleSortMode ( IDC_LIST_GENRES );

                                    ListView_SortItems ( lpnmhdr->hwndFrom, CompareGenreItems, (LPARAM) generator );

                                    //  Toggle indicator in same column

                                    generator->UpdateColumnHeader ( hWnd, IDC_LIST_GENRES, list_view->iSubItem, mode );
                                }
                                else
                                {
                                    int old_column = generator->SetSortColumn ( list_view->iSubItem, IDC_LIST_GENRES );
                                    generator->SetSortMode ( SORT_ASCENDING, IDC_LIST_GENRES );
                        
                                    ListView_SortItems ( lpnmhdr->hwndFrom, CompareGenreItems, (LPARAM) generator );

                                    //  Remove sort indicator from old column and set ASCENDING order in new column

                                    if ( old_column != -1 )
                                    {
                                        generator->UpdateColumnHeader ( hWnd, IDC_LIST_GENRES, old_column, SORT_NONE );
                                    }
                                    generator->UpdateColumnHeader ( hWnd, IDC_LIST_GENRES, list_view->iSubItem, SORT_ASCENDING );
                                }
                            }
                        }

                        break;

                    case NM_CUSTOMDRAW:

                        {
                            NMLVCUSTOMDRAW * custom = (NMLVCUSTOMDRAW *) lpnmhdr;

                            switch ( custom->nmcd.dwDrawStage )
                            {

                                case CDDS_PREPAINT:

                                    PaintListViewBackground ( hWnd, IDC_LIST_GENRES, custom->nmcd.hdc );

                                    SetWindowLong ( hWnd, DWL_MSGRESULT, (long) CDRF_NOTIFYITEMDRAW );
                                    rv = CDRF_NOTIFYITEMDRAW;                                   

                                    break;

                                default:

                                    if ( custom->nmcd.dwDrawStage | CDDS_ITEM )
                                    {
                                       	custom->clrText   = RGB(0,0,0); 
                                        custom->clrTextBk = (custom->nmcd.dwItemSpec % 2) ? RGB(224,224,224) : RGB(255,255,255);
										rv = CDRF_NEWFONT;

                                        SetWindowLong ( hWnd, DWL_MSGRESULT, (long) rv );
                                    }
                                    else
                                    {
                                        rv = CDRF_DODEFAULT;
                                    }

                                    break;
                            }
                        }

                        break;
                }
            }

            break;

        case IDC_LIST_ARTISTS:

            {
                switch ( lpnmhdr->code )
                {
                    case LVN_ITEMCHANGED:
                        {
                            NMLISTVIEW * list_view = (NMLISTVIEW *) lpnmhdr;

                            if ( generator->_loading_list_views != true && list_view->uChanged & LVIF_STATE )
                            {
                                if ( (list_view->uNewState & LVIS_STATEIMAGEMASK) ^ (list_view->uOldState & LVIS_STATEIMAGEMASK) )
                                {
                                    LVITEM  item;
                                    TCHAR    buffer [ MAX_PATH ];

                                    ::ZeroMemory ( & item, sizeof( item ) );                                   

                                    //  Get the artist name

                                    item.mask       = LVIF_TEXT | LVIF_PARAM;
                                    item.iItem      = list_view->iItem;
                                    item.iSubItem   = 1;
                                    item.pszText    = buffer;
                                    item.cchTextMax = MAX_PATH;

                                    ListView_GetItem ( list_view->hdr.hwndFrom, & item );

                                    AudioIndex * artist = static_cast<AudioIndex *>( (void *) list_view->lParam );
                                    BOOL state = ListView_GetCheckState ( lpnmhdr->hwndFrom, list_view->iItem );

                                    artist->_state = (DWORD) state;

                                    if ( state )
                                    {
                                        generator->AddArtistToListView ( hWnd, buffer );
                                    }
                                    else
                                    {
                                        generator->RemoveArtistFromListView ( hWnd, buffer );
                                    }

                                    if ( !generator->_process_multiple_items && ListView_GetSelectedCount ( lpnmhdr->hwndFrom ) > 1 )
                                    {
                                        generator->_process_multiple_items = true;
                                        ::PostMessage ( hWnd, WM_PROCESSMULTIPLE, (WPARAM)state, (LPARAM)lpnmhdr->hwndFrom );
                                    }
                                    
                                    ::PostMessage ( hWnd, WM_SORTPLAYLIST, 0, 0 );
                                }
                            }
                        }

                        break;

                    case NM_RCLICK:
                        {
                            POINT pt;
                            ::GetCursorPos ( &pt );

                            HMENU menu = ::CreatePopupMenu ( );

                            MENUITEMINFO menu_item;
                            ::ZeroMemory ( & menu_item, sizeof( menu_item ) );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_CHECKSELECTEDARTISTS;
                            menu_item.dwTypeData    = _T("Check selected artists(s)");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_ARTISTS, LVM_GETSELECTEDCOUNT, 0, 0 ) != 0 && !generator->_worker_thread  ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 0, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_UNCHECKSELECTEDARTISTS;
                            menu_item.dwTypeData    = _T("Uncheck selected artists(s)");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_ARTISTS, LVM_GETSELECTEDCOUNT, 0, 0 ) != 0 && !generator->_worker_thread ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 1, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_FTYPE;
					        menu_item.fType			= MFT_SEPARATOR;

                            ::InsertMenuItem ( menu, 2, TRUE, & menu_item );

                            menu_item.cbSize        = sizeof ( menu_item );
                            menu_item.fMask         = MIIM_ID | MIIM_STRING | MIIM_STATE;
                            menu_item.wID           = ID_SELECTALLARTISTS;
                            menu_item.dwTypeData    = _T("Select all");
                            menu_item.fState        = ( ::SendDlgItemMessage ( hWnd, IDC_LIST_ARTISTS, LVM_GETITEMCOUNT, 0, 0 ) != 0 && !generator->_worker_thread ) ? MFS_ENABLED : MFS_DISABLED;

                            ::InsertMenuItem ( menu, 3, TRUE, & menu_item );

                            ::TrackPopupMenu ( menu,
                                               TPM_LEFTALIGN | TPM_TOPALIGN,
                                               pt.x, pt.y,
                                               0,
                                               hWnd,
                                               NULL );
                        }

                        break;

                    case LVN_COLUMNCLICK:
                        {

                            NMLISTVIEW * list_view = (NMLISTVIEW *) lpnmhdr;

                            if ( ListView_GetItemCount ( lpnmhdr->hwndFrom) != 0 )
                            {
                                if ( list_view->iSubItem == generator->GetSortColumn ( IDC_LIST_ARTISTS) )
                                {
                                    SortMode mode = generator->ToggleSortMode ( IDC_LIST_ARTISTS );

                                    ListView_SortItems ( lpnmhdr->hwndFrom, CompareArtistItems, (LPARAM) generator );

                                    //  Toggle indicator in same column

                                    generator->UpdateColumnHeader ( hWnd, IDC_LIST_ARTISTS, list_view->iSubItem, mode );
                                }
                                else
                                {
                                    int old_column = generator->SetSortColumn ( list_view->iSubItem, IDC_LIST_ARTISTS );
                                    generator->SetSortMode ( SORT_ASCENDING, IDC_LIST_ARTISTS );
                        
                                    ListView_SortItems ( lpnmhdr->hwndFrom, CompareArtistItems, (LPARAM) generator );

                                    //  Remove sort indicator from old column and set ASCENDING order in new column

                                    if ( old_column != -1 )
                                    {
                                        generator->UpdateColumnHeader ( hWnd, IDC_LIST_ARTISTS, old_column, SORT_NONE );
                                    }
                                    generator->UpdateColumnHeader ( hWnd, IDC_LIST_ARTISTS, list_view->iSubItem, SORT_ASCENDING );
                                }
                            }
                        }

                        break;

                    case NM_CUSTOMDRAW:

                        {
                            NMLVCUSTOMDRAW * custom = (NMLVCUSTOMDRAW *) lpnmhdr;

                            switch ( custom->nmcd.dwDrawStage )
                            {

                                case CDDS_PREPAINT:

                                    PaintListViewBackground ( hWnd, IDC_LIST_ARTISTS, custom->nmcd.hdc );

                                    SetWindowLong ( hWnd, DWL_MSGRESULT, (long) CDRF_NOTIFYITEMDRAW );
                                    rv = CDRF_NOTIFYITEMDRAW;                                   

                                    break;

                                default:

                                    if ( custom->nmcd.dwDrawStage | CDDS_ITEM )
                                    {
                                       	custom->clrText   = RGB(0,0,0); 
                                        custom->clrTextBk = (custom->nmcd.dwItemSpec % 2) ? RGB(224,224,224) : RGB(255,255,255);
										rv = CDRF_NEWFONT;

                                        SetWindowLong ( hWnd, DWL_MSGRESULT, (long) rv );
                                    }
                                    else
                                    {
                                        rv = CDRF_DODEFAULT;
                                    }

                                    break;
                            }
                        }

                        break;
                }
            }

            break;

        case IDC_BUTTON_BROWSE:
        case IDC_BUTTON_BROWSE_2:
        case IDC_BUTTON_LOAD:
        case IDC_BUTTON_CANCEL:
        case IDC_BUTTON_GENERATE:
        case IDC_RADIO_PLAYLIST_BY_NUMBER:
        case IDC_RADIO_PLAYLIST_BY_SIZE:
        case IDC_RADIO_PLAYLIST_BY_DURATION:
        case IDC_CHECK_COPY_FILE:
        case IDC_CHECK_RELATIVE_PATHS:

            {
                switch ( lpnmhdr->code  )
                {
                    case BCN_HOTITEMCHANGE:

                        {
                            NMBCHOTITEM * hotitem = (NMBCHOTITEM *) lpnmhdr;

                            if ( hotitem->dwFlags & HICF_ENTERING )
                            {
                                ::UpdateStatusInfo ( generator, id );
                            }
                            else
                            {
                                ::UpdateStatusInfo ( generator, -1 );
                            }
                        }

                        break;
                }
            }

            break;

        default:

            {
                //  Handle notification messages from list view header control

                if ( ListView_GetHeader ( ::GetDlgItem ( hWnd, IDC_LIST_TUNES ) ) == lpnmhdr->hwndFrom )
                {
                    NMHEADER * nmheader = (NMHEADER *) lpnmhdr;                    

                    switch ( nmheader->hdr.code )
                    {
                        case HDN_BEGINTRACK:

                            _RPT0 ( _CRT_WARN, "IDC_LIST_TUNES header begin track\r\n" );

                            //  Disable default sizing on first and last columns

                            if ( nmheader->iItem == 0 || nmheader->iItem == 7 )
                            {
                                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) TRUE );
                                rv = TRUE;
                            }
                            else
                            {
                                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) FALSE );
                                rv = FALSE;
                            }

                            break;

                        case HDN_ITEMCHANGING:

                            //_RPT0 ( _CRT_WARN, _T("IDC_LIST_TUNES header item changing\r\n" );

                            {
                                if ( nmheader->iItem != 7 )
                                {
                                    ::PostMessage ( ::GetDlgItem ( hWnd, IDC_LIST_TUNES ), LVM_SETCOLUMNWIDTH, 7, LVSCW_AUTOSIZE_USEHEADER );
                                }

                                rv = FALSE;
                                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) FALSE );
                            }

                            break;

                    }
                }
                else
                if ( ListView_GetHeader ( ::GetDlgItem ( hWnd, IDC_LIST_GENRES ) ) == lpnmhdr->hwndFrom )
                {
                    NMHEADER * nmheader = (NMHEADER *) lpnmhdr;

                    switch ( nmheader->hdr.code )
                    {
                        case HDN_BEGINTRACK:

                            //  Disable default sizing on first and last columns

                            if ( nmheader->iItem == 0 || nmheader->iItem == 2 )
                            {
                                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) TRUE );
                                rv = TRUE;
                            }
                            else
                            {
                                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) FALSE );
                                rv = FALSE;
                            }

                            break;

                        case HDN_ITEMCHANGING:

                            {
                                if ( nmheader->iItem != 2 )
                                {
                                    ::PostMessage ( ::GetDlgItem ( hWnd, IDC_LIST_GENRES ), LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE_USEHEADER );
                                }

                                rv = FALSE;
                                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) FALSE );

                            }

                            break;

                    }
                }
                else
                if ( ListView_GetHeader ( ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS ) ) == lpnmhdr->hwndFrom )
                {
                    NMHEADER * nmheader = (NMHEADER *) lpnmhdr;

                    switch ( nmheader->hdr.code )
                    {
                        case HDN_BEGINTRACK:

                            //  Disable default sizing on first and last columns

                            if ( nmheader->iItem == 0 || nmheader->iItem == 2 )
                            {
                                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) TRUE );
                                rv = TRUE;
                            }
                            else
                            {
                                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) FALSE );
                                rv = FALSE;
                            }

                            break;

                        case HDN_ITEMCHANGING:

                            {
                                if ( nmheader->iItem != 2 )
                                {
                                    ::PostMessage ( ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS ), LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE_USEHEADER );
                                }

                                rv = FALSE;
                                ::SetWindowLong ( hWnd, DWL_MSGRESULT, (long) FALSE );

                            }

                            break;

                    }
                }
            }

            break;
    }

    return rv;
}

static int Playlist_OnTimer ( HWND hWnd, int id )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    if ( id == 1 )
    {
        TCHAR buffer [ MAX_PATH ];

        unsigned elapsed, current = ::GetTickCount ();
        elapsed = ( current >= generator->_elapsed_start ) ? (current - generator->_elapsed_start) : ((0xFFFFFFFF -  generator->_elapsed_start) + current);

        int hours   = elapsed / 3600000;
        elapsed -= hours * 3600000;
        int minutes = elapsed / 60000;
        elapsed -= minutes * 60000;
        int seconds = elapsed / 1000;

        _stprintf ( buffer, _T("Elapsed time: %02d:%02d:%02d"), hours, minutes, seconds );
        generator->_status_bar.SetPaneText ( 1, buffer );
    }

    return 0;
}

static void Playlist_OnSortPlaylist ( HWND hWnd )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    if ( generator->GetSortColumn ( IDC_LIST_TUNES ) != -1 && generator->GetSortMode ( IDC_LIST_TUNES ) != SORT_NONE )
    {
        generator->UpdateColumnHeader ( hWnd, IDC_LIST_TUNES, generator->GetSortColumn ( IDC_LIST_TUNES ), generator->GetSortMode ( IDC_LIST_TUNES ) );
        ListView_SortItems ( ::GetDlgItem ( hWnd, IDC_LIST_TUNES ), CompareItems, (LPARAM) generator );
    }
}

static void Playlist_OnLastUpdated ( HWND hWnd )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    TCHAR buffer [ MAX_PATH ];

    _stprintf ( buffer, _T("It has been more than %d days since your music image was updated.\n\nWould you like to update it now?"), generator->_days );

    if ( IDYES == ::MessageBox ( hWnd, buffer, MSGBOX_TITLE, MB_ICONQUESTION | MB_YESNO ) )
    {
        ::PostMessage ( ::GetDlgItem ( hWnd, IDC_BUTTON_LOAD ), BM_CLICK, 0, 0 );        
    }
}

static void Playlist_OnUpdateFileSize ( HWND hWnd, int new_size_mode )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    if ( generator->_playlist_mode != IDC_RADIO_PLAYLIST_BY_SIZE )
        return;

    GetMaxSize ( hWnd, generator );

    generator->_size_mode = new_size_mode;

    SetMaxSize ( hWnd, generator );
}

static void Playlist_OnNoTunes ( HWND hWnd )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    TCHAR message [ 1024 ];
    ::LoadString ( generator->_instance, IDS_IMAGEEMPTY, message, 1024 );

    if ( IDYES == ::MessageBox ( generator->_form_hwnd, message, MSGBOX_TITLE, MB_ICONQUESTION | MB_YESNO ) )
    {
        generator->_auto_update_image = true;
        ::PostMessage ( ::GetDlgItem ( hWnd, IDC_BUTTON_BROWSE ), BM_CLICK, 0, 0 );
    }
}

static void Playlist_OnUpdateTuneCount ( HWND hWnd )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    TCHAR buffer [ MAX_PATH ];

    _stprintf ( buffer, _T("Total Tunes Found: %d"), generator->_audio_list.size ()  );
    generator->_status_bar.SetPaneText ( 1, buffer );
    
    _stprintf ( buffer, _T("Total Genres Found: %d"), generator->_genre_list.size ()  );
    generator->_status_bar.SetPaneText ( 2, buffer );
    
    _stprintf ( buffer, _T("Total Artists Found: %d"), generator->_artist_list.size ()  );
    generator->_status_bar.SetPaneText ( 3, buffer );
}

static void Playlist_OnProcessMultiple ( HWND hWnd, int state, HWND hwndList )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    int index = -1;

    while ( (index = ListView_GetNextItem ( hwndList, index, LVNI_ALL | LVNI_SELECTED ) ) != -1 )
    {
        ListView_SetCheckState ( hwndList, index, state );
    }

    generator->_process_multiple_items = 0;
}

static void Playlist_OnLatestVersion ( HWND hWnd, BOOL bNotify )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    if ( bNotify )
    {
        if ( IDNO == ::DialogBoxParam ( generator->_instance, MAKEINTRESOURCE(IDD_NOTIFYUPDATE), hWnd, Notify_DlgProc, (LPARAM) generator ) )
        {
            return;
        }
    }

    generator->_status_bar.SetPaneText ( 0, _T("Checking for latest version") );

    //  Have to spin up WinSock

    WSADATA data;
    char buffer [ 2048 ];
    TCHAR message [ 1024 ];
    DWORD length, response_length;

    DWORD error = ::WSAStartup ( MAKEWORD(2,0), & data );
    if ( error )
    {
        _RPT1 ( _CRT_WARN, "WSAStartup error code %lu", ::GetLastError() );

        _stprintf ( message, _T("WSAStartup error code %lu"), ::GetLastError() );
        generator->_status_bar.SetPaneText ( 0, message );

        ::LoadString ( generator->_instance, IDS_WINSOCKFAILURE, message, 1024 );
        ::MessageBox ( hWnd, message, MSGBOX_TITLE, MB_ICONERROR );

        return;
    }

    SOCKET s = ::socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( s == SOCKET_ERROR )
    {
        _RPT1 ( _CRT_WARN, "socket error code %lu", ::WSAGetLastError() );

        _stprintf ( message, _T("Failure creating socket %lu"), ::WSAGetLastError() );
        generator->_status_bar.SetPaneText ( 0, message );

        ::LoadString ( generator->_instance, IDS_SOCKETFAILURE, message, 1024 );
        ::MessageBox ( hWnd, message, MSGBOX_TITLE, MB_ICONERROR );

        goto bail;
    }

    //  Don't care about IP/port for binding (what about multi-homed?)

    SOCKADDR_IN sa_out;
    memset ( & sa_out, 0, sizeof(sa_out) );

    sa_out.sin_family               = AF_INET;
    sa_out.sin_addr.S_un.S_addr     = INADDR_ANY;
    sa_out.sin_port                 = INADDR_ANY;

    error = ::bind ( s, (SOCKADDR* ) & sa_out, sizeof(sa_out) );
    if ( error == SOCKET_ERROR )
    {
        _RPT1 ( _CRT_WARN, "bind error code %lu", ::WSAGetLastError() );

        _stprintf ( message, _T("Failure binding socket %lu"), ::WSAGetLastError() );
        generator->_status_bar.SetPaneText ( 0, message );

        ::LoadString ( generator->_instance, IDS_SOCKETFAILURE, message, 1024 );
        ::MessageBox ( hWnd, message, MSGBOX_TITLE, MB_ICONERROR );

        goto bail;
    }

    //  Resolve www.davewolf.net

    addrinfo hints;
    addrinfo * list;

    memset ( & hints, 0, sizeof(hints) );
    list = NULL;
    
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    error = ::getaddrinfo ( WWW_DAVEWOLF_NET, "80", & hints, & list );

    if ( error != 0 || list == NULL )
    {
        _RPT1 ( _CRT_WARN, "getaddrinfo error code %lu", error );

        _stprintf ( message, _T("Failure calling getaddrinfo %lu"), ::WSAGetLastError() );
        generator->_status_bar.SetPaneText ( 0, message );

        ::LoadString ( generator->_instance, IDS_SOCKETFAILURE, message, 1024 );
        ::MessageBox ( hWnd, message, MSGBOX_TITLE, MB_ICONERROR );

        goto bail;
    }

    //  Connect to www.davewolf.net

    SOCKADDR_IN sa_in;
    memset ( & sa_in, 0,  sizeof(sa_in) );

    sa_in.sin_family                = AF_INET;
    sa_in.sin_addr.S_un.S_addr      = ((SOCKADDR_IN *) list->ai_addr)->sin_addr.S_un.S_addr;
    sa_in.sin_port                  = htons(80);

    if ( list )
        freeaddrinfo ( list );

    error = ::connect ( s, (SOCKADDR *) & sa_in, sizeof(sa_in) );
    if ( error == SOCKET_ERROR )
    {
        _RPT1 ( _CRT_WARN, "connect error code %lu", ::WSAGetLastError() );

        _stprintf ( message, _T("Failure connecting socket %lu"), ::WSAGetLastError() );
        generator->_status_bar.SetPaneText ( 0, message );

        ::LoadString ( generator->_instance, IDS_SOCKETFAILURE, message, 1024 );
        ::MessageBox ( hWnd, message, MSGBOX_TITLE, MB_ICONERROR );

        goto bail;
    }

    //  Request the version of the RPCwG

    strcpy ( buffer, "GET /software/download/rpcg.version.txt HTTP/1.1\r\n" );
    strcat ( buffer, "Host: www.davewolf.net\r\n" );
    strcat ( buffer, "\r\n" );

    //  Dont' need looping code (yet)

    length = strlen ( buffer );

    length = ::send ( s, buffer, length, 0 );
    if ( length == SOCKET_ERROR )
    {
        _RPT1 ( _CRT_WARN, "send error code %lu", ::WSAGetLastError() );

        _stprintf ( message, _T("Failure sending GET command %lu"), ::WSAGetLastError() );
        generator->_status_bar.SetPaneText ( 0, message );

        ::LoadString ( generator->_instance, IDS_SOCKETFAILURE, message, 1024 );
        ::MessageBox ( hWnd, message, MSGBOX_TITLE, MB_ICONERROR );

        goto bail;
    }

    //  Should all be able to fit in 1 bufffer (hopefully)

    response_length = ::recv ( s, buffer, 2047, 0 );
    if ( response_length == SOCKET_ERROR )
    {
        _RPT1 ( _CRT_WARN, "recv error code %lu", ::WSAGetLastError() );

        _stprintf ( message, _T("Failure receiving response from www.davewolf.net %lu"), ::WSAGetLastError() );
        generator->_status_bar.SetPaneText ( 0, message );

        ::LoadString ( generator->_instance, IDS_SOCKETFAILURE, message, 1024 );
        ::MessageBox ( hWnd, message, MSGBOX_TITLE, MB_ICONERROR );

        goto bail;
    }

     buffer [ response_length ] = '\0';

    //  Find our payload

    if ( response_length != 0 )
    {
        //  Is the status success?
        
        int major, minor, status_code;
        TCHAR message [ 1024 ];

        int fields = sscanf ( buffer, "HTTP/%d.%d %d %s\r\n", & major, & minor, & status_code, message );
        if ( fields != 4 )
        {
            _RPT1 ( _CRT_WARN, "Failed to convert fields in response header (%d)", fields  );

            _stprintf ( message, _T("Failed to convert fields in response header (%d)"), fields  );
            generator->_status_bar.SetPaneText ( 0, message );

            goto bail;
        }

        if ( status_code == 200 )
        {
            char * header = strstr ( buffer, "Content-Length:" );
            if ( !header )
            {
                _RPT0 ( _CRT_WARN, "Content-Length not found" );

                generator->_status_bar.SetPaneText ( 0, _T("Content-Length not found") );

                goto bail;
            }

            fields = sscanf ( header, "Content-Length: %d\r\n", & length );
            if ( fields != 1 )
            {
                _RPT0 ( _CRT_WARN, "Failed to convert field in Content-Length" );

                generator->_status_bar.SetPaneText ( 0, _T("Failed to convert field in Content-Length") );

                goto bail;
            }

            DWORD major, minor;
            WORD t [ 4 ];

            fields = sscanf ( & buffer [ response_length - length ], "%hu.%hu.%hu.%hu", & t [ 1 ], & t [ 0 ], & t [ 3 ], & t [ 2 ] );
            if ( fields != 4 )
            {
                _RPT1 ( _CRT_WARN, "Failed to convert fields in version string (%d)", fields );

                _stprintf ( message, _T("Failed to convert fields in version string (%d)"), fields  );
                generator->_status_bar.SetPaneText ( 0, message );

                goto bail;
            }

            _RPT4 ( _CRT_WARN, "Version returned from host\t%d.%d.%d.%d\n", t [ 1 ], t [ 0 ], t [ 3 ], t [ 2 ] );

            if ( generator->GetVersion ( & major, & minor ) == true )
            {
                if ( * (DWORD *) & t [ 0 ] > major || * (DWORD *) & t [ 2 ] > minor )
                {
                    _stprintf (  message,
                                _T("An updated version (%d.%d.%d.%d) of \"Random Playlist Creator with Genres\" is now available.\n\n") \
                                _T("Would you like to download?") ,
                                t [ 1 ], t [ 0 ], t [ 3 ], t [ 2 ] );
                    
                    if ( ::MessageBox ( hWnd, message, MSGBOX_TITLE, MB_ICONINFORMATION | MB_YESNO ) == IDYES )
                    {
                        ::ShellExecute ( ::GetDesktopWindow(),
                                         _T("open"),
                                         _T("http://www.davewolf.net/software/products.php"),
                                         NULL,
                                         NULL,
                                         SW_SHOWNORMAL );
                    }
                }
                else
                {
                    _stprintf (  message,
                                _T("You are using the lastest version (%d.%d.%d.%d) of the application."),
                                t [ 1 ], t [ 0 ], t [ 3 ], t [ 2 ] );

                    ::MessageBox ( hWnd, message, MSGBOX_TITLE, MB_ICONINFORMATION | MB_OK );
                }
            }

            //  Update the registry with the current time

            ::GetSystemTimeAsFileTime ( & generator->_last_updated_version );
            generator->SaveLastTime ( _T("LastUpdatedVersion"), generator->_last_updated_version );

        }
    }
    else
    {
        _RPT0 ( _CRT_WARN, "No response to HTTP GET command" );

        generator->_status_bar.SetPaneText ( 0, _T("No response to HTTP GET command") );
    }

bail:

    if ( s )
    {
        ::shutdown ( s, SD_SEND );
        while ( 1 )
        {
            error = ::recv ( s, buffer, 2048, 0 );
            if ( error == SOCKET_ERROR || error == 0 )
            {
                break;
            }
        }

        ::closesocket ( s );
    }

    ::WSACleanup ( );

    generator->_status_bar.SetPaneText ( 0, _T("Done") );
}

static void Playlist_OnLoadLibrary ( HWND hWnd )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    generator->ReadLibrary();

 	//	Load the list view controls ( true means to restore the column widths)

	LoadListViews ( hWnd, generator );

    //  Test to see if we are able to generate a playlist

    int enable = ::CanGenerate ( hWnd, generator );
    ::EnableWindow ( ::GetDlgItem( hWnd, IDC_BUTTON_GENERATE ), enable );

    generator->_logo = ::LoadImage ( generator->_instance,
                                     MAKEINTRESOURCE ( IDB_PLAYLIST ),
                                     IMAGE_BITMAP,
                                     0, 0,
                                     LR_LOADTRANSPARENT | LR_CREATEDIBSECTION | LR_DEFAULTSIZE );

    //  Do we need to update the library?

    if ( generator->_audio_list.empty ( ) )
    {
        ::PostMessage ( hWnd, WM_NOTUNES, 0, 0 );
    }
    else
    if ( generator->IsTimeToUpdate ( RPC_UPDATE_LIBARAY ) )
    {
        ::PostMessage ( hWnd, WM_LASTUPDATED, 0, 0 );
    }

    //  Make sure to announce that we are going to connect to www.davewolf.net

    if ( generator->_updated_version == BST_CHECKED && generator->IsTimeToUpdate ( RPG_UPDATE_VERSION) )
    {
        BOOL notifiy = (generator->_notify_update == BST_CHECKED) ? TRUE : FALSE;
        ::PostMessage ( hWnd, WM_LATESTVERSION, notifiy, 0 );
    }

    Playlist_OnUpdateTuneCount ( hWnd );
}

static DWORD CreateListViewHeaders ( HWND hWnd, HWND hWndList, TCHAR * headers [ ], int count )
{
    LVCOLUMN    column;
    int         index;

    ::ZeroMemory ( & column, sizeof( column ) );

    RECT rc;
    ::GetClientRect ( hWndList, & rc );

    int width = (rc.right - 28) / (count - 1);

    //  For the check column

    column.mask     = LVCF_WIDTH | LVCF_IMAGE;
    column.cx       = 28;
    column.iImage   = 0;

    index = ListView_InsertColumn ( hWndList, 0, & column );
    if ( index == -1 )
    {
        return ::DisplaySystemError ( hWnd );
    }

    for ( int i = 1; i < count; i++ )
    {
        column.mask     = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
        column.fmt      = LVCFMT_LEFT;
        column.pszText  = (TCHAR *) headers [ i ];
        column.cx       = width;

        index = ListView_InsertColumn ( hWndList, i, & column );
        if ( index == -1 )
        {
            return ::DisplaySystemError ( hWnd );
        }
    }
   
    return ERROR_SUCCESS;
}

static void UpdateStatusInfo ( PlaylistGenerator * generator, int id )
{
    TCHAR buffer [ MAX_PATH ];

    //  System has closed the menu

    if ( id == 0xFFFFFFFF)
    {
        _tcscpy ( buffer, _T("Ready") );
    }
    else
    if ( ::LoadString ( generator->_instance, id, buffer, MAX_PATH ) == 0 )
    {
        return;
    }

    generator->_status_bar.SetPaneText ( 0, buffer );
}

static BOOL CanGenerate ( HWND hWnd, PlaylistGenerator * generator )
{
    //  List view must have files shown and we must have a non-zero playlist size or a non-zero byte size

    if ( ListView_GetItemCount ( ::GetDlgItem ( hWnd, IDC_LIST_TUNES ) ) ==  0 )
        return FALSE;

    if ( ::IsDlgButtonChecked ( hWnd, IDC_RADIO_PLAYLIST_BY_NUMBER ) == BST_CHECKED && generator->_max_tunes != 0 )
        return TRUE;
    else
    if ( ::IsDlgButtonChecked ( hWnd, IDC_RADIO_PLAYLIST_BY_SIZE ) == BST_CHECKED && generator->_max_size != 0 )
        return TRUE;
    else
    if ( ::IsDlgButtonChecked ( hWnd, IDC_RADIO_PLAYLIST_BY_DURATION ) == BST_CHECKED && generator->_max_duration != 0 )
        return TRUE;

    return FALSE;
}

static void LoadListViews ( HWND hWnd, PlaylistGenerator * generator )
{
	//	Populate the list view controls (setting _loading_flag to true so we won't trigger events from genre list view to
	//	tunes list view).

	generator->_loading_list_views = true;

    generator->ReadLibrary();

	generator->PopulateTunesListView ( );
    generator->PopulateGenreListView ( );
    generator->PopulateArtistListView ( );

	generator->_loading_list_views = false;

    //  If there is data in our list view, then we can generate a playlist

	HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );

    if ( ListView_GetItemCount ( list_view ) )
    {
        //  Do we sort?

        if ( generator->GetSortColumn ( IDC_LIST_TUNES ) != -1 && generator->GetSortMode ( IDC_LIST_TUNES ) != SORT_NONE )
        {
            generator->UpdateColumnHeader ( hWnd, IDC_LIST_TUNES, generator->GetSortColumn ( IDC_LIST_TUNES ), generator->GetSortMode ( IDC_LIST_TUNES ) );
            ListView_SortItems ( list_view, CompareItems, (LPARAM) generator );
        }
    }
}

static void RestoreListViewPositions ( HWND hWnd, PlaylistGenerator * generator )
{
    RECT rcList, rc;
    int top = 0, width = 0;

    HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );

    ::GetWindowRect ( hWnd, & rc );
    ::GetWindowRect ( list_view, & rcList );

    ::ScreenToClient ( hWnd, (POINT *) & rcList.left );
    ::ScreenToClient ( hWnd, (POINT *) & rcList.right );

    //  If this position is unknown, we just go with the default

    if ( generator->_splitter_position != 0 )
    {
        ::MoveWindow ( list_view, rcList.left, rcList.top, rcList.right - rcList.left, generator->_splitter_position - rcList.top, TRUE );
    }

    //  Restore the column widths
    
    int column_widths [ 8 ];
    ::ZeroMemory ( & column_widths, sizeof ( column_widths ) );

    if ( generator->ReadListView ( _T("Tunes"), 8, (int *) & column_widths ) )
    {
        for ( int i = 1; i < 8; i++ )
        {
            ListView_SetColumnWidth ( list_view, i, column_widths [ i ] );
        }
    }

    //  Restore the genres list view

    list_view = ::GetDlgItem ( hWnd, IDC_LIST_GENRES );

    //  If this position is unknown, we just go with the default

    if ( generator->_splitter_position != 0 )
    {
        top     = generator->_splitter_position + LIST_VIEW_SEPARATOR;
        width   = ((rc.right - rc.left) - LIST_VIEW_SEPARATOR) >> 1;

        ::MoveWindow ( list_view, rc.left, top, width, rc.bottom - top, TRUE );
    }

    //  Restore the column widths
    
    ::ZeroMemory ( & column_widths, sizeof ( column_widths ) );

    if ( generator->ReadListView ( _T("Genres"), 3, (int *) & column_widths ) )
    {
        for ( int i = 1; i < 3; i++ )
        {
            ListView_SetColumnWidth ( list_view, i, column_widths [ i ] );
        }
    }

    //  Restore the artists list view

    list_view = ::GetDlgItem ( hWnd, IDC_LIST_ARTISTS );

    //  If this position is unknown, we just go with the default

    if ( generator->_splitter_position != 0 )
    {
        int left = rc.left + width + LIST_VIEW_SEPARATOR;

        ::MoveWindow ( list_view,left, top, width, rc.bottom - top, TRUE );
    }

    //  Restore the column widths
    
    ::ZeroMemory ( & column_widths, sizeof ( column_widths ) );

    if ( generator->ReadListView ( _T("Artists"), 3, (int *) & column_widths ) )
    {
        for ( int i = 1; i < 3; i++ )
        {
            ListView_SetColumnWidth ( list_view, i, column_widths [ i ] );
        }
    }
}

static void GetMaxSize ( HWND hWnd, PlaylistGenerator * generator )
{
    TCHAR buffer [ MAX_PATH ];
    __int64 value;

    ::GetDlgItemText ( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, buffer, MAX_PATH );
    
    _stscanf ( buffer, _T("%I64d"), & value );

    if ( generator->_size_mode == IDC_RADIO_SIZE_IN_KILOBYTES )
    {
        value *= 1024;
    }
    else
    if ( generator->_size_mode == IDC_RADIO_SIZE_IN_MEGABYTES )
    {
        value *= 1048576;
    }
    else
    if ( generator->_size_mode == IDC_RADIO_SIZE_IN_GIGABYTES )
    {
        value *= 1073741824;
    }

    generator->_max_size = value;
}

static void SetMaxSize ( HWND hWnd, PlaylistGenerator * generator )
{
    TCHAR buffer [ MAX_PATH ];
    TCHAR * mode;
    __int64 value;

    if ( generator->_size_mode == IDC_RADIO_SIZE_IN_BYTES )
    {
        value = generator->_max_size;
        mode = _T("Bytes");
    }
    else
    if ( generator->_size_mode == IDC_RADIO_SIZE_IN_KILOBYTES )
    {
        value = generator->_max_size / 1024;
        mode = _T("KB");
    }
    else
    if ( generator->_size_mode == IDC_RADIO_SIZE_IN_MEGABYTES )
    {
        value = generator->_max_size / 1048576;
        mode = _T("MB");
    }
    else
    {
        value = generator->_max_size / 1073741824;
        mode = _T("GB");
    }

    _stprintf ( buffer, _T("%I64d"), value );

    ::SetDlgItemText( hWnd, IDC_EDIT_PLAYLIST_MAXIMUM, buffer );
    ::SetDlgItemText ( hWnd, IDC_STATIC_MAXIMUM, mode );
}

static void PaintListViewBackground ( HWND hWnd, int ctrl_id, HDC dc )
{
    RECT rc, rect, rcRow;
    HBRUSH hbr [ 2 ];

    //  Get the height of the client rectangle of the list view

    ::GetClientRect ( ::GetDlgItem ( hWnd, ctrl_id ), & rect );

    //  Get the item height of the first item (if it fails, set defaults)

    rc.left = LVIR_BOUNDS;

    int rv = SendDlgItemMessage ( hWnd, ctrl_id, LVM_GETITEMRECT, 0, (LPARAM)& rc );
    if ( !rv )
    {
        rc.left     = 0;
        rc.top      = 20;
        rc.right    = rect.right;
        rc.bottom   = 37;
    }

    //  Determine how many lines the control will actually hold at its current height

    int count = (rect.bottom / (rc.bottom - rc.top) ) + 1;

    //  How many line are actually populated (the background on these will be set when
    //  the item is actually painted)

    int item_count = SendDlgItemMessage ( hWnd, ctrl_id, LVM_GETITEMCOUNT, 0, 0 );

    //  If we are sized for more lines than we actually hold, then we must paint
    //  the background for these additional lines.

    if ( count > item_count )
    {
        rcRow.left     = rc.left;
        rcRow.top      = rc.top;
        rcRow.right    = rc.right;
        rcRow.bottom   = rc.bottom;

        int row_height = rc.bottom - rc.top;

        hbr [ 0 ] = ::CreateSolidBrush ( RGB(224,224,224) );
        hbr [ 1 ] = ::CreateSolidBrush ( RGB(255,255,255) );

        int rows_to_add = (count - item_count);

        //  Advance to the first line we have to paint

        rcRow.top    += item_count * row_height;
        rcRow.bottom += item_count * row_height;

        for ( int i = item_count; i < count; i++ )
        {
            ::FillRect ( dc, & rcRow, (i & 1) ? hbr [ 0 ] : hbr [ 1 ] );

            rcRow.top    += row_height;
            rcRow.bottom += row_height;
        }

        ::DeleteObject ( hbr [ 0 ] );
        ::DeleteObject ( hbr [ 1 ] );
    }
}

static void LoadCheckBoxText ( HWND hWnd )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) ::GetWindowLong ( hWnd, GWL_USERDATA );

    //  Load the tune selection string

    TCHAR buffer [ 1024 ];
    ::LoadString ( generator->_instance, IDS_SELECT, buffer, 1024 );

    ::SetDlgItemText ( hWnd, IDC_STATIC_SELECT, buffer );
}

// End of File

