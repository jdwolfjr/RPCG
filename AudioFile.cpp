
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "PlaylistGenerator.h"
#include "AudioFile.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

static const DWORD g_frame_id [ ] =
{
    MAKE_FRAME_ID('T','I','T','2'),
    MAKE_FRAME_ID('T','P','E','1'),
    MAKE_FRAME_ID('T','A','L','B'),
    MAKE_FRAME_ID('T','Y','E','R'),
    MAKE_FRAME_ID('T','C','O','M'),
    MAKE_FRAME_ID('T','R','C','K'),
    MAKE_FRAME_ID('T','C','O','N')
};

static const DWORD g_frame_id_v2 [ ] =
{
    MAKE_FRAME_ID('T','T','2','\0'),
    MAKE_FRAME_ID('T','P','1','\0'),
    MAKE_FRAME_ID('T','A','L','\0'),
    MAKE_FRAME_ID('T','Y','E','\0'),
    MAKE_FRAME_ID('C','O','M','\0'),
    MAKE_FRAME_ID('T','R','K','\0'),
    MAKE_FRAME_ID('T','C','O','\0')
};

static const int FRAME_ID_TITLE     = 0;
static const int FRAME_ID_ARTIST    = 1;
static const int FRAME_ID_ALBUM     = 2;
static const int FRAME_ID_YEAR      = 3;
static const int FRAME_ID_COMMENT   = 4;
static const int FRAME_ID_TRACK     = 5;
static const int FRAME_ID_GENRE     = 6;

static LPCTSTR g_genre_tags [ ] = 
{
    _T("Blues"),            _T("Classic Rock"),         _T("Country"),              _T("Dance"),
    _T("Disco"),            _T("Funk"),                 _T("Grunge"),               _T("Hip-Hop"),
    _T("Jazz"),             _T("Metal"),                _T("New Age"),              _T("Oldies"),
    _T("Other"),            _T("Pop"),                  _T("R&B"),                  _T("Rap"),
    _T("Reggae"),           _T("Rock"),                 _T("Techno"),               _T("Industrial"),
    _T("Alternative"),      _T("Ska"),                  _T("Death Metal"),          _T("Pranks"),
    _T("Soundtrack"),       _T("Euro-Techno"),          _T("Ambient"),              _T("Trip-Hop"),
    _T("Vocal"),            _T("Jazz+Funk"),            _T("Fusion"),               _T("Trance"),
    _T("Classical"),        _T("Instrumental"),         _T("Acid"),                 _T("House"),
    _T("Game"),             _T("Sound Clip"),           _T("Gospel"),               _T("Noise"),
    _T("AlternRock"),       _T("Bass"),                 _T("Soul"),                 _T("Punk"),
    _T("Space"),            _T("Meditative"),           _T("Instrumental Pop"),     _T("Instrumental Rock"),
    _T("Ethnic"),           _T("Gothic"),               _T("Darkwave"),             _T("Techno-Industrial"),
    _T("Electronic"),       _T("Pop-Folk"),             _T("Eurodance"),            _T("Dream"),
    _T("Southern Rock"),    _T("Comedy"),               _T("Cult"),                 _T("Gangsta"),
    _T("Top 40"),           _T("Christian Rap"),        _T("Pop/Funk"),             _T("Jungle"),
    _T("Native American"),  _T("Cabaret"),              _T("New Wave"),             _T("Psychadelic"),
    _T("Rave"),             _T("Showtunes"),            _T("Trailer"),              _T("Lo-Fi"),
    _T("Tribal"),           _T("Acid Punk"),            _T("Acid Jazz"),            _T("Polka"),
    _T("Retro"),            _T("Musical"),              _T("Rock & Roll"),          _T("Hard Rock"),

    //  The following genres are Winamp extensions

    _T("Folk"),             _T("Folk-Rock"),            _T("National Folk"),        _T("Swing"),
    _T("Fast Fusion"),      _T("Bebob"),                _T("Latin"),                _T("Revival"),
    _T("Celtic"),           _T("Bluegrass"),            _T("Avantgarde"),           _T("Gothic Rock"),
    _T("Progressive Rock"), _T("Psychedelic Rock"),     _T("Symphonic Rock"),       _T("Slow Rock"),
    _T("Big Band"),         _T("Chorus"),               _T("Easy Listening"),       _T("Acoustic"),
    _T("Humour"),           _T("Speech"),               _T("Chanson"),              _T("Opera"),
    _T("Chamber Music"),    _T("Sonata"),               _T("Symphony"),             _T("Booty Bass"),
    _T("Primus"),           _T("Porn Groove"),          _T("Satire"),               _T("Slow Jam"),
    _T("Club"),             _T("Tango"),                _T("Samba"),                _T("Folklore"),
    _T("Ballad"),           _T("Power Ballad"),         _T("Rhythmic Soul"),        _T("Freestyle"),
    _T("Duet"),             _T("Punk Rock"),            _T("Drum Solo"),            _T("Acapella"),
    _T("Euro-House"),       _T("Dance Hall")
};

static LPCTSTR g_file_type [ ] = { _T(""), _T("MP3"), _T("WMA"), _T("FLAC"), _T("OGG"), _T("MP4"), _T("M4A") };

static int mpeg_bitrate [ 4 ][ 4 ][ 16 ] =      //   [ id ] [ layer ] [ bitrate index ]
{
    {   { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 },          //  MPEG 2.5
        { 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 },
        { 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 },
        { 0,  32,  48,  56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }  },        

    {   { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 },          //  reserved 
        { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 },
        { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }, 
        { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 }  },

    {   { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 },          //  MPEG 2 
        { 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 },
        { 0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 },
        { 0,  32,  48,  56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0 }  },        

    {   { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0 },          //  MPEG 1 
        { 0,  32,  40,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0 },
        { 0,  32,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0 },
        { 0,  32,  64,  96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 }  }
};

static int sample_frequency [ 4 ] [ 4 ] =
{
    { 11025, 12000,  8000, 0 },         //  MPEG 2.5
    {     0,     0,     0, 0 },         //  reserved
    { 22050, 24000, 16000, 0 },         //  MPEG 2
    { 44100, 48000, 32000, 0 }          //  MPEG 1
};

static int samples_per_frame [ 4 ] = { 0, 1152, 1152, 384 };

static TCHAR * mpeg_id           [ ] =  { _T("MPEG Version 2.5"), _T("reserved"), _T("MPEG Version 2 (ISO/IEC 13818-3)"), _T("MPEG Version 1 (ISO/IEC 11172-3)") };
static TCHAR * mpeg_layer        [ ] =  { _T("reserved"), _T("Layer-3"), _T("Layer-2"), _T("Layer-1") };
static TCHAR * mpeg_protection   [ ] =  { _T("Protected by CRC (16bit crc follows header)"), _T("Not protected") };
static TCHAR * mpeg_padding      [ ] =  { _T("Frame is not padded"), _T("Frame is padded with one extra slot") };
static TCHAR * mpeg_private      [ ] =  { _T("Private bit is not set"), _T("Private bit is not set") };
static TCHAR * mpeg_mode         [ ] =  { _T("Stereo"), _T("Joint stereo"), _T("Dual channel"), _T("Single channel") };

