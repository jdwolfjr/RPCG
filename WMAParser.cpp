
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include "PlaylistOptions.h"
#include "WMAParser.h"

#include <initguid.h>

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

// 10.1 Top-level ASF object GUIDS

DEFINE_GUID(ASF_Header_Object_GUID,                         0x75B22630, 0x668E, 0x11CF, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C);
DEFINE_GUID(ASF_Data_Object_GUID,                           0x75B22636, 0x668E, 0x11CF, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C);
DEFINE_GUID(ASF_Simple_Index_Object_GUID,                   0x33000890, 0xE5B1, 0x11CF, 0x89, 0xF4, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB);
DEFINE_GUID(ASF_Index_Object_GUID,                          0xD6E229D3, 0x35DA, 0x11D1, 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE);
DEFINE_GUID(ASF_Media_Object_Index_Object_GUID,             0xFEB103F8, 0x12AD, 0x4C64, 0x84, 0x0F, 0x2A, 0x1D, 0x2F, 0x7A, 0xD4, 0x8C );
DEFINE_GUID(ASF_Timecode_Index_Object_GUID,                 0x3CB73FD0, 0x0C4A, 0x4803, 0x95, 0x3D, 0xED, 0xF7, 0xB6, 0x22, 0x8F, 0x0C);

// 10.2 Header object GUIDS

DEFINE_GUID(ASF_File_Properties_Object_GUID,                0x8CABDCA1, 0xA947, 0x11CF, 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65);
DEFINE_GUID(ASF_Stream_Properties_Object_GUID,              0xB7DC0791, 0xA9B7, 0x11CF, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65);
DEFINE_GUID(ASF_Codec_List_Object_GUID,                     0x86D15240, 0x311D, 0x11D0, 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6);
DEFINE_GUID(ASF_Content_Description_Object_GUID,            0x75B22633, 0x668E, 0X11CF, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C);
DEFINE_GUID(ASF_Extended_Content_Description_Object_GUID,   0xD2D0A440, 0xE307, 0x11D2, 0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50);

// 10.4 Stream Properties Object Stream Type GUIDs

DEFINE_GUID(ASF_Audio_Media_GUID,                           0xF8699E40, 0x5B4D, 0x11CF, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B);

bool WMAParser::OpenWMAFile ( HANDLE file )
{
    ASF_Header_Object asf_header;
    DWORD bytes_read = 0;

    //  Look for ASF header object

    ::SetFilePointer ( file, 0, NULL, FILE_BEGIN );

    if ( !::ReadFile ( file, & asf_header, sizeof(asf_header), & bytes_read, NULL ) || bytes_read != sizeof(asf_header) )
    {
        return false;
    }

    //  Validate the ASF Header Object

    if ( !IsEqualGUID( asf_header.guid, ASF_Header_Object_GUID) ||
         ( asf_header.size < 30 ) ||
         ( asf_header.object_count < 3 ) ||
         ( asf_header.reserved_2 != 0x02 ) )
    {
        return false;
    }

    //  Map the ASF header obejct 

    _map = ::CreateFileMapping ( file, NULL, PAGE_READONLY, (asf_header.size >> 32), (asf_header.size & 0xFFFFFFFF), NULL );
    if ( !_map )
    {
        return false;
    }

    _view = ::MapViewOfFile ( _map, FILE_MAP_READ, 0, 0, 0 );
    if ( !_view )
    {
        return false;
    }

    return true;
}

