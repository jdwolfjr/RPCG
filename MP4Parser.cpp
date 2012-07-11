
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "MP4Parser.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

#define MAKE_ATOM(w,x,y,z) (w | x << 8 | y << 16 | z << 24)

typedef ULONGLONG QWORD;

inline void WORD_SWAP ( void * x )
{
    _asm
    {
        mov     eax, x
        mov     bx, WORD ptr [ eax ]
        xchg    bh, bl
        mov     WORD ptr [ eax ], bx
    }
}

inline void DWORD_SWAP ( void * x )
{
    _asm
    {
        mov     eax, x
        mov     ebx, DWORD ptr [ eax ]
        bswap   ebx
        mov     DWORD ptr [ eax ], ebx
    }
}

inline void QWORD_SWAP ( void * x )
{
    _asm
    {
        mov         eax, x
        movq        mm0, QWORD ptr [ eax ]
        movq        mm1, mm0

        punpckhdq   mm1, mm1
    
        movd        ebx, mm0
        bswap       ebx
        movd        mm0, ebx

        movd        ebx, mm1
        bswap       ebx
        movd        mm1, ebx

        punpckldq   mm1, mm0
        movq        QWORD ptr [ eax ], mm1
    }
}

bool MP4Parser::OpenMP4File ( HANDLE file )
{
    MPEG4_Atom_Header header;
    DWORD bytes_read = 0;

    //  Look for 'ftype' atom    

    ::SetFilePointer ( file, 0, NULL, FILE_BEGIN );

    if ( !::ReadFile ( file, & header, sizeof(MPEG4_Atom_Header), & bytes_read, NULL ) || bytes_read != sizeof(MPEG4_Atom_Header) )
    {
        return false;
    }

    //  Is the 'ftyp' atom  present?

    if ( * ( unsigned * ) header.atom != MAKE_ATOM ('f','t','y','p')  )
    {
        return false;
    }

    //  Go ahead and map the file for parsing

    ::SetFilePointer ( file, 0, NULL, FILE_BEGIN );

    _length = ::GetFileSize ( file, NULL );

    //  Map the file

    _map = ::CreateFileMapping ( file, NULL, PAGE_READONLY, 0, 0, NULL );
    if ( !_map )
    {
        return false;
    }

    _view = ::MapViewOfFile ( _map, FILE_MAP_READ, 0, 0, 0 );
    if ( !_view )
    {
        return false;
    }

    bool is_audio = false;

    MPEG4_Atom_Header * initial_header = (MPEG4_Atom_Header *) _view;
    if ( * ( unsigned * ) & initial_header->atom [ 0 ] == MAKE_ATOM ('f','t','y','p') )
    {
        unsigned length = initial_header->length;
        DWORD_SWAP ( & length );

        if ( length >= 12 )
        {
            char codec [ 5 ];
            ::ZeroMemory ( & codec, sizeof(codec) );

            memcpy ( codec, (BYTE *)(initial_header + 1), 4 );

            if ( stricmp ( codec, "MP41" ) == 0 )
            {
                is_audio = true;
                _type = MP41;
            }
            else
            if ( stricmp ( codec, "MP42" ) == 0 )
            {
                is_audio = true;
                _type = MP42;
            }
            else
            if ( stricmp ( codec, "M4A " ) == 0 )
            {
                is_audio = true;
                _type = M4A;
            }
        }
    }

    return is_audio;
}

BasicString MP4Parser::GetTitle    ( )
{
    BasicString title = _T("");

    MPEG4_IndexListData_Header * header = FindIndexListAtom ( MAKE_ATOM ( 0xA9,'n','a','m') );
    if ( header )
    {
        unsigned length = header->header.length;
        DWORD_SWAP ( & length );

        if ( header->version == 0x00 && header->flags [ 2 ] == 0x01 )
        {
#ifndef _UNICODE
            title.assign ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#else
            title = SingleByteToUnicode ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#endif
        }
    }

    return title;
}

BasicString MP4Parser::GetArtist    ( )
{
    BasicString artist = _T("");

    MPEG4_IndexListData_Header * header = FindIndexListAtom ( MAKE_ATOM (0xA9,'A','R','T') );
    if ( header )
    {
        unsigned length = header->header.length;
        DWORD_SWAP ( & length );

        if ( header->version == 0x00 && header->flags [ 2 ] == 0x01 )
        {
#ifndef _UNICODE
            artist.assign ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#else
            artist = SingleByteToUnicode ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#endif
        }
    }

    return artist;
}

BasicString MP4Parser::GetAlbum    ( )
{
    BasicString album = _T("");

    MPEG4_IndexListData_Header * header = FindIndexListAtom ( MAKE_ATOM (0xA9,'a','l','b') );
    if ( header )
    {
        unsigned length = header->header.length;
        DWORD_SWAP ( & length );

        if ( header->version == 0x00 && header->flags [ 2 ] == 0x01 )
        {
#ifndef _UNICODE
            album.assign ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#else
            album = SingleByteToUnicode ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#endif
        }
    }

    return album;
}

