
#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "PlaylistNameDlg.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

/*
    PLAYLIST_HEADER History:
    
    4/13/06 - Version 1.2 - consolidate multiple *.DAT files into PlaylistGen.dat
              removed element_count
              added tune_count, genre_count, artist_count, aggregate_count

    5/11/09 - Version 1.3 - added audio count to audio files to limit frequency of
              a song being repeated in consecutive play lists               
                
*/

static const DWORD NEW_MAGIC_0 = 0x128A0E9C;
static const DWORD NEW_MAGIC_1 = 0x4B5DF361;

static const DWORD MAGIC_0 = 0x44332211;
static const DWORD MAGIC_1 = 0x88776655;

typedef struct _PLAYLIST_HEADER
{
    DWORD   magic_0;
    DWORD   magic_1;
    DWORD   major_version;
    DWORD   minor_version;
    DWORD   tune_count;
    DWORD   genre_count;
    DWORD   artist_count;
    DWORD   aggregate_count;
    DWORD   reserved [ 56 ];

} PLAYLIST_HEADER;


static const DWORD MAJOR_VERSION = 1;
static const DWORD MINOR_VERSION = 3;

static const TCHAR g_szPlaylistGenerator [ ] = _T("SOFTWARE\\WW Software\\Playlist Generator");

void SortIndex ( IndexList & list )
{
    if ( list.empty () )
        return;

    IndexListIterator begin = list.begin (), end = list.end();

    //  We use a temporary list that will be constructed in order of decending count

    IndexList temp;

    temp.push_back ( (*begin) );
    begin++;

    while ( begin != end )
    {
        IndexListIterator t_begin = temp.begin (), t_end = temp.end();

        while ( t_begin != t_end )
        {
            if ( (*begin)._count > (*t_begin)._count )
                break;
            t_begin++;
        }

        temp.insert ( t_begin, (*begin) );        

        begin++;
    }

    //  Splicing is cheaper than assigning

    list.clear ( );
    list.splice ( list.end(), temp );
}

int CALLBACK CompareItems ( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) lParamSort;

    return generator->SortListView ( lParam1, lParam2 );
}

int CALLBACK CompareGenreItems ( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) lParamSort;

    return generator->SortGenreListView ( lParam1, lParam2 );
}

int CALLBACK CompareArtistItems ( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
    PlaylistGenerator * generator = (PlaylistGenerator *) lParamSort;

    return generator->SortArtistListView ( lParam1, lParam2 );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PlaylistGenerator::PlaylistGenerator ( ) :
    _instance                           ( NULL ),
    _main_hwnd                          ( NULL ),
    _form_hwnd                          ( NULL ),
    _sizing_hwnd                        ( NULL ),
    _logo                               ( NULL ),
    _loading_list_views		            ( false ),
    _update_column_width                ( false ),
    _auto_update_image                  ( false ),
    _process_multiple_items             ( false ),
    _total_tune_size                    ( 0 ),
    _total_tune_duration                ( 0 ),
    _max_tunes                          ( 0 ),
    _max_size                           ( 0 ),
    _max_duration                       ( 0 ),
    _copy_tunes                         ( 0 ),
    _reminder                           ( 0 ),
    _days                               ( 0 ),
    _aggregate                          ( 0 ),
    _aggregate_size                     ( 0 ),
    _maintain_selections                ( 0 ),
    _prompt_for_playlist                ( 0 ),
    _size_mode                          ( 0 ),
    _randomize_manual_selections        ( 0 ),
    _duplicate_mode                     ( LIMIT_REPEATS ),
    _repeat_count                       ( 5 ),
    _limit_artist                       ( 0 ),
    _limit_artist_count                 ( 0 ),
    _playlist_mode                      ( 0 ),
    _splitter_position                  ( 0 ),
    _make_paths_relative                ( 0 ),
    _log_library                        ( 0 ),
    _updated_version                    ( 0 ),
    _updated_days                       ( 0 ),
    _notify_update                      ( 0 ),
    _worker_thread                      ( NULL ),
    _cancel                             ( false )
{
    _sort_info [ 0 ]._sort_mode     = SORT_NONE;
    _sort_info [ 0 ]._sort_column   = -1;
    _sort_info [ 1 ]._sort_mode     = SORT_NONE;
    _sort_info [ 1 ]._sort_column   = -1;
    _sort_info [ 2 ]._sort_mode     = SORT_NONE;
    _sort_info [ 2 ]._sort_column   = -1;

    _window_name            [ 0 ]  = '\0';
    _class_name             [ 0 ]  = '\0';
    _music_directory        [ 0 ]  = '\0';
    _playlist_directory     [ 0 ]  = '\0';
    _default_aggregate_name [ 0 ]  = '\0';
    _default_playlist_name  [ 0 ]  = '\0';

    _last_updated.dwHighDateTime = 0;
    _last_updated.dwLowDateTime  = 0;

    _last_updated_version.dwHighDateTime = 0;
    _last_updated_version.dwLowDateTime  = 0;
}

PlaylistGenerator::~PlaylistGenerator ( )
{

}

void PlaylistGenerator::Initialize ( HWND hWnd )
{
    HKEY key;
    long success;
    DWORD size, type;

    _form_hwnd = hWnd;

    //  Initialize with hardcoded defaults in case registry key is not present

    ::GetModuleFileName ( _instance, _music_directory, MAX_PATH );
    ::PathRemoveFileSpec ( _music_directory );

    _tcscpy ( _playlist_directory, _music_directory );

    _max_tunes                      = 25;
    _max_size                       = 100 * 1024 * 1024;
    _max_duration                   = 3600;
    _copy_tunes                     = BST_UNCHECKED;
    _reminder                       = BST_CHECKED;
    _days                           = 7;
    _aggregate                      = BST_UNCHECKED;
    _aggregate_size                 = 2;
    _maintain_selections            = BST_UNCHECKED;
    _prompt_for_playlist            = BST_CHECKED;
    _size_mode                      = IDC_RADIO_SIZE_IN_MEGABYTES;
    _randomize_manual_selections    = BST_UNCHECKED;
    _duplicate_mode                 = LIMIT_REPEATS;
    _repeat_count                   = 5;
    _limit_artist                   = BST_UNCHECKED;
    _limit_artist_count             = 1;
    _playlist_mode                  = IDC_RADIO_PLAYLIST_BY_NUMBER;
    _splitter_position              = 0;
    _make_paths_relative            = BST_UNCHECKED;
    _log_library                    = BST_CHECKED;
    _updated_version                = BST_CHECKED;
    _updated_days                   = 7;
    _notify_update                  = BST_CHECKED;

    _tcscpy ( _default_aggregate_name, _T("Aggregate") );
    _tcscpy ( _default_playlist_name, _T("Playlist.m3u") );

    _sort_info [ 0 ]._sort_mode     = SORT_NONE;
    _sort_info [ 0 ]._sort_column   = -1;
    _sort_info [ 1 ]._sort_mode     = SORT_NONE;
    _sort_info [ 1 ]._sort_column   = -1;
    _sort_info [ 2 ]._sort_mode     = SORT_NONE;
    _sort_info [ 2 ]._sort_column   = -1;

    //  Try to read registry keys

    success = ::RegOpenKeyEx ( HKEY_LOCAL_MACHINE, g_szPlaylistGenerator, NULL, KEY_READ, & key );

    if (success == ERROR_SUCCESS)
    {
        size = sizeof( _music_directory );
        type = REG_SZ;

        success = ::RegQueryValueEx ( key, _T("MusicDirectory"), 0, & type, (BYTE *) & _music_directory, & size );

        if ( success != ERROR_SUCCESS )
        {
            ::GetModuleFileName ( _instance, _music_directory, MAX_PATH );
            ::PathRemoveFileSpec ( _music_directory );
        }

        size = sizeof( _max_tunes );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("MaxTunes"), 0, & type, (BYTE *) & _max_tunes, & size );

        if ( success != ERROR_SUCCESS )
        {
            _max_tunes = 25;
        }

        size = sizeof( _max_size );
        type = REG_BINARY;

        success = ::RegQueryValueEx ( key, _T("MaxSize"), 0, & type, (BYTE *) & _max_size, & size );    

        if ( success != ERROR_SUCCESS )
        {
            _max_size = 100 * 1024 * 1024;
        }

        size = sizeof( _max_duration );
        type = REG_BINARY;

        success = ::RegQueryValueEx ( key, _T("MaxDuration"), 0, & type, (BYTE *) & _max_duration, & size );    

        if ( success != ERROR_SUCCESS )
        {
            _max_duration = 3600;
        }

        size = sizeof( _copy_tunes );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("CopyTunes"), 0, & type, (BYTE *) & _copy_tunes, & size );

        if ( success != ERROR_SUCCESS )
        {
            _copy_tunes = BST_UNCHECKED;
        }

        size = sizeof( _playlist_directory );
        type = REG_SZ;

        success = ::RegQueryValueEx ( key, _T("PlaylistDirectory"), 0, & type, (BYTE *) & _playlist_directory, & size );

        if ( success != ERROR_SUCCESS )
        {
            ::GetModuleFileName ( _instance, _playlist_directory, MAX_PATH );
            ::PathRemoveFileSpec ( _playlist_directory );
        }

        size = sizeof( _reminder );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("UpdateReminder"), 0, & type, (BYTE *) & _reminder, & size );

        if ( success != ERROR_SUCCESS )
        {
            _reminder = BST_CHECKED;
        }

        size = sizeof( _days );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("NumberOfDays"), 0, & type, (BYTE *) & _days, & size );

        if ( success != ERROR_SUCCESS )
        {
            _days = 7;
        }

        size = sizeof( _aggregate );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("EnableAggregation"), 0, & type, (BYTE *) & _aggregate, & size );

        if ( success != ERROR_SUCCESS )
        {
            _aggregate = BST_UNCHECKED;
        }

        size = sizeof( _aggregate_size );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("AggregateSize"), 0, & type, (BYTE *) & _aggregate_size, & size );

        if ( success != ERROR_SUCCESS )
        {
            _aggregate_size = 2;
        }

        size = sizeof( _default_aggregate_name );
        type = REG_SZ;

        success = ::RegQueryValueEx ( key, _T("DefaultAggregateName"), 0, & type, (BYTE *) & _default_aggregate_name, & size );

        if ( success != ERROR_SUCCESS )
        {
            _tcscpy ( _default_aggregate_name, _T("Aggregate") );
        }

        size = sizeof( _maintain_selections );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("MaintainSelections"), 0, & type, (BYTE *) & _maintain_selections, & size );

        if ( success != ERROR_SUCCESS )
        {
            _maintain_selections = BST_UNCHECKED;
        }        

        size = sizeof( _last_updated );
        type = REG_BINARY;

        success = ::RegQueryValueEx ( key, _T("LastUpdated"), 0, & type, (BYTE *) & _last_updated, & size );

        if ( success != ERROR_SUCCESS )
        {
            _last_updated.dwHighDateTime = 0;
            _last_updated.dwLowDateTime  = 0;
        }

        size = sizeof( _prompt_for_playlist );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("PromptForPlaylist"), 0, & type, (BYTE *) & _prompt_for_playlist, & size );

        if ( success != ERROR_SUCCESS )
        {
            _prompt_for_playlist = BST_CHECKED;
        }

        size = sizeof ( _size_mode );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("SizeMode"), 0, & type, (BYTE *) & _size_mode, & size );

        if ( success != ERROR_SUCCESS )
        {
            _size_mode = IDC_RADIO_SIZE_IN_MEGABYTES;
        }

        size = sizeof ( _randomize_manual_selections );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("RandomizeManualSelections"), 0, & type, (BYTE *) & _randomize_manual_selections, & size );

        if ( success != ERROR_SUCCESS )
        {
            _randomize_manual_selections = BST_UNCHECKED;
        }

        size = sizeof ( _duplicate_mode );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("DuplicateMode"), 0, & type, (BYTE *) & _duplicate_mode, & size );

        if ( success != ERROR_SUCCESS ||
             ( _duplicate_mode < LIMIT_REPEATS  || LIMIT_REPEATS > ALLOW_DUPLICATES ) )
        {
            _duplicate_mode = LIMIT_REPEATS;
        }

        size = sizeof( _repeat_count );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("RepeatCount"), 0, & type, (BYTE *) & _repeat_count, & size );

        if ( success != ERROR_SUCCESS )
        {
            _repeat_count = 5;
        }

        size = sizeof( _limit_artist );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("LimitArtist"), 0, & type, (BYTE *) & _limit_artist, & size );

        if ( success != ERROR_SUCCESS )
        {
            _limit_artist = BST_UNCHECKED;
        }

        size = sizeof( _limit_artist_count );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("LimitArtistCount"), 0, & type, (BYTE *) & _limit_artist_count, & size );

        if ( success != ERROR_SUCCESS )
        {
            _limit_artist_count = 1;
        }

        size = sizeof ( _playlist_mode );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("PlaylistMode"), 0, & type, (BYTE *) & _playlist_mode, & size );

        if ( success != ERROR_SUCCESS )
        {
            _playlist_mode = IDC_RADIO_PLAYLIST_BY_NUMBER;
        }

        size = sizeof ( _splitter_position );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("SplitterPos"), 0, & type, (BYTE *) & _splitter_position, & size );

        if ( success != ERROR_SUCCESS )
        {
            _splitter_position = 0;
        }

        size = sizeof ( _make_paths_relative );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("RelativePaths"), 0, & type, (BYTE *) & _make_paths_relative, & size );

        if ( success != ERROR_SUCCESS )
        {
            _make_paths_relative = BST_UNCHECKED;
        }

        size = sizeof ( _log_library );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("LogLibrary"), 0, & type, (BYTE *) & _log_library, & size );

        if ( success != ERROR_SUCCESS )
        {
            _log_library = BST_CHECKED;
        }

        size = sizeof ( _updated_version );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("UpdatedVersion"), 0, & type, (BYTE *) & _updated_version, & size );

        if ( success != ERROR_SUCCESS )
        {
            _updated_version = BST_CHECKED;
        }   
        
        size = sizeof ( _updated_days );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("NumberOfDaysVersionUpdate"), 0, & type, (BYTE *) & _updated_days, & size );

        if ( success != ERROR_SUCCESS )
        {
            _updated_days = 7;
        }

        size = sizeof ( _notify_update );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("NotifyUpdate"), 0, & type, (BYTE *) & _notify_update, & size );

        if ( success != ERROR_SUCCESS )
        {
            _notify_update = BST_CHECKED;
        }   

        size = sizeof( _last_updated_version );
        type = REG_BINARY;

        success = ::RegQueryValueEx ( key, _T("LastUpdatedVersion"), 0, & type, (BYTE *) & _last_updated_version, & size );

        if ( success != ERROR_SUCCESS )
        {
            _last_updated_version.dwHighDateTime = 0;
            _last_updated_version.dwLowDateTime  = 0;
        }

        size = sizeof( _default_playlist_name );
        type = REG_SZ;

        success = ::RegQueryValueEx ( key, _T("DefaultPlaylist"), 0, & type, (BYTE *) & _default_playlist_name, & size );

        if ( success != ERROR_SUCCESS )
        {
            _tcscpy ( _default_playlist_name, _T("Playlist.m3u") );
        }

        size = sizeof( _sort_info [ 0 ]._sort_column );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("SortColumn"), 0, & type, (BYTE *) & _sort_info [ 0 ]._sort_column, & size );

        if ( success != ERROR_SUCCESS )
        {
            _sort_info [ 0 ]._sort_column = -1;
        }

        size = sizeof( _sort_info [ 0 ]._sort_mode );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("SortMode"), 0, & type, (BYTE *) & _sort_info [ 0 ]._sort_mode, & size );

        if ( success != ERROR_SUCCESS )
        {
            _sort_info [ 0 ]._sort_mode = SORT_NONE;
        }

        ::RegCloseKey ( key );
    }

    //ReadLibrary ( );
}