//  Mode extension only if joint stereo

static TCHAR * mpeg_mode_ext [ 4 ] [ 4 ] = 
{
    { _T(""), _T(""), _T(""), _T("") },
    { _T("Intensity stereo OFF - MS stereo OFF"), _T("Intensity stereo ON - MS stereo OFF"), _T("Intensity stereo OFF - MS stereo ON"), _T("Intensity stereo ON - MS stereo ON") },
    { _T("Bands 4 to 31"), _T("Bands 8 to 31"), _T("Bands 12 to 31"), _T("Bands 16 to 31") },
    { _T("Bands 4 to 31"), _T("Bands 8 to 31"), _T("Bands 12 to 31"), _T("Bands 16 to 31") }
};

static TCHAR * mpeg_copyright    [ ] =  { _T("Audio is not copyrighted"), _T("Audio is copyrighted") };
static TCHAR * mpeg_original     [ ] =  { _T("Copy of original media"), _T("Original media") };
static TCHAR * mpeg_emphasis     [ ] =  { _T("No emphasis"), _T("50/15 ms emphasis"), _T("reserved"), _T("CCIT J.17 emphasis") };

DWORD AudioFile::ParseID3v2 ( HWND hWnd, HANDLE file, ID3v2 & id3v2 )
{
    DWORD size = ((id3v2.size [ 0 ] & 0x7F) << 21) |
                 ((id3v2.size [ 1 ] & 0x7F) << 14) |
                 ((id3v2.size [ 2 ] & 0x7F) <<  7) |
                  (id3v2.size [ 3 ] & 0x7F);

    DWORD bytes_to_read = sizeof(ID3v2) + size, bytes_read = 0;

    BYTE * buffer = new BYTE [ bytes_to_read ];

    if ( !buffer )
    {
        return ::DisplaySystemError ( hWnd );
    }

    if ( !::ReadFile ( file, buffer, bytes_to_read, & bytes_read, NULL ) || bytes_read != bytes_to_read )
    {
        delete [ ] buffer;
        return ::DisplaySystemError ( hWnd );
    }

    BYTE * current = buffer + sizeof( ID3v2 );

    //  Parse ID3v2.2

    unsigned frame_size;

    if ( id3v2.version [ 0 ] == 0x02 )
    {
        while ( size > sizeof(ID3v2_2_Frame) )
        {
            ID3v2_2_Frame * id3v2_frame = (ID3v2_2_Frame *)current;

            frame_size = (id3v2_frame->size [ 0 ] << 16) |
                         (id3v2_frame->size [ 1 ] <<  8) |
                         (id3v2_frame->size [ 2 ]);

            //  If frame size doesn't make sense

            if ( frame_size == 0 || frame_size > size )
            {
                break;
            }

            //  Gather text frames of interest

            int index = IsID3v22FrameIDInTable ( * (DWORD *) & id3v2_frame->frame_id [ 0 ] );
            if ( index != -1 )
            {
                //  How is the text of this frame encoded?

                BYTE * data =  (BYTE *) & id3v2_frame [ 1 ];

                //  Only supporting ISO-8859-1 encoding for now

                if ( data [ 0 ] == 0x00 )
                {
                    BasicString value = CopyBuffer ( (char *) & data [ 1 ], frame_size - 1 );

                    ::EatWhite ( value );

                    //  Use could code using the ID3v1 genre byte

                    if ( index == FRAME_ID_GENRE )
                    {
                        CleanUpGenre ( value );
                    }

                    if ( value.length () != 0 )
                    {
                        _fm.insert( TextMapValue( g_frame_id [ index ], value) );
                    }
                }

                //  Added support for Unicode 1/08/2007

                else
                if ( data [ 0 ] == 0x01 )
                {
                    DWORD key = * (DWORD *) & id3v2_frame->frame_id;

                    BasicString value = ConvertToISO8859 ( (char *) & data [ 1 ], frame_size - 1 );

                    ::EatWhite ( value );

                    //  Use could code using the ID3v1 genre byte

                    if ( key == g_frame_id [ FRAME_ID_GENRE ] )
                    {
                        CleanUpGenre ( value );
                    }

                    if ( value.length () != 0 )
                    {
                        _fm.insert( TextMapValue( key, value) );
                    }
                }                
            }
            
            //  When we reach the end of the actual data, we might be pointing at padding bytes

            else
            if ( id3v2_frame->frame_id [ 0 ] == 0x00 &&
                 id3v2_frame->frame_id [ 1 ] == 0x00 &&
                 id3v2_frame->frame_id [ 2 ] == 0x00 )
            {
                break;
            }

            frame_size += sizeof(ID3v2_2_Frame);

            current += frame_size;
            size    -= frame_size;
        }
    }
    else
    {
        //  Step over extended header
        
        if ( id3v2.flags [ 0 ] & ID3V2_FLAG_EXTENDED )
        {
            _ASSERT ( 0 );

            if ( id3v2.version [ 0 ] == 0x03 )
            {
                current += sizeof( ID3v2_3_ExtHeader );
            }
            else
            if ( id3v2.version [ 0 ] == 0x04  )
            {
                current += sizeof( ID3v2_4_ExtHeader );
            }
        }

        while ( size > sizeof(ID3v2_3_Frame) )
        {
            ID3v2_3_Frame * id3v2_frame = (ID3v2_3_Frame *)current;

            if ( id3v2.version [ 0 ] == 0x03 )
            {
                frame_size = (id3v2_frame->size [ 0 ] << 24) |
                             (id3v2_frame->size [ 1 ] << 16) |
                             (id3v2_frame->size [ 2 ] <<  8) |
                             (id3v2_frame->size [ 3 ]);
            }
            else
            {
                frame_size = ((id3v2_frame->size [ 0 ] & 0x7F) << 21) |
                             ((id3v2_frame->size [ 1 ] & 0x7F) << 14) |
                             ((id3v2_frame->size [ 2 ] & 0x7F) <<  7) |
                              (id3v2_frame->size [ 3 ] & 0x7F);
            }

            //  If frame size doesn't make sense

            if ( frame_size == 0 || frame_size > size )
            {
                break;
            }

            //  Gather text frames of interest

            if ( IsID3v2FrameIDInTable ( * (DWORD *) & id3v2_frame->frame_id [ 0 ] ) )
            {
                //  How is the text of this frame encoded?

                BYTE * data =  (BYTE *) & id3v2_frame [ 1 ];

                //  Only supporting ISO-8859-1 encoding for now

                if ( data [ 0 ] == 0x00 )
                {
                    DWORD key = * (DWORD *) & id3v2_frame->frame_id;

                    BasicString value = CopyBuffer ( (char *) & data [ 1 ], frame_size - 1 );

                    ::EatWhite ( value );

                    //  Use could code using the ID3v1 genre byte

                    if ( key == g_frame_id [ FRAME_ID_GENRE ] )
                    {
                        CleanUpGenre ( value );
                    }

                    if ( value.length () != 0 )
                    {
                        _fm.insert( TextMapValue( key, value) );
                    }
                }

                //  Added support for Unicode 1/08/2007

                else
                if ( data [ 0 ] == 0x01 )
                {
                    DWORD key = * (DWORD *) & id3v2_frame->frame_id;

                    BasicString value = ConvertToISO8859 ( (char *) & data [ 1 ], frame_size - 1 );

                    ::EatWhite ( value );

                    //  Use could code using the ID3v1 genre byte

                    if ( key == g_frame_id [ FRAME_ID_GENRE ] )
                    {
                        CleanUpGenre ( value );
                    }

                    if ( value.length () != 0 )
                    {
                        _fm.insert( TextMapValue( key, value) );
                    }
                }
            }

            //  When we reach the end of the actual data, we might be pointing at padding bytes

            else
            if ( id3v2_frame->frame_id [ 0 ] == 0x00 &&
                 id3v2_frame->frame_id [ 1 ] == 0x00 &&
                 id3v2_frame->frame_id [ 2 ] == 0x00 &&
                 id3v2_frame->frame_id [ 3 ] == 0x00 )
            {
                break;
            }

            frame_size += sizeof(ID3v2_3_Frame);

            current += frame_size;
            size    -= frame_size;
        }
    }

    //   Use the song file name as the title

    if ( GetTitle ( ) == NULL )
    {
        BasicString title  = ::SongTitleFromFilename ( _fn );        

        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TITLE ], title ) );
    }

    delete [ ] buffer;

    //  Set the file type

    _type = MP3_TYPE;

    return ERROR_SUCCESS;
}