bool WMAParser::IsAudio ( WAVEFORMATEX * wave_fmt )
{
    ASF_Stream_Properties_Object * stream_prop_object = NULL;
    UINT64 length = 0;
    bool is_audio = false;

    if ( !_map || !_view )
    {
        return false;
    }

    //  We have a Windows Media File, but is is an audio file?
    //  ASF spec says a stream properties object is mandatory in the header so lets get it.

    if ( !GetASFHeaderObject ( ASF_Stream_Properties_Object_GUID, (void ** ) & stream_prop_object, & length ) )
    {
        return false;
    }
    
    //  We have a stream property object

    if ( stream_prop_object )
    {
        is_audio = IsEqualGUID( stream_prop_object->stream_type, ASF_Audio_Media_GUID ) ? true : false;

        if ( wave_fmt )
        {
            ::CopyMemory ( & wave_fmt, (WAVEFORMATEX *)((BYTE *) stream_prop_object + 78), sizeof(WAVEFORMATEX) );
        }

        delete [ ] stream_prop_object;
    }

    return is_audio;
}

BasicString WMAParser::GetTitle    ( )
{
    BasicString title = GetExtendedContentDescription ( L"WM/Title" );
    if ( title.empty() )
    {
        //  Try for a content description object

        title = GetContentDescription ( L"WM/Title" );
    }

    return title;
}

BasicString WMAParser::GetArtist    ( )
{
    BasicString author = GetExtendedContentDescription ( L"WM/Author" );
    if ( author.empty() )
    {
        //  Try for a content description object

        author = GetContentDescription ( L"WM/Author" );
    }

    return author;
}

BasicString WMAParser::GetAlbum    ( )
{
    BasicString album = GetExtendedContentDescription ( L"WM/AlbumTitle" );

    //  Not available in a content description object

    return album;
}

BasicString WMAParser::GetYear    ( )
{
    BasicString year = GetExtendedContentDescription ( L"WM/Year" );
    
    //  Not available in a content description object

    return year;
}

BasicString WMAParser::GetTrack    ( )
{
    BasicString track = GetExtendedContentDescription ( L"WM/TrackNumber" );

    //  Not available in a content description object

    return track;
}

BasicString WMAParser::GetGenre    ( )
{
    BasicString genre = GetExtendedContentDescription ( L"WM/Genre" );

    //  Not available in a content description object

    return genre;
}

DWORD WMAParser::GetDuration ( )
{
    DWORD duration = 0;

    ASF_File_Properties_Object * file_prop_object = NULL;
    UINT64 length = 0;
    bool is_audio = false;

    if ( !_map || !_view )
    {
        return false;
    }

    //  ASF spec says a file properties object is mandatory in the header so lets get it.

    if ( !GetASFHeaderObject ( ASF_File_Properties_Object_GUID, (void ** ) & file_prop_object, & length ) )
    {
        return false;
    }
    
    //  We have a file property object

    if ( file_prop_object )
    {
        duration = file_prop_object->play_duration / 10000000;

        delete [ ] file_prop_object;
    }

    return duration;
}

BasicString WMAParser::GetCodecName ()
{
    BasicString codec_name = _T("");

    ASF_Codec_List_Object * codec_list_object = NULL;
    UINT64 length = 0;

    //  ASF spec says a codec list object is optional.

    if ( !GetASFHeaderObject ( ASF_Codec_List_Object_GUID, (void ** ) & codec_list_object, & length ) )
    {
        return false;
    }
    
    //  We have a codec list object

    if ( codec_list_object )
    {
        if ( codec_list_object->count > 0 )
        {
            BYTE * data = (BYTE *) ( codec_list_object + 1 );

            WORD    type       = * (WORD *)  data;
            WORD    name_len   = * (WORD *) (data + 2);
            WCHAR * name       =   (WCHAR *)(data + 4);
            WORD    desc_len   = * (WORD *) (data + 4 + name_len * 2);
            WCHAR * desc       =   (WCHAR *)(data + 6 + name_len * 2);
            WORD    info_len   = * (WORD *) (data + 6 + (name_len + desc_len) * 2 );
            BYTE  * info       =   (BYTE *) (data + 8 + (name_len + desc_len) * 2 );

#ifndef _UNICODE

            int alloc_len = ::WideCharToMultiByte ( CP_ACP, 0, name, name_len, NULL, 0, NULL, NULL );
            if ( alloc_len )
            {
                char * temp = new char [ alloc_len + 1 ] ;
                if ( temp )
                {
                    ::WideCharToMultiByte ( CP_ACP, 0, name, name_len, temp, alloc_len, NULL, NULL );
                    codec_name.assign ( temp );
                    delete [ ] temp;
                }
            }

#else
            codec_name.assign ( name, name_len );
#endif
        }

        delete [ ] codec_list_object;
    }

    return codec_name;
}