void PlaylistGenerator::Uninitialize ( )
{
    HKEY key;
    long success;

    WriteLibrary ( );

    success = ::RegCreateKeyEx ( HKEY_LOCAL_MACHINE, g_szPlaylistGenerator, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, & key, NULL );

    if (success == ERROR_SUCCESS)
    {  
        success = ::RegSetValueEx( key, _T("MusicDirectory"), 0, REG_SZ, (BYTE *) _music_directory, _tcslen ( _music_directory ) * sizeof(TCHAR) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("MaxTunes"), 0, REG_DWORD, (BYTE *) & _max_tunes, sizeof(_max_tunes) );
    
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("MaxSize"), 0, REG_BINARY, (BYTE *) & _max_size, sizeof(_max_size) );
   
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("MaxDuration"), 0, REG_BINARY, (BYTE *) & _max_duration, sizeof(_max_duration) );
   
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("CopyTunes"), 0, REG_DWORD, (BYTE *) & _copy_tunes, sizeof(_copy_tunes) );
    
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("PlaylistDirectory"), 0, REG_SZ, (BYTE *) _playlist_directory, _tcslen ( _playlist_directory ) * sizeof(TCHAR) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("UpdateReminder"), 0, REG_DWORD, (BYTE *) & _reminder, sizeof(_reminder) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("NumberOfDays"), 0, REG_DWORD, (BYTE *) & _days, sizeof(_days) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("EnableAggregation"), 0, REG_DWORD, (BYTE *) & _aggregate, sizeof(_aggregate) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("AggregateSize"), 0, REG_DWORD, (BYTE *) & _aggregate_size, sizeof(_aggregate_size) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("DefaultAggregateName"), 0, REG_SZ, (BYTE *) _default_aggregate_name, _tcslen ( _default_aggregate_name ) * sizeof(TCHAR) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("MaintainSelections"), 0, REG_DWORD, (BYTE *) & _maintain_selections, sizeof(_maintain_selections) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("PromptForPlaylist"), 0, REG_DWORD, (BYTE *) & _prompt_for_playlist, sizeof(_prompt_for_playlist) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("SizeMode"), 0, REG_DWORD, (BYTE *) & _size_mode, sizeof(_size_mode) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("RandomizeManualSelections"), 0, REG_DWORD, (BYTE *) & _randomize_manual_selections, sizeof(_randomize_manual_selections) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("DuplicateMode"), 0, REG_DWORD, (BYTE *) & _duplicate_mode, sizeof(_duplicate_mode) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("RepeatCount"), 0, REG_DWORD, (BYTE *) & _repeat_count, sizeof(_repeat_count) );
    
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("LimitArtist"), 0, REG_DWORD, (BYTE *) & _limit_artist, sizeof(_limit_artist) );
    
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("LimitArtistCount"), 0, REG_DWORD, (BYTE *) & _limit_artist_count, sizeof(_limit_artist_count) );
    
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("PlaylistMode"), 0, REG_DWORD, (BYTE *) & _playlist_mode, sizeof(_playlist_mode) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("SplitterPos"), 0, REG_DWORD, (BYTE *) & _splitter_position, sizeof(_splitter_position) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("RelativePaths"), 0, REG_DWORD, (BYTE *) & _make_paths_relative, sizeof(_make_paths_relative) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("LogLibrary"), 0, REG_DWORD, (BYTE *) & _log_library, sizeof(_log_library) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("UpdatedVersion"), 0, REG_DWORD, (BYTE *) & _updated_version, sizeof(_updated_version) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }
     
        success = ::RegSetValueEx ( key, _T("NumberOfDaysVersionUpdate"), 0, REG_DWORD, (BYTE *) & _updated_days, sizeof(_updated_days) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("NotifyUpdate"), 0, REG_DWORD, (BYTE *) & _notify_update, sizeof(_notify_update) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("DefaultPlaylist"), 0, REG_SZ, (BYTE *) _default_playlist_name, _tcslen ( _default_playlist_name ) * sizeof(TCHAR) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("SortColumn"), 0, REG_DWORD, (BYTE *) & _sort_info [ 0 ]._sort_column, sizeof(_sort_info [ 0 ]._sort_column) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("SortMode"), 0, REG_DWORD, (BYTE *) & _sort_info [ 0 ]._sort_mode, sizeof(_sort_info [ 0 ]._sort_mode) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        ::RegCloseKey ( key );
    }

    SavePosition ( );
    SaveListView ( _T("Tunes"), 8, IDC_LIST_TUNES );
	SaveListView ( _T("Genres"), 2, IDC_LIST_GENRES );
    SaveListView ( _T("Artists"), 2, IDC_LIST_ARTISTS );

    ::DeleteObject ( _logo );
    ::DestroyWindow ( _form_hwnd );
}

void PlaylistGenerator::CleanUp ( )
{
    _audio_list.clear ( );
    _selected_list.clear ( );
    _genre_list.clear ( );
    _artist_list.clear ( );
    _aggregate_list.clear ( );
}

void PlaylistGenerator::InitializeLibrary ( )
{
    HANDLE              file;
    WIN32_FIND_DATA     find;
    BOOL                success = TRUE;
    TCHAR               temp        [ MAX_PATH ];
    TCHAR               file_name   [ MAX_PATH ];
    TCHAR               root_name   [ MAX_PATH ];
    TCHAR               search_name [ MAX_PATH ];
    TCHAR               buffer      [ MAX_PATH ];

    TCHAR               module_path [ MAX_PATH ];
    TCHAR               log_file    [ MAX_PATH ];
    FILE *              out = NULL;

    if ( _log_library )
    {
        ::GetModuleFileName ( _instance, module_path, MAX_PATH );
        ::PathRemoveFileSpec ( module_path );

        _tcscpy ( log_file, module_path );
        _tcscat ( log_file, _T("\\library.log") );

        out = _tfopen ( log_file, _T("wt") );
    }

    unsigned len = _tcslen ( _music_directory );
    if ( len == 0 )
    {
        if ( out )
        {
            _ftprintf ( out, _T("The 'music directory' has not been specified\n") );
        }
        fclose ( out );
        return;
    }

    typedef std::list<BasicString>              DirectoryList;
    typedef std::list<BasicString>              FileList;
    typedef std::list<BasicString>::iterator    StringListIterator;

    DirectoryList directories;
    FileList files;

    _status_bar.SetPaneText ( 0, _T("\nDetermining number of audio files in directory\n") );

    if ( out )
    {
        _ftprintf ( out, _T("Determining number of audio files in directory\n") );
    }

    //  No tunes or genres or artists

    _status_bar.SetPaneText ( 1, _T("") );
    _status_bar.SetPaneText ( 2, _T("") );
    _status_bar.SetPaneText ( 3, _T("") );

	//	Make the last part bigger   

    int width, parts [ 4 ];

    _status_bar.Lock ();

    width = _status_bar.GetBarWidth ( );

    parts [ 0 ] = width - 345;
    parts [ 1 ] = width - 220;
    parts [ 2 ] = width - 200;
    parts [ 3 ] = -1;

    ::SendMessage ( _status_bar._status_hwnd, SB_SETPARTS, 4, (LPARAM) & parts );

    _status_bar.Unlock ();

    _elapsed_start = ::GetTickCount ( );
    ::SetTimer ( _form_hwnd, 1, 1000, NULL );
    
    _status_bar.SetPaneText ( 1, _T("Elapsed time: 00:00:00") );

    //  Recurse the music directory and get all files

    directories.push_back ( BasicString(_music_directory) );

    int state = 0;

    while ( !directories.empty () )
    {
        try
        {
            state = 1;
            BasicString directory = directories.front ( );
            state = 2;
            directories.pop_front ( );
            state = 3;

            unsigned len = directory.length ( );
            if ( len == 0 )
            {
                if ( out )
                {
                    _ftprintf ( out, _T("The directory recursion list is empty\n") );
                }
                continue;
            }

            state = 4;

            _tcscpy ( temp, directory.c_str () );

            if ( out )
            {
                _ftprintf ( out, _T("Recursing '%s'\n"), temp );
            }

            state = 5;

            if ( temp [ len - 1 ] != '\\' )
                _tcscat ( temp, _T("\\") );

            state = 6;

            _tcscpy ( root_name, temp );
            _tcscpy ( search_name, root_name );
            _tcscat ( search_name, _T("*.*") );

            state = 7;

            file = ::FindFirstFile ( search_name, & find );
            if ( file == INVALID_HANDLE_VALUE )
            {
                if ( out )
                {
                    _ftprintf ( out, _T("Failure of Windows API function FindFirstFile\tError code %lu\n"), ::GetLastError() );
                    fclose ( out );
                }
                ::DisplaySystemError ( _main_hwnd );
                return;
            }

            state = 8;

            success = TRUE;

            while ( success )
            {
                state = 9;

                if ( _cancel )
                {
                    if ( out )
                    {
                        _ftprintf ( out, _T("The user has selected \"Cancel\".\nTerminating music directory recursion\n") );
                    }

				    break;
                }

                state = 10;

                //  Is this a directory?

                if ( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {
                    state = 11;

                    if ( !( find.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ) )
                    {
                        if ( _tcsicmp ( find.cFileName, _T(".") ) != 0 && _tcsicmp ( find.cFileName, _T("..") ) != 0 )
                        {
                            TCHAR next_directory [ MAX_PATH ];

                            state = 12;

                            _tcscpy ( next_directory, temp );
                            _tcscat ( next_directory, find.cFileName );

                            state = 13;

                            directories.push_back ( BasicString(next_directory) );

                            state = 14;
                        }
                    }
                    else
                    {
                        if ( out )
                        {
                            _ftprintf ( out, _T("Skipping directory '%s' (0x%0.8X)\n"), find.cFileName, find.dwFileAttributes );
                        }
                    }
                }

                //   Is this not a system file or a temporary file or a sparse file?

                else
                if ( !( find.dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_SPARSE_FILE) ) )
                {
                    state = 15;

                    _tcscpy ( file_name, root_name );
                    _tcscat ( file_name, find.cFileName );

                    state = 16;

                    files.push_back ( BasicString(file_name) );

                    state = 17;

                    //  Keep a running count of the number of files

                    _stprintf ( buffer, _T("Number of files %d"), files.size () );
                    _status_bar.SetPaneText ( 3, buffer );

                    state = 18;

                    if ( out )
                    {
                        _ftprintf ( out, _T("'%s' (0x%0.8X)\n"), file_name, find.dwFileAttributes );
                    }
                }
                else
                {
                    state = 19;

                    _tcscpy ( file_name, root_name );
                    _tcscat ( file_name, find.cFileName );

                    state = 20;

                    if ( out )
                    {
                        _ftprintf ( out, _T("Omit '%s' (0x%0.8X"), file_name, find.dwFileAttributes );

                        if ( find.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM )
                        {
                            _ftprintf ( out, _T(") SYSTEM FILE") );
                        }
                        if ( find.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY )
                        {
                            _ftprintf ( out, _T(") TEMP FILE") );
                        }
                        if ( find.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE )
                        {
                            _ftprintf ( out, _T(") SPARSE FILE") );
                        }

                        _ftprintf ( out, _T(")\n") );
                    }
                }

                state = 21;

                success = ::FindNextFile ( file, & find );

                state = 22;
            }
        }
        catch ( ... )
        {
            if ( out )
            {
                _ftprintf ( out, _T("***EXCEPTION***\n") );
                _ftprintf ( out, _T("\tAn exception occurred while recursing the music directory.\tState %lu\n"), state );
                _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
                _ftprintf ( out, _T("\tNumber of directories:\t%lu\n"), directories.size() );
                _ftprintf ( out, _T("\tNumber of file:\t%lu\n"), files.size() );
                _ftprintf ( out, _T("\tNumber of file:\t%lu\n"), files.size() );
            }
        }

        state = 23;

        ::FindClose ( file );

        state = 24;

        if ( _cancel )
            break;

        state = 25;
    }

    //  Process the list of files for audio files

    unsigned count = 0, total = files.size();

    if ( out )
    {
        _ftprintf ( out, _T("\n\nNumber of file found under %s (%lu)\n\n"), _music_directory, total );
    }

    _stprintf ( buffer, _T("Number of files in directory %d"), files.size() );
    _status_bar.SetPaneText ( 0, buffer );

    _status_bar.Lock ();

    width = _status_bar.GetBarWidth ( );

    parts [ 0 ] = width - 525;
    parts [ 1 ] = width - 400;
    parts [ 2 ] = width - 200;
    parts [ 3 ] = -1;

    _status_bar.CreateProgressBar ( 4, parts, total );

    _status_bar.Unlock ();

    AudioList           audio_list;
    IndexList           genre_list;
    IndexList           artist_list;

    StringListIterator begin = files.begin(), end = files.end();

    state = 0;

    while ( begin != end )
    {
        try
        {

            state = 1;

            if ( _cancel )
                break;

            state = 2;

            BasicString & file_name = * begin;

            state = 3;

            BasicString song = ::SongTitleFromFilename ( file_name );

            state = 4;

            //  Update status with the file being processed

            _status_bar.SetPaneText ( 0, (TCHAR *) song.c_str() );        

            HANDLE audio_file = ::CreateFile ( file_name.c_str (), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, OPEN_EXISTING, NULL );

            state = 5;

            if ( audio_file != INVALID_HANDLE_VALUE )
            {
                state = 6;

                DWORD fs = ::GetFileSize ( audio_file, NULL );

                state = 7;

                AudioFile library_entry ( file_name, fs );

                state = 8;

                //  Does the file/size appear in the current image?

                AudioListIterator audio_list_end = _audio_list.end ();
                AudioListIterator file_found = std::find ( _audio_list.begin(), audio_list_end, library_entry );

                state = 9;

                bool is_audio = false;

                if ( file_found != audio_list_end && fs == (*file_found)._fs )
                {
                    state = 10;

                    library_entry = (*file_found);
                    is_audio = true;

                    state = 11;

                    if ( out )
                    {
                        _ftprintf ( out, _T("'%s' found in current image\n"), file_name.c_str () );
                    }
                }

                //  Too bad - we have to analyze the file

                else

                /*if ( IsfLaCAudio ( audio_file, library_entry )  ||
                     IsOggAudio  ( audio_file, library_entry )  ||
                     IsMP4Audio  ( audio_file, library_entry )  ||
                     IsMPEGAudio ( audio_file, library_entry )  ||
                     IsWMAAudio  ( audio_file, library_entry ) )
                     
                //  Better
                if ( IsMPEGAudio ( audio_file, library_entry )  ||
                     IsMP4Audio  ( audio_file, library_entry )  ||
                     IsWMAAudio  ( audio_file, library_entry )  ||
                     IsOggAudio  ( audio_file, library_entry )  ||
                     IsfLaCAudio ( audio_file, library_entry ) )
                {
                    state = 12;

                    is_audio = true;
                }*/

                {
                    //  Do some tests by file extention (non-case specific)

                    TCHAR * ext = _tcsrchr ( file_name.c_str(), '.' );
                    if ( ext )
                    {
                        if ( ::StrStrI ( ext, _T(".mp3") ) && IsMPEGAudio ( audio_file, library_entry ) )
                        {
                            state = 12;
                            is_audio = true;
                        }
                        else
                        if ( ( ::StrStrI ( ext, _T(".mp4") ) || ::StrStrI ( ext, _T(".m4a") ) ) &&
                              IsMP4Audio ( audio_file, library_entry ) )
                        {
                            state = 12;
                            is_audio = true;
                        }
                        else
                        if ( ::StrStrI ( ext, _T(".wma") ) && IsWMAAudio ( audio_file, library_entry ) )
                        {
                            state = 12;
                            is_audio = true;
                        }
                        else
                        if ( ::StrStrI ( ext, _T(".ogg") ) && IsOggAudio ( audio_file, library_entry ) )
                        {
                            state = 12;
                            is_audio = true;
                        }
                        else
                        if ( ::StrStrI ( ext, _T(".flac") ) && IsfLaCAudio ( audio_file, library_entry ) )
                        {
                            state = 12;
                            is_audio = true;
                        }
                    }

                    if ( !is_audio )
                    {
                        if ( IsMPEGAudio ( audio_file, library_entry )  ||
                             IsMP4Audio  ( audio_file, library_entry )  ||
                             IsWMAAudio  ( audio_file, library_entry )  ||
                             IsOggAudio  ( audio_file, library_entry )  ||
                             IsfLaCAudio ( audio_file, library_entry ) )
                        {
                            state = 12;
                            is_audio = true;
                        }
                    }
                }

                if ( is_audio )
                {
                    state = 13;

                    LPCTSTR genre_type = library_entry.GetGenre ();
                    if ( genre_type )
                    {
                        state = 14;

                        AudioIndex genre ( genre_type );

                        state = 15;

                        IndexListIterator iter = std::find ( genre_list.begin (), genre_list.end (), genre );
                        if ( iter != genre_list.end () )
                        {
                            state = 16;

                            (*iter)._count ++;
                        }
                        else
                        {
                            state = 17;

                            genre._count ++;
                            genre_list.push_back ( genre );
                        }

                        state = 18;
                    }

                    state = 19;

                    LPCTSTR artist_name = library_entry.GetArtist ();
                    if ( artist_name )
                    {
                        state = 20;

                        AudioIndex artist ( artist_name );

                        state = 21;

                        IndexListIterator iter = std::find ( artist_list.begin (), artist_list.end (), artist );
                        if ( iter != artist_list.end () )
                        {
                            state = 22;

                            (*iter)._count ++;
                        }
                        else
                        {
                            state = 23;

                            artist._count ++;
                            artist_list.push_back ( artist );
                        }

                        state = 24;
                    }

                    if ( out )
                    {
                        _ftprintf ( out, _T("'%s' (%s)\n"), file_name.c_str (), library_entry.GetFileType () );
                    }

                    state = 25;

                    audio_list.push_back ( library_entry );

                    state = 26;
                }
                else
                if ( out )
                {
                    _ftprintf ( out, _T("Didn't recognize '%s'\n"), file_name.c_str () );
                }

                state = 27;

                ::CloseHandle ( audio_file );

                state = 28;
            }
        }
        catch ( ... )
        {
            if ( out )
            {
                _ftprintf ( out, _T("***EXCEPTION***\n") );
                _ftprintf ( out, _T("\tAn exception occurred while processing the music files\tState %lu\n"), state );
                _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
                _ftprintf ( out, _T("\tNumber of files processed:\t%lu\n"), count );
                _ftprintf ( out, _T("\tNumber of items in music list:\t%lu\n"), audio_list.size() );
                _ftprintf ( out, _T("\tNumber of items in genre list:\t%lu\n"), genre_list.size() );
                _ftprintf ( out, _T("\tNumber of items in artist list:\t%lu\n"), artist_list.size() );
                _ftprintf ( out, _T("\tCurrent file:\t%s\n"), (*begin).c_str() );
            }
        }

        state = 29;

        _stprintf ( buffer, _T("Processed %lu / %lu"), ++count, total );
        _status_bar.SetPaneText ( 3, buffer );

        state = 30;

        _status_bar.UpdateProgressBar ( count );

        state = 31;

        begin ++;

        state = 32;
    }

    if ( out )
    {
        _ftprintf ( out, _T("\n\nNumber of music files (%lu)\n"), audio_list.size() );
    }

    //  Delete the progress bar and update the part widths

    _status_bar.Lock ();

    width = _status_bar.GetBarWidth ( );

    parts [ 0 ] = width - 190;
    parts [ 1 ] = width - 65;
    parts [ 2 ] = width - 45;
    parts [ 3 ] = -1;

    if ( out )
    {
        _ftprintf ( out, _T("\n\nDestroying progress bar\n") );
    }

    _status_bar.DestroyProgressBar ( 4, parts );

    _status_bar.Unlock ();

    _status_bar.SetPaneText ( 3, _T("") );

    //  If we are preserving the current status values

    if ( _maintain_selections == BST_CHECKED )
    {
        _status_bar.SetPaneText ( 0, _T("Preserving current selections") );

        if ( out )
        {
            _ftprintf ( out, _T("Preserving audio list state\n") );
        }

        //  For each item in our local list, we have to look up that item in the class list and get the state        
        try
        {
            PreserveAudioListState ( audio_list );
        }
        catch ( ... )
        {
            if ( out )
            {
                _ftprintf ( out, _T("***EXCEPTION***\n") );
                _ftprintf ( out, _T("\tAn exception occurred in PreserveAudioListState\n") );
                _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
            }
        }

        if ( out )
        {
            _ftprintf ( out, _T("Preserving genre list state\n") );
        }

        try
        {
            PreserveIndexListState ( genre_list, _genre_list );
        }
        catch ( ... )
        {
            if ( out )
            {
                _ftprintf ( out, _T("***EXCEPTION***\n") );
                _ftprintf ( out, _T("\tAn exception occurred in PreserveIndexListState(genres)\n") );
                _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
            }
        }

        if ( out )
        {
            _ftprintf ( out, _T("Preserving artist list state\n") );
        }

        try
        {
            PreserveIndexListState ( artist_list, _artist_list );
        }
        catch ( ... )
        {
            if ( out )
            {
                _ftprintf ( out, _T("***EXCEPTION***\n") );
                _ftprintf ( out, _T("\tAn exception occurred in PreserveIndexListState(artists)\n") );
                _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
            }
        }
    }
    else
    {
        if ( out )
        {
            _ftprintf ( out, _T("Cleaning up\n") );
        }

        try
        {
            CleanUp ( );
        }
        catch ( ... )
        {
            if ( out )
            {
                _ftprintf ( out, _T("***EXCEPTION***\n") );
                _ftprintf ( out, _T("\tAn exception occurred in CleanUp\n") );
                _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
            }
        }

        if ( out )
        {
            _ftprintf ( out, _T("Splicing data lists\n") );
        }

        //  Splicing is cheaper than assigning

        try
        {
            if ( out )
            {
                _ftprintf ( out, _T("Splicing audio list\n") );
            }

            _audio_list.splice  ( _audio_list.end(), audio_list );

            if ( out )
            {
                _ftprintf ( out, _T("Splicing genre list\n") );
            }

            _genre_list.splice  ( _genre_list.end(), genre_list );

            if ( out )
            {
                _ftprintf ( out, _T("Splicing artist list\n") );
            }

            _artist_list.splice ( _artist_list.end(), artist_list );
        }
        catch ( ... )
        {
            if ( out )
            {
                _ftprintf ( out, _T("***EXCEPTION***\n") );
                _ftprintf ( out, _T("\tAn exception occurred splicing\n") );
                _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
            }
        }
    }

    _status_bar.SetPaneText ( 0, _T("Loading tunes") );

    if ( out )
    {
        _ftprintf ( out, _T("Loading the tunes listview control\n") );
    }

    try
    {
        RepopulateTunesListView ( );
    }
    catch ( ... )
    {
        if ( out )
        {
            _ftprintf ( out, _T("***EXCEPTION***\n") );
            _ftprintf ( out, _T("\tAn exception occurred in RepopulateTunesListView\n") );
            _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
        }
    }

    if ( _aggregate == BST_CHECKED )
    {
        _status_bar.SetPaneText ( 0, _T("Aggregating genre") );

        if ( out )
        {
            _ftprintf ( out, _T("Aggregating genre\n") );
        }

        try
        {
            EnableAggregation ( BST_CHECKED, _aggregate_size, this->_default_aggregate_name );
        }
        catch ( ... )
        {
            if ( out )
            {
                _ftprintf ( out, _T("***EXCEPTION***\n") );
                _ftprintf ( out, _T("\tAn exception occurred in EnableAggregation\n") );
                _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
            }
        }
    }
    else
    {
        _status_bar.SetPaneText ( 0, _T("Sorting genre") );

        if ( out )
        {
            _ftprintf ( out, _T("Sorting genre\n") );
        }

        try
        {
            SortGenre ();
        }
        catch ( ... )
        {
            if ( out )
            {
                _ftprintf ( out, _T("***EXCEPTION***\n") );
                _ftprintf ( out, _T("\tAn exception occurred in SortGenre\n") );
                _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
            }
        }        
    }

    _status_bar.SetPaneText ( 0, _T("Sorting artists") );

    if ( out )
    {
        _ftprintf ( out, _T("Sorting artists\n") );
    }

    try
    {
        SortArtist ();
    }
    catch ( ... )
    {
        if ( out )
        {
            _ftprintf ( out, _T("***EXCEPTION***\n") );
            _ftprintf ( out, _T("\tAn exception occurred in SortArtist\n") );
            _ftprintf ( out, _T("\tGetLastError: %lu\n"), ::GetLastError() );
        }
    }

    _status_bar.SetPaneText ( 0, _T("Done") );

    if ( out )
    {
        _ftprintf ( out, _T("Updating the image last update time\n") );
    }

    //  Update the registry with the current time

    ::GetSystemTimeAsFileTime ( & _last_updated );
    SaveLastTime ( _T("LastUpdated"), _last_updated );

    ::KillTimer( _form_hwnd, 1 );
    _status_bar.SetPaneText ( 1, buffer );

    if ( out )
    {
        _ftprintf ( out, _T("\n\nDone!\n") );
        fclose ( out );
    }
}

