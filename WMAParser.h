
#ifndef WMA_PARSER_H
#define WMA_PARSER_H

#pragma pack(push,1)

typedef struct ASF_Header_Object
{
    GUID    guid;
    UINT64  size;
    DWORD   object_count;
    BYTE    reserved_1;
    BYTE    reserved_2;
} ASF_Header_Object;

typedef struct ASF_Stream_Properties_Object
{
    GUID    guid;
    UINT64  size;
    GUID    stream_type;
    GUID    error_correction_type;
    UINT64  time_offset;
    DWORD   type_specific_data_length;
    DWORD   error_correction_data_length;
    WORD    flags;
    DWORD   reserved;
    BYTE  * type_specific_data;
    BYTE  * error_correction_data;
} ASF_Stream_Properties_Object;

typedef struct ASF_Header
{
    GUID    guid;
    UINT64  size;
} ASF_Header;

typedef struct Ext_Content_Description_Object
{
    GUID    guid;
    UINT64  length;
    WORD    count;
    /*
        array of descriptors
    */

} Ext_Content_Description_Object;

typedef struct Content_Description_Object
{
    GUID    guid;
    UINT64  length;
    WORD    title_length;
    WORD    author_length;
    WORD    copyright_length;
    WORD    desciption_length;
    WORD    rating_length;

    /*
        descriptors
    */

} Content_Description_Object;

/*

typedef struct Codec_Entry
{
    WORD    type;
    WORD    name_length;
    WCHAR   name [ ];
    WORD    desc_length;
    WCHAR   desc [ ];
    WORD    info_length;
    BYTE    info [ ];
} Codec_Entry;

  */

typedef struct ASF_Codec_List_Object
{
    GUID    guid;
    UINT64  length;
    GUID    reserved;
    DWORD   count;

    /*
        array of codec entries
    */

} ASF_Codec_List_Object;

typedef struct ASF_File_Properties_Object
{
    GUID    guid;
    UINT64  length;
    GUID    file_id;
    UINT64  file_size;
    UINT64  creation_date;
    UINT64  data_packets_count;
    UINT64  play_duration;
    UINT64  send_duration;
    UINT64  preroll;
    DWORD   flags;
    DWORD   min_data_packet_size;
    DWORD   max_data_packet_size;
    DWORD   max_bitrate;
} ASF_File_Properties_Object;

#pragma pack(pop)

class WMAParser
{
public:

    HANDLE          _map;
    void          * _view;
  
    WMAParser ( ) : _map ( NULL ), _view ( NULL ) { ; }    
    ~WMAParser ( )
    {
        if ( _view )
            ::UnmapViewOfFile ( _view );
        if ( _map )
            ::CloseHandle ( _map );
    }

    bool OpenWMAFile ( HANDLE file );
    bool IsAudio ( WAVEFORMATEX * wave_fmt = NULL );

    BasicString GetTitle    ( );
    BasicString GetArtist   ( );
    BasicString GetAlbum    ( );
    BasicString GetYear     ( );
    BasicString GetTrack    ( );
    BasicString GetGenre    ( );

    DWORD GetDuration ( );

    BasicString GetCodecName ();
    BasicString GetCodecDescription ();
    DWORD GetSampleRate ();
    DWORD GetBitrate ();

private:

    bool GetASFHeaderObject ( const GUID & guid, void ** object, UINT64 * length );
    BasicString GetExtendedContentDescription ( LPWSTR name );
    BasicString GetContentDescription ( LPWSTR name );
};

#endif  //  WMA_PARSER_H

