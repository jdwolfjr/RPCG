
#ifndef AUDIO_FILE_H
#define AUDIO_FILE_H

#define MAKE_FRAME_ID(w,x,y,z) (w | x << 8 | y << 16 | z << 24 )

enum AudioFileType { UNKNOWN_TYPE = 0, MP3_TYPE, WMA_TYPE, FLAC_TYPE, OGG_TYPE, MP4_TYPE, M4A_TYPE };

#include "WMAParser.h"
#include "FLACParser.h"
#include "OggParser.h"
#include "MP4Parser.h"

//  ID3 V1 tag is always located at the end of the file. Look for the marker "TAG" 128 bytes from the end of the file.

typedef struct ID3v1
{
    BYTE    tag     [ 3  ];             // MUST be 'T' 'A' 'G'
    BYTE    title   [ 30 ];
    BYTE    artist  [ 30 ];
    BYTE    album   [ 30 ];             
    BYTE    year    [ 4  ];
    union
    {
        BYTE    comment_v10 [ 30 ];
        struct V11
        {
            BYTE    comment_v11 [ 28 ];     //  ID3_v10 and ID3_v11
            BYTE    track       [ 2  ];     //  BYTE 0 should always be '\0' so the comment field appears NULL terminated
        }   v11;
    };
    BYTE    genre   [ 1  ];
} ID3v1;

//  It is most likely located at the beginning of the file. Look for the marker "ID3" in the first 3 bytes of the file.
//  If it's not there, it could be at the end of the file (if the tag is ID3v2.4).
//  Look for the marker "3DI" 10 bytes from the end of the file, or 10 bytes before the beginning of an ID3v1 tag.

static const BYTE ID3V2_FLAG_UNSYNC         = 0x80;
static const BYTE ID3V2_FLAG_EXTENDED       = 0x40;
static const BYTE ID3V2_FLAG_EXPERIMENTAL   = 0x20;

/*
   The version is followed by one the ID3v2 flags field, of which
   currently only three flags are used.


   a - Unsynchronisation

     Bit 7 in the 'ID3v2 flags' indicates whether or not
     unsynchronisation is used (see section 5 for details); a set bit
     indicates usage.


   b - Extended header

     The second bit (bit 6) indicates whether or not the header is
     followed by an extended header. The extended header is described in
     section 3.2.


   c - Experimental indicator

     The third bit (bit 5) should be used as an 'experimental
     indicator'. This flag should always be set when the tag is in an
     experimental stage.

   All the other flags should be cleared. If one of these undefined
   flags are set that might mean that the tag is not readable for a
   parser that does not know the flags function.

   The ID3v2 tag size is encoded with four bytes where the most
   significant bit (bit 7) is set to zero in every byte, making a total
   of 28 bits. The zeroed bits are ignored, so a 257 bytes long tag is
   represented as $00 00 02 01.

   The ID3v2 tag size is the size of the complete tag after
   unsychronisation, including padding, excluding the header but not
   excluding the extended header (total tag size - 10). Only 28 bits
   (representing up to 256MB) are used in the size description to avoid
   the introducuction of 'false syncsignals'.

   An ID3v2 tag can be detected with the following pattern:
     $49 44 33 yy yy xx zz zz zz zz
   Where yy is less than $FF, xx is the 'flags' byte and zz is less than
   $80.
*/

typedef struct ID3v2
{
    BYTE    tag     [ 3 ];              // MUST be 'I' 'D' '3'
    BYTE    version [ 2 ];
    BYTE    flags   [ 1 ];              //  See flag bits above
    BYTE    size    [ 4 ];
} ID3v2;

typedef struct ID3v2_3_ExtHeader
{
    BYTE    size            [ 4 ];
    BYTE    flags           [ 2 ];
    BYTE    padding_size    [ 4 ];
} ID3v2_3_ExtHeader;

typedef struct ID3v2_4_ExtHeader
{
    BYTE    size            [ 4 ];
    BYTE    flags           [ 2 ];
    BYTE    padding_size    [ 4 ];
} ID3v2_4_ExtHeader;

typedef struct ID3v2_3_Frame
{
    BYTE    frame_id        [ 4 ];
    BYTE    size            [ 4 ];
    BYTE    flags           [ 2 ];
} ID3v2_3_Frame;

typedef struct ID3v2_2_Frame
{
    BYTE    frame_id        [ 3 ];
    BYTE    size            [ 3 ];
} ID3v2_2_Frame;

class AudioFile
{
public:

    BasicString     _fn;
    DWORD           _fs;
    DWORD           _state;
    AudioFileType   _type;
    DWORD           _duration;

    DWORD           _repeat_count;

    typedef std::map<DWORD, BasicString>                TextMap;
    typedef std::map<DWORD, BasicString>::iterator      TextMapIterator;
    typedef std::map<DWORD, BasicString>::value_type    TextMapValue;

    TextMap         _fm;
   
    AudioFile ( ) : _fn ( ), _fs ( 0 ), _state ( 0 ), _type ( UNKNOWN_TYPE ), _duration ( 0 ), _repeat_count ( 0 ) { }    
    AudioFile ( BasicString fn, DWORD fs ) : _fn ( fn ), _fs ( fs ), _state ( 0 ), _type ( UNKNOWN_TYPE ), _duration ( 0 ), _repeat_count ( 0 ) { }
    AudioFile ( LPCTSTR fn, DWORD fs ) : _fn ( fn ), _fs ( fs ), _state ( 0 ), _type ( UNKNOWN_TYPE ), _duration ( 0 ), _repeat_count ( 0 ) { }
    AudioFile ( const AudioFile & audio ) : _fn ( ), _fs ( 0 ), _state ( 0 ), _type ( UNKNOWN_TYPE ), _duration ( 0 ), _repeat_count ( 0 )
    {
        _fn             = audio._fn;
        _fs             = audio._fs;
        _state          = audio._state;
        _fm             = audio._fm;
        _type           = audio._type;
        _duration       = audio._duration;
        _repeat_count   = audio._repeat_count;
    }
    ~AudioFile ( )
    {

    }

    AudioFile & operator= ( const AudioFile & audio )
    {
        _fn             = audio._fn;
        _fs             = audio._fs;
        _state          = audio._state;
        _fm             = audio._fm;
        _type           = audio._type;
        _duration       = audio._duration;
        _repeat_count   = audio._repeat_count;

        return * this;
    }

    DWORD ParseID3v2 ( HWND hWnd, HANDLE file, ID3v2 & id3v2 );
    DWORD ParseID3v1 ( ID3v1 & id3v1 );
    DWORD ParseWMA   ( HWND hWnd, WMAParser & parser );
    DWORD ParsefLaC  ( HWND hWnd, fLaCParser & parser );
    DWORD ParseOgg   ( HWND hWnd, OggParser & parser );
    DWORD ParseMP4   ( HWND hWnd, MP4Parser & parser );

    DWORD ParseMP3Frames ( HANDLE file );

    //  Functions to get audio file information

    LPCTSTR GetTitle    ();
    LPCTSTR GetArtist   ();
    LPCTSTR GetAlbum    ();
    LPCTSTR GetYear     ();
    LPCTSTR GetTrack    ();
    LPCTSTR GetGenre    ();
    LPCTSTR GetFileType ();

    void AddGenre ( LPCTSTR genre );

    bool operator == ( const AudioFile & audio )
    {
        return ( _fn == audio._fn );
    }

    bool Read ( FILE * in, DWORD major, DWORD minor );
    void Write ( FILE * out );

    void CalculateDuration ( HANDLE file, ID3v1 & id3v1, ID3v2 & id3v2 );
    void PopulateAudioInformation ( HWND hWnd );

private:

    bool IsID3v2FrameIDInTable ( DWORD frame_id );
    int  IsID3v22FrameIDInTable ( DWORD frame_id );
    BasicString CopyBuffer ( char *  buffer, unsigned length );
    BasicString CopyBuffer ( wchar_t *  buffer, unsigned length );
    BasicString ConvertToISO8859 ( char * buffer, unsigned length );
    void CleanUpGenre ( BasicString & genre );

    void GetMP3SpecificInfo  ( HWND hWnd );
    void GetWMASpecificInfo  ( HWND hWnd );
    void GetfLaCSpecificInfo ( HWND hWnd );
    void GetOggSpecificInfo  ( HWND hWnd );
    void GetMP4SpecificInfo  ( HWND hWnd );
};

inline void ClearRepeatCount ( AudioFile & audio )
{
    audio._repeat_count = 0;
}

inline void DecrementRepeatCount ( AudioFile & audio )
{
    if ( audio._repeat_count != 0 )
        audio._repeat_count--;
}

class SetRepeatCount
{
    int _count;

public:
    SetRepeatCount ( int count ) : _count ( count ) { ; }
    ~SetRepeatCount ( ) { ; }

    void operator() ( AudioFile * audio )
    {
        audio->_repeat_count = _count;        
    }
};

#endif  //  AUDIO_FILE_H