bool PlaylistGenerator::IsMPEGAudio ( HANDLE file, AudioFile & audio_file )
{
    ID3v2 id3v2;
    ID3v1 id3v1;
    DWORD bytes_read = 0, error;
    bool  id3_tag_found = false;

    try
    {
        //  Look for ID3v2 tag    

        ::SetFilePointer ( file, 0, NULL, FILE_BEGIN );

        if ( !::ReadFile ( file, & id3v2, sizeof(id3v2), & bytes_read, NULL ) || bytes_read != sizeof(id3v2) )
        {
            return false;
        }

        //  Does the file contain an ID3v2 tag?

        if ( id3v2.tag [ 0 ] == 'I' && id3v2.tag [ 1 ] == 'D' && id3v2.tag [ 2 ] == '3' )
        {
            ::SetFilePointer ( file, 0, NULL, FILE_BEGIN );

            error = audio_file.ParseID3v2 ( _main_hwnd, file, id3v2 );

            if ( error != ERROR_SUCCESS )
                return false;

            id3_tag_found = true;
        }

        //  Look for ID3v1 tag

        ::SetFilePointer ( file, -128, NULL, FILE_END );

        if ( !::ReadFile ( file, & id3v1, sizeof(id3v1), & bytes_read, NULL ) || bytes_read != sizeof(id3v1) )
        {
            return false;
        }

        //  Does this file contain an ID3v1 tag?

        if ( id3v1.tag [ 0 ] == 'T' && id3v1.tag [ 1 ] == 'A' && id3v1.tag [ 2 ] == 'G' )
        {
            error = audio_file.ParseID3v1 ( id3v1 );

            if ( error != ERROR_SUCCESS )
                return false;

            id3_tag_found = true;
        }
        else
        if ( id3_tag_found )
        {
            //  Make sure we have a genre

            LPCTSTR genre = audio_file.GetGenre ();
            if ( !genre )
            {
                audio_file.AddGenre ( _T("Unknown") );
            }
        }

        //  Add logic here to determine if we really are an MPEG audio file.  However, for now,
        //  we will just return false if the file doesn't contain any tagged information.

        if ( !id3_tag_found )
        {
            ::SetFilePointer ( file, 0, NULL, FILE_BEGIN );

            error = audio_file.ParseMP3Frames ( file );
            if ( error != ERROR_SUCCESS )
                return false;

            id3_tag_found = true;
        }

        if ( id3_tag_found )
        {
            ::SetFilePointer ( file, 0, NULL, FILE_BEGIN );

            audio_file.CalculateDuration ( file, id3v1, id3v2 );
        }
    }
    catch ( ... )
    {
        //  Must not be MPEG

        id3_tag_found = false;
    }
   
    return id3_tag_found;
}

bool PlaylistGenerator::IsWMAAudio ( HANDLE file, AudioFile & audio_file )
{
    WMAParser parser;
    DWORD error;

    try
    {
        if ( !parser.OpenWMAFile ( file ) )
        {
            return false;
        }

        error = audio_file.ParseWMA ( _main_hwnd, parser );
    
        if ( error == ERROR_SUCCESS )
        {
            return true;
        }
    }
    catch ( ... )
    {
        //  Must not be WMA
    }

    return false;
}

bool PlaylistGenerator::IsfLaCAudio ( HANDLE file, AudioFile & audio_file )
{
    fLaCParser parser;
    DWORD error;

    try
    {
        if ( !parser.OpenfLaCFile ( file ) )
        {
            return false;
        }

        error = audio_file.ParsefLaC ( _main_hwnd, parser );
    
        if ( error == ERROR_SUCCESS )
        {
            return true;
        }
    }
    catch ( ... )
    {
        //  Must not be fLaC
    }

    return false;
}

bool PlaylistGenerator::IsOggAudio ( HANDLE file, AudioFile & audio_file )
{
    OggParser parser;
    DWORD error;

    try
    {
        if ( !parser.OpenOggFile ( file ) )
        {
            return false;
        }

        error = audio_file.ParseOgg ( _main_hwnd, parser );
    
        if ( error == ERROR_SUCCESS )
        {
            return true;
        }
    }
    catch ( ... )
    {
        //  Must not be OGG
    }

    return false;
}

bool PlaylistGenerator::IsMP4Audio ( HANDLE file, AudioFile & audio_file )
{
    MP4Parser parser;
    DWORD error;

    try
    {
        if ( !parser.OpenMP4File ( file ) )
        {
            return false;
        }

        error = audio_file.ParseMP4 ( _main_hwnd, parser );
    
        if ( error == ERROR_SUCCESS )
        {
            return true;
        }
    }
    catch ( ... )
    {
        //  Must not be MPEG-4
    }

    return false;
}

DWORD PlaylistGenerator::PopulateTunesListView ( )
{
    int column_widths [ 7 ];
    ::ZeroMemory ( & column_widths, sizeof(column_widths) );

    HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );

    //  Disable redraw until we have inserted all items

    ::SendMessage ( list_view, WM_SETREDRAW, FALSE, 0 );

    AudioListIterator begin = _audio_list.begin (), end = _audio_list.end ();
    while ( begin != end )
    {
        AddTuneToListView ( & (*begin), (int *) & column_widths );
        begin++;
    }

    //  Enable redraw

    ::SendMessage ( list_view, WM_SETREDRAW, TRUE, 0 );

    return ERROR_SUCCESS;
}

DWORD PlaylistGenerator::PopulateGenreListView ( )
{
    int column_widths [ 3 ];

    ::ZeroMemory ( & column_widths, sizeof(column_widths) );

    HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_GENRES );

    //  Disable redraw until we have inserted all items

    ::SendMessage ( list_view, WM_SETREDRAW, FALSE, 0 );

    //  Get the width of the header text

    GetColumnHeadingWidth ( _form_hwnd, list_view, 2, column_widths );

    TCHAR buffer [ MAX_PATH ];

    LVITEM item;
    ::ZeroMemory ( & item, sizeof( item ) );

    IndexListIterator begin = _genre_list.begin (), end = _genre_list.end();
    while ( begin != end )
    {
        //  Insert the LPARAM

        item.mask       = LVIF_PARAM;
        item.iItem      = ListView_GetItemCount ( list_view );
        item.iSubItem   = 0;
        item.lParam     = (LPARAM) & (*begin);

        int index = ListView_InsertItem ( list_view, & item );
        if ( index == -1 )
        {
            ::DisplaySystemError ( _form_hwnd );
            break;
        }

        ListView_SetCheckState ( list_view, index, begin->_state );

        //  Insert the genre

        item.mask       = LVIF_TEXT;
        item.iItem      = index;
        item.iSubItem   = 1;
        item.pszText    = (TCHAR *)begin->_name.c_str ();

        if ( !ListView_SetItem ( list_view, & item ) )
        {
            ::DisplaySystemError ( _form_hwnd );
            break;
        }

        if ( !begin->_name.empty ( ) )
            UpdateWidth ( list_view, item.pszText, & column_widths [ 1 ] );

        //  Insert the count

        _stprintf ( buffer, _T("%d"), begin->_count );

        item.mask       = LVIF_TEXT;
        item.iItem      = index;
        item.iSubItem   = 2;
        item.pszText    = buffer;

        if ( !ListView_SetItem ( list_view, & item ) )
        {
            ::DisplaySystemError ( _form_hwnd );
            break;
        }

		if ( _tcslen( buffer ) )
            UpdateWidth ( list_view, item.pszText, & column_widths [ 2 ] );

        begin ++;
    }

    //  Update the widths of the columns

    ListView_SetColumnWidth ( list_view, 1, column_widths [ 1 ] );
    ListView_SetColumnWidth ( list_view, 2, column_widths [ 2 ] );

    //  Enable redraw

    ::SendMessage ( list_view, WM_SETREDRAW, TRUE, 0 );

    return ERROR_SUCCESS;
}

void PlaylistGenerator::RepopulateTunesListView ( )
{
    HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );

    //  Disable redraw and set the loading flag

    ::SendMessage ( list_view, WM_SETREDRAW, FALSE, 0 );
    _loading_list_views = true;

    //  Delete the current items (not the most efficient but easier just to walk
    //  the vector and reinsert our items).

    ListView_DeleteAllItems ( list_view );    

    AudioListIterator begin = _audio_list.begin (), end = _audio_list.end ();
    while ( begin != end )
    {
        AddTuneToListView ( & (*begin), NULL );
        begin++;
    }

    //  Make sure to the last column to fill up the width of the control

    ListView_SetColumnWidth ( list_view, 7, LVSCW_AUTOSIZE_USEHEADER );

    _loading_list_views = false;

     //  If there is data in our list view, then we can generate a playlist

    if ( ListView_GetItemCount ( list_view ) )
    {
        //  Do we sort?

        if ( _sort_info [ 0 ]._sort_column != -1 && _sort_info [ 0 ]._sort_mode != SORT_NONE )
        {
            UpdateColumnHeader ( _form_hwnd, IDC_LIST_TUNES, _sort_info [ 0 ]._sort_column, _sort_info [ 0 ]._sort_mode );
            ListView_SortItems ( list_view, CompareItems, (LPARAM) this );
        }
    }

    //  Enable redraw

    ::SendMessage ( list_view, WM_SETREDRAW, TRUE, 0 );
}

