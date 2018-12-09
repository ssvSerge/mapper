#include "pch.h"
#include <iostream>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <Windows.h>
#include <sstream>

using namespace std;

uint64_t            start_offset;
uint64_t            end_offset;
uint64_t            read_offset;
uint64_t            max_offset;
uint64_t            data_part;

char io_buffer [10 * 1024 * 1024];


template <class T> bool from_string (T &t, const std::string &s, std::ios_base & (*f)(std::ios_base&)) {

    std::istringstream iss (s);
    return !(iss >> f >> t).fail ();
}

int64_t S64 (const char *s) {

    int64_t i = 0;
    int     len = (int) strlen (s);
    
    if ( len <= 2 ) {
        from_string<__int64> (i, s, std::dec);
    } else
    if ( (s [0] == '0') && (s [1] == 'x') ) {
        from_string<__int64> (i, s, std::hex);
    } else {
        from_string<__int64> (i, s, std::dec);
    }

    return i;
}

int main(int argc, char* argv[]) {

    HANDLE              hInFile;
    HANDLE              hOutFile;
    DWORD               io_cnt;

    if ( argc != 5 ) {
        cout << "lfc.exe [in_file] [out_file] [start_offset] [end_offset]" << endl;
        return -1;
    }


    hInFile = CreateFile (argv [1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if ( hInFile == INVALID_HANDLE_VALUE ) {
        cout << "Cannot open input file: " << argv [1] << endl;
        return -2;
    }

    {   LARGE_INTEGER       in_file_size;
        GetFileSizeEx (hInFile, &in_file_size);
        max_offset = (uint64_t)in_file_size.QuadPart;
    }

    hOutFile = CreateFile (argv [2], GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    if ( hOutFile == INVALID_HANDLE_VALUE ) {
        cout << "Cannot open output file: " << argv [2] << endl;
        return -3;
    }

    start_offset = S64 ( argv [3] );
    end_offset   = S64 ( argv [4] );

    if ( max_offset <= end_offset ) {
        cout << "File is too small: " << end_offset << endl;
        return -4;
    }

    read_offset  = 0;

    while ( read_offset < start_offset ) {

        data_part = start_offset - read_offset;

        if ( data_part > sizeof (io_buffer) ) {
            data_part = sizeof (io_buffer);
        }

        ReadFile  (hInFile,  io_buffer, (DWORD)data_part, NULL, NULL);
        WriteFile (hOutFile, io_buffer, (DWORD)data_part, &io_cnt, NULL);

        read_offset += data_part;
    }


    LARGE_INTEGER tmp;
    tmp.QuadPart = end_offset;
    SetFilePointerEx (hInFile, tmp, NULL, FILE_BEGIN);

    read_offset = end_offset;
    while ( read_offset < max_offset ) {

        data_part = max_offset - read_offset;

        if ( data_part > sizeof (io_buffer) ) {
            data_part = sizeof (io_buffer);
        }

        ReadFile (hInFile, io_buffer, (DWORD)data_part, NULL, NULL);
        WriteFile (hOutFile, io_buffer, (DWORD)data_part, &io_cnt, NULL);

        read_offset += data_part;
    }

    CloseHandle (hInFile);
    CloseHandle (hOutFile);

    return 0;
}