DWORD AudioFile::ParseID3v1 ( ID3v1 & id3v1 )
{
    BasicString value;
    bool title_valid = false;

    //  The nice thing about maps is if the key value already exists,
    //  the insert will not automatically over write.

    //  Insert title

    if ( id3v1.title [ 0 ] != '\0' )
    {
        value = CopyBuffer ( (char *) & id3v1.title [ 0 ], sizeof(id3v1.title) );

        ::EatWhite ( value );
    }

    //   Use the song file name as the title

    if ( value.empty () )
    {
        value = ::SongTitleFromFilename ( _fn );
    }
    else
    {
        title_valid = true;
    }

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TITLE ], value) );

    //  Insert artist

    if ( id3v1.artist [ 0 ] != '\0' )
    {
        value = CopyBuffer ( (char *) & id3v1.artist [ 0 ], sizeof(id3v1.artist) );
        if ( !value.empty () )
        {
            _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ARTIST ], value) );
        }
    }

    //  Insert album

    if ( id3v1.album [ 0 ] != '\0' )
    {
        value = CopyBuffer ( (char *) & id3v1.album [ 0 ], sizeof(id3v1.album) );
        if ( !value.empty () )
        {
            _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ALBUM ], value) );
        }
    }

    //  Insert year

    if ( id3v1.year [ 0 ] != '\0' )
    {
        value = CopyBuffer ( (char *) & id3v1.year [ 0 ], sizeof(id3v1.year) );
        if ( !value.empty () )
        {
            _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_YEAR ], value) );
        }
    }

    //  We differentiate between ID3v1 and ID3v11 by NULL termination of the comment string

    if ( id3v1.v11.track [ 0 ] != '\0' )
    {
        //  Insert comment
        
        value = CopyBuffer ( (char *) & id3v1.comment_v10 [ 0 ], sizeof(id3v1.comment_v10) );
        if ( !value.empty () )
        {
            _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_COMMENT ], value) );
        }
    }
    else
    {
        //  Insert comment

        if ( id3v1.v11.comment_v11 [ 0 ] != '\0' )
        {
            value = CopyBuffer ( (char *) & id3v1.v11.comment_v11 [ 0 ], sizeof(id3v1.v11.comment_v11) );
            if ( !value.empty () )
            {
                _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_COMMENT ], value) );
            }
        }

        //  Insert track (assuming track 0 would be invalid)

        if ( id3v1.v11.track [ 1 ] != 0x00 )
        {
            TCHAR buffer [ 16 ];
            _stprintf ( buffer, _T("%d"), id3v1.v11.track [ 1 ] );

            _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TRACK ], BasicString(buffer)) );
        }

    }

    //  Insert genre

    BYTE index = id3v1.genre [ 0 ];

    //  Use a simple metric to determine if a genre value 0x00 ('blues') makes sense

    if ( !title_valid && index == 0x00 )
        index = 0xFF;

    value = ( index <= 125 ) ? g_genre_tags [ index ] : _T("Unknown");
    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_GENRE ], value) );

    //  Set the file type

    _type = MP3_TYPE;

    return ERROR_SUCCESS;
}

DWORD AudioFile::ParseWMA   ( HWND hWnd, WMAParser & parser )
{
    BasicString temp, value;
    bool title_valid = false;

    if ( !parser.IsAudio ( ) )
        return ERROR_UNSUPPORTED_TYPE;

    //  Insert title

    temp = parser.GetTitle ( );

    ::EatWhite ( temp );

    //   Use the song file name as the title

    if ( temp.empty() )
    {
        value = ::SongTitleFromFilename ( _fn );
    }
    else
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        title_valid = true;
    }

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TITLE ], value) );

    //  Insert artist

    temp = parser.GetArtist ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ARTIST ], value) );
    }

    //  Insert album

    temp = parser.GetAlbum ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ALBUM ], value) );
    }

    //  Insert year

    temp = parser.GetYear ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_YEAR ], value) );
    }

    //  Insert track

    temp = parser.GetTrack ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TRACK ], value) );
    }

    //  Insert genre

    temp = parser.GetGenre ( );

    value = ( !temp.empty () ) ? CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () ) : _T("Unknown");

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_GENRE ], value) );

    //  Duration

    _duration = parser.GetDuration ();

    //  Set the file type

    _type = WMA_TYPE;

    return ERROR_SUCCESS;
}

DWORD AudioFile::ParsefLaC  ( HWND hWnd, fLaCParser & parser )
{
    BasicString temp, value;
    bool title_valid = false;

    //  Insert title

    temp = parser.GetTitle ( );

    ::EatWhite ( temp );

    //   Use the song file name as the title

    if ( temp.empty() )
    {
        value = ::SongTitleFromFilename ( _fn );
    }
    else
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        title_valid = true;
    }

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TITLE ], value) );

    //  Insert artist

    temp = parser.GetArtist ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ARTIST ], value) );
    }

    //  Insert album

    temp = parser.GetAlbum ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ALBUM ], value) );
    }

    //  Insert year

    temp = parser.GetYear ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_YEAR ], value) );
    }

    //  Insert track

    temp = parser.GetTrack ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TRACK ], value) );
    }

    //  Insert genre

    temp = parser.GetGenre ( );

    value = ( !temp.empty () ) ? CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () ) : _T("Unknown");

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_GENRE ], value) );

    //  Duration

    _duration = parser.GetDuration ();

    //  Set the file type

    _type = FLAC_TYPE;

    return ERROR_SUCCESS;
}