void PlaylistGenerator::RepopulateListView ( int ctrl_id, IndexList & list )
{
    //  Update the genre list view

    HWND list_view = ::GetDlgItem ( _form_hwnd, ctrl_id );

    //  Disable redraw and set the loading flag

    ::SendMessage ( list_view, WM_SETREDRAW, FALSE, 0 );
    _loading_list_views = true;

    //  Delete the current items (not the most efficient but easier just to walk
    //  the vector and reinsert our items).

    ListView_DeleteAllItems ( list_view );    

    LVITEM item;
    TCHAR buffer [ MAX_PATH ];

    ::ZeroMemory ( & item, sizeof( item ) );

    IndexListIterator begin = list.begin(), end = list.end();

    for ( ; begin != end; begin++ )
    {
        //  Insert the LPARAM

        item.mask       = LVIF_PARAM;
        item.iItem      = ListView_GetItemCount ( list_view ); 
        item.iSubItem   = 0;
        item.lParam     = (LPARAM) & (*begin);

        int index = ListView_InsertItem ( list_view, & item );
        if ( index == -1 )
        {
            ::DisplaySystemError ( _form_hwnd );
        }

        ListView_SetCheckState ( list_view, index, (*begin)._state );

        //  Insert the genre/artist

        item.mask       = LVIF_TEXT;
        item.iItem      = index;
        item.iSubItem   = 1;
        item.pszText    = (TCHAR *)(*begin)._name.c_str();
        
        if ( !ListView_SetItem ( list_view, & item ) )
        {
            ::DisplaySystemError ( _form_hwnd );
        }

        //  Insert the count

        _stprintf ( buffer, _T("%d"), (*begin)._count );

        item.mask       = LVIF_TEXT;
        item.iItem      = index;
        item.iSubItem   = 2;
        item.pszText    = buffer;

        if ( !ListView_SetItem ( list_view, & item ) )
        {
            ::DisplaySystemError ( _form_hwnd );
        }
    }

    //  Make sure to the last column to fill up the width of the control

    ListView_SetColumnWidth ( list_view, 2, LVSCW_AUTOSIZE_USEHEADER );

    _loading_list_views = false;

    //  Enable redraw

    ::SendMessage ( list_view, WM_SETREDRAW, TRUE, 0 );
}

DWORD PlaylistGenerator::PopulateArtistListView ( )
{
    int column_widths [ 2 ];

    HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_ARTISTS );

    //  Disable redraw until we have inserted all items

    ::SendMessage ( list_view, WM_SETREDRAW, FALSE, 0 );

    //  Get the width of the header text

    GetColumnHeadingWidth ( _form_hwnd, list_view, 2, column_widths );

    TCHAR buffer [ MAX_PATH ];

    LVITEM item;
    ::ZeroMemory ( & item, sizeof( item ) );

    IndexListIterator begin = _artist_list.begin (), end = _artist_list.end();
    while ( begin != end )
    {
        //  Insert the LPARAM

        item.mask       = LVIF_PARAM;
        item.iItem      = ListView_GetItemCount ( list_view ); 
        item.iSubItem   = 0;
        item.lParam     = (LPARAM) & (*begin);

        int index = ListView_InsertItem ( list_view, & item );
        if ( index == -1 )
        {
            ::DisplaySystemError ( _form_hwnd );
        }

        ListView_SetCheckState ( list_view, index, (*begin)._state );

        //  Insert the artist
        
        item.mask       = LVIF_TEXT;
        item.iItem      = index;
        item.iSubItem   = 1;
        item.pszText    = (TCHAR *)begin->_name.c_str ();
        
        if ( !ListView_SetItem ( list_view, & item ) )
        {
            ::DisplaySystemError ( _form_hwnd );
            break;
        }

        if ( !begin->_name.empty ( ) )
            UpdateWidth ( list_view, item.pszText, & column_widths [ 1 ] );

        //  Insert the count

        _stprintf ( buffer, _T("%d"), begin->_count );

        item.mask       = LVIF_TEXT;
        item.iItem      = index;
        item.iSubItem   = 2;
        item.pszText    = buffer;

        if ( !ListView_SetItem ( list_view, & item ) )
        {
            ::DisplaySystemError ( _form_hwnd );
            break;
        }

		if ( _tcslen( buffer ) )
            UpdateWidth ( list_view, item.pszText, & column_widths [ 2 ] );

        begin ++;
    }

    //  Update the widths of the columns

    ListView_SetColumnWidth ( list_view, 1, column_widths [ 1 ] );
    ListView_SetColumnWidth ( list_view, 2, column_widths [ 2 ] );

    //  Enable redraw

    ::SendMessage ( list_view, WM_SETREDRAW, TRUE, 0 );

    return ERROR_SUCCESS;
}

DWORD PlaylistGenerator::AddMusicFile ( LPCTSTR file_name )
{
    BasicString song = SongTitleFromFilename ( BasicString (file_name) );

    _status_bar.SetPaneText ( 3, (TCHAR *) song.c_str() );

    HANDLE audio_file = ::CreateFile ( file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, OPEN_EXISTING, NULL );

    bool file_added = false;

    if ( audio_file != INVALID_HANDLE_VALUE )
    {
        AudioFile library_entry ( file_name, ::GetFileSize ( audio_file, NULL ) );

        /*if ( IsfLaCAudio ( audio_file, library_entry )  ||
             IsOggAudio  ( audio_file, library_entry )  ||
             IsMP4Audio  ( audio_file, library_entry )  ||
             IsMPEGAudio ( audio_file, library_entry )  ||
             IsWMAAudio  ( audio_file, library_entry ) )*/

        if ( IsMPEGAudio ( audio_file, library_entry )  ||
             IsMP4Audio  ( audio_file, library_entry )  ||
             IsWMAAudio  ( audio_file, library_entry )  ||
             IsOggAudio  ( audio_file, library_entry )  ||
             IsfLaCAudio ( audio_file, library_entry ) )
        {
            LPCTSTR genre_type = library_entry.GetGenre ();
            if ( genre_type )
            {
                AudioIndex genre ( genre_type );

                IndexListIterator iter = std::find ( _genre_list.begin (), _genre_list.end (), genre );
                if ( iter != _genre_list.end () )
                {
                    (*iter)._count ++;
                }
                else
                {
                    if ( _aggregate == BST_CHECKED )
                    {
                        //  We must test for the aggregate list as well

                        iter = std::find ( _aggregate_list.begin (), _aggregate_list.end (), genre );

                        if ( iter != _aggregate_list.end () )
                        {
                            //  Update the count of the genre in the aggregate list

                            (*iter)._count ++;

                            //  Did we grow enough to get off the list?

                            if ( (*iter)._count > _aggregate_size )
                            {
                                //  Add the genre back to the genre list and remove it from the aggregate list

                                _genre_list.push_back ( (*iter) );
                                _aggregate_list.remove ( (*iter) );
                            }
                            else
                            {
                                //  Update the aggregate count

                                iter = std::find ( _genre_list.begin (), _genre_list.end (), AudioIndex ( _default_aggregate_name ) );
                                if ( iter != _genre_list.end () )
                                {
                                    (*iter)._count ++;
                                }
                            }
                        }
                        else
                        {
                            //  Not it the aggregate list so we need to add it there

                            genre._count ++;
                            _aggregate_list.push_back ( genre );

                            //  Update the aggregate count

                            iter = std::find ( _genre_list.begin (), _genre_list.end (), AudioIndex ( _default_aggregate_name ) );
                            if ( iter != _genre_list.end () )
                            {
                                (*iter)._count ++;
                            }
                        }
                    }
                    else
                    {
                        genre._count ++;
                        _genre_list.push_back ( genre );
                    }
                }
            }

            LPCTSTR artist_name = library_entry.GetArtist ();
            if ( artist_name )
            {
                AudioIndex artist ( artist_name );

                IndexListIterator iter = std::find ( _artist_list.begin (), _artist_list.end (), artist );
                if ( iter != _artist_list.end () )
                {
                    (*iter)._count ++;
                }
                else
                {
                    artist._count ++;
                    _artist_list.push_back ( artist );
                }
            }

            _audio_list.push_back ( library_entry );
            file_added = true;

        } 

        ::CloseHandle ( audio_file );
    }

    if ( file_added )
    {
        AudioFile & audio_file = _audio_list.back ();
        AddTuneToListView ( & audio_file );
    }

    return ERROR_SUCCESS;
}

DWORD PlaylistGenerator::AddGenreToListView ( HWND hWnd, TCHAR * genre )
{
    LVITEM  item;
    ::ZeroMemory ( & item, sizeof( item ) );

    HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );

    //  Add genre to the list view    

    AudioListIterator begin = _audio_list.begin (), end = _audio_list.end ();
    while ( begin != end )
    {
        bool add_tune = false;
        TCHAR * tune_genre = (TCHAR *) begin->GetGenre();

        //  Can we match the tunes genre outright?

        if ( _tcsicmp ( genre, tune_genre ) == 0 )
        {
            add_tune = true;
        }

        //  If we didn't match outright, then if 'aggregating' try the to match the genre in the aggregate list

        if ( !add_tune && _aggregate == BST_CHECKED && _tcscmp ( genre, _default_aggregate_name ) == 0 )
        {
            IndexListIterator iter = std::find ( _aggregate_list.begin(), _aggregate_list.end(), AudioIndex ( tune_genre ) );
            if ( iter != _aggregate_list.end() )
            {
                add_tune = true;
                tune_genre = _default_aggregate_name;
            }
        }

        if ( add_tune )
        {
            //  Insert the LPARAM

            item.mask       = LVIF_PARAM;
            item.iItem      = ListView_GetItemCount ( list_view );
            item.iSubItem   = 0;
            item.lParam     = (LPARAM) & * begin;

            int index = ListView_InsertItem ( list_view, & item );
            if ( index == -1 )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the title

            LPTSTR text = (TCHAR *)begin->GetTitle ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 1;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the artist

            text = (TCHAR *)begin->GetArtist ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 2;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the album

            text = (TCHAR *)begin->GetAlbum ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 3;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the year

            text = (TCHAR *)begin->GetYear ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 4;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the track

            text = (TCHAR *)begin->GetTrack ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 5;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the genre

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 6;
            item.pszText    = tune_genre;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the file type

            text = (TCHAR *)begin->GetFileType ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 7;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( _main_hwnd );
            }
        }

        begin ++;
    }

    return ERROR_SUCCESS;
}

DWORD PlaylistGenerator::RemoveGenreFromListView ( HWND hWnd, TCHAR * genre )
{
    HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );

    LVITEM item;
    ::ZeroMemory ( & item, sizeof( item ) );

    TCHAR buffer [ MAX_PATH ];

    item.mask       = LVIF_TEXT | LVIF_PARAM;
    item.iSubItem   = 6;
    item.pszText    = buffer;
    item.cchTextMax = MAX_PATH;

    ::SendMessage ( list_view, WM_SETREDRAW, FALSE, 0 );

    for ( int j = ListView_GetItemCount ( list_view ); j > 0; j-- )
    {
        item.iItem = j - 1;

        ListView_GetItem ( list_view, & item );

        if ( _tcsicmp ( genre, buffer ) == 0 )
        {
            ListView_DeleteItem ( list_view, j - 1);
            
            _selected_list.remove ( static_cast<AudioFile *>( (void*) item.lParam ) );
        }
    }

    ::SendMessage ( list_view, WM_SETREDRAW, TRUE, 0 );

    return ERROR_SUCCESS;
}

DWORD PlaylistGenerator::AddArtistToListView ( HWND hWnd, TCHAR * artist )
{
    LVITEM  item;
    ::ZeroMemory ( & item, sizeof( item ) );

    HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );

    //  Add artist to the list view

    AudioListIterator begin = _audio_list.begin (), end = _audio_list.end ();
    while ( begin != end )
    {
        LPCTSTR tune_artist = (*begin).GetArtist();
        if ( tune_artist && _tcsicmp ( artist, tune_artist ) == 0 )
        {
            //  Insert the LPARAM

            item.mask       = LVIF_PARAM;
            item.iItem      = ListView_GetItemCount ( list_view );
            item.iSubItem   = 0;
            item.lParam     = (LPARAM) & * begin;

            int index = ListView_InsertItem ( list_view, & item );
            if ( index == -1 )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the title

            LPTSTR text = (TCHAR *)begin->GetTitle ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 1;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the artist

            text = (TCHAR *)begin->GetArtist ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 2;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the album

            text = (TCHAR *)begin->GetAlbum ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 3;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the year

            text = (TCHAR *)begin->GetYear ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 4;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the track

            text = (TCHAR *)begin->GetTrack ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 5;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the genre

            text = (TCHAR *)begin->GetGenre ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 6;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( hWnd );
            }

            //  Insert the file type

            text = (TCHAR *)begin->GetFileType ();

            item.mask       = LVIF_TEXT;
            item.iItem      = index;
            item.iSubItem   = 7;
            item.pszText    = text;

            if ( !ListView_SetItem ( list_view, & item ) )
            {
                return ::DisplaySystemError ( _main_hwnd );
            }
        }

        begin ++;
    }

    return ERROR_SUCCESS;
}

DWORD PlaylistGenerator::RemoveArtistFromListView ( HWND hWnd, TCHAR * artist )
{
    HWND list_view = ::GetDlgItem ( hWnd, IDC_LIST_TUNES );

    LVITEM item;
    ::ZeroMemory ( & item, sizeof( item ) );

    TCHAR buffer [ MAX_PATH ];

    item.mask       = LVIF_TEXT | LVIF_PARAM;
    item.iSubItem   = 2;
    item.pszText    = buffer;
    item.cchTextMax = MAX_PATH;

    ::SendMessage ( list_view, WM_SETREDRAW, FALSE, 0 );

    for ( int j = ListView_GetItemCount ( list_view ); j > 0; j-- )
    {
        item.iItem = j - 1;

        ListView_GetItem ( list_view, & item );

        if ( _tcsicmp ( artist, buffer ) == 0 )
        {
            ListView_DeleteItem ( list_view, j - 1);

            _selected_list.remove ( static_cast<AudioFile *>( (void*) item.lParam ) );
        }
    }

    ::SendMessage ( list_view, WM_SETREDRAW, TRUE, 0 );

    return ERROR_SUCCESS;
}