BasicString WMAParser::GetCodecDescription ()
{
    BasicString codec_desc = _T("");

    ASF_Codec_List_Object * codec_list_object = NULL;
    UINT64 length = 0;

    //  ASF spec says a codec list object is optional.

    if ( !GetASFHeaderObject ( ASF_Codec_List_Object_GUID, (void ** ) & codec_list_object, & length ) )
    {
        return false;
    }
    
    //  We have a codec list object

    if ( codec_list_object )
    {
        if ( codec_list_object->count > 0 )
        {
            BYTE * data = (BYTE *) ( codec_list_object + 1 );

            WORD    type       = * (WORD *)  data;
            WORD    name_len   = * (WORD *) (data + 2);
            WCHAR * name       =   (WCHAR *)(data + 4);
            WORD    desc_len   = * (WORD *) (data + 4 + name_len * 2);
            WCHAR * desc       =   (WCHAR *)(data + 6 + name_len * 2);
            WORD    info_len   = * (WORD *) (data + 6 + (name_len + desc_len) * 2 );
            BYTE  * info       =   (BYTE *) (data + 8 + (name_len + desc_len) * 2 );

#ifndef _UNICODE

            int alloc_len = ::WideCharToMultiByte ( CP_ACP, 0, desc, desc_len, NULL, 0, NULL, NULL );
            if ( alloc_len )
            {
                char * temp = new char [ alloc_len + 1 ] ;
                if ( temp )
                {
                    ::WideCharToMultiByte ( CP_ACP, 0, desc, desc_len, temp, alloc_len, NULL, NULL );
                    codec_desc.assign ( temp );
                    delete [ ] temp;
                }
            }

#else
            codec_desc.assign ( name, name_len );
#endif
        }

        delete [ ] codec_list_object;
    }

    return codec_desc;
}

DWORD WMAParser::GetSampleRate ()
{
    WAVEFORMATEX wave_fmt;
    ::ZeroMemory ( & wave_fmt, sizeof(wave_fmt) );

    IsAudio ( & wave_fmt );

    return wave_fmt.nSamplesPerSec;
}

DWORD WMAParser::GetBitrate ()
{
    DWORD bitrate = 0;

    return bitrate;
}
   
bool WMAParser::GetASFHeaderObject ( const GUID & guid, void ** object, UINT64 * length )
{
    * object = NULL;
    * length = 0;

    ASF_Header_Object * asf_header = (ASF_Header_Object *) _view;

    BYTE * data = (BYTE *) ((BYTE *)_view + (BYTE) sizeof(ASF_Header_Object) );
    
    for ( DWORD i = 0; i < asf_header->object_count; i++ )
    {
        ASF_Header * header = (ASF_Header * )data;

        if ( IsEqualGUID( header->guid, guid ) )
        {
            * length    = header->size;
            * object    = new BYTE [ header->size ];

            if ( ! *object )
            {
                return false;
            }

            memcpy ( * object, header, header->size );

            return true;
        }
        else
        {
            data += header->size;
        }
    }

    return false;
}