DWORD AudioFile::ParseOgg  ( HWND hWnd, OggParser & parser )
{
    BasicString temp, value;
    bool title_valid = false;

    //  Insert title

    temp = parser.GetTitle ( );

    ::EatWhite ( temp );

    //   Use the song file name as the title

    if ( temp.empty() )
    {
        value = ::SongTitleFromFilename ( _fn );
    }
    else
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        title_valid = true;
    }

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TITLE ], value) );

    //  Insert artist

    temp = parser.GetArtist ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ARTIST ], value) );
    }

    //  Insert album

    temp = parser.GetAlbum ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ALBUM ], value) );
    }

    //  Insert year

    temp = parser.GetYear ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_YEAR ], value) );
    }

    //  Insert track

    temp = parser.GetTrack ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TRACK ], value) );
    }

    //  Insert genre

    temp = parser.GetGenre ( );

    value = ( !temp.empty () ) ? CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () ) : _T("Unknown");

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_GENRE ], value) );

    //  Duration

    _duration = parser.GetDuration ();

    //  Set the file type

    _type = OGG_TYPE;

    return ERROR_SUCCESS;
}

DWORD AudioFile::ParseMP4  ( HWND hWnd, MP4Parser & parser )
{
    BasicString temp, value;
    bool title_valid = false;

    //  Insert title

    temp = parser.GetTitle ( );

    ::EatWhite ( temp );

    //   Use the song file name as the title

    if ( temp.empty() )
    {
        value = ::SongTitleFromFilename ( _fn );
    }
    else
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        title_valid = true;
    }

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TITLE ], value) );

    //  Insert artist

    temp = parser.GetArtist ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ARTIST ], value) );
    }

    //  Insert album

    temp = parser.GetAlbum ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_ALBUM ], value) );
    }

    //  Insert year

    temp = parser.GetYear ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_YEAR ], value) );
    }

    //  Insert track

    temp = parser.GetTrack ( );

    if ( !temp.empty () )
    {
        value = CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () );
        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TRACK ], value) );
    }

    //  Insert genre

    int genre_id;

    temp = parser.GetGenre ( & genre_id );

    value = ( !temp.empty () ) ? CopyBuffer ( (TCHAR *) temp.c_str (), temp.length () ) : _T("Unknown");

    if ( genre_id != -1 )
    {
        value = ( genre_id < sizeof(g_genre_tags) / sizeof(g_genre_tags [ 0 ] ) ) ? g_genre_tags [ genre_id ] : _T("Unknown");
    }

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_GENRE ], value) );

    //  Duration

    _duration = parser.GetDuration ();

    //  Set the file type

    _type = ( parser._type == M4A ) ? M4A_TYPE : MP4_TYPE;

    return ERROR_SUCCESS;
}

DWORD AudioFile::ParseMP3Frames ( HANDLE file )
{
    DWORD audio_frame_count = 0;
    DWORD bytes_parsed      = 0;

    int frame_length, audio_bitrate, audio_sample_rate = 0, audio_layer = 0;

#define BUFFERED_METHOD
#ifdef BUFFERED_METHOD

    BYTE buffer [ 4096 ];
    DWORD read;

    if ( !::ReadFile ( file, buffer, 4096, & read, NULL ) || read != sizeof(buffer) )
    {
        return ERROR_BAD_FORMAT;
    }

    //  Idea would be to see if we can sync on the audio sync word

    BYTE * file_data = buffer;
    DWORD file_length = sizeof(buffer);

    while ( file_length > 4 && bytes_parsed < sizeof(buffer) && audio_frame_count < 5 )
    {
        DWORD frame_header = file_data [ 0 ] << 24 | file_data [ 1 ] << 16 | file_data [ 2 ] << 8 | file_data [ 3 ];                

        if ( (frame_header & 0xFFE00000) == 0xFFE00000 )
        {
            BYTE id             = (frame_header & 0x00180000) >> 19;
            BYTE layer          = (frame_header & 0x00060000) >> 17;                    
            BYTE bitrate_index  = (frame_header & 0x0000F000) >> 12;
            BYTE sampling       = (frame_header & 0x00000C00) >> 10;
            BYTE padding        = (frame_header & 0x00000200) >>  9;

            bytes_parsed += 4;

            //  Validate header

            if ( id == 1 || layer == 0 || bitrate_index == 0 || bitrate_index == 15 || sampling == 3 )
            {
                //  Framing error

                file_data    ++;
                file_length  --;

                audio_frame_count = 0;

                continue;
            }

            //  Nothing obviously wrong with header so see if we can step to the next frame

            audio_bitrate     = mpeg_bitrate [ id ] [ layer ] [ bitrate_index ] * 1000;
            audio_sample_rate = sample_frequency [ id ][ sampling ];
            audio_layer       = layer;

            switch ( audio_layer )
            {
                case 1:     // layer-3
                case 2:     // layer-2

                    frame_length = ( ( 144 * audio_bitrate ) / audio_sample_rate ) + padding;

                    break;

                case 3:     // layer-1

                    frame_length = (12 * audio_bitrate / audio_sample_rate + padding) * 4;

                    break;
            }

            //  If we don't have enough data left, then we are done

            if ( frame_length > file_length )
            {
                break;
            }

            //  Looks valid at this point

            audio_frame_count ++;

            file_data    += frame_length;
            file_length  -= frame_length;

            bytes_parsed += frame_length;
        }
        else
        {
            bytes_parsed ++;
            file_data    ++;
            file_length  --;
        }
    }

#else

    HANDLE map = ::CreateFileMapping ( file, NULL, PAGE_READONLY, 0, 0, NULL );
    if ( map != INVALID_HANDLE_VALUE )
    {
        void * view = ::MapViewOfFile ( map, FILE_MAP_READ, 0, 0, 0 );
        if ( view )
        {
            BYTE * file_data = (BYTE *) view;
            DWORD file_length = _fs;          

            //  Idea would be to see if we can sync on the audio sync word
            
            while ( file_length > 4 && bytes_parsed < 4096 && audio_frame_count < 5 )
            {
                DWORD frame_header = file_data [ 0 ] << 24 | file_data [ 1 ] << 16 | file_data [ 2 ] << 8 | file_data [ 3 ];                

                if ( (frame_header & 0xFFE00000) == 0xFFE00000 )
                {
                    BYTE id             = (frame_header & 0x00180000) >> 19;
                    BYTE layer          = (frame_header & 0x00060000) >> 17;                    
                    BYTE bitrate_index  = (frame_header & 0x0000F000) >> 12;
                    BYTE sampling       = (frame_header & 0x00000C00) >> 10;
                    BYTE padding        = (frame_header & 0x00000200) >>  9;

                    bytes_parsed += 4;

                    //  Validate header

                    if ( id == 1 || layer == 0 || bitrate_index == 0 || bitrate_index == 15 || sampling == 3 )
                    {
                        //  Framing error

                        file_data    ++;
                        file_length  --;

                        audio_frame_count = 0;

                        continue;
                    }

                    //  Nothing obviously wrong with header so see if we can step to the next frame

                    audio_bitrate     = mpeg_bitrate [ id ] [ layer ] [ bitrate_index ] * 1000;
                    audio_sample_rate = sample_frequency [ id ][ sampling ];
                    audio_layer       = layer;

                    switch ( audio_layer )
                    {
                        case 1:     // layer-3
                        case 2:     // layer-2

                            frame_length = ( ( 144 * audio_bitrate ) / audio_sample_rate ) + padding;

                            break;

                        case 3:     // layer-1

                            frame_length = (12 * audio_bitrate / audio_sample_rate + padding) * 4;

                            break;
                    }

                    //  If we don't have enough data left, then we are done

                    if ( frame_length > file_length )
                    {
                        break;
                    }

                    //  Looks valid at this point

                    audio_frame_count ++;

                    file_data    += frame_length;
                    file_length  -= frame_length;

                    bytes_parsed += frame_length;
                }
                else
                {
                    bytes_parsed ++;
                    file_data    ++;
                    file_length  --;
                }
            }

            ::UnmapViewOfFile ( view );
        }

        ::CloseHandle ( map );
    }

#endif  //  BUFFERED_METHOD

    if (audio_frame_count != 5)
    {
        return ERROR_BAD_FORMAT;
    }

   //   Use the song file name as the title

    if ( GetTitle ( ) == NULL )
    {
        BasicString title  = ::SongTitleFromFilename ( _fn );        

        _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_TITLE ], title ) );
    }

    //  Set the file type

    _type = MP3_TYPE;

    //  Set the genre

    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_GENRE ], _T("Unknown") ) );

    return ERROR_SUCCESS;
}