//DWORD PlaylistGenerator::GeneratePlaylist ( HWND hWnd, TCHAR * playlist )
DWORD PlaylistGenerator::GeneratePlaylist ( )
{
    TCHAR directory [ MAX_PATH ], file_name [ MAX_PATH ];
    TCHAR big_buffer [ 4096 ], buffer [ 512 ];

    PlaylistNameData data;

    _tcscpy ( data.playlist_name, _default_playlist_name );

    if ( _prompt_for_playlist == BST_CHECKED )
    {                              
        if ( IDCANCEL == ::DialogBoxParam ( _instance, MAKEINTRESOURCE(IDD_PLAYLIST_NAME), _form_hwnd, PlaylistName_DlgProc, (LPARAM) & data ) )
        {
            return ERROR_SUCCESS;
        }
    }

    //  If the user has specified a directory

    _tcscpy ( directory, _playlist_directory );

    unsigned length = _tcslen ( directory );
    if ( length )
    {
        if ( !PathIsDirectory ( directory ) )
        {
			TCHAR buffer [ MAX_PATH ];

			_stprintf ( buffer, _T("The specified path '%s' does not exist.\n\nWould you like to create it?"), directory );

			if ( IDNO == ::MessageBox ( _form_hwnd, buffer, _T("Playlist Creator"), MB_ICONQUESTION | MB_YESNO ) )
			{
				return ERROR_SUCCESS;
			}

			if ( !::CreateDirectory ( directory, NULL ) )
			{
				return ::DisplaySystemError ( _form_hwnd );
			}
        }
    }

    //  Use the path to this executable

    else
    {
        ::GetModuleFileName ( _instance, directory, MAX_PATH );
        ::PathRemoveFileSpec ( directory );
    }

    _tcscpy ( file_name, directory );

    length = _tcslen ( file_name );
    if ( file_name [ length - 1 ] != '\\' )
    {
        _tcscat ( file_name, _T("\\") );
    }

    _tcscat ( file_name, data.playlist_name );

    //  If playlist exists, ask to over write

    HANDLE exists = ::CreateFile ( file_name, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( exists != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle ( exists );

        _stprintf ( buffer, _T("The playlist file '%s' already exists.\n\nWould you like to overwrite this file?"), file_name );
        if ( IDNO == ::MessageBox ( _form_hwnd, buffer, _T("Playlist Creator"), MB_ICONQUESTION | MB_YESNO ) )
        {
            return ERROR_SUCCESS;
        }
    }

    HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );
    int list_view_count = ListView_GetItemCount ( list_view );

    /*
    //  Do we have enough elements in the list view to generate the playlist without duplicates?

    if ( _allow_duplicates != BST_CHECKED )
    {
        TCHAR buffer [ 512 ];
        bool display_message = false;

        switch ( _playlist_mode )
        {
            case IDC_RADIO_PLAYLIST_BY_NUMBER:

                {                
                    if ( _max_tunes > list_view_count )
                    {
                        _stprintf (  buffer,
                                    _T("The number of songs available for inclusion in the playlist (%d)\n")        \
                                    _T("is less than the number of tunes requested for the playlist (%d).\n\n")     \
                                    _T("Would you like to allow duplicate entries in the playlist to satisfy the request?"),
                                    list_view_count,
                                    _max_tunes );

                        display_message = true;
                    }
                }
                break;

            case IDC_RADIO_PLAYLIST_BY_SIZE:

                {
                    if ( _max_size > _total_tune_size )
                    {
                        _stprintf (  buffer,
                                    _T("The total size (in bytes) available for inclusion in the playlist (%I64d)\n")        \
                                    _T("is less than the total size (in bytes) requested for the playlist (%I64d).\n\n")     \
                                    _T("Would you like to allow duplicate entries in the playlist to satisfy the request?"),
                                    _total_tune_size,
                                    _max_size );

                        display_message = true;
                    }
                }
                break;

            case IDC_RADIO_PLAYLIST_BY_DURATION:

                {
                    if ( _max_duration > _total_tune_duration )
                    {
                        _stprintf (  buffer,
                                    _T("The total duration (in seconds) available for inclusion in the playlist (%I64d)\n")        \
                                    _T("is less than the total duration (in seconds) requested for the playlist (%I64d).\n\n")     \
                                    _T("Would you like to allow duplicate entries in the playlist to satisfy the request?"),
                                    _total_tune_duration,
                                    _max_duration );

                        display_message = true;
                    }

                }

                break;
        }

        if ( display_message )
        {
            if ( IDYES == ::MessageBox ( hWnd, buffer, _T("Playlist Creator"), MB_ICONQUESTION | MB_YESNO ) )
            {
                _allow_duplicates = BST_CHECKED;
             }
        }
    }
    */

    //  Go ahead and pre-allocate storage equal to the size of the tunes list view.
    //  We use a vector for the random and playlist because we can use the random/
    //  access operator, [ ], to get to the element in constant time.

    AudioPtrVector random_vector, playlist_vector;
    AudioPtrVectorIterator v_begin, v_end;
    AudioPtrListIterator iter, begin, end;

    ArtistVector artist_vector;

    random_vector.reserve ( list_view_count );
    playlist_vector.reserve ( list_view_count );

    bool done = false;

    unsigned tunes      = 0;
    __int64  size       = 0;
    __int64  duration   = 0;

    //  Has the user selected certains tunes from their catalog?   

    if ( !_selected_list.empty () )
    {
        begin = _selected_list.begin(), end = _selected_list.end ();

        for ( ; begin != end; begin ++ )
        {
            if ( _cancel )
                break;

            playlist_vector.push_back ( (*begin) );

            //  Update our counters

            size += (*begin)->_fs;
            tunes ++;
            duration += (*begin)->_duration;

            //  Test for completion

            if ( ( _playlist_mode == IDC_RADIO_PLAYLIST_BY_SIZE && size > _max_size )       ||
                 ( _playlist_mode == IDC_RADIO_PLAYLIST_BY_NUMBER && tunes == _max_tunes )  ||
                 ( _playlist_mode == IDC_RADIO_PLAYLIST_BY_DURATION && duration > _max_duration ) )
            {
                done = true;
                break;
            }
        }
    }

    //  Add the rest of the tunes that are in the tunes list view (do not duplicate those in the first x positions
    //  from the selected list

    LVITEM item;
    ::ZeroMemory ( & item, sizeof(item) );

    item.mask = LVIF_PARAM;

    begin = _selected_list.begin(), end = _selected_list.end ();

    for ( int i = 0; i < list_view_count; i++ )
    {
        if ( _cancel )
            break;

        item.iItem = i;

        if ( ! ListView_GetItem ( list_view, & item ) )
        {
            ::DisplaySystemError ( _form_hwnd );
            break;
        }

        AudioFile * audio_file = static_cast<AudioFile *>( (void *) item.lParam );

        //  If the repeat limiting is enabled, then use only those songs whose repeat count is zero
        
        if ( _duplicate_mode == LIMIT_REPEATS && audio_file->_repeat_count != 0 )
            continue;

        //  If this tune wasn't manually selected, then add it to our randon vector

        AudioPtrListIterator iter = std::find ( begin, end, audio_file );
        if ( iter == end )
        {
            //  If user is limiting number of songs by same artist

            if ( _limit_artist && _limit_artist_count != 0 )
            {
                //  Convert artist name to lower case

                LPCTSTR song_artist = audio_file->GetArtist();
                if ( song_artist )
                {
                    ArtistCount artist ( song_artist );
                    std::transform ( artist._artist.begin(), artist._artist.end(), artist._artist.begin(), tolower );

                    //  If the title is missing or "unknown", then we won't check

                    if ( artist._artist != _T("unknown") )
                    {
                        ArtistVectorIterator iter = std::find ( artist_vector.begin(), artist_vector.end(), artist );
                        if ( iter == artist_vector.end() )
                        {
                            artist_vector.push_back ( artist );
                        }
                        else
                        {
                            if ( iter->_count == _limit_artist_count ) 
                                continue;

                            iter->Increment();
                        }
                    }
                }
            }

            random_vector.push_back ( audio_file );
        }

        /*
        //  If we are allowing duplicate entries, then we shouldn't exclude the selected tunes
        else
        if ( _allow_duplicates == BST_CHECKED )
        {
            random_vector.push_back ( audio_file );
        }
        else
        {
            AudioPtrListIterator iter = std::find ( begin, end, audio_file );
            if ( iter == end )
            {
                random_vector.push_back ( audio_file );
            }
        }*/
    }

    //  Seed the random number generator and set our starting modulus.  The modulus will get updated
    //  with each iteration is we ARE NOT allowing duplicates because the vector size will change.

    srand( ::GetTickCount () );
    
    //  Build the remaining playlist from the tunes in the random vector

    while ( !done && !random_vector.empty () )
    {
        if ( _cancel )
            break;

        //  Randomly select an item from the vector

        unsigned modulus = rand ( ) % random_vector.size ();

        AudioFile * audio_file = random_vector [ modulus ];

        playlist_vector.push_back ( audio_file );

        //  Update our counters

        size += audio_file->_fs;
        tunes ++;
        duration += audio_file->_duration;

        //  Test for completion

        if ( ( _playlist_mode == IDC_RADIO_PLAYLIST_BY_SIZE && size > _max_size )       ||
             ( _playlist_mode == IDC_RADIO_PLAYLIST_BY_NUMBER && tunes == _max_tunes )  ||
             ( _playlist_mode == IDC_RADIO_PLAYLIST_BY_DURATION && duration > _max_duration ) )
        {
            done = true;
            break;
        }

        //  If the repeat limiting is enabled, then we need to update the repeat count for those tunes we use

        if ( _duplicate_mode == LIMIT_REPEATS )
        {
            audio_file->_repeat_count = _repeat_count + 1;

            //  Remove this tune from our random vector after using it

            v_begin = random_vector.begin ();
            v_begin += modulus;
            random_vector.erase ( v_begin );
        }

        /*

        //  If not allowing duplicates, then we need to remove this element from the random vector

        else
        if ( _allow_duplicates != BST_CHECKED )
        {
            v_begin = random_vector.begin ();
            v_begin += modulus;
            random_vector.erase ( v_begin );
        }*/
    }

    //  If the repeat limiting is enabled, then update the repeat counts
    
    if (_duplicate_mode == LIMIT_REPEATS )
    {
        //  All songs should have thir repeat count decremented ( except those already at zero )        

        std::for_each ( _audio_list.begin(), _audio_list.end(), DecrementRepeatCount );
    }

    //  Finally we can generate the playlist

    FILE * file = _tfopen ( file_name, _T("wt") );
    if ( !file )
    {
        return ::DisplaySystemError ( _form_hwnd );
    }

    _fputts ( _T("#EXTM3U\n"), file );

	//	Save the current parts

    int width, parts [ 4 ];
    int count = 0, total = playlist_vector.size ();

    if ( _copy_tunes == BST_CHECKED )
    {
        _status_bar.SetPaneText ( 0, _T("Preparing to create playlist and copy files") );

        //  No tunes or genres or artists

        _status_bar.SetPaneText ( 1, _T("") );
        _status_bar.SetPaneText ( 2, _T("") );
        _status_bar.SetPaneText ( 3, _T("") );

       _status_bar.Lock ();

        width = _status_bar.GetBarWidth ( );

        parts [ 0 ] = width - 420;
        parts [ 1 ] = width - 400;
        parts [ 2 ] = width - 200;
        parts [ 3 ] = -1;

        _status_bar.CreateProgressBar ( 4, parts, total );

        _status_bar.Unlock ();
    }

    //  If randomizing the selected tunes, then we will randomly choose values from the playlist

    if ( _randomize_manual_selections == BST_CHECKED )
    {
        while ( !playlist_vector.empty () )
        {
            if ( _cancel )
                break;

            //  Randomly' select an item from the playlist

            unsigned modulus = rand ( ) % playlist_vector.size ();

            AudioFile * audio_file = playlist_vector [ modulus ];

            if ( AddSongToPlaylist ( audio_file, file, directory ) != ERROR_SUCCESS )
            {
                break;
            }
                
            //  Delete the item from the playlist vector

            v_begin = playlist_vector.begin ();
            v_begin += modulus;
            playlist_vector.erase ( v_begin );

            if ( _copy_tunes == BST_CHECKED )
            {
                TCHAR buffer [ MAX_PATH ];

                _stprintf ( buffer, _T("Processed %lu / %lu"), ++count, total );
                _status_bar.SetPaneText ( 3, buffer );

                _status_bar.UpdateProgressBar ( count );
            }
        }
    }
    else
    {
        for ( v_begin = playlist_vector.begin (), v_end = playlist_vector.end (); v_begin != v_end; v_begin++ )
        {
            if ( _cancel )
                break;

            if ( AddSongToPlaylist ( *v_begin, file, directory ) != ERROR_SUCCESS )
            {
                break;
            }

            if ( _copy_tunes == BST_CHECKED )
            {
                TCHAR buffer [ MAX_PATH ];

                _stprintf ( buffer, _T("Processed %lu / %lu"), ++count, total );
                _status_bar.SetPaneText ( 3, buffer );

                _status_bar.UpdateProgressBar ( count );
            }
        }
    }

    if ( _copy_tunes == BST_CHECKED )
    {
        _status_bar.Lock ();

        width = _status_bar.GetBarWidth ( );

        parts [ 0 ] = width - 525;
        parts [ 1 ] = width - 350;
        parts [ 2 ] = width - 175;
        parts [ 3 ] = -1;

        _status_bar.DestroyProgressBar ( 4, parts );

        _status_bar.Unlock ();

        _status_bar.SetPaneText ( 0, _T("Ready") );
        _status_bar.SetPaneText ( 3, _T("") );

        ::PostMessage ( _form_hwnd, WM_UPDATETUNECOUNT, 0, 0 );
    }

    //  If we are not yet done

    fclose ( file );

    //  Update the _T("Creation Info")    

    _stprintf ( buffer, _T("Last Playlist Created: %s\r\n\n"), data.playlist_name );
    _tcscpy ( big_buffer, buffer );

    _stprintf ( buffer, _T("Playlist Contained:\r\n") );
    _tcscat ( big_buffer, buffer );

    _stprintf ( buffer, _T("%d Songs\r\n"), tunes );
    _tcscat ( big_buffer, buffer );

    if ( size >= 1073741824 )
    {
        float temp_size = (float) size / 1073741824.0f;
        _stprintf ( buffer, _T("%0.1f (GBytes) Total Size\r\n"), temp_size );
    }
    else
    if ( size >= 1048576 )
    {
        float temp_size = (float) size / 1048576.0f;
        _stprintf ( buffer, _T("%0.1f (MBytes) Total Size\r\n"), temp_size );
    }
    else
    if ( size >= 102400 )
    {
        float temp_size = (float) size;
        _stprintf ( buffer, _T("%0.1f (KBytes) Total Size\r\n"), temp_size );
    }
    else
    {
        _stprintf ( buffer, _T("%I64d (Bytes) Total Size\r\n"), size );
    }

    _tcscat ( big_buffer, buffer );

    int hours   = duration / 3600;
    int minutes = (duration - (hours * 3600)) / 60;
    int seconds = (duration - hours * 3600 - minutes * 60);

    _stprintf ( buffer, _T("Duration %0.2d:%0.2d:%0.2d\r\n\n"), hours, minutes, seconds );
    _tcscat ( big_buffer, buffer );

    ::SetDlgItemText ( _form_hwnd, IDC_STATIC_CREATION_INFO, big_buffer );

    return ERROR_SUCCESS;
}

void PlaylistGenerator::AddSelected    ( AudioFile * audio_file )
{
    _selected_list.push_back ( audio_file );
}

void PlaylistGenerator::RemoveSelected   ( AudioFile * audio_file )
{
    _selected_list.remove ( audio_file );
}

void PlaylistGenerator::DeleteTune ( LVITEM & item )
{
    _selected_list.remove ( static_cast<AudioFile*>( (void *) item.lParam ) );

    AudioFile audio_file = * (AudioFile *) item.lParam;

    AudioListIterator iter = std::find ( _audio_list.begin (), _audio_list.end(), audio_file );
    if ( iter != _audio_list.end() )
    {
        //  Keep the genre list in sync with the audio file list

        bool success = DeleteAudioIndex ( _genre_list, (*iter).GetGenre (), IDC_LIST_GENRES );

        //  Then try the aggregated list

        if ( !success )
        {
            IndexListIterator v_iter = std::find ( _aggregate_list.begin (), _aggregate_list.end(), AudioIndex ( (*iter).GetGenre () ) );
            if ( v_iter != _aggregate_list.end() )
            {
                //  This genre has been aggregated so update the 'aggregated' member in the GENRE list view

                success = DeleteAudioIndex ( _genre_list, _default_aggregate_name, IDC_LIST_GENRES );

                if ( success )
                {
                    (* v_iter)._count--;
                    if ( !(* v_iter)._count )
                    {
                        _aggregate_list.erase ( v_iter );
                    }
                }
            }
        }

        //  Keep the artist list in sync with the audio file list

        LPCTSTR artist_name = (*iter).GetArtist ();
        if ( artist_name )
        {
            DeleteAudioIndex ( _artist_list, artist_name, IDC_LIST_ARTISTS );
        }

        /*

        AudioIndex genre_index( (*iter).GetGenre () );

        IndexListIterator v_iter = std::find ( _genre_list.begin (), _genre_list.end(), genre_index );
        if ( v_iter != _genre_list.end() )
        {
            LVFINDINFO item;
            ::ZeroMemory ( & item, sizeof( item ) );

            item.flags  = LVFI_PARAM;
            item.lParam = (LPARAM) & (*v_iter);

            HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_GENRES );

            int index = ListView_FindItem ( list_view, -1, & item );

            (* v_iter)._count--;

            if ( !(* v_iter)._count )
            {
                _genre_list.erase ( v_iter );

                if ( index != -1 )
                {
                    ListView_DeleteItem ( list_view, index );
                }
            }
            else
            {
                if ( index != -1 )
                {
                    TCHAR buffer [ MAX_PATH ];
                    _stprintf ( buffer, _T("%d"), (* v_iter)._count );
                    ListView_SetItemText ( list_view, index, 1, buffer );
                }
            }
        }

        //  else this must be aggregated genre

        else
        {
            IndexListIterator v_iter = std::find ( _aggregate_list.begin (), _aggregate_list.end(), genre_index );
            if ( v_iter != _aggregate_list.end() )
            {
                LVFINDINFO item;
                ::ZeroMemory ( & item, sizeof( item ) );

                item.flags  = LVFI_PARAM;
                item.lParam = (LPARAM) & (*v_iter);

                HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_GENRES );

                int index = ListView_FindItem ( list_view, -1, & item );

                (* v_iter)._count--;

                if ( !(* v_iter)._count )
                {
                    _aggregate_list.erase ( v_iter );

                    if ( index != -1 )
                    {
                        ListView_DeleteItem ( list_view, index );
                    }
                }
                else
                {
                    if ( index != -1 )
                    {
                        TCHAR buffer [ MAX_PATH ];
                        _stprintf ( buffer, _T("%d"), (* v_iter)._count );
                        ListView_SetItemText ( list_view, index, 1, buffer );
                    }
                }
            }
        }

        //  Keep the artist list in sync with the audio file list

        LPCTSTR artist_name = (*iter).GetArtist ();
        if ( artist_name )
        {
            v_iter = std::find ( _artist_list.begin (), _artist_list.end(), AudioIndex ( artist_name ) );
            if ( v_iter != _artist_list.end() )
            {
                LVFINDINFO item;
                ::ZeroMemory ( & item, sizeof( item ) );

                item.flags  = LVFI_PARAM;
                item.lParam = (LPARAM) & (*v_iter);

                HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_ARTISTS );

                int index = ListView_FindItem ( list_view, -1, & item );

                (* v_iter)._count--;

                if ( !(* v_iter)._count )
                {
                    _artist_list.erase ( v_iter );

                    if ( index != -1 )
                    {
                        ListView_DeleteItem ( list_view, index );
                    }
                }
                else
                {
                    if ( index != -1 )
                    {
                        TCHAR buffer [ MAX_PATH ];
                        _stprintf ( buffer, _T("%d"), (* v_iter)._count );
                        ListView_SetItemText ( list_view, index, 1, buffer );
                    }
                }
            }
        }
        */

        _total_tune_size        -= (*iter)._fs;
        _total_tune_duration    -= (*iter)._duration;

        _audio_list.erase ( iter );        
    }
}

void PlaylistGenerator::SortGenre ( )
{
    SortIndex ( _genre_list );

    RepopulateListView ( IDC_LIST_GENRES, _genre_list );
}

void PlaylistGenerator::SortArtist ( )
{
    SortIndex ( _artist_list );

    RepopulateListView ( IDC_LIST_ARTISTS, _artist_list );
}

void PlaylistGenerator::EnableAggregation ( unsigned state, int size, TCHAR * name )
{
    //  If aggregation is selected, we have to update both the tunes list and the genre list

    if ( state == BST_CHECKED )
    {
        //  Build the aggregate list from the genre list (default is for this to be checked)

        AudioIndex aggregate;
        
        aggregate._name.assign ( name );
        aggregate._count = 0;
        aggregate._state = 1;

        _aggregate_list.clear ();
        
        IndexListIterator begin = _genre_list.begin (), end = _genre_list.end ();

        while ( begin != end )
        {
            if ( (*begin)._count <= size )
            {
                _RPT2 ( _CRT_WARN, "Aggregating the genre %s\tCount %d\r\n", (*begin)._name.c_str(), (*begin)._count );

                _aggregate_list.push_back ( (*begin) );
                aggregate._count += (*begin)._count;

                //  If we erase this member from our list, the return value is the next element

                begin = _genre_list.erase ( begin );

                continue;
            }

            begin ++;
        }

        //  Insert the aggregate index

        if ( aggregate._count != 0 )
        {
            _genre_list.push_back ( aggregate );
        }

        //  Resort the genre and update the list view

        SortGenre ();

        //  Update the tunes list view

        HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );

        LVITEM item;
        TCHAR buffer [ MAX_PATH ];

        ::ZeroMemory ( & item, sizeof( item ) );

        item.mask       = LVIF_TEXT | LVIF_PARAM;
        item.pszText    = buffer;
        item.cchTextMax = MAX_PATH;

        int count = ListView_GetItemCount ( list_view );

        for ( int i = 0; i < count; i++ )
        {
            ListView_GetItemText ( list_view, i, 6, buffer, MAX_PATH );

            IndexListIterator iter = std::find ( _aggregate_list.begin (), _aggregate_list.end(), AudioIndex (buffer) );
            if ( iter != _aggregate_list.end() )
            {
                ListView_SetItemText ( list_view, i, 6, name );
            }
        }
    }
    else
    {
        //  Remove the 'aggregate' genre and replace with the actual genre

        _RPT2 ( _CRT_WARN, "Before removing aggregated genre '%s' %d\r\n", _default_aggregate_name, _genre_list.size () );

        _genre_list.remove ( AudioIndex ( _default_aggregate_name ) );

        _RPT1 ( _CRT_WARN, "Before splicing aggregated genre %d \r\n", _genre_list.size () );

        _genre_list.splice ( _genre_list.end (), _aggregate_list );

        _RPT1 ( _CRT_WARN, "After splicing aggregated genre %d \r\n", _genre_list.size () );

        //  Resort the genre and update the list view

        SortGenre ();

        //  Finally, we must update the tunes list view

        HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );

        LVITEM item;
        TCHAR buffer [ MAX_PATH ];

        ::ZeroMemory ( & item, sizeof( item ) );

        item.mask       = LVIF_TEXT | LVIF_PARAM;
        item.iSubItem   = 6;
        item.pszText    = buffer;
        item.cchTextMax = MAX_PATH;

        int count = ListView_GetItemCount ( list_view );

        for ( int i = 0; i < count; i++ )
        {
            item.iItem   = i;

            if ( !ListView_GetItem ( list_view, & item ) )
            {
                ::DisplaySystemError ( _form_hwnd );
            }

            if ( _tcsicmp ( buffer, _default_aggregate_name ) == 0 )
            {
                AudioFile * audio_file = static_cast<AudioFile *>( (void*) item.lParam );
                if ( audio_file )
                {
                    ListView_SetItemText ( list_view, i, 6, (TCHAR *) audio_file->GetGenre() );
                }
            }
        }
    }

    //  Save the new aggregate values
    
    _aggregate      = state;
    _aggregate_size = size;
    _tcscpy ( _default_aggregate_name, name );
}

void PlaylistGenerator::SetAggregateSize  ( unsigned size )
{
    //  If aggregation is selected, we have to update both the tunes list and the genre list

    if ( _aggregate == BST_CHECKED )
    {
        if ( size > _aggregate_size )
        {
            AudioIndex aggregate ( _default_aggregate_name );

            //  If we are increasing the size of the aggregated

            IndexListIterator begin = _genre_list.begin(), end = _genre_list.end();

            IndexListIterator aggregate_iter = std::find ( begin, end, AudioIndex ( _default_aggregate_name ) );
            if ( aggregate_iter != end )
            {
                aggregate._count = (*aggregate_iter)._count;
                aggregate._state = (*aggregate_iter)._state;
            }

            IndexList aggregate_add;
           
            while ( begin != end )
            {
                if ( (*begin)._count <= size && begin != aggregate_iter )
                {
                    _RPT2 ( _CRT_WARN, "Aggregating the genre %s\tCount %d\r\n", (*begin)._name.c_str(), (*begin)._count );

                    aggregate_add.push_back ( (*begin) );
                    aggregate._count += (*begin)._count;

                    //  If we erase this member from our list, the return value is the next element

                    begin = _genre_list.erase ( begin );

                    continue;
                }

                begin ++;
            }

            if ( aggregate_add.empty () )
            {
                _RPT1 ( _CRT_WARN, "No new elements to aggregate\tSetting new aggregate size %d\n", size );

                _aggregate_size = size; 
                return;
            }

            if ( aggregate_iter != end )
            {
                _genre_list.erase ( aggregate_iter );
            }

            //  Insert the aggregate index and resort

            if ( aggregate._count != 0 )
            {
                _genre_list.push_back ( aggregate );
            }

            //  Resort the genre and update the list view

            SortGenre ();

            //  Update the tunes list view

            HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );

            LVITEM item;
            TCHAR buffer [ MAX_PATH ];

            ::ZeroMemory ( & item, sizeof( item ) );

            item.mask       = LVIF_TEXT | LVIF_PARAM;
            item.pszText    = buffer;
            item.cchTextMax = MAX_PATH;

            int count = ListView_GetItemCount ( list_view );

            for ( int i = 0; i < count; i++ )
            {
                ListView_GetItemText ( list_view, i, 6, buffer, MAX_PATH );

                IndexListIterator iter = std::find ( aggregate_add.begin (), aggregate_add.end(), AudioIndex (buffer) );
                if ( iter != _aggregate_list.end() )
                {
                    ListView_SetItemText ( list_view, i, 6, _default_aggregate_name );
                }
            }

            //  Add the new aggrates to our member list

            _aggregate_list.splice ( _genre_list.end(), aggregate_add );
        }
        else
        {
            AudioIndex aggregate ( _default_aggregate_name );

            //  If we are decreasing the size of the aggregated

            IndexListIterator begin = _genre_list.begin(), end = _genre_list.end();

            IndexListIterator aggregate_iter = std::find ( begin, end, AudioIndex ( _default_aggregate_name ) );
            if ( aggregate_iter != end )
            {
                aggregate._count = (*aggregate_iter)._count;
                aggregate._state = (*aggregate_iter)._state;
            }

            IndexList aggregate_remove;

            begin = _aggregate_list.begin(), end = _aggregate_list.end();
           
            while ( begin != end )
            {
                if ( (*begin)._count > size && begin != aggregate_iter )
                {
                    _RPT2 ( _CRT_WARN, "Removing genre %s from the aggregate\tCount %d\r\n", (*begin)._name.c_str(), (*begin)._count );

                    aggregate_remove.push_back ( (*begin) );
                    _genre_list.push_back ( (*begin) );

                    aggregate._count -= (*begin)._count;

                    //  If we erase this member from our list, the return value is the next element

                    begin = _aggregate_list.erase ( begin );

                    continue;
                }

                begin ++;
            }

            if ( aggregate_remove.empty () )
            {
                _RPT1 ( _CRT_WARN, "No elements to remove from aggregate\tSetting new aggregate size %d\n", size );

                _aggregate_size = size; 
                return;
            }

            if ( aggregate_iter != end )
            {
                _genre_list.erase ( aggregate_iter );
            }

             //  Insert the aggregate index and resort

            if ( aggregate._count != 0 )
            {
                _genre_list.push_back ( aggregate );
            }

            //  Resort the genre and update the list view

            SortGenre ();

            //  Update the tunes list view (restore the genre of those tunes that are no longer aggregated)

            HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );

            LVITEM item;
            ::ZeroMemory ( & item, sizeof( item ) );
        
            item.mask = LVIF_PARAM;

            int count = ListView_GetItemCount ( list_view );

            for ( int i = 0; i < count; i++ )
            {
                item.iItem = i;

                if ( !ListView_GetItem ( list_view, & item ) )
                {
                    ::DisplaySystemError ( _form_hwnd );
                }

                AudioFile * audio_file = static_cast<AudioFile*>( (void *)item.lParam );
                if ( audio_file )
                {
                    TCHAR * genre = (TCHAR *) audio_file->GetGenre();

                    IndexListIterator iter = std::find ( aggregate_remove.begin (), aggregate_remove.end(), AudioIndex (genre) );
                    if ( iter != aggregate_remove.end() )
                    {
                        ListView_SetItemText ( list_view, i, 6, genre );
                    }
                }
            }
        }
    }

    //  Save the new aggregate size

    _aggregate_size = size;
}

void PlaylistGenerator::SetAggregateName  ( TCHAR * name )
{
    //  If aggregation is selected, we have to update both the tunes list and the genre list

    if ( _aggregate == BST_CHECKED )
    {
        //  Find the current name for the aggregate in the genre list

        IndexListIterator iter = std::find ( _genre_list.begin (), _genre_list.end(), AudioIndex ( _default_aggregate_name ) );
        if ( iter != _genre_list.end() )
        {
            (*iter)._name.assign ( name );
        }

        //  Update the genre list view

        LVFINDINFO find_item;
        ::ZeroMemory ( & find_item, sizeof( find_item ) );

        find_item.flags  = LVFI_PARAM;
        find_item.lParam = (LPARAM) & (*iter);

        HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_GENRES );

        int index = ListView_FindItem ( list_view, -1, & find_item );
        if ( index != -1 )
        {
            ListView_SetItemText ( list_view, index, 1, name );
        }

        //  Finally, we must update the tunes list view

        list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );

        LVITEM item;
        TCHAR buffer [ MAX_PATH ];

        ::ZeroMemory ( & item, sizeof( item ) );

        item.mask       = LVIF_TEXT;
        item.pszText    = buffer;
        item.cchTextMax = MAX_PATH;

        int count = ListView_GetItemCount ( list_view );

        for ( int i = 0; i < count; i++ )
        {
            ListView_GetItemText ( list_view, i, 6, buffer, MAX_PATH );

            if ( _tcsicmp ( buffer, _default_aggregate_name ) == 0 )
            {
                ListView_SetItemText ( list_view, i, 6, name );
            }
        }
    }

    //  Save the new aggregate name

    _tcscpy ( _default_aggregate_name, name );
}

int PlaylistGenerator::SortListView ( LPARAM lParam1, LPARAM lParam2 )
{
    AudioFile * first = (AudioFile *) lParam1, * second = (AudioFile *) lParam2;
    LPTSTR string1, string2;
    TCHAR buffer1 [ 64 ], buffer2 [ 64 ];
    int value;

    if ( !first || !second )
        return 0;

    switch ( _sort_info [ 0 ]._sort_column )
    {
        case 0:

            string1 = buffer1;
            string2 = buffer2;

            _stprintf ( buffer1, _T("%d"), first->_state );
            _stprintf ( buffer2, _T("%d"), second->_state );

            break;

        case 1:

            string1 = (LPTSTR)first->GetTitle ();
            string2 = (LPTSTR)second->GetTitle ();

            break;

        case 2:

            string1 = (LPTSTR)first->GetArtist ();
            string2 = (LPTSTR)second->GetArtist ();

            break;

        case 3:

            string1 = (LPTSTR)first->GetAlbum ();
            string2 = (LPTSTR)second->GetAlbum ();

            break;

        case 4:

            string1 = (LPTSTR)first->GetYear ();
            string2 = (LPTSTR)second->GetYear ();

            break;

        case 5:

            string1 = (LPTSTR)first->GetTrack ();
            string2 = (LPTSTR)second->GetTrack ();

            break;

        case 6:

            string1 = (LPTSTR)first->GetGenre ();
            string2 = (LPTSTR)second->GetGenre ();

            if ( _aggregate == BST_CHECKED )
            {
                IndexListIterator iter, begin = _aggregate_list.begin (), end = _aggregate_list.end ();

                iter = std::find ( begin, end, AudioIndex ( string1 ) );
                if ( iter != end )
                    string1 = _default_aggregate_name;

                iter = std::find ( begin, end, AudioIndex ( string2 ) );
                if ( iter != end )
                    string2 = _default_aggregate_name;
            }

            break;

        case 7:

            string1 = (LPTSTR)first->GetFileType ();
            string2 = (LPTSTR)second->GetFileType ();

            break;
    }

    if ( string1 == NULL )
        string1 = _T("");
    if ( string2 == NULL )
        string2 = _T("");

    if ( _sort_info [ 0 ]._sort_mode == SORT_DESCENDING )
    {
        value = _tcsicmp ( string1, string2 );
    }
    else
    {
        value = _tcsicmp ( string2, string1 );
    }

    return value;
}

int PlaylistGenerator::SortGenreListView ( LPARAM lParam1, LPARAM lParam2 )
{
    AudioIndex * first = (AudioIndex *) lParam1, * second = (AudioIndex *) lParam2;
    LPTSTR string1, string2;
    TCHAR buffer1 [ 64 ], buffer2 [ 64 ];
    int value;

    if ( !first || !second )
        return 0;

    switch ( _sort_info [ 1 ]._sort_column )
    {
        case 0:

            string1 = buffer1;
            string2 = buffer2;

            _stprintf ( buffer1, _T("%d"), first->_state );
            _stprintf ( buffer2, _T("%d"), second->_state );

            break;

        case 1:

            string1 = (LPTSTR)first->_name.c_str ();
            string2 = (LPTSTR)second->_name.c_str ();

            break;

        case 2:

            if ( _sort_info [ 1 ]._sort_mode == SORT_DESCENDING )
            {
                if ( first->_count > second->_count )
                    value = 1;
                else
                if ( first->_count < second->_count )
                    value = -1;
                else
                    value = 0;
            }
            else
            {
                if ( second->_count > first->_count )
                    value = 1;
                else
                if ( second->_count < first->_count )
                    value = -1;
                else
                    value = 0;
            }

            return value;
    }

    if ( string1 == NULL )
        string1 = _T("");
    if ( string2 == NULL )
        string2 = _T("");

    if ( _sort_info [ 1 ]._sort_mode == SORT_DESCENDING )
    {
        value = _tcsicmp ( string1, string2 );
    }
    else
    {
        value = _tcsicmp ( string2, string1 );
    }

    return value;
}

