
#ifndef PLAYLISTNAME_DLG_H
#define PLAYLISTNAME_DLG_H

/*==========================================================================*/
/*                    P u b l i c   P r o t o t y p e s                     */
/*==========================================================================*/

typedef struct PlaylistNameData
{
    TCHAR    playlist_name [ MAX_PATH ];
} PlaylistNameData;

INT_PTR CALLBACK PlaylistName_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

#endif  // PLAYLISTNAME_DLG_H