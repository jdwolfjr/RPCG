
#ifndef OPTIONS_DLG_H
#define OPTIONS_DLG_H

/*==========================================================================*/
/*                    P u b l i c   P r o t o t y p e s                     */
/*==========================================================================*/

typedef struct OptionsData
{
    unsigned    reminder;
    unsigned    days;
    unsigned    prompt_for_playlist;
    char        default_playlist_name [ MAX_PATH ];
    unsigned    size_mode;
} OptionsData;

INT_PTR CALLBACK Options_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

#endif  // OPTIONS_DLG_H