
#ifndef PLAYLIST_GENERATOR_H
#define PLAYLIST_GENERATOR_H

#include "AudioFile.h"
#include "RPGStatusBar.h"

class PlaylistGenerator;

class RPGOptions
{
public:

    PlaylistGenerator   * _generator;
    bool                  _dirty;
    bool                  _loading;

    RPGOptions ( ) : _generator ( NULL ), _dirty ( false ), _loading ( false ) { ; }
    RPGOptions ( PlaylistGenerator * generator ) : _generator ( generator ), _dirty ( false ), _loading ( false ) { ; }
    ~RPGOptions ( ) { ; }

    bool IsDirty ( )    { return _dirty ; }
    bool IsLoading ( )  { return _loading; }

    void SetDirty   ( bool dirty = true )   { _dirty = dirty; }
    void SetLoading ( bool loading = true ) { _loading = loading; }

    PlaylistGenerator * GetGenerator ( ) { return _generator; }
};

inline DWORD DWORD_SWAP(DWORD x)
{
    return  ((DWORD)( ((x) << 24) | ((x) >> 24) |
                      (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8)));
}

class AudioIndex
{
public:

    BasicString     _name;
    DWORD           _count;
    DWORD           _state;

    AudioIndex ( ) : _count ( 0 ), _state ( BST_CHECKED )   { }
    AudioIndex ( LPCTSTR name ) : _name ( name ), _count ( 0 ), _state ( BST_CHECKED )   { }
    AudioIndex ( const AudioIndex & index ) : _name ( index._name ), _count ( index._count ), _state ( index._state)  { }

    ~AudioIndex ( ) { }

    bool operator==( const AudioIndex & index )
    {
        return ( _name == index._name );
    }

    void Write ( FILE * out )
    {
        //  Length of genre name

        BasicString::size_type length = _name.length ();

        //  Genre

        fwrite ( & length, sizeof( length ), 1, out );
        fwrite ( _name.c_str(), length * sizeof(TCHAR), 1, out );

        //  Number of titles in this genre

        fwrite ( & _count, sizeof (_count), 1, out );

        //  Is this genre to be displayed

        fwrite ( & _state, sizeof (_state), 1, out );
    }

    bool Read ( FILE * in )
    {
        DWORD length;

        //  Read the length of the name
        
        if ( fread ( (void *) & length, sizeof(length), 1, in ) != 1 )
            return false;

        //  Allocate a buffer and read the name

        TCHAR * name = new TCHAR [ length ];
        if ( !name )
            return false;

        if ( fread ( (void *) name, length * sizeof(TCHAR), 1, in ) != 1 )
        {
            delete [ ] name;
            return false;
        }

        _name.assign ( name, length );

        delete [ ] name;

        //  Read the count

        if ( fread ( (void *) & _count, sizeof(_count), 1, in ) != 1 )
            return false;

        //  Read the state

        if ( fread ( (void *) & _state, sizeof(_state), 1, in ) != 1 )
            return false;

        _RPT1 ( _CRT_WARN, "%s\r\n", _name.c_str() );

        return true;
    }
};

typedef std::list<AudioIndex>                   IndexList;
typedef std::list<AudioIndex>::iterator         IndexListIterator;

class ArtistCount
{
public:
    BasicString _artist;
    int         _count;

    ArtistCount ( LPCTSTR artist ) : _artist ( artist ), _count ( 1 ) { ; }
    ArtistCount ( const ArtistCount & rArtist ) : _artist ( rArtist._artist ), _count ( rArtist._count ) { ; }
    ~ArtistCount ( ) { ; }

    void Increment()  { _count++; }
    bool operator== ( const ArtistCount & rArtist )
    {
        return _artist == rArtist._artist;
    }
};

typedef std::vector<ArtistCount>                ArtistVector;
typedef std::vector<ArtistCount>::iterator      ArtistVectorIterator;