LPCTSTR AudioFile::GetTitle ()
{
    TextMapIterator iter = _fm.find ( g_frame_id [ FRAME_ID_TITLE ] );
    if ( iter == _fm.end () )
        return NULL;

    return (iter->second).c_str ();
}

LPCTSTR AudioFile::GetArtist ()
{
    TextMapIterator iter = _fm.find ( g_frame_id [ FRAME_ID_ARTIST ] );
    if ( iter == _fm.end () )
        return NULL;

    return (iter->second).c_str ();
}

LPCTSTR AudioFile::GetAlbum ()
{
    TextMapIterator iter = _fm.find ( g_frame_id [ FRAME_ID_ALBUM ] );
    if ( iter == _fm.end () )
        return NULL;

    return (iter->second).c_str ();
}

LPCTSTR AudioFile::GetYear ()
{
    TextMapIterator iter = _fm.find ( g_frame_id [ FRAME_ID_YEAR ] );
    if ( iter == _fm.end () )
        return NULL;

    return (iter->second).c_str ();
}

LPCTSTR AudioFile::GetTrack ()
{
    TextMapIterator iter = _fm.find ( g_frame_id [ FRAME_ID_TRACK ] );
    if ( iter == _fm.end () )
        return NULL;

    return (iter->second).c_str ();
}

LPCTSTR AudioFile::GetGenre ()
{
    TextMapIterator iter = _fm.find ( g_frame_id [ FRAME_ID_GENRE ] );
    if ( iter == _fm.end () )
        return NULL;

    return (iter->second).c_str ();
}

LPCTSTR AudioFile::GetFileType ()
{
    if ( _type < UNKNOWN_TYPE || _type > M4A_TYPE )
        return g_file_type [ UNKNOWN_TYPE ];

    return g_file_type [ _type ];
}

void AudioFile::AddGenre ( LPCTSTR genre )
{
    _fm.insert( TextMapValue( g_frame_id [ FRAME_ID_GENRE ], BasicString(genre) ) );
}

bool AudioFile::Read ( FILE * in, DWORD major, DWORD minor )
{
    DWORD length, count;

    //  Length of file name

    if ( fread ( & length, sizeof(length), 1, in ) != 1 )
        return false;

    TCHAR * name = new TCHAR [ length ];
    if ( !name )
        return false;

    //  File name

    if ( fread ( (void *) name, length * sizeof(TCHAR), 1, in ) != 1 )
    {
        delete [ ] name;
        return false;
    }

    _fn.assign ( name, length );

    delete [ ] name;

    //  File size

    if ( fread ( & _fs, sizeof(_fs), 1, in ) != 1 )
        return false;

    //  Selection state

    if ( fread ( & _state, sizeof(_state), 1, in ) != 1 )
        return false;

    //  File type added for version 1.1

    if ( fread ( & _type, sizeof(_type), 1, in ) != 1 )
        return false;

    //  Duration

    if ( fread ( & _duration, sizeof(_duration), 1, in ) != 1 )
        return false;

    //  Read the play count starting with version 1.3

    if ( major > 1 || major == 1 && minor >= 3 )
    {
        if ( fread ( & _repeat_count, sizeof(_repeat_count), 1, in ) != 1 )
            return false;
    }

    //  Number of text elements

    if ( fread ( & count, sizeof(count), 1, in ) != 1 )
        return false;

    while ( count )
    {
        DWORD index;
        BasicString value;

        //  Element index

        if ( fread ( & index, sizeof(index), 1, in ) != 1 )
            return false;

        //  Length of element text

        if ( fread ( & length, sizeof(length), 1, in ) != 1 )
            return false;

        name = new TCHAR [ length ];
        if ( !name )
            return false;

        //  Element text

        if ( fread ( (void *) name, length * sizeof(TCHAR), 1, in ) != 1 )
        {
            delete [ ] name;
            return false;
        }

        value.assign ( name, length );

        delete [ ] name;

        _fm.insert ( AudioFile::TextMapValue ( index, value ) );

        count --;
    }

#ifndef _UNICODE
    _RPT1 ( _CRT_WARN, "%s\r\n", _fn.c_str() );
#endif

    return true;
}

void AudioFile::Write ( FILE * out )
{
    //  Length of file name

    BasicString::size_type length = _fn.length ();

    //  File name

    fwrite ( & length, sizeof( length ), 1, out );
    fwrite ( _fn.c_str(), length * sizeof(TCHAR), 1, out );

    //  File size

    fwrite ( & _fs, sizeof (_fs), 1, out );

    //  Set the state

    fwrite ( & _state, sizeof (_state), 1, out );

    //  File type added for version 1.1

    fwrite ( & _type, sizeof (_type), 1, out );

    //  Duration

    fwrite ( & _duration, sizeof(_duration), 1, out );

    //  Play count

    fwrite ( & _repeat_count, sizeof(_repeat_count), 1, out );

    //  Number of text elements

    BasicString::size_type count = _fm.size();
    fwrite ( & count, sizeof (count), 1, out );

    if ( count )
    {
        AudioFile::TextMapIterator text_begin = _fm.begin(), text_end = _fm.end ();
        while ( text_begin != text_end )
        {
            fwrite ( & text_begin->first, sizeof( text_begin->first ), 1, out );

            length = (text_begin->second).length ();
            fwrite ( & length, sizeof( length ), 1, out );
            fwrite ( (text_begin->second).c_str(), length * sizeof(TCHAR), 1, out );

            text_begin ++;
        }
    }
}

