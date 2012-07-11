
#ifndef OGG_PARSER_H
#define OGG_PARSER_H

const unsigned OGG_MAGIC    = 'O' | 'g' << 8 | 'g' << 16 | 'S' << 24;
const BYTE     VORBIS [ ]   = { 'v', 'o', 'r', 'b', 'i', 's' };

const int      OGG_AUDIO_HEADER             = 0x00;
const int      OGG_IDENTIFICATION_HEADER    = 0x01;
const int      OGG_COMMENT_HEADER           = 0x03;
const int      OGG_SETUP_HEADER             = 0x05;

#pragma pack(push,1)

typedef struct Ogg_Header
{
    BYTE    header_id;
    BYTE    vorbis [ 6 ];

} Ogg_Header;

typedef struct Ogg_ID_Header
{
    Ogg_Header  header;
    UINT        version;
    BYTE        channels;
    UINT        sample_rate;
    UINT        max_bit_rate;
    UINT        normal_bit_rate;
    UINT        min_bit_rate;
    BYTE        block_size;     /* 4 bits blocksize 0 and 4 bits blocksize 1*/
    BYTE        framing_flag;   /* 1 bit */
} Ogg_ID_Header;

typedef struct Ogg_Page_Header
{
    UINT        magic;
    BYTE        capture_pattern;
    BYTE        header_type_flag;
    UINT64      abs_granule_position;
    UINT        stream_serial_number;
    UINT        page_seq_number;
    UINT        page_check_sum;
    BYTE        page_segments;
    BYTE        segment_table;      /* total size of page = 27 + number of segments + segment_table [ 0 ] ... segment_table [ n ] */
} Ogg_Page_Header;

 
#pragma pack(pop)

class OggParser
{
public:

    HANDLE          _map;
    void          * _view;
    unsigned        _length;

    Ogg_ID_Header   _id_header;
  
    OggParser ( ) : _map ( NULL ), _view ( NULL ), _length ( 0 )
    {
        ::ZeroMemory ( & _id_header, sizeof(_id_header) );
    }
    ~OggParser ( )
    {
        if ( _view )
            ::UnmapViewOfFile ( _view );
        if ( _map )
            ::CloseHandle ( _map );
    }

    bool OpenOggFile ( HANDLE file );

    BasicString GetTitle    ( );
    BasicString GetArtist   ( );
    BasicString GetAlbum    ( );
    BasicString GetYear     ( );
    BasicString GetTrack    ( );
    BasicString GetGenre    ( );

    DWORD GetDuration ( );

    BasicString GetCodecName ();

    DWORD GetVersion    ( )    { return _id_header.version;                     }
    DWORD GetChannels   ( )    { return _id_header.channels;                    }
    DWORD GetSampleRate ( )    { return _id_header.sample_rate;                 }
    DWORD GetMaxBitrate ( )    { return _id_header.max_bit_rate;                }
    DWORD GetMinBitrate ( )    { return _id_header.min_bit_rate;                }
    DWORD GetBitrate    ( )    { return _id_header.normal_bit_rate;             }
    DWORD GetBlocksize0 ( )    { return pow ( 2, _id_header.block_size & 0x0F); }
    DWORD GetBlocksize1 ( )    { return pow ( 2, _id_header.block_size >> 4);   }
    DWORD GetFraming    ( )    { return _id_header.framing_flag;                }

private:

    Ogg_Header * GetHeaderID ( int header_id, unsigned * length );
    BasicString GetVorbisComment ( TCHAR * name );
    DWORD CountAudioSamples ( );


};

#endif  //  OGG_PARSER_H

