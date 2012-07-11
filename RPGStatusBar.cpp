
#include "PlaylistOptions.h"
#include "RPGStatusBar.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

void RPGStatusBar::CreateProgressBar ( int parts, int * parts_array, int range )
{
    ::SendMessage ( _status_hwnd, SB_SETPARTS, parts, (LPARAM) parts_array );

    //  Create a progess window in the 2nd field of the status window

    RECT rc;
    ::SendMessage ( _status_hwnd, SB_GETRECT, 2, (LPARAM) & rc );

    rc.left     += 3;
    rc.right    -= 6;
    rc.top      += 3;
    rc.bottom   -= 3;

    _progress_hwnd = ::CreateWindowEx ( 0,
                                        PROGRESS_CLASS,
                                        NULL,
                                        WS_VISIBLE | WS_CHILD,
                                        rc.left,
                                        rc.top,
                                        rc.right - rc.left,
                                        rc.bottom - rc.top,
                                        _status_hwnd,
                                        NULL, NULL, NULL );

    ::SendMessage ( _progress_hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, range) );
    ::SendMessage ( _progress_hwnd, PBM_SETPOS, 0, 0 );
}

void RPGStatusBar::DestroyProgressBar ( int parts, int * parts_array )
{
    if ( ::IsWindow ( _progress_hwnd ) )
    {
        ::DestroyWindow ( _progress_hwnd );
        _progress_hwnd = NULL;
    }

    ::SendMessage ( _status_hwnd, SB_SETPARTS, parts, (LPARAM) parts_array );
}

void RPGStatusBar::UpdateProgressBar ( int count )
{
    if ( ::IsWindow ( _progress_hwnd ) )
    {
        ::SendMessage ( _progress_hwnd, PBM_SETPOS, count, 0 );
    }
}

void RPGStatusBar::SetPaneText ( int pane, TCHAR * buffer )
{
    if ( ::IsWindow ( _status_hwnd ) )
    {
        ::SendMessage ( _status_hwnd, SB_SETTEXT, pane, (LPARAM) buffer );
    }
}

void RPGStatusBar::SetPaneDelta ( int delta )
{
    int panes = ::SendMessage ( _status_hwnd, SB_GETPARTS, 0, 0 );

    if ( panes != _parts )
    {
        if ( _parts_array )
            delete [ ] _parts_array;

        _parts          = panes;
        _parts_array    = new int [ _parts ];
    }

    if ( _parts_array )
    {
        ::SendMessage ( _status_hwnd, SB_GETPARTS, _parts, (LPARAM ) _parts_array );

        for ( int i = 0; i < _parts; i++ )
        {
            if ( _parts_array [ i ] != -1 )
            {
                _parts_array [ i ] += delta;
            }                
        }

        ::SendMessage ( _status_hwnd, SB_SETPARTS, _parts, (LPARAM ) _parts_array );
    }

	//	Update progress position

    if ( ::IsWindow ( _progress_hwnd ) )
    {
        RECT rc;
		::SendMessage ( _status_hwnd, SB_GETRECT, 2, (LPARAM) & rc );

        rc.left     += 3;
        rc.right    -= 6;
        rc.top      += 3;
        rc.bottom   -= 3;

		::SetWindowPos ( _progress_hwnd, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
    }
}

void RPGStatusBar::SetBarWidth ( int new_width )
{
    _width = new_width;
}

// End of file
