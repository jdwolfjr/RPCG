
#ifndef RPG_STATUS_BAR_H
#define RPG_STATUS_BAR_H

class RPGStatusBar
{
public:

    HWND                _status_hwnd;
    HWND                _progress_hwnd;
    int                 _width;

    int                 _parts;
    int               * _parts_array;

    CRITICAL_SECTION    _cs;

    RPGStatusBar ( ) : _status_hwnd ( NULL ), _progress_hwnd ( NULL ), _width ( 0 ), _parts ( 0 ), _parts_array ( NULL )
    {
        ::InitializeCriticalSection ( & _cs );
    }
    ~RPGStatusBar ( )
    { 
        if ( _progress_hwnd )
            ::DestroyWindow ( _progress_hwnd );
        if ( _status_hwnd )
           ::DestroyWindow ( _status_hwnd );
        if ( _parts_array )
            delete [ ] _parts_array;
        ::DeleteCriticalSection ( & _cs );
    }

    void CreateProgressBar ( int parts, int * parts_array, int range );
    void DestroyProgressBar ( int parts, int * parts_array );
    void UpdateProgressBar ( int count );

    void SetPaneText ( int pane, TCHAR * buffer );
    void SetPaneDelta ( int delta );

    void SetBarWidth ( int new_width );
    int  GetBarWidth ( )    { return _width; }

    void Lock ()    { ::EnterCriticalSection ( & _cs ); }
    void Unlock ()  { ::LeaveCriticalSection ( & _cs ); }

};

#endif  //  RPG_STATUS_BAR_H