BasicString WMAParser::GetExtendedContentDescription ( LPWSTR name )
{
    LPTSTR description = NULL;
    void * stream_desc_object = NULL;
    UINT64 length = 0;
    BasicString desc_string = _T("");

    //  Try for a extended content description object first

    if ( GetASFHeaderObject ( ASF_Extended_Content_Description_Object_GUID, (void ** ) & stream_desc_object, & length ) && stream_desc_object )
    {
        Ext_Content_Description_Object * ext_content_desc = (Ext_Content_Description_Object *)stream_desc_object;
        BYTE * data = (BYTE *)(ext_content_desc + 1);

        for ( DWORD i = 0; i < ext_content_desc->count; i++ )
        {
            WORD    desc_name_len   = * (WORD *)  ( data );
            WCHAR * desc_name       =   (WCHAR *) ( data + 2 );
            WORD    value_type      = * (WORD *)  ( data + 2 + desc_name_len );
            WORD    value_len       = * (WORD *)  ( data + 4 + desc_name_len );
            WCHAR * value           =   (WCHAR *) ( data + 6 + desc_name_len );

            if ( memcmp ( desc_name, name, desc_name_len ) == 0 )
            {
                switch ( value_type )
                {
                    case 0:         //  0x0000	Unicode string	varies
                        
                        {
#ifndef _UNICODE
                            int alloc_len = ::WideCharToMultiByte ( CP_ACP, 0, value, value_len, description, 0, NULL, NULL );
                            if ( alloc_len )
                            {
                                description = new char [ alloc_len + 1 ] ;
                                if ( description )
                                {
                                    alloc_len = ::WideCharToMultiByte ( CP_ACP, 0, value, value_len, description, alloc_len, NULL, NULL );
                                }
                            }

#else                            
                            desc_string.assign ( value, value_len );
#endif
                        }

                        break;

                    case 1:        //   0x0001	BYTE array	varies

                        break;

                    case 2:         //  0x0002	BOOL
                        break;

                    case 3:         //  0x0003	DWORD

                        break;

                    case 4:         //  0x0004	QWORD

                        break;

                    case 5:         //  0x0005  WORD

                        break;
                }

                break;
            }
            else
            {
                data += ( 6 + desc_name_len + value_len );
            }
        }
    }

    if ( stream_desc_object )
    {
        delete [ ] stream_desc_object;
    }

    if ( description )
    {
        desc_string.assign ( description );
        delete [ ] description;
    }

    return desc_string;
}

BasicString WMAParser::GetContentDescription ( LPWSTR name )
{
    LPTSTR description = NULL;
    void * stream_desc_object = NULL;
    UINT64 length = 0;
    BasicString desc_string = _T("");

    //  Try for a content description object first

    if ( GetASFHeaderObject ( ASF_Content_Description_Object_GUID, (void ** ) & stream_desc_object, & length ) && stream_desc_object )
    {
        Content_Description_Object * content_desc = (Content_Description_Object *)stream_desc_object;
        BYTE * data = (BYTE *)(content_desc + 1);

        //  Now we are pointing at the wide character data

        if ( wcscmp ( name, L"WM/Title") == 0 )
        {

#ifndef _UNICODE
            description = new char [ content_desc->title_length + 1 ] ;

            if ( description )
            {
                ::WideCharToMultiByte ( CP_ACP, 0, (LPWSTR) data, content_desc->title_length, description, content_desc->title_length, NULL, NULL );
            }

#else
            desc_string.assign ( (const unsigned short *)data, content_desc->title_length );
#endif

        }
        else
        if ( wcscmp ( name, L"WM/Author") == 0 )
        {
            data += content_desc->title_length;

#ifndef _UNICODE
            description = new char [ content_desc->author_length + 1 ] ;

            if ( description )
            {
                ::WideCharToMultiByte ( CP_ACP, 0, (LPWSTR) data, content_desc->author_length, description, content_desc->author_length, NULL, NULL );
            }

#else
            desc_string.assign ( (const unsigned short *)data, content_desc->author_length );
#endif

        }
    }

    if ( stream_desc_object )
    {
        delete [ ] stream_desc_object;
    }

    if ( description )
    {
        desc_string.assign ( description );
        delete [ ] description;
    }

    return desc_string;
}


// End of File
