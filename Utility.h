
#ifndef UTILITY_H
#define UTILITY_H

DWORD DisplaySystemError ( HWND hWnd );
BasicString SongTitleFromFilename ( BasicString & fn );
void EatWhite ( BasicString & fn );

std::wstring SingleByteToUnicode ( char * in_string, unsigned length );
std::string UnicodeToSingleByte ( wchar_t * in_string, unsigned length );

#endif  //  UTILITY_H
