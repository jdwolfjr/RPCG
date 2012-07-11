
#ifndef MP4_PARSER_H
#define MP4_PARSER_H

#pragma pack(push,1)

typedef struct MPEG4_Atom_Header
{
    unsigned length;
    char     atom [ 4 ];

} MPEG4_Atom_Header;

typedef struct MPEG4_MediaStream_Header
{
    MPEG4_Atom_Header   header;
    BYTE                version;
    BYTE                flags [ 3 ];
    ULONG               created_date;
    ULONG               modified_date;
    ULONG               media_unit_time;
    ULONG               media_length;
    WORD                language;
    SHORT               quality;

} MPEG4_MediaStream_Header;

typedef struct MPEG4_MediaStream_Header_Ext
{
    MPEG4_Atom_Header   header;
    BYTE                version;
    BYTE                flags [ 3 ];
    ULONGLONG           created_date;
    ULONGLONG           modified_date;
    ULONG               media_unit_time;
    ULONGLONG           media_length;
    WORD                language;
    SHORT               quality;

} MPEG4_MediaStream_Header_Ext;

typedef struct MPEG4_MetaData_Header
{
    MPEG4_Atom_Header   header;
    BYTE                version;
    BYTE                flags [ 3 ];

} MPEG4_MetaData_Header;

typedef struct MPEG4_IndexListData_Header
{
    MPEG4_Atom_Header   header;
    BYTE                version;
    BYTE                flags [ 3 ];
    UINT                reserved;
    
    /*
        data follows here
    */

} MPEG4_IndexListData_Header;


#pragma pack(pop)

enum MP4Types { UNKNOWN = 0, M4A, MP41, MP42 };

class MP4Parser
{
public:

    HANDLE              _map;
    void              * _view;
    unsigned            _length;
    MP4Types            _type;
  
    MP4Parser ( ) : _map ( NULL ), _view ( NULL ), _length ( 0 ), _type ( UNKNOWN ) { ; }
    ~MP4Parser ( )
    {
        if ( _view )
            ::UnmapViewOfFile ( _view );
        if ( _map )
            ::CloseHandle ( _map );
    }

    bool OpenMP4File ( HANDLE file );

    BasicString GetTitle    ( );
    BasicString GetArtist   ( );
    BasicString GetAlbum    ( );
    BasicString GetYear     ( );
    BasicString GetTrack    ( );
    BasicString GetGenre    ( int * genre_id );

    DWORD GetDuration ( );
    BasicString GetCodecName ();
    DWORD GetSampleRate ();
    DWORD GetBitrate ();

private:
    

    MPEG4_IndexListData_Header * FindIndexListAtom ( unsigned atom );
    MPEG4_Atom_Header * FindAtom ( BYTE * data, unsigned length, unsigned atom );
};

#endif  //  MP4_PARSER_H

