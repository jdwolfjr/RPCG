
#ifndef ABOUT_DLG_H
#define ABOUT_DLG_H

/*==========================================================================*/
/*                    P u b l i c   P r o t o t y p e s                     */
/*==========================================================================*/

typedef struct AboutData
{
    PlaylistGenerator   * generator;
    HBRUSH                background;
    HFONT                 font;
} AboutData;

INT_PTR CALLBACK About_DlgProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

#endif  // ABOUT_DLG_H