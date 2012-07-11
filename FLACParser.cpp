
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "FLACParser.h"
#include "AudioFile.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

#define MAKE_FLAC(w,x,y,z) (w | x << 8 | y << 16 | z << 24)

//  Metablock types

const int STREAMINFO        = 0;
const int PADDING           = 1;
const int APPLICATION       = 2;
const int SEEKTABLE         = 3;
const int VORBIS_COMMENT    = 4;
const int CUESHEET          = 5;

/*
    6-126 : reserved 
    127   : invalid, to avoid confusion with a frame sync code 
*/

bool fLaCParser::OpenfLaCFile ( HANDLE file )
{
    _length = ::GetFileSize ( file, NULL );

    //  Look for 'fLaC' tag    

    ::SetFilePointer ( file, 0, NULL, FILE_BEGIN );

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

    unsigned magic = * (unsigned *) _view;

    if ( magic != FLAC_MAGIC )
    {
        return false;
    }

    bool is_audio = false;

    try
    {
        BYTE * stream_info = GetMetablock ( STREAMINFO );
        if ( stream_info  )
        {
            int block_length = (stream_info [ 1 ] << 16 | stream_info [ 2 ] << 8 | stream_info [ 3 ] );

            /*

            <16>  The minimum block size (in samples) used in the stream.  
            <16>  The maximum block size (in samples) used in the stream. (Minimum blocksize == maximum blocksize) implies a fixed-blocksize stream.  
            <24>  The minimum frame size (in bytes) used in the stream. May be 0 to imply the value is not known.  
            <24>  The maximum frame size (in bytes) used in the stream. May be 0 to imply the value is not known.  
            <20>  Sample rate in Hz. Though 20 bits are available, the maximum sample rate is limited by the structure of frame headers to 1048570Hz. Also, a value of 0 is invalid.  
            <3>  (number of channels)-1. FLAC supports from 1 to 8 channels  
            <5>  (bits per sample)-1. FLAC supports from 4 to 32 bits per sample. Currently the reference encoder and decoders only support up to 24 bits per sample.  
            <36>  Total samples in stream. 'Samples' means inter-channel sample, i.e. one second of 44.1Khz audio will have 44100 samples regardless of the number of channels. A value of zero here means the number of total samples is unknown.  
            <128>  MD5 signature of the unencoded audio data. This allows the decoder to determine if an error exists in the audio data even when the error does not result in an invalid bitstream.  

            */

            BYTE * data = stream_info + 4;

            if ( block_length >= 34 )
            {           
                _sample_rate   = data [ 10 ] << 12 | data [ 11 ] << 4 | data [ 12 ] >> 4;
                _bit_rate      = (data [ 12 ] & 0x01) << 4 | data [ 13 ] >> 4;
                _total_samples = (data [ 13 ] & 0x0F) << 32 | data [ 14 ] << 24 | data [ 15 ] << 16 | data [ 16 ] << 8 | data [ 17 ];

                is_audio = true;
            }
        }
    }
    catch ( ... )
    {
    }

    return is_audio;
}

BasicString fLaCParser::GetTitle    ( )
{
    BasicString title = GetVorbisComment ( _T("Title") );

    return title;
}

BasicString fLaCParser::GetArtist    ( )
{
    BasicString author = GetVorbisComment ( _T("Artist"));

    return author;
}

BasicString fLaCParser::GetAlbum    ( )
{
    BasicString album = GetVorbisComment ( _T("AlbumTitle"));

    return album;
}

BasicString fLaCParser::GetYear    ( )
{
    BasicString date = GetVorbisComment ( _T("Date"));
  
    return date;
}

BasicString fLaCParser::GetTrack    ( )
{
    BasicString track = GetVorbisComment ( _T("TrackNumber"));

    return track;
}

BasicString fLaCParser::GetGenre    ( )
{
    BasicString genre = GetVorbisComment ( _T("Genre"));

    return genre;
}

DWORD fLaCParser::GetDuration ( )
{
    if ( _sample_rate != 0 )
    {
        return _total_samples / _sample_rate;
    }

    return 0;
}

BasicString fLaCParser::GetCodecName ()
{
    BasicString codec_name = _T("");

    try
    {
        BYTE * vorbis = GetMetablock ( VORBIS_COMMENT );
        if ( vorbis )
        {
            int block_length = (vorbis [ 1 ] << 16 | vorbis [ 2 ] << 8 | vorbis [ 3 ] );

            /*
                Courtesy of Ogg Vorbis (http://xiph.org/vorbis/doc/v-comment.html)

                Vendor string length (32 bit unsigned quantity specifying number of octets) 
                Vendor string ([vendor string length] octets coded from beginning of string to end of string, not null terminated) 
                Number of comment fields (32 bit unsigned quantity specifying number of fields) 
                Comment field 0 length (if [Number of comment fields]>0; 32 bit unsigned quantity specifying number of octets) 
                Comment field 0 ([Comment field 0 length] octets coded from beginning of string to end of string, not null terminated) 
                Comment field 1 length (if [Number of comment fields]>1...)... 

            */

            BYTE * data = vorbis + 4;

            //  Get the vendor length and string

            unsigned length = *(unsigned * ) data;

            codec_name.assign ( (TCHAR *)(data + 4), length );
        }
    }
    catch ( ... )
    {
    }

    return codec_name;
}