typedef std::list<AudioFile>                    AudioList;
typedef std::list<AudioFile>::iterator          AudioListIterator;

typedef std::list<AudioFile*>                   AudioPtrList;
typedef std::list<AudioFile*>::iterator         AudioPtrListIterator;

typedef std::vector<AudioFile*>                 AudioPtrVector;
typedef std::vector<AudioFile*>::iterator       AudioPtrVectorIterator;

enum SortMode { SORT_NONE = 0, SORT_ASCENDING, SORT_DESCENDING };
int CALLBACK CompareItems ( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );
int CALLBACK CompareGenreItems ( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );
int CALLBACK CompareArtistItems ( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );

enum UpdateItem { RPC_UPDATE_LIBARAY = 0, RPG_UPDATE_VERSION };
enum DuplicateMode { LIMIT_REPEATS = 0, ALLOW_DUPLICATES };

class PlaylistGenerator
{
public:
   
    HINSTANCE           _instance;
    HWND                _main_hwnd;
    HWND                _form_hwnd;
    HWND                _sizing_hwnd;
    HANDLE              _logo;

    RPGStatusBar        _status_bar;

	bool				_loading_list_views;
    bool                _update_column_width;
    bool                _auto_update_image;
    bool                _process_multiple_items;

    __int64             _total_tune_size;
    __int64             _total_tune_duration;

    TCHAR               _window_name [ MAX_PATH ];
    TCHAR               _class_name  [ MAX_PATH ];

    typedef struct SortInfo
    {
        SortMode        _sort_mode;
        int             _sort_column;
    } SortInfo;

    SortInfo            _sort_info [ 3 ];

    //  Registry preserved data

    TCHAR               _music_directory [ MAX_PATH ];
    TCHAR               _playlist_directory  [ MAX_PATH ];
    unsigned            _max_tunes;
    __int64             _max_size;
    __int64             _max_duration;
    unsigned            _copy_tunes;
    unsigned            _reminder;
    unsigned            _days;
    unsigned            _aggregate;
    unsigned            _aggregate_size;
    TCHAR               _default_aggregate_name [ MAX_PATH ];
    unsigned            _maintain_selections;
    FILETIME            _last_updated;
    unsigned            _prompt_for_playlist;
    TCHAR               _default_playlist_name [ MAX_PATH ];
    unsigned            _size_mode;
    unsigned            _randomize_manual_selections;
    DuplicateMode       _duplicate_mode;
    unsigned            _repeat_count;
    unsigned            _limit_artist;
    unsigned            _limit_artist_count;
    unsigned            _playlist_mode;    
    unsigned            _splitter_position;
    unsigned            _make_paths_relative;
    unsigned            _log_library;
    unsigned            _updated_version;
    unsigned            _updated_days;
    unsigned            _notify_update;
    FILETIME            _last_updated_version;

    //  Music image data

    AudioList           _audio_list;
    AudioPtrList        _selected_list;

    IndexList           _genre_list;
    IndexList           _aggregate_list;
    IndexList           _artist_list;

    HANDLE              _worker_thread;
    bool                _cancel;

    unsigned            _elapsed_start;

    PlaylistGenerator ( );
    ~PlaylistGenerator ( );

    void Initialize ( HWND hWnd );
    void Uninitialize ( );

    void InitializeLibrary ( );
    void  CleanUp ( );

    DWORD PopulateTunesListView ( );
    DWORD PopulateGenreListView ( );
    DWORD PopulateArtistListView ( );

    void RepopulateTunesListView ( );
    void RepopulateListView ( int ctrl_id, IndexList & list );

    DWORD AddMusicFile ( LPCTSTR file_name );

    DWORD AddGenreToListView ( HWND hWnd, TCHAR * genre );
    DWORD RemoveGenreFromListView ( HWND hWnd, TCHAR * genre );

    DWORD AddArtistToListView ( HWND hWnd, TCHAR * genre );
    DWORD RemoveArtistFromListView ( HWND hWnd, TCHAR * genre );