int PlaylistGenerator::SortArtistListView ( LPARAM lParam1, LPARAM lParam2 )
{
    AudioIndex * first = (AudioIndex *) lParam1, * second = (AudioIndex *) lParam2;
    LPTSTR string1, string2;
    TCHAR buffer1 [ 64 ], buffer2 [ 64 ];
    int value;

    if ( !first || !second )
        return 0;

    switch ( _sort_info [ 2 ]._sort_column )
    {
        case 0:

            string1 = buffer1;
            string2 = buffer2;

            _stprintf ( buffer1, _T("%d"), first->_state );
            _stprintf ( buffer2, _T("%d"), second->_state );

            break;

        case 1:

            string1 = (LPTSTR)first->_name.c_str ();
            string2 = (LPTSTR)second->_name.c_str ();

            break;

        case 2:

            if ( _sort_info [ 2 ]._sort_mode == SORT_DESCENDING )
            {
                if ( first->_count > second->_count )
                    value = 1;
                else
                if ( first->_count < second->_count )
                    value = -1;
                else
                    value = 0;
            }
            else
            {
                if ( second->_count > first->_count )
                    value = 1;
                else
                if ( second->_count < first->_count )
                    value = -1;
                else
                    value = 0;
            }

            return value;
    }

    if ( string1 == NULL )
        string1 = _T("");
    if ( string2 == NULL )
        string2 = _T("");

    if ( _sort_info [ 2 ]._sort_mode == SORT_DESCENDING )
    {
        value = _tcsicmp ( string1, string2 );
    }
    else
    {
        value = _tcsicmp ( string2, string1 );
    }

    return value;
}

DWORD PlaylistGenerator::AddSongToPlaylist ( AudioFile * audio_file, FILE * file, LPCTSTR directory )
{
    if ( _copy_tunes == BST_CHECKED )
    {
        TCHAR stripped_filename [ MAX_PATH ], buffer [ MAX_PATH ];

        _tcscpy ( stripped_filename, audio_file->_fn.c_str () );
        ::PathStripPath ( stripped_filename );

        _ftprintf( file, _T("%s\n"), stripped_filename );

        _stprintf ( buffer, _T("Copying '%s'"), stripped_filename );
        _status_bar.SetPaneText ( 0, buffer );
        
        DWORD error = CopyFileToDirectory ( audio_file->_fn.c_str (), directory, stripped_filename );
        if ( error != ERROR_SUCCESS )
            return error;
    }
    else
    {
        //  Make path relative to the music directory

        if ( _make_paths_relative == BST_CHECKED )
        {
            TCHAR filename [ MAX_PATH ];

            _tcscpy ( filename, audio_file->_fn.c_str () );

            unsigned length = _tcslen ( _music_directory );

            TCHAR * temp = filename;
            temp += length;

            if ( *temp == '\\' )
            {
                _ftprintf ( file, _T("%s\n"), & temp [ 1 ] );
            }
            else
            {
                _ftprintf ( file, _T("%s\n"), temp );
            }
        }
        else
        {
            _ftprintf ( file, _T("%s\n"), audio_file->_fn.c_str () );
        }
    }

    return ERROR_SUCCESS;
}

DWORD PlaylistGenerator::CopyFileToDirectory ( LPCTSTR file_to_copy, LPCTSTR directory, LPCTSTR stripped_filename )
{
    TCHAR file_name [ MAX_PATH ];   

    _tcscpy ( file_name, directory );
    _tcscat ( file_name, _T("\\") );
    _tcscat ( file_name, stripped_filename );

    if ( ::CopyFile ( file_to_copy, file_name, FALSE ) == 0 )
    {
        return ::DisplaySystemError ( _main_hwnd );
    }

    return ERROR_SUCCESS;
}

void PlaylistGenerator::ReadLibrary ( )
{
    TCHAR input_path [ MAX_PATH ];
    TCHAR file_name  [ MAX_PATH ];

    ::GetModuleFileName ( _instance, input_path, MAX_PATH );
    ::PathRemoveFileSpec ( input_path );

    _tcscpy ( file_name, input_path );
    _tcscat( file_name, _T("\\PlaylistGen.dat") );

    //  Try for the new format

    bool did_read = false;

    FILE * fp = _tfopen ( file_name, _T("rb") );
    if ( fp )
    {
        PLAYLIST_HEADER header;

        if ( fread ( & header, sizeof(PLAYLIST_HEADER), 1, fp ) != 1 )
        {
            ::DisplaySystemError ( _form_hwnd );
            fclose ( fp );
            return;
        }

        //  Is this the new style format?

        if ( header.magic_0 == NEW_MAGIC_0 && header.magic_1 == NEW_MAGIC_1 )
        {
            for ( int i = 0; i < header.tune_count; i++ )
            {
                AudioFile tune;

                if ( !tune.Read ( fp, header.major_version, header.minor_version ) )
                {
                    ::DisplaySystemError ( _form_hwnd );
                    fclose ( fp );
                    return;
                }

                _audio_list.push_back( tune );

                if ( tune._state == BST_CHECKED )
                {
                    _selected_list.push_back ( & _audio_list.back() );
                }
            }

            for ( i = 0; i < header.genre_count; i++ )
            {
                AudioIndex genre;

                if ( !genre.Read ( fp ) )
                {
                    ::DisplaySystemError ( _form_hwnd );
                    fclose ( fp );
                    return;
                }
                
                _genre_list.push_back ( genre );
            }

            for ( i = 0; i < header.artist_count; i++ )
            {
                AudioIndex artist;

                if ( !artist.Read ( fp ) )
                {
                    ::DisplaySystemError ( _form_hwnd );
                    fclose ( fp );
                    return;
                }
                
                _artist_list.push_back ( artist );
            }

            for ( i = 0; i < header.aggregate_count; i++ )
            {
                AudioIndex aggregate;

                if ( !aggregate.Read ( fp ) )
                {
                    ::DisplaySystemError ( _form_hwnd );
                    fclose ( fp );
                    return;
                }
                
                _aggregate_list.push_back ( aggregate );
            }

            did_read = true;
        }

        fclose ( fp );
    }

    if ( did_read )
        return;

    HANDLE in_file = ::CreateFile ( file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( in_file != INVALID_HANDLE_VALUE )
    {       
        HANDLE map_file = ::CreateFileMapping ( in_file, NULL, PAGE_READONLY, 0, 0, NULL );
        if ( map_file != NULL )
        {
            void * data = ::MapViewOfFile ( map_file, FILE_MAP_READ, 0, 0, 0 );

            DWORD file_length = ::GetFileSize ( in_file, NULL );            

            if ( data && file_length )
            {
                BYTE * audio_data = (BYTE *) data;               

                //  Does the header exist?

                PLAYLIST_HEADER * header = (PLAYLIST_HEADER * ) data;

                if ( header->magic_0 == MAGIC_0 && header->magic_1 == MAGIC_1 )
                {
                    audio_data  += sizeof ( PLAYLIST_HEADER );
                    file_length -= sizeof ( PLAYLIST_HEADER );
                }
                else
                {
                    header = NULL;
                }

                try
                {
                    while ( file_length != 0 )
                    {
                        AudioFile library_entry;

                        //  Length of file name

                        DWORD length = *(DWORD *)audio_data;

                        audio_data  += 4;
                        file_length -= 4;

                        //  File name

                        library_entry._fn.assign ( (TCHAR * )audio_data, length );

                        audio_data  += length;
                        file_length -= length;

                        //  File size

                        library_entry._fs = *(DWORD *)audio_data;

                        audio_data  += 4;
                        file_length -= 4;

                        //  Selected state not implemented until introduction of header

                        if ( header )
                        {
                            library_entry._state = *(DWORD *)audio_data;

                            audio_data  += 4;
                            file_length -= 4;

                            //  File type added for version 1.1

                            if ( header->major_version >= 1 && header->minor_version >= 1 )
                            {
                                library_entry._type = *(AudioFileType *)audio_data;

                                audio_data  += 4;
                                file_length -= 4;
                            }
                        }

                        //  Number of text elements

                        DWORD count = *(DWORD *)audio_data;

                        audio_data  += 4;
                        file_length -= 4;

                        while ( count != 0 )
                        {
                            DWORD index;
                            BasicString value;

                            index = *(DWORD *)audio_data;

                            audio_data  += 4;
                            file_length -= 4;

                            length = *(DWORD *)audio_data;

                            audio_data  += 4;
                            file_length -= 4;

                            value.assign ( (TCHAR * )audio_data, length );

                            audio_data  += length;
                            file_length -= length;

                            //  Removed _T("comment") element starting with version 1.1

                            if ( index != MAKE_FRAME_ID('T','C','O','M') )
                            {
                                library_entry._fm.insert ( AudioFile::TextMapValue (index, value ) );
                            }

                            count --;
                        }

                        _RPT1 ( _CRT_WARN, "%s\r\n", library_entry._fn.c_str() );

                        _audio_list.push_back ( library_entry );
                    }
                }
                catch ( ... )
                {

                }

                ::UnmapViewOfFile ( data );
            }

            ::CloseHandle ( map_file );

        }

        ::CloseHandle ( in_file );
    }

    _tcscpy ( file_name, input_path );
    _tcscat( file_name, _T("\\Genre.dat") );

    in_file = ::CreateFile ( file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( in_file != INVALID_HANDLE_VALUE )
    {
        HANDLE map_file = ::CreateFileMapping ( in_file, NULL, PAGE_READONLY, 0, 0, NULL );
        if ( map_file != NULL )
        {
            void * data = ::MapViewOfFile ( map_file, FILE_MAP_READ, 0, 0, 0 );

            DWORD file_length = ::GetFileSize ( in_file, NULL );

            if ( data && file_length )
            {
                BYTE * genre_data = (BYTE *) data;

                //  Does the header exist?

                PLAYLIST_HEADER * header = (PLAYLIST_HEADER * ) data;

                if ( header->magic_0 == MAGIC_0 && header->magic_1 == MAGIC_1 )
                {
                    genre_data  += sizeof ( PLAYLIST_HEADER );
                    file_length -= sizeof ( PLAYLIST_HEADER );
                }
                else
                {
                    header = NULL;
                }

                try
                {
                    while ( file_length != 0 )
                    {
                        AudioIndex genre;

                        //  Length of genre name

                        DWORD length = *(DWORD *)genre_data;

                        genre_data  += 4;
                        file_length -= 4;

                        //  Genre name

                        genre._name.assign ( (TCHAR * )genre_data, length );

                        genre_data  += length;
                        file_length -= length;

                        //  Number of titles of this genre

                        genre._count = *(DWORD *)genre_data;

                        genre_data  += 4;
                        file_length -= 4;

                        //  Is this genre to be displayed

                        genre._state = *(DWORD *)genre_data;

                        genre_data  += 4;
                        file_length -= 4;

                        _RPT1 ( _CRT_WARN, "%s\r\n", genre._name.c_str() );
                       
                        _genre_list.push_back ( genre );
                    }
                }
                catch ( ... )
                {

                }

                ::UnmapViewOfFile ( data );
            }

            ::CloseHandle ( map_file );

        }

        ::CloseHandle ( in_file );
    }

    _tcscpy ( file_name, input_path );
    _tcscat( file_name, _T("\\Artist.dat") );

    in_file = ::CreateFile ( file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( in_file != INVALID_HANDLE_VALUE )
    {
        HANDLE map_file = ::CreateFileMapping ( in_file, NULL, PAGE_READONLY, 0, 0, NULL );
        if ( map_file != NULL )
        {
            void * data = ::MapViewOfFile ( map_file, FILE_MAP_READ, 0, 0, 0 );

            DWORD file_length = ::GetFileSize ( in_file, NULL );

            if ( data && file_length )
            {
                BYTE * artist_data = (BYTE *) data;

                //  Does the header exist?

                PLAYLIST_HEADER * header = (PLAYLIST_HEADER * ) data;

                if ( header->magic_0 == MAGIC_0 && header->magic_1 == MAGIC_1 )
                {
                    artist_data += sizeof ( PLAYLIST_HEADER );
                    file_length -= sizeof ( PLAYLIST_HEADER );
                }
                else
                {
                    header = NULL;
                }

                try
                {
                    while ( file_length != 0 )
                    {
                        AudioIndex artist;

                        //  Length of artist name

                        DWORD length = *(DWORD *)artist_data;

                        artist_data  += 4;
                        file_length -= 4;

                        //  Artist name

                        artist._name.assign ( (TCHAR * )artist_data, length );

                        artist_data  += length;
                        file_length -= length;

                        //  Number of titles of this genre

                        artist._count = *(DWORD *)artist_data;

                        artist_data  += 4;
                        file_length -= 4;

                        //  Is this genre to be displayed

                        artist._state = *(DWORD *)artist_data;

                        artist_data  += 4;
                        file_length -= 4;

                        _RPT1 ( _CRT_WARN, "%s\r\n", artist._name.c_str() );
                       
                        _artist_list.push_back ( artist );
                    }
                }
                catch ( ... )
                {

                }

                ::UnmapViewOfFile ( data );
            }

            ::CloseHandle ( map_file );

        }

        ::CloseHandle ( in_file );
    }
}

void PlaylistGenerator::WriteLibrary ( )
{
    TCHAR output_path [ MAX_PATH ];
    TCHAR file_name [ MAX_PATH ];

    ::GetModuleFileName ( _instance, output_path, MAX_PATH );
    ::PathRemoveFileSpec ( output_path );

    _tcscpy ( file_name, output_path );
    _tcscat( file_name, _T("\\PlaylistGen.dat") );

    FILE * out = _tfopen ( file_name, _T("wb") );
    if ( out )
    {
        PLAYLIST_HEADER header;
        ::ZeroMemory ( & header, sizeof( header ) );

        header.magic_0          = NEW_MAGIC_0;
        header.magic_1          = NEW_MAGIC_1;
        header.major_version    = MAJOR_VERSION;
        header.minor_version    = MINOR_VERSION;
        header.tune_count       = _audio_list.size ();
        header.genre_count      = _genre_list.size ();
        header.artist_count     = _artist_list.size ();
        header.aggregate_count  = _aggregate_list.size ();

        fwrite ( & header, sizeof( header ), 1, out );

        //  Write out the tunes list

        AudioListIterator begin = _audio_list.begin (), end = _audio_list.end ();
        for ( ; begin != end; begin ++ )
        {
            begin->Write ( out );
        }

        //  Write out the genre list

        IndexListIterator i_begin = _genre_list.begin (), i_end = _genre_list.end ();
        for ( ; i_begin != i_end; i_begin ++ )
        {
            i_begin->Write ( out );
        }

        //  Write out the artist list

        i_begin = _artist_list.begin (), i_end = _artist_list.end ();
        for ( ; i_begin != i_end; i_begin ++ )
        {
            i_begin->Write ( out );
        }

        //  Write out the aggregate list

        i_begin = _aggregate_list.begin (), i_end = _aggregate_list.end ();
        for ( ; i_begin != i_end; i_begin ++ )
        {
            i_begin->Write ( out );
        }

        fclose ( out );
    }
}

DWORD PlaylistGenerator::AddTuneToListView ( AudioFile * audio_file, int * column_width )
{
    LVITEM item;    
    ::ZeroMemory ( & item, sizeof( item ) );

    //  First thing to do is make sure the genre for this title is enabled

    LPTSTR genre = (TCHAR *)audio_file->GetGenre ();

    IndexListIterator v_iter = std::find ( _genre_list.begin (), _genre_list.end(), AudioIndex ( genre ) );

    //  If we didn't find the genre in the list, then if aggregating, check that list

    if ( v_iter == _genre_list.end() )
    {
        if ( _aggregate == BST_CHECKED )
        {
            v_iter = std::find ( _aggregate_list.begin(), _aggregate_list.end(),  AudioIndex ( genre ) );

            //  Didn't find in the aggregate list either

            if ( v_iter == _aggregate_list.end() )
            {
                return ERROR_SUCCESS;    
            }

            //  Is the aggregate genre enabled?

            v_iter = std::find ( _genre_list.begin (), _genre_list.end(), AudioIndex ( _default_aggregate_name ) );

            if ( v_iter == _aggregate_list.end() )
            {
                return ERROR_SUCCESS;    
            }

            //  This tune will be inserted with the aggregate name

            genre = _default_aggregate_name;
        }
        else
        {
            //  Not in the genre list and we aren't aggregating to don't display

            return ERROR_SUCCESS;
        }
    }

    //  It the genre isn't enabled, then we dont' add the title

    if ( (*v_iter)._state != BST_CHECKED )
    {
        return ERROR_SUCCESS;
    }

    HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );

    //  Set the LPARAM

    item.mask       = LVIF_PARAM;
    item.iItem      = ListView_GetItemCount ( list_view );
    item.iSubItem   = 0;
    item.lParam     = (LPARAM) audio_file;

    int index = ListView_InsertItem ( list_view, & item );
    if ( index == -1 )
    {
        return ::DisplaySystemError ( _main_hwnd );
    }

    _total_tune_size        += audio_file->_fs;
    _total_tune_duration    += audio_file->_duration;

    ListView_SetCheckState ( list_view, index, audio_file->_state );

    //  Insert the title

    LPTSTR text = (TCHAR *)audio_file->GetTitle ();

    item.mask       = LVIF_TEXT;
    item.iItem      = index;
    item.iSubItem   = 1;
    item.pszText    = text;

    if ( !ListView_SetItem ( list_view, & item ) )
    {
        return ::DisplaySystemError ( _main_hwnd );
    }

    if ( column_width && text )
        UpdateWidth ( list_view, text, & column_width [ 1 ] );

    //  Insert the artist

    text = (TCHAR *)audio_file->GetArtist ();

    item.mask       = LVIF_TEXT;
    item.iItem      = index;
    item.iSubItem   = 2;
    item.pszText    = text;

    if ( !ListView_SetItem ( list_view, & item ) )
    {
        return ::DisplaySystemError ( _main_hwnd );
    }

    if ( column_width && text )
        UpdateWidth ( list_view, text, & column_width [ 2 ] );

    //  Insert the album

    text = (TCHAR *)audio_file->GetAlbum ();

    item.mask       = LVIF_TEXT;
    item.iItem      = index;
    item.iSubItem   = 3;
    item.pszText    = text;

    if ( !ListView_SetItem ( list_view, & item ) )
    {
        return ::DisplaySystemError ( _main_hwnd );
    }

    if ( column_width && text )
        UpdateWidth ( list_view, text, & column_width [ 3 ] );

    //  Insert the year

    text = (TCHAR *)audio_file->GetYear ();

    item.mask       = LVIF_TEXT;
    item.iItem      = index;
    item.iSubItem   = 4;
    item.pszText    = text;

    if ( !ListView_SetItem ( list_view, & item ) )
    {
        return ::DisplaySystemError ( _main_hwnd );
    }

    if ( column_width && text )
        UpdateWidth ( list_view, text, & column_width [ 4 ] );

    //  Insert the track

    text = (TCHAR *)audio_file->GetTrack ();

    item.mask       = LVIF_TEXT;
    item.iItem      = index;
    item.iSubItem   = 5;
    item.pszText    = text;

    if ( !ListView_SetItem ( list_view, & item ) )
    {
        return ::DisplaySystemError ( _main_hwnd );
    }

    if ( column_width && text )
        UpdateWidth ( list_view, text, & column_width [ 5 ] );

    //  Insert the genre

    item.mask       = LVIF_TEXT;
    item.iItem      = index;
    item.iSubItem   = 6;
    item.pszText    = genre;

    if ( !ListView_SetItem ( list_view, & item ) )
    {
        return ::DisplaySystemError ( _main_hwnd );
    }

    if ( column_width && text )
        UpdateWidth ( list_view, text, & column_width [ 6 ] );

    //  Insert the file type

    text = (TCHAR *)audio_file->GetFileType ();

    item.mask       = LVIF_TEXT;
    item.iItem      = index;
    item.iSubItem   = 7;
    item.pszText    = text;

    if ( !ListView_SetItem ( list_view, & item ) )
    {
        return ::DisplaySystemError ( _main_hwnd );
    }

    if ( column_width && text )
        UpdateWidth ( list_view, text, & column_width [ 7 ] );

    return ERROR_SUCCESS;
}

void PlaylistGenerator::SavePosition ( )
{
    RECT rc;
    TCHAR buffer [ MAX_PATH ];
    HKEY key;
    long success;

    //  This causes problems on the next load so if we are in minimized or maximized, don't
    //  update the last position.
    
    if ( ::IsIconic ( _main_hwnd ) || ::IsZoomed ( _main_hwnd ) )
        return;

    _tcscpy ( buffer, g_szPlaylistGenerator );
    _tcscat ( buffer, _T("\\Position") );

    ::GetWindowRect ( _main_hwnd, & rc );

    int width   = rc.right - rc.left;
    int height  = rc.bottom - rc.top;
    
    success = ::RegCreateKeyEx ( HKEY_LOCAL_MACHINE, buffer, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, & key, NULL );

    if (success == ERROR_SUCCESS)
    {  
        success = ::RegSetValueEx ( key, _T("X"), 0, REG_DWORD, (BYTE *) & rc.left, sizeof ( rc.left ) );
    
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("Y"), 0, REG_DWORD, (BYTE *) & rc.top, sizeof ( rc.top ) );
    
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("Width"), 0, REG_DWORD, (BYTE *) & width, sizeof ( width ) );
    
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        success = ::RegSetValueEx ( key, _T("Height"), 0, REG_DWORD, (BYTE *) & height, sizeof ( height ) );
    
        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        ::RegCloseKey ( key );
    }
}