void AudioFile::CalculateDuration ( HANDLE file, ID3v1 & id3v1, ID3v2 & id3v2 )
{
    //  Counting frames is slow but ACCURATE!

    DWORD file_length = _fs;

    //  If ID3v1 tag is present, remove the size from the total length

    if ( id3v1.tag [ 0 ] == 'T' && id3v1.tag [ 1 ] == 'A' && id3v1.tag [ 2 ] == 'G' )
        file_length -= 128;

    //  If ID3v2 tag is present, we offset into the file by the header size

    DWORD bytes_to_read = 0;

    if ( id3v2.tag [ 0 ] == 'I' && id3v2.tag [ 1 ] == 'D' && id3v2.tag [ 2 ] == '3' )
    {
        DWORD size = ((id3v2.size [ 0 ] & 0x7F) << 21) |
                     ((id3v2.size [ 1 ] & 0x7F) << 14) |
                     ((id3v2.size [ 2 ] & 0x7F) <<  7) |
                      (id3v2.size [ 3 ] & 0x7F);

        bytes_to_read = sizeof(ID3v2) + size;
    }

    HANDLE map = ::CreateFileMapping ( file, NULL, PAGE_READONLY, 0, 0, NULL );
    if ( map != INVALID_HANDLE_VALUE )
    {
        void * view = ::MapViewOfFile ( map, FILE_MAP_READ, 0, 0, 0 );
        if ( view )
        {
            BYTE * file_data = (BYTE *) view;
            DWORD file_length = _fs;

            file_data   += bytes_to_read;
            file_length -= bytes_to_read;
            
            DWORD audio_frame_count = 0;
            int frame_length, audio_bitrate, audio_sample_rate = 0, audio_layer = 0;

            while ( file_length > 4 )
            {
                DWORD frame_header = file_data [ 0 ] << 24 | file_data [ 1 ] << 16 | file_data [ 2 ] << 8 | file_data [ 3 ];
                if ( (frame_header & 0xFFE00000) == 0xFFE00000 )
                {
                    BYTE id             = (frame_header & 0x00180000) >> 19;
                    BYTE layer          = (frame_header & 0x00060000) >> 17;                    
                    BYTE bitrate_index  = (frame_header & 0x0000F000) >> 12;
                    BYTE sampling       = (frame_header & 0x00000C00) >> 10;
                    BYTE padding        = (frame_header & 0x00000200) >>  9;

                    //  Validate header

                    if ( id == 1 || layer == 0 || bitrate_index == 0 || bitrate_index == 15 || sampling == 3 )
                    {
                        //  Framing error

                        file_data    ++;
                        file_length  --;                        

                        continue;
                    }

                    audio_bitrate     = mpeg_bitrate [ id ] [ layer ] [ bitrate_index ] * 1000;
                    audio_sample_rate = sample_frequency [ id ][ sampling ];
                    audio_layer       = layer;

                    switch ( audio_layer )
                    {
                        case 1:     // layer-3
                        case 2:     // layer-2

                            frame_length = ( ( 144 * audio_bitrate ) / audio_sample_rate ) + padding;

                            break;

                        case 3:     // layer-1

                            frame_length = (12 * audio_bitrate / audio_sample_rate + padding) * 4;

                            break;
                    }

                    //  If we don't have enough data left, then we are done

                    if ( frame_length > file_length )
                    {
                        break;
                    }

                    //  Looks valid at this point

                    audio_frame_count ++;

                    file_data    += frame_length;
                    file_length  -= frame_length;

                    continue;

                    break;
                }
                else
                {
                    file_data    ++;
                    file_length  --;
                }
            }

            //  Determine duration from values extracted from last frame header

            if ( audio_frame_count != 0 && audio_sample_rate != 0 && audio_layer != 0 )
            {
                long total_samples = audio_frame_count * samples_per_frame [ audio_layer ];
                
                _duration = (float)total_samples / (float)audio_sample_rate;

                _RPT2 ( _CRT_WARN, "Duration of tune %s is %lu seconds\r\n", _fn.c_str(), _duration );
            }

            ::UnmapViewOfFile ( view );
        }

        ::CloseHandle ( map );
    }
}

void AudioFile::PopulateAudioInformation ( HWND hWnd )
{
    TCHAR buffer [ MAX_PATH ];

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("Audio Info:") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("----------") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );

    _stprintf ( buffer, _T("Title:\t%s"), GetTitle() );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Artist:\t%s"), GetArtist() );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Album:\t%s"), GetAlbum() );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Year:\t%s"), GetYear() );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Track:\t%s"), GetTrack() );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Genre:\t%s"), GetGenre () );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );
   
    _stprintf ( buffer, _T("State:\t%s"), (_state == BST_CHECKED) ? _T("Selected") : _T("Not selected") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );
    
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("File Info:") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("----------") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );

    _stprintf ( buffer, _T("Name:\t%s"), _fn.c_str ( ) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Size:\t%lu"), _fs );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer ); 

    _stprintf ( buffer, _T("Type:\t%s"), GetFileType ( ) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );
    
    _stprintf ( buffer, _T("Duration:\t%0.1d:%0.2d"), (_duration / 60), (_duration % 60) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );

    switch ( _type )
    {
        case MP3_TYPE:

            GetMP3SpecificInfo ( hWnd );

            break;

        case WMA_TYPE:

            GetWMASpecificInfo ( hWnd );

            break;

        case FLAC_TYPE:

            GetfLaCSpecificInfo ( hWnd );

            break;

        case OGG_TYPE:

            GetOggSpecificInfo ( hWnd );

            break;

        case MP4_TYPE:
        case M4A_TYPE:

            GetMP4SpecificInfo ( hWnd );

            break;
    }
}

bool AudioFile::IsID3v2FrameIDInTable ( DWORD frame_id )
{
    for ( int i = 0; i < sizeof(g_frame_id) / sizeof(g_frame_id [ 0 ]); i++ )
    {
        if ( g_frame_id [ i ] == frame_id )
            return true;
    }

    return false;
}


int AudioFile::IsID3v22FrameIDInTable ( DWORD frame_id )
{
    BYTE * temp = (BYTE *) & frame_id;
    temp [ 3 ] = 0x00;

    for ( int i = 0; i < sizeof(g_frame_id_v2) / sizeof(g_frame_id_v2 [ 0 ]); i++ )
    {
        if ( g_frame_id_v2 [ i ] == frame_id )
            return i;
    }

    return -1;
}

BasicString AudioFile::CopyBuffer ( char *  buffer, unsigned length )
{
    BasicString data;
    unsigned count = 0, trailing = length - 1;

    //  Strip any leading white space

    while ( count < length )
    {
        if ( buffer [ count ] != NULL && !isspace ( buffer [ count ] ) )
             break;

        count++;
    }

    //  If buffer is empty

    if ( count == length )
        return data;

    //  Remove any trailing white space

    while ( trailing != count )
    {
        if ( buffer [ trailing ] != NULL && !isspace ( buffer [ trailing ] ) )
             break;

        trailing--;
    }

#ifndef _UNICODE     
    data.assign ( (char * ) & buffer [ count ], trailing + 1 );
#else
    data = ::SingleByteToUnicode ( (char * ) & buffer [ count ], trailing + 1 );
#endif

    return data;
}

BasicString AudioFile::CopyBuffer ( wchar_t *  buffer, unsigned length )
{
    BasicString data;
    unsigned count = 0, trailing = length - 1;

    //  Strip any leading white space

    while ( count < length )
    {
        if ( buffer [ count ] != NULL && !iswspace ( buffer [ count ] ) )
             break;

        count++;
    }

    //  If buffer is empty

    if ( count == length )
        return data;

    //  Remove any trailing white space

    while ( trailing != count )
    {
        if ( buffer [ trailing ] != NULL && !iswspace ( buffer [ trailing ] ) )
             break;

        trailing--;
    }

#ifdef _UNICODE     
    data.assign ( (wchar_t *)& buffer [ count ], trailing + 1 );
#else
    data = ::UnicodeToSingleByte ( (wchar_t * ) & buffer [ count ], trailing + 1 );
#endif

    return data;
}

BasicString AudioFile::ConvertToISO8859 ( char *  buffer, unsigned length )
{
    BasicString iso_8859_1;
    char * data = buffer + 2;
    unsigned data_len = length - 2;

    if ( (char) buffer [ 0 ] == (char) 0xFF && (char) buffer [ 1 ] == (char) 0xFE )
    {
        for ( unsigned i = 0; i < data_len; i += 2 )
        {
            iso_8859_1.append ( (TCHAR *) & data [ i + 0 ] );
        }
    }
    else
    if ( (char) buffer [ 0 ] == (char) 0xFE && (char) buffer [ 1 ] == (char) 0xFF )
    {
        for ( unsigned i = 0; i < data_len; i += 2 )
        {
            iso_8859_1.append ( (TCHAR *) & data [ i + 1 ] );
        }
    }

    return iso_8859_1;    
}

void AudioFile::CleanUpGenre ( BasicString & value )
{
    BasicString::size_type offset = value.find ( _T("") );

    if ( offset != BasicString::npos )
    {
        int code = _ttoi ( (LPCTSTR) & value [ offset + 1 ] );
        if ( code <= 125 )
            value = g_genre_tags [ code ];
    }
    else
    if ( value.length () != 0 )
    {
        bool replace_next = true;

        // Make sure all words are capitalized

        for ( offset = 0; offset < value.length(); offset ++ )
        {
            if ( isalpha( value [ offset ] ) && replace_next )
            {
                value.replace ( offset, 1, 1, toupper ( value [ offset ] ) );
                replace_next = false;
            }
            else
            if ( isspace( value [ offset ] ) )
            {
                replace_next = true;
            }
        }

    }
}

void AudioFile::GetMP3SpecificInfo ( HWND hWnd )
{
    DWORD mpeg_header = 0, bytes_read;
    ID3v2 id3v2;

    ::ZeroMemory ( & id3v2, sizeof(id3v2) );

    HANDLE file = ::CreateFile ( _fn.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, OPEN_EXISTING, NULL );
    if ( file == INVALID_HANDLE_VALUE )
        return;

    if ( !::ReadFile ( file, (void *) & id3v2, sizeof(id3v2), & bytes_read, NULL ) || bytes_read != sizeof(id3v2) )
    {
        ::CloseHandle ( file );
        return;
    }

    //  If ID3v2 tag is present, we offset into the file by the header size

    DWORD bytes_to_read = 0;

    if ( id3v2.tag [ 0 ] == 'I' && id3v2.tag [ 1 ] == 'D' && id3v2.tag [ 2 ] == '3' )
    {
        DWORD size = ((id3v2.size [ 0 ] & 0x7F) << 21) |
                     ((id3v2.size [ 1 ] & 0x7F) << 14) |
                     ((id3v2.size [ 2 ] & 0x7F) <<  7) |
                      (id3v2.size [ 3 ] & 0x7F);

        bytes_to_read = sizeof(ID3v2) + size;
    }

    HANDLE map = ::CreateFileMapping ( file, NULL, PAGE_READONLY, 0, 0, NULL );
    if ( map != INVALID_HANDLE_VALUE )
    {
        void * view = ::MapViewOfFile ( map, FILE_MAP_READ, 0, 0, 0 );
        if ( view )
        {
            BYTE * file_data = (BYTE *) view;
            DWORD file_length = _fs;

            file_data   += bytes_to_read;
            file_length -= bytes_to_read;
            
            while ( file_length > 4 )
            {
                DWORD frame_header = file_data [ 0 ] << 24 | file_data [ 1 ] << 16 | file_data [ 2 ] << 8 | file_data [ 3 ];
                if ( (frame_header & 0xFFE00000) == 0xFFE00000 )
                {
                    BYTE id             = (frame_header & 0x00180000) >> 19;
                    BYTE layer          = (frame_header & 0x00060000) >> 17;
                    BYTE bitrate_index  = (frame_header & 0x0000F000) >> 12;
                    BYTE sampling       = (frame_header & 0x00000C00) >> 10;

                    //  Validate header

                    if ( id != 1 && layer != 0 && bitrate_index != 15 && sampling != 3 )
                    {
                        mpeg_header = frame_header;
                        break;
                    }
                }
                else
                {
                    file_data   ++;
                    file_length --;
                }
            }
        }
    }

    ::CloseHandle ( file );

    if ( mpeg_header != 0 )
    {
        TCHAR buffer [ MAX_PATH ];

        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("MPEG Specific Info:") );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("----------") );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );

        BYTE id             = (mpeg_header & 0x00180000) >> 19;
        BYTE layer          = (mpeg_header & 0x00060000) >> 17;
        BYTE protection     = (mpeg_header & 0x00010000) >> 16;
        BYTE bitrate_index  = (mpeg_header & 0x0000F000) >> 12;
        BYTE sampling       = (mpeg_header & 0x00000C00) >> 10;
        BYTE padding        = (mpeg_header & 0x00000200) >>  9;
        BYTE private_bit    = (mpeg_header & 0x00000100) >>  8;
        BYTE mode           = (mpeg_header & 0x000000C0) >>  6;
        BYTE mode_ext       = (mpeg_header & 0x00000030) >>  4;
        BYTE copyright      = (mpeg_header & 0x00000008) >>  3;
        BYTE orig_copy      = (mpeg_header & 0x00000004) >>  2;
        BYTE emphasis       = (mpeg_header & 0x00000003) >>  0;

        _stprintf ( buffer, _T("Version:\t%s"), mpeg_id [ id ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Layer:\t%s"), mpeg_layer [ layer ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Protection:\t%s"), mpeg_protection [ protection ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Bit Rate:\t%d kbps"), mpeg_bitrate [ id ] [ layer ] [ bitrate_index ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Sample Rate:\t%d Hz"), sample_frequency [ id ][ sampling ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Padding\t%s"), mpeg_padding [ padding ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Private\t%s"), mpeg_private [ private_bit ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Mode:\t%s"), mpeg_mode [ mode ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        if ( mode == 1 )
        {
            _stprintf ( buffer, _T("Mode Extension:\t%s"), mpeg_mode_ext [layer ] [ mode_ext ] );
            ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );
        }

        _stprintf ( buffer, _T("Copyright:\t%s"), mpeg_copyright [ copyright ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Original/Copy:\t%s"), mpeg_original [ orig_copy ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Emphasis:\t%s"), mpeg_emphasis [ emphasis ] );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );


    }
}

void AudioFile::GetWMASpecificInfo ( HWND hWnd )
{
    WMAParser parser;

    HANDLE file = ::CreateFile ( _fn.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, OPEN_EXISTING, NULL );
    if ( file == INVALID_HANDLE_VALUE )
        return;

    if ( !parser.OpenWMAFile ( file ) )
    {
        ::CloseHandle ( file );
        return;
    }

    if ( parser.IsAudio ( ) )
    {
        TCHAR buffer [ MAX_PATH ];

        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("WMA Specific Info:") );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("----------") );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );

        BasicString temp = parser.GetCodecName ( );

        _stprintf ( buffer, _T("Codec:\t%s"), temp.c_str () );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        temp = parser.GetCodecDescription ( );

        _stprintf ( buffer, _T("Description:\t%s"), temp.c_str () );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        /*
        _stprintf ( buffer, _T("Bit Rate:\t%d kbps"), parser.GetBitrate ( ) );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Sample Rate\t%0.1f kHz"), parser.GetSampleRate ( ) / 1000.0 );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );
        */

        //_stprintf ( buffer, _T("Mode:\t%s"), mpeg_mode [ mode ] );
        //::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );
    }

    ::CloseHandle ( file );    
}

void AudioFile::GetfLaCSpecificInfo ( HWND hWnd )
{
    fLaCParser parser;

    HANDLE file = ::CreateFile ( _fn.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, OPEN_EXISTING, NULL );
    if ( file == INVALID_HANDLE_VALUE )
        return;

    if ( !parser.OpenfLaCFile ( file ) )
    {
        ::CloseHandle ( file );
        return;
    }

    TCHAR buffer [ MAX_PATH ];

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("fLaC Specific Info:") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("----------") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );

    BasicString temp = parser.GetCodecName ( );

    _stprintf ( buffer, _T("Codec:\t%s"), temp.c_str () );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    /*
    _stprintf ( buffer, _T("Bits/Sample:\t%d"), parser.GetBitrate ( ) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );
    */

    _stprintf ( buffer, _T("Sample Rate\t%0.1f kHz"), parser.GetSampleRate ( ) / 1000.0 );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    ::CloseHandle ( file );    
}

void AudioFile::GetOggSpecificInfo ( HWND hWnd )
{
    OggParser parser;

    HANDLE file = ::CreateFile ( _fn.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, OPEN_EXISTING, NULL );
    if ( file == INVALID_HANDLE_VALUE )
        return;

    if ( !parser.OpenOggFile ( file ) )
    {
        ::CloseHandle ( file );
        return;
    }

    TCHAR buffer [ MAX_PATH ];

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("Ogg Specific Info:") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("----------") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );

    BasicString temp = parser.GetCodecName ( );

    _stprintf ( buffer, _T("Codec:\t%s"), temp.c_str () );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Version:\t%d"), parser.GetVersion () );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Channels:\t%d"), parser.GetChannels () );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Sample Rate\t%0.1f kHz"), parser.GetSampleRate ( ) / 1000.0 );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    if ( !parser.GetMaxBitrate ( ) && !parser.GetMinBitrate ( ) )
    {
        _stprintf ( buffer, _T("Bit Rate\t%0.1f kHz CBR"), parser.GetBitrate ( ) / 1000.0 );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );
    }
    else
    {
        _stprintf ( buffer, _T("Bit Rate\t%0.1f kHz VBR"), parser.GetBitrate ( ) / 1000.0 );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Min Bit Rate\t%0.1f kHz"), parser.GetMinBitrate ( ) / 1000.0 );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Max Bit Rate\t%0.1f kHz"), parser.GetMaxBitrate ( ) / 1000.0 );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );
    }

    _stprintf ( buffer, _T("Block size 0\t%d"), parser.GetBlocksize0 ( ) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Block size 1\t%d"), parser.GetBlocksize1 ( ) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Framing bit\t%d"), parser.GetFraming ( ) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    ::CloseHandle ( file );    
}

void AudioFile::GetMP4SpecificInfo ( HWND hWnd )
{
    MP4Parser parser;

    HANDLE file = ::CreateFile ( _fn.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, OPEN_EXISTING, NULL );
    if ( file == INVALID_HANDLE_VALUE )
        return;

    if ( !parser.OpenMP4File ( file ) )
    {
        ::CloseHandle ( file );
        return;
    }

    if ( _type == MP4_TYPE )
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("MP4 Specific Info:") );
    else
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("M4A Specific Info:") );

    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("----------") );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) _T("") );

    TCHAR buffer [ MAX_PATH ];

    BasicString temp = parser.GetCodecName ( );

    _stprintf ( buffer, _T("Codec:\t%s"), temp.c_str () );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Sample Rate\t%0.1f kHz"), parser.GetSampleRate ( ) / 1000.0 );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );


    /*
    BasicString temp = parser.GetCodecName ( );

    _stprintf ( buffer, _T("Codec:\t%s"), temp.c_str () );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Version:\t%d"), parser.GetVersion () );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Channels:\t%d"), parser.GetChannels () );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Sample Rate\t%0.1f kHz"), parser.GetSampleRate ( ) / 1000.0 );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    if ( !parser.GetMaxBitrate ( ) && !parser.GetMinBitrate ( ) )
    {
        _stprintf ( buffer, _T("Bit Rate\t%0.1f kHz CBR"), parser.GetBitrate ( ) / 1000.0 );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );
    }
    else
    {
        _stprintf ( buffer, _T("Bit Rate\t%0.1f kHz VBR"), parser.GetBitrate ( ) / 1000.0 );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Min Bit Rate\t%0.1f kHz"), parser.GetMinBitrate ( ) / 1000.0 );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

        _stprintf ( buffer, _T("Max Bit Rate\t%0.1f kHz"), parser.GetMaxBitrate ( ) / 1000.0 );
        ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );
    }

    _stprintf ( buffer, _T("Block size 0\t%d"), parser.GetBlocksize0 ( ) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Block size 1\t%d"), parser.GetBlocksize1 ( ) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

    _stprintf ( buffer, _T("Framing bit\t%d"), parser.GetFraming ( ) );
    ::SendDlgItemMessage ( hWnd, IDC_LIST_INFO, LB_ADDSTRING, 0, (LPARAM) buffer );

  */

    ::CloseHandle ( file );    
}


// End of File