    //DWORD GeneratePlaylist ( HWND hWnd, TCHAR * playlist );    
    DWORD GeneratePlaylist ( );

    void AddSelected    ( AudioFile * audio_file );
    void RemoveSelected ( AudioFile * audio_file );
    void DeleteTune     ( LVITEM & item );

    void SortGenre ( );
    void SortArtist ( );

    void EnableAggregation ( unsigned state, int size, TCHAR * name );
    void SetAggregateSize  ( unsigned size );
    void SetAggregateName  ( TCHAR * name );

    int GetSortColumn ( int ctrl ) { return _sort_info [ ctrl - IDC_LIST_TUNES]._sort_column; } 
    int SetSortColumn ( int column, int ctrl )
    {
        SortInfo * info = & _sort_info [ ctrl - IDC_LIST_TUNES ];

        int old_column = info->_sort_column;
        info->_sort_column = column;
        return old_column;
    }
    SortMode GetSortMode ( int ctrl ) { return _sort_info [ ctrl - IDC_LIST_TUNES]._sort_mode; } 
    SortMode SetSortMode ( SortMode mode, int ctrl )
    {
        SortInfo * info = & _sort_info [ ctrl - IDC_LIST_TUNES ];

        SortMode old_mode = info->_sort_mode;
        info->_sort_mode = mode;
        return old_mode;
    }
    SortMode ToggleSortMode ( int ctrl )
    {
        SortInfo * info = & _sort_info [ ctrl - IDC_LIST_TUNES ];

        info->_sort_mode = ( info->_sort_mode == SORT_ASCENDING ) ? SORT_DESCENDING : SORT_ASCENDING;
        return info->_sort_mode;
    }

    int SortListView ( LPARAM elem1, LPARAM elem2 );
    int SortGenreListView ( LPARAM elem1, LPARAM elem2 );
    int SortArtistListView ( LPARAM elem1, LPARAM elem2 );

    void ReadPosition ( int & x, int & y, int & width, int & height );    
	bool ReadListView ( TCHAR * key_name, int count, int * widths );

    bool IsTimeToUpdate ( UpdateItem item );
    void UpdateColumnHeader ( HWND hWnd, int ctrl, int column, SortMode mode );

    bool GetVersion ( DWORD * major, DWORD * minor );
    void SaveLastTime ( TCHAR * key, FILETIME updated_time );

    DWORD PlaySelected ( HWND hWnd );

    void ReadLibrary ( );
    void WriteLibrary ( );

private:

    bool IsMPEGAudio ( HANDLE file, AudioFile & audio_file );    
    bool IsWMAAudio ( HANDLE file, AudioFile & audio_file );
    bool IsfLaCAudio ( HANDLE file, AudioFile & audio_file );
    bool IsOggAudio ( HANDLE file, AudioFile & audio_file );
    bool IsMP4Audio ( HANDLE file, AudioFile & audio_file );

    DWORD AddSongToPlaylist ( AudioFile * audio_file, FILE * file, LPCTSTR directory );
    DWORD CopyFileToDirectory ( LPCTSTR file_to_copy, LPCTSTR directory, LPCTSTR stripped_filename );

    DWORD AddTuneToListView ( AudioFile * audio_file, int * widths = NULL );

    void SavePosition ( );
    void SaveListView ( TCHAR * key_name, int count, int ctrl_id );

    void GetColumnHeadingWidth ( HWND hWnd, HWND hWndList, int count, int * widths );
    void UpdateWidth ( HWND hWndList, LPTSTR text, int * width );

    void PreserveAudioListState ( AudioList & list );
    void PreserveIndexListState ( IndexList & list, IndexList & original );

    bool DeleteAudioIndex ( IndexList & index_list, const TCHAR * name, int ctrl );
};

#endif  //  PLAYLIST_GENERATOR_H



