
#ifndef FLAC_PARSER_H
#define FLAC_PARSER_H

const unsigned FLAC_MAGIC    = 'f' | 'L' << 8 | 'a' << 16 | 'C' << 24;

#pragma pack(push,1)

typedef struct Metadata_BlockHeader
{
    DWORD   last;
    DWORD   type;
    DWORD   length;

} Metadata_BlockHeader;
 
#pragma pack(pop)

class fLaCParser
{
public:

    HANDLE          _map;
    void          * _view;
    DWORD           _length;

    DWORD           _sample_rate;
    DWORD           _total_samples;
    DWORD           _bit_rate;
  
    fLaCParser ( ) : _map ( NULL ), _view ( NULL ), _length ( 0 ), _sample_rate ( 0 ), _total_samples ( 0 ), _bit_rate ( 0 ) { ; }    
    ~fLaCParser ( )
    {
        if ( _view )
            ::UnmapViewOfFile ( _view );
        if ( _map )
            ::CloseHandle ( _map );
    }

    bool OpenfLaCFile ( HANDLE file );

    BasicString GetTitle    ( );
    BasicString GetArtist   ( );
    BasicString GetAlbum    ( );
    BasicString GetYear     ( );
    BasicString GetTrack    ( );
    BasicString GetGenre    ( );

    DWORD GetDuration ( );

    BasicString GetCodecName ();
    DWORD GetSampleRate ();
    DWORD GetBitrate ();

private:

    BYTE * GetMetablock ( int metablock_type );
    BasicString GetVorbisComment ( TCHAR * name );


};

#endif  //  FLAC_PARSER_H

