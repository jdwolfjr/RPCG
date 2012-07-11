
/*==========================================================================*/
/*                           I n c l u d e s                                */
/*==========================================================================*/

#include  "PlaylistOptions.h"
#include  "OggParser.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

int VORBIS_COMMENT = 0;

bool OggParser::OpenOggFile ( HANDLE file )
{
    Ogg_Page_Header header;
    DWORD bytes_read = 0;

    //  Look for 'Oggs' tag    

    ::SetFilePointer ( file, 0, NULL, FILE_BEGIN );

    if ( !::ReadFile ( file, & header, sizeof(Ogg_Page_Header), & bytes_read, NULL ) || bytes_read != sizeof(Ogg_Page_Header) )
    {
        return false;
    }

    //  Does the file contain an 'OggS' tag and is it the initial page?

    if ( header.magic != OGG_MAGIC || header.page_seq_number != 0 )
    {
        return false;
    }

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

    unsigned length;

    Ogg_ID_Header * id_header = (Ogg_ID_Header *) GetHeaderID ( OGG_IDENTIFICATION_HEADER, & length );

    if ( !id_header || length == 0 )
    {
        return false;
    }

    memcpy ( & _id_header, id_header, length );

    return true;
}

BasicString OggParser::GetTitle    ( )
{
    BasicString title = GetVorbisComment (  _T("Title") );

    return title;
}

BasicString OggParser::GetArtist    ( )
{
    BasicString author = GetVorbisComment (  _T("Artist") );

    return author;
}

BasicString OggParser::GetAlbum    ( )
{
    BasicString album = GetVorbisComment (  _T("AlbumTitle") );

    return album;
}

BasicString OggParser::GetYear    ( )
{
    BasicString date = GetVorbisComment (  _T("Date") );
  
    return date;
}

BasicString OggParser::GetTrack    ( )
{
    BasicString track = GetVorbisComment (  _T("TrackNumber") );

    return track;
}

BasicString OggParser::GetGenre    ( )
{
    BasicString genre = GetVorbisComment (  _T("Genre") );

    return genre;
}

DWORD OggParser::GetDuration ( )
{
    DWORD duration = 0;

    DWORD samples = CountAudioSamples ( );

    if ( _id_header.sample_rate )
    {
        duration = samples / _id_header.sample_rate;
    }

    return duration;
}

BasicString OggParser::GetCodecName ()
{
    BasicString temp, codec_name =  _T("");
    unsigned header_length;

    Ogg_Header * vorbis = GetHeaderID ( OGG_COMMENT_HEADER, & header_length );
    if ( vorbis )
    {
         BYTE * data = (BYTE *) vorbis + sizeof(Ogg_Header);

        //  Get the vendor length and string

        unsigned length = *(unsigned * ) data;

        codec_name.assign ( (TCHAR *) data + 4, length );
    }

    return codec_name;
}

Ogg_Header * OggParser::GetHeaderID ( int header_id, unsigned * length )
{
    BYTE * data                 = (BYTE *) _view;
    unsigned bytes_remaining    = _length;

    * length = 0;

    try
    {
        while ( bytes_remaining )
        {
            Ogg_Page_Header * page_header = (Ogg_Page_Header *) data;

            if ( page_header->magic == OGG_MAGIC )
            {
                DWORD total_seg_length = 0;

                for ( int i = 0; i < page_header->page_segments; i++ )
                {
                    unsigned seg_length = ((BYTE *) & page_header->segment_table) [ i ];

                    Ogg_Header * ogg_header = (Ogg_Header *) (data + 27 + page_header->page_segments + total_seg_length);

                    if ( ogg_header->header_id == header_id && memcmp ( ogg_header->vorbis, VORBIS, sizeof(VORBIS) ) == 0 )
                    {
                        * length = seg_length;
                        return ogg_header;
                    }

                    total_seg_length += seg_length;
                }

                data            += (27 + page_header->page_segments + total_seg_length);
                bytes_remaining -= (27 + page_header->page_segments + total_seg_length);
            }
            else
            {
                data            ++;
                bytes_remaining --;
            }
        }
    }
    catch ( ... )
    {

    }

    return NULL;
}

BasicString OggParser::GetVorbisComment ( TCHAR * name )
{
    BasicString temp, comment = _T("");
    unsigned header_length;

    try
    {
        Ogg_Header * vorbis = GetHeaderID ( OGG_COMMENT_HEADER, & header_length );
        if ( vorbis )
        {
            BYTE * data = (BYTE *) vorbis + sizeof(Ogg_Header);

            //  Step over the vendor length and string

            unsigned length = *(unsigned * ) data;

            data            += (length + 4);
            header_length   -= (length + 4);

            unsigned comments = *(unsigned * ) data;

            data            += 4;
            header_length   -= 4;

            for ( unsigned i = 0; i < comments && header_length > 0; i++ )
            {
                length = *(unsigned * ) data;

                data            += 4;
                header_length   -= 4;

                temp.assign ( (TCHAR *) data, length );

                // comment should be of the form 'key=value' where key is what we want to match to 'name'

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
                header_length   -= length;

            }
        }
    }
    catch ( ... )
    {

    }

    return comment;
}

DWORD OggParser::CountAudioSamples ( )
{
    BYTE * data                 = (BYTE *) _view;
    unsigned bytes_remaining    = _length;
    
    DWORD samples = 0;

    try
    {
        while ( bytes_remaining )
        {
            Ogg_Page_Header * page_header = (Ogg_Page_Header *) data;

            if ( page_header->magic == OGG_MAGIC )
            {
                samples = page_header->abs_granule_position;

                DWORD total_seg_length = 0;

                for ( int i = 0; i < page_header->page_segments; i++ )
                {
                    unsigned seg_length = ((BYTE *) & page_header->segment_table) [ i ];
                    total_seg_length += seg_length;
                }

                data            += (27 + page_header->page_segments + total_seg_length);
                bytes_remaining -= (27 + page_header->page_segments + total_seg_length);
            }
            else
            {
                data            ++;
                bytes_remaining --;
            }
        }
    }
    catch ( ... )
    {
    }

    return samples;
}

// End of File