void PlaylistGenerator::SaveListView ( TCHAR * key_name, int count, int ctrl_id )
{
    TCHAR buffer [ MAX_PATH ];
    HKEY key;
    long success;
    
    _tcscpy ( buffer, g_szPlaylistGenerator );
	_tcscat ( buffer, _T("\\") );
    _tcscat ( buffer, key_name );
    
    success = ::RegCreateKeyEx ( HKEY_LOCAL_MACHINE, buffer, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, & key, NULL );

    if (success == ERROR_SUCCESS)
    {
        HWND list_view = ::GetDlgItem ( _form_hwnd, ctrl_id );
        int width;

        for ( int i = 0; i < count; i++ )
        {
            _stprintf ( buffer, _T("%d"), i );
            width = ListView_GetColumnWidth ( list_view, i );

            success = ::RegSetValueEx ( key, buffer, 0, REG_DWORD, (BYTE *) & width, sizeof ( width ) );
   
            if (success != ERROR_SUCCESS)
            {
                ::DisplaySystemError ( _main_hwnd );
            }
        }

        ::RegCloseKey ( key );
    }
}

void PlaylistGenerator::SaveLastTime ( TCHAR * key_name, FILETIME updated_time )
{
    HKEY key;
    long success;
       
    success = ::RegCreateKeyEx ( HKEY_LOCAL_MACHINE, g_szPlaylistGenerator, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, & key, NULL );

    if (success == ERROR_SUCCESS)
    {
        success = ::RegSetValueEx ( key, key_name, 0, REG_BINARY, (BYTE *) & updated_time, sizeof ( updated_time ) );

        if (success != ERROR_SUCCESS)
        {
            ::DisplaySystemError ( _main_hwnd );
        }

        ::RegCloseKey ( key );
    }
}

DWORD PlaylistGenerator::PlaySelected ( HWND hWnd )
{
    TCHAR filename [ 512 ];

    ::GetModuleFileName ( NULL, filename, 512 );
    ::PathRemoveFileSpec ( filename );

    _tcscat ( filename, _T("\\rpcg.m3u") );

    FILE * file = _tfopen ( filename, _T("wt") );
    if ( !file )
    {
        return ::DisplaySystemError ( hWnd );
    }

    _fputts ( _T("#EXTM3U\n"), file );

    int index = -1, count = 0;
    HWND list_view = ::GetDlgItem ( _form_hwnd, IDC_LIST_TUNES );

    while ( (index = ListView_GetNextItem ( list_view, index, LVNI_SELECTED )) != -1 )
    {
        LVITEM item;
        ::ZeroMemory ( & item, sizeof(item) );

        item.mask   = LVIF_PARAM;
        item.iItem  = index;

        if ( !ListView_GetItem ( list_view, & item ) )
        {
            ::DisplaySystemError ( hWnd );
        }
        else
        {
            AudioFile * audio_file = static_cast<AudioFile *>( (void *) item.lParam );
            if ( audio_file )
            {
                _ftprintf( file, _T("%s\n"), audio_file->_fn.c_str () );
                count++;
            }
        }
    }

    fclose ( file );

    if ( count )
    {
        HINSTANCE instance = ::ShellExecute ( ::GetDesktopWindow(), _T("open"), filename, NULL, NULL, SW_SHOW );
    }

    return ERROR_SUCCESS;
}

void PlaylistGenerator::ReadPosition ( int & x, int & y, int & width, int & height )
{
    TCHAR buffer [ MAX_PATH ];
    HKEY key;
    long success;
    DWORD size, type;
    int value;

    _tcscpy ( buffer, g_szPlaylistGenerator );
    _tcscat ( buffer, _T("\\Position") );

    success = ::RegOpenKeyEx ( HKEY_LOCAL_MACHINE, buffer, 0, KEY_READ, & key );

    if (success == ERROR_SUCCESS)
    {  
        size = sizeof( value );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("X"), 0, & type, (BYTE *) & value, & size );
    
        if (success == ERROR_SUCCESS)
        {
            x = value;
        }

        size = sizeof( value );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("Y"), 0, & type, (BYTE *) & value, & size );
    
        if (success == ERROR_SUCCESS)
        {
            y = value;
        }

        size = sizeof( value );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("Width"), 0, & type, (BYTE *) & value, & size );
    
        if (success == ERROR_SUCCESS)
        {
            width = value;
        }

        size = sizeof( value );
        type = REG_DWORD;

        success = ::RegQueryValueEx ( key, _T("Height"), 0, & type, (BYTE *) & value, & size );
    
        if (success == ERROR_SUCCESS)
        {
            height = value;
        }

        ::RegCloseKey ( key );
    }
}

bool PlaylistGenerator::ReadListView ( TCHAR * key_name, int count, int * column_width )
{
    TCHAR buffer [ MAX_PATH ];
    HKEY key;
    long success;
    DWORD size, type;
    
    int good_count = 0;

    _tcscpy ( buffer, g_szPlaylistGenerator );
	_tcscat ( buffer, _T("\\") );
    _tcscat ( buffer, key_name );

    success = ::RegOpenKeyEx ( HKEY_LOCAL_MACHINE, buffer, 0, KEY_READ, & key );

    if (success == ERROR_SUCCESS)
    {
        int width;

        for ( int i = 0; i < count; i++ )
        {
            _stprintf ( buffer, _T("%d"), i );

            size = sizeof( width );
            type = REG_DWORD;

            success = ::RegQueryValueEx ( key, buffer, 0, & type, (BYTE *) & width, & size );
  
            if (success  == ERROR_SUCCESS)
            {
                column_width [ i ] = width;
                good_count ++;
            }
        }

        ::RegCloseKey ( key );
    }

    return (good_count == count) ? true : false;
}

bool PlaylistGenerator::IsTimeToUpdate ( UpdateItem item )
{
    ULARGE_INTEGER last_time, current_time, delta, length_of_day;
    unsigned days;

    if ( item == RPC_UPDATE_LIBARAY )
    {
        if ( _reminder == BST_UNCHECKED )
            return false;

        last_time.u.LowPart     = _last_updated.dwLowDateTime;
        last_time.u.HighPart    = _last_updated.dwHighDateTime;

        days = _days;
    }
    else
    if ( item == RPG_UPDATE_VERSION )
    {
        last_time.u.LowPart     = _last_updated_version.dwLowDateTime;
        last_time.u.HighPart    = _last_updated_version.dwHighDateTime;

        days = _updated_days;
    }
    else
    {
        return false;
    }

    //  Get the current time

    FILETIME time_now;
    ::GetSystemTimeAsFileTime ( & time_now );

    current_time.u.LowPart     = time_now.dwLowDateTime;
    current_time.u.HighPart    = time_now.dwHighDateTime;

    delta.QuadPart = current_time.QuadPart - last_time.QuadPart;

    length_of_day.QuadPart = 864000000000;      // Number of 100 nanosecond units in 1 day

    return ( delta.QuadPart < 0 || delta.QuadPart >= (length_of_day.QuadPart * days) ) ? true : false;
}

void PlaylistGenerator::UpdateColumnHeader ( HWND hWnd, int ctrl, int column, SortMode mode )
{
    HDITEM item;
    TCHAR buffer [ MAX_PATH ];

    ::ZeroMemory ( & item, sizeof ( item ) );

    HWND header = ListView_GetHeader ( ::GetDlgItem ( hWnd, ctrl) );

    item.mask         = HDI_TEXT | HDI_IMAGE;
    item.pszText      = buffer;
    item.cchTextMax   = MAX_PATH;

    Header_GetItem ( header, column, & item );

    if ( column != 0 )
    {
        item.mask   = HDI_FORMAT | HDI_TEXT;
        item.fmt    = HDF_LEFT | HDF_STRING;

        if ( mode == SORT_ASCENDING )
        {
            item.fmt |= HDF_SORTUP;
        }
        else
        if ( mode == SORT_DESCENDING )
        {
            item.fmt |= HDF_SORTDOWN;
        }
    }
    else
    {
        item.mask   = HDI_FORMAT | HDI_IMAGE;
        item.fmt    = HDF_IMAGE;
    }

    Header_SetItem ( header, column, & item );
}

bool PlaylistGenerator::GetVersion ( DWORD * major, DWORD * minor )
{
    DWORD length;
    VS_FIXEDFILEINFO * ver_info;
    void * info;
    bool success = false;

    * major = * minor = 0;

    HRSRC hr = ::FindResource ( NULL, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION );
    if ( hr )
    {
        HGLOBAL hg = ::LoadResource ( NULL, hr );
        if ( hg )
        {
            info = LockResource ( hg );

            if ( ::VerQueryValue ( info, _T("\\"), (void **) & ver_info, (unsigned *) & length ) )
            {
                * major = ver_info->dwFileVersionMS;
                * minor = ver_info->dwFileVersionLS;
                success = true;
            }
            
            UnlockResource ( hg );
        }
    }

    return success;
}

void PlaylistGenerator::GetColumnHeadingWidth ( HWND hWnd, HWND hWndList, int count, int * widths )
{
    TCHAR buffer [ MAX_PATH ];
    
    LVCOLUMN column;
    ::ZeroMemory ( & column, sizeof( column ) );

    column.mask         = LVCF_TEXT;
    column.pszText      = buffer;
    column.cchTextMax   = MAX_PATH;
    
    for ( int i = 0; i < count; i++ )
    {
        ListView_GetColumn ( hWndList, i, & column );

        int text_width = ListView_GetStringWidth ( hWndList, buffer );

        int col_width = ListView_GetColumnWidth ( hWndList, i );
        
        widths [ i ] = ( text_width > col_width ) ? text_width : col_width;
    }
}

void PlaylistGenerator::UpdateWidth ( HWND hWndList, LPTSTR text, int * width )
{
    int actual = ListView_GetStringWidth ( hWndList, text );

    if ( actual > * width )
        * width = actual;
}

void PlaylistGenerator::PreserveAudioListState ( AudioList & list )
{
    AudioListIterator iter, begin = list.begin(), end = list.end ();
    AudioListIterator a_begin = _audio_list.begin(), a_end = _audio_list.end ();

    _selected_list.clear ();

    for ( ; begin != end; begin++ )
    {
        iter = std::find ( a_begin, a_end, (*begin) );
        if ( iter != a_end )
        {
            (*begin)._state = (*iter)._state;

            if ( (*begin)._state == BST_CHECKED )
            {
                _selected_list.push_back ( & (*begin) );
            }
        }
    }

    //  Splicing is TONS more efficient than assignment.  Another benefit
    //  is we don't have to walk the new list to determine the selected.

    _audio_list.clear  ( );
    _audio_list.splice ( _audio_list.end (), list );
}

void PlaylistGenerator::PreserveIndexListState ( IndexList & list, IndexList & original )
{
    IndexListIterator iter, begin = list.begin(), end = list.end ();
    IndexListIterator a_begin = original.begin(), a_end = original.end ();

    for ( ; begin != end; begin++ )
    {
        iter = std::find ( a_begin, a_end, (*begin) );
        if ( iter != a_end )
        {
            (*begin)._state = (*iter)._state;
        }
    }

    //  Splicing is TONS more efficient than assignment

    original.clear  ( );
    original = list;
}

bool PlaylistGenerator::DeleteAudioIndex ( IndexList & index_list, const TCHAR * name, int ctrl )
{
    IndexListIterator v_iter = std::find ( index_list.begin (), index_list.end(), AudioIndex ( name ) );
    if ( v_iter == index_list.end() )
        return false;

    LVFINDINFO item;
    ::ZeroMemory ( & item, sizeof( item ) );

    item.flags  = LVFI_PARAM;
    item.lParam = (LPARAM) & (*v_iter);

    HWND list_view = ::GetDlgItem ( _form_hwnd, ctrl );

    int index = ListView_FindItem ( list_view, -1, & item );

    (* v_iter)._count--;

    if ( !(* v_iter)._count )
    {
        index_list.erase ( v_iter );

        if ( index != -1 )
        {
            ListView_DeleteItem ( list_view, index );
        }
    }
    else
    {
        if ( index != -1 )
        {
            TCHAR buffer [ MAX_PATH ];
            _stprintf ( buffer, _T("%d"), (* v_iter)._count );
            ListView_SetItemText ( list_view, index, 1, buffer );
        }
    }

    return true;
}

// End of file