DWORD fLaCParser::GetSampleRate ()
{
    return _sample_rate;
}

DWORD fLaCParser::GetBitrate ()
{
    return _bit_rate;
}

BYTE * fLaCParser::GetMetablock ( int metablock_type )
{
    BYTE * data = (BYTE *) _view;
    data += 4;

    try
    {
        while ( data )
        {
            Metadata_BlockHeader block;

            block.last      = (data [ 0 ] & 0x80) >> 7;
            block.type      = (data [ 0 ] & 0x7F) >> 0;
            block.length    = (data [ 1 ] << 16 | data [ 2 ] << 8 | data [ 3 ] );

            if ( block.type == metablock_type )
                return data;

            if ( block.last )
                return NULL;

            data += (block.length + 4);
        }
    }
    catch ( ... )
    {
    }

    return NULL;
}

BasicString fLaCParser::GetVorbisComment ( TCHAR * name )
{
    TCHAR buffer [ 256 ];
    BasicString temp, comment = _T("");

    try
    {
        BYTE * vorbis = GetMetablock ( VORBIS_COMMENT );
        if ( vorbis )
        {
            int block_length = (vorbis [ 1 ] << 16 | vorbis [ 2 ] << 8 | vorbis [ 3 ] );

            /*
                Courtesy of Ogg Vorbis (http://xiph.org/vorbis/doc/v-comment.html)

                Vendor string length (32 bit unsigned quantity specifying number of octets) 
                Vendor string ([vendor string length] octets coded from beginning of string to end of string, not null terminated) 
                Number of comment fields (32 bit unsigned quantity specifying number of fields) 
                Comment field 0 length (if [Number of comment fields]>0; 32 bit unsigned quantity specifying number of octets) 
                Comment field 0 ([Comment field 0 length] octets coded from beginning of string to end of string, not null terminated) 
                Comment field 1 length (if [Number of comment fields]>1...)... 

            */

            BYTE * data = vorbis + 4;

            //  Step over the vendor length and string

            unsigned length = *(unsigned * ) data;

            data            += (length + 4);
            block_length    -= (length + 4);

            unsigned comments = *(unsigned * ) data;

            data            += 4;
            block_length    -= 4;

            for ( unsigned i = 0; i < comments && block_length > 0; i++ )
            {
                length = *(unsigned * ) data;

                data            += 4;
                block_length    -= 4;

                temp.assign ( (TCHAR *) data, length );

                /* comment should be of the form 'key=value' where key is what we want to match to 'name' */

                int offset = temp.find('=');
                if ( offset != 0 )
                {
                    BasicString key( temp, 0, offset);

                    if ( _tcsicmp ( key.c_str (), name ) == 0 )
                    {
                        comment.assign ( temp, offset + 1, length );
                        return comment;
                    }
                }

                data            += length;
                block_length    -= length;

            }

            //  Might be an ID3v1 tag

            data = (BYTE *) _view;
            data += (_length - 128);

            ID3v1 * id3 = (ID3v1 *) data;

            if ( id3->tag [ 0 ] == 'T' && id3->tag [ 1 ] == 'A' && id3->tag [ 2 ] == 'G' )
            {
                if ( _tcsicmp ( name, _T("Title")) == 0 )
                {
                    comment.assign ( (TCHAR *) id3->title, 30 );
                }
                else
                if ( _tcsicmp ( name, _T("Artist")) == 0 )
                {
                    comment.assign ( (TCHAR *) id3->artist, 30 );
                }
                else
                if ( _tcsicmp ( name, _T("AlbumTitle")) == 0 )
                {
                    comment.assign ( (TCHAR *) id3->album, 30 );
                }
                else
                if ( _tcsicmp ( name, _T("Date")) == 0 )
                {
                    comment.assign ( (TCHAR *) id3->year, 4 );
                }
                else
                if ( _tcsicmp ( name, _T("TrackNumber")) == 0 )
                {
                    if ( id3->v11.track [ 0 ] == 0x00 )
                    {
                        _stprintf ( buffer, _T("%d"), (TCHAR *) id3->v11.track [ 1 ] );
                        comment.assign ( buffer );
                    }
                }
                else
                if ( _tcsicmp ( name, _T("Genre")) == 0 )
                {
                    _stprintf ( buffer, _T("%d"), (TCHAR *) id3->genre );
                    comment.assign ( buffer );
                }
            }
        }
    }
    catch ( ... )
    {
    }

    return comment;
}

// End of File
