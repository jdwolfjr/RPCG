
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

DWORD DisplaySystemError ( HWND hWnd )
{
    TCHAR error_buffer [ 512 ], error_message [ 256 ];
    DWORD error = ::GetLastError ();

    DWORD message_length = ::FormatMessage ( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                                             NULL,
                                             error,
                                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                             error_message,
                                             256,
                                             NULL );

    if ( !message_length )
    {
        _stprintf ( error_message, _T("0x%0.8X (message string not found)"), error );
    }

    _stprintf ( error_buffer, _T("The operating system returned the following error: %s"), error_message );

    ::MessageBox ( hWnd, error_buffer,_T("Playlist Creator Error"), MB_ICONERROR );

    return error;
}

BasicString SongTitleFromFilename ( BasicString & fn )
{
    BasicString song = fn;

    BasicString::size_type index = song.find_last_of ( '\\' );
    if ( index != std::string::npos )
    {
        song.erase ( 0, index + 1 );
    }

    //  Remove the file extension if it is of the form ".xxx"

    index = song.rfind ( '.' );
    if ( index != std::string::npos && song.length() - index <= 4 )
    {
        song.erase ( index );
    }

    return song;
}

void EatWhite ( BasicString & fn )
{
    //  Remove leading white space

    unsigned len = fn.length ( );
    if  ( len == 0 )
        return;

    while ( !fn.empty () && _istspace ( fn [ 0 ] ) )
    {
        fn.erase ( 0, 1 );
    }

    len = fn.length ( );
    if  ( len == 0 )
        return;

    //  Remove the trailing white space

    while ( !fn.empty () && _istspace ( fn [ len ] ) )
    {
        fn.erase ( len, 1 );
        len = fn.length ( );
    }

}

std::wstring SingleByteToUnicode ( char * data, unsigned length )
{
    std::wstring out_string = L"";

    wchar_t * temp = new wchar_t [ length + 1 ];
    if ( temp )
    {
        memset ( temp, 0, sizeof(TCHAR) * (length + 1) );

        if ( ::MultiByteToWideChar ( CP_ACP, 0, data, length, temp, length ) != 0 )
            out_string = temp;

        delete [ ] temp;
    }

    return out_string;
}

std::string UnicodeToSingleByte ( wchar_t * data, unsigned length )
{
    std::string out_string = "";

    char * temp = new char [ length + 1 ];
    if ( temp )
    {
        memset ( temp, 0, sizeof(char) * (length + 1) );

        if ( ::WideCharToMultiByte ( CP_ACP, 0, data, length, temp, length, NULL, NULL ) != 0 )
            out_string = temp;

        delete [ ] temp;
    }

    return out_string;
}

// End of File