BasicString MP4Parser::GetYear    ( )
{
    BasicString year = _T("");

    MPEG4_IndexListData_Header * header = FindIndexListAtom ( MAKE_ATOM (0xA9,'d','a','y') );
    if ( header )
    {
        unsigned length = header->header.length;
        DWORD_SWAP ( & length );

        if ( header->version == 0x00 && header->flags [ 2 ] == 0x01 )
        {
#ifndef _UNICODE
            year.assign ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#else
            year = SingleByteToUnicode ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#endif
        }
    }

    return year;
}

BasicString MP4Parser::GetTrack    ( )
{
    BasicString track = _T("");

    MPEG4_IndexListData_Header * header = FindIndexListAtom ( MAKE_ATOM ('t','r','k','n') );
    if ( header )
    {
        unsigned length = header->header.length;
        DWORD_SWAP ( & length );

        if ( header->version == 0x00 && header->flags [ 2 ] == 0x00 )
        {
            unsigned track_number = * (unsigned *)(header + 1);
            DWORD_SWAP ( & track_number );

            TCHAR buffer [ MAX_PATH ];
            _stprintf ( buffer, _T("%d"), track_number );

            track.assign ( buffer );
        }
    }

    return track;
}

BasicString MP4Parser::GetGenre    ( int * genre_id )
{
    BasicString genre = _T("");

    * genre_id = -1;

    MPEG4_IndexListData_Header * header = FindIndexListAtom ( MAKE_ATOM ('g','n','r','e') );
    if ( header )
    {
        unsigned length = header->header.length;
        DWORD_SWAP ( & length );

        if ( header->version == 0x00 )
        {
            if ( header->flags [ 2 ] == 0x00 )
            {
                short id = * (short *)(header + 1);
                WORD_SWAP ( & id );

                * genre_id = (id - 1);
            }
            else
            if ( header->flags [ 2 ] == 0x01 )
            {
#ifndef _UNICODE
                genre.assign ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#else
                genre = SingleByteToUnicode ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#endif
            }
        }
    }

    return genre;
}

DWORD MP4Parser::GetDuration ( )
{
    DWORD duration = 0;

    MPEG4_Atom_Header * header = FindAtom ( (BYTE*) _view, _length, MAKE_ATOM ('m','o','o','v') );

    if ( header )
    {
        unsigned length = header->length;
        DWORD_SWAP ( & length );

        header += 1;
        length -= sizeof(header);

        header = FindAtom ( (BYTE *) header, length, MAKE_ATOM ('t','r','a','k') );
        if ( header )
        {
            length = header->length;
            DWORD_SWAP ( & length );

            header += 1;
            length -= sizeof(header);

            header = FindAtom ( (BYTE *) header, length, MAKE_ATOM ('m','d','i','a') );
            if ( header )
            {
                length = header->length;
                DWORD_SWAP ( & length );

                header += 1;
                length -= sizeof(header);

                header = FindAtom ( (BYTE *) header, length, MAKE_ATOM ('m','d','h','d') );
                if ( header )
                {
                    MPEG4_MediaStream_Header * mdhd = (MPEG4_MediaStream_Header *) header;

                    if ( mdhd->version == 0 )
                    {
                        MPEG4_MediaStream_Header mdhd_header;

                        ::CopyMemory ( & mdhd_header, mdhd, sizeof(MPEG4_MediaStream_Header) );
                      
                        DWORD_SWAP ( & mdhd_header.media_unit_time );
                        DWORD_SWAP ( & mdhd_header.media_length );

                         if ( mdhd_header.media_unit_time )
                         {
                             duration = mdhd_header.media_length / mdhd_header.media_unit_time;
                         }
                    }
                    else
                    {
                        MPEG4_MediaStream_Header_Ext mdhd_header;

                        ::CopyMemory ( & mdhd_header, mdhd, sizeof(MPEG4_MediaStream_Header_Ext) );

                        DWORD_SWAP ( & mdhd_header.media_unit_time );
                        QWORD_SWAP ( & mdhd_header.media_length );

                         if ( mdhd_header.media_unit_time )
                         {
                             duration = mdhd_header.media_length / mdhd_header.media_unit_time;
                         }
                    }
                }
            }
        }
    }

    return duration;
}

BasicString MP4Parser::GetCodecName ()
{
    BasicString codec_name = _T("");

    MPEG4_IndexListData_Header * header = FindIndexListAtom ( MAKE_ATOM (0xA9, 't','o','o') );
    if ( header )
    {
        unsigned length = header->header.length;
        DWORD_SWAP ( & length );

        if ( header->version == 0x00 && header->flags [ 2 ] == 0x01 )
        {
#ifndef _UNICODE
            codec_name.assign ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#else
            codec_name = SingleByteToUnicode ( (char *)(header + 1), length - sizeof(MPEG4_IndexListData_Header) );
#endif
        }
    }

    return codec_name;
}

DWORD MP4Parser::GetSampleRate ()
{
    DWORD sample_rate = 0;

    MPEG4_Atom_Header * header = FindAtom ( (BYTE*) _view, _length, MAKE_ATOM ('m','o','o','v') );

    if ( header )
    {
        unsigned length = header->length;
        DWORD_SWAP ( & length );

        header += 1;
        length -= sizeof(header);

        header = FindAtom ( (BYTE *) header, length, MAKE_ATOM ('t','r','a','k') );
        if ( header )
        {
            length = header->length;
            DWORD_SWAP ( & length );

            header += 1;
            length -= sizeof(header);

            header = FindAtom ( (BYTE *) header, length, MAKE_ATOM ('m','d','i','a') );
            if ( header )
            {
                length = header->length;
                DWORD_SWAP ( & length );

                header += 1;
                length -= sizeof(header);

                header = FindAtom ( (BYTE *) header, length, MAKE_ATOM ('m','d','h','d') );
                if ( header )
                {
                    MPEG4_MediaStream_Header * mdhd = (MPEG4_MediaStream_Header *) header;

                    if ( mdhd->version == 0 )
                    {
                        MPEG4_MediaStream_Header mdhd_header;

                        ::CopyMemory ( & mdhd_header, mdhd, sizeof(MPEG4_MediaStream_Header) );
                  
                        DWORD_SWAP ( & mdhd_header.media_unit_time );
                        sample_rate = mdhd_header.media_unit_time;
                    }
                    else
                    {
                        MPEG4_MediaStream_Header_Ext mdhd_header;

                        ::CopyMemory ( & mdhd_header, mdhd, sizeof(MPEG4_MediaStream_Header_Ext) );

                        DWORD_SWAP ( & mdhd_header.media_unit_time );
                        sample_rate = mdhd_header.media_unit_time;
                    }
                }
            }
        }
    }

    return sample_rate;
}

DWORD MP4Parser::GetBitrate ()
{
    DWORD bit_rate = 0;

    return bit_rate;
}

MPEG4_IndexListData_Header * MP4Parser::FindIndexListAtom ( unsigned atom )
{
    unsigned length;
    BYTE * data;

    try
    {
        MPEG4_Atom_Header * header = FindAtom ( (BYTE*) _view, _length, MAKE_ATOM ('m','o','o','v') );

        if ( header )
        {
            length = header->length;
            DWORD_SWAP ( & length );

            data = (BYTE * ) header;

            data    += sizeof(MPEG4_Atom_Header);
            length  -= sizeof(MPEG4_Atom_Header);

            header = FindAtom ( data, length, MAKE_ATOM ('u','d','t','a') );

            if ( header )
            {
                length = header->length;
                DWORD_SWAP ( & length );

                data = (BYTE * ) header;

                data    += sizeof(MPEG4_Atom_Header);
                length  -= sizeof(MPEG4_Atom_Header);

                header = FindAtom ( data, length, MAKE_ATOM ('m','e','t','a') );

                if ( header )
                {
                    length = header->length;
                    DWORD_SWAP ( & length );

                    data = (BYTE * ) header;

                    data    += sizeof(MPEG4_MetaData_Header);
                    length  -= sizeof(MPEG4_MetaData_Header);

                    header = FindAtom ( data, length, MAKE_ATOM ('i','l','s','t') );
                    if ( header )
                    {
                        length = header->length;
                        DWORD_SWAP ( & length );

                        data = (BYTE * ) header;

                        data    += sizeof(MPEG4_Atom_Header);
                        length  -= sizeof(MPEG4_Atom_Header);

                        header = FindAtom ( data, length, atom );
                        if ( header )
                        {
                            data    = (BYTE * ) header;
                            data    += sizeof(MPEG4_Atom_Header);

                            return (MPEG4_IndexListData_Header *) data;
                        }
                    }
                }
            }
        }
    }
    catch ( ... )
    {
    }


    return NULL;
}

MPEG4_Atom_Header * MP4Parser::FindAtom ( BYTE * data, unsigned length, unsigned atom )
{
    __int64 bytes_remaining = length;

    try
    {
        while ( bytes_remaining > 8 )
        {
            MPEG4_Atom_Header * header = (MPEG4_Atom_Header *) data;

            if ( * (unsigned *) & header->atom [ 0 ] == atom )
            {
                return header;
            }
        
            unsigned length_len = header->length;
            DWORD_SWAP ( & length_len );

            data            += length_len;        
            bytes_remaining -= length_len;
        }
    }
    catch ( ... )
    {
    }

    return NULL;
}

// End of File
