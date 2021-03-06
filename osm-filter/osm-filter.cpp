

#include <Windows.h>
#include <string.h>
#include <expat.h>
#include <vector>
#include <iostream>
#include <sstream>


using namespace std;


#define  IO_BUFF_LEN    (10 * 1024 * 1024)


typedef enum tag_xml_state {
    XML_STATE_IDLE,                     // 
    XML_STATE_NODE_NAME_BEGIN,          // "<"
    XML_STATE_NODE_NAME,                // "<node"
    XML_STATE_NODE_NAME_END,            // "<node   "
    XML_STATE_PARAM_NAME,               // "<node     param"
    XML_STATE_PARAM_NAME_END,           // "<node     param   "
    XML_STATE_PARAM_EQU,                // "<node     param   ="
    XML_STATE_PARAM_VAL,                // "<node     param   =""
    XML_STATE_PARAM_VAL_END,            // "<node     param   =   \"value\""
    XML_STATE_NODE_END,                 // "<node param="val" /"
}   XML_STATE;


class InStream {

    public:

        InStream() {
            m_iViewSize     = IO_BUFF_LEN;
            m_hFile         = INVALID_HANDLE_VALUE;
            m_hView         = INVALID_HANDLE_VALUE;
            m_iFileSize     = 0;
            m_pData         = NULL;
        }


        bool Init(string oInFileName) {

            bool retVal = false;

            m_hFile = CreateFile(oInFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

            if ( m_hFile != INVALID_HANDLE_VALUE ) {

                LARGE_INTEGER iInFileSize;
                SIZE_T view_size;
                GetFileSizeEx(m_hFile, &iInFileSize);
                m_iFileSize = iInFileSize.QuadPart;

                view_size = m_iFileSize;
                if ( view_size > IO_BUFF_LEN ) {
                    view_size = IO_BUFF_LEN;
                }

                m_hView = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
                if ( m_hView != INVALID_HANDLE_VALUE ) {
                    LARGE_INTEGER iViewOffset;
                    iViewOffset.QuadPart = 0;
                    m_pData = (char*)MapViewOfFile(m_hView, FILE_MAP_READ, iViewOffset.HighPart, iViewOffset.LowPart, view_size);
                    if ( m_pData != NULL ) {
                        retVal = true;
                    }
                }
            }

            if ( !retVal ) {
                cout << "InStream Error Init failed" << endl;
            }

            return retVal;
        }


       ~InStream() {

           if ( m_pData != NULL ) {
               UnmapViewOfFile(m_pData);
               m_pData = NULL;
           }

           if ( m_hView != INVALID_HANDLE_VALUE ) {
               CloseHandle(m_hView);
               m_hView = INVALID_HANDLE_VALUE;
           }

           if ( m_hFile != INVALID_HANDLE_VALUE ) {
               CloseHandle(m_hFile);
               m_hFile = INVALID_HANDLE_VALUE;
           }

           m_iFileSize = 0;
           m_iPageId   = 0;
           m_iViewSize = 0;

        }


        uint64_t GetSize(void) {

            return m_iFileSize;
        }


        char operator[] (uint64_t idx) {

            uint64_t view_page_id;
            uint64_t view_page_offset;

            view_page_id     = idx / m_iViewSize;
            view_page_offset = idx % m_iViewSize;

            if ( idx >= m_iFileSize ) {
                cout << "InStream Error idx out of range " << endl;
            }

            if ( view_page_id != m_iPageId ) {

                LARGE_INTEGER iViewOffset;
                int64_t iViewSize;
                iViewOffset.QuadPart  = view_page_id;
                iViewOffset.QuadPart *= m_iViewSize;

                iViewSize = m_iFileSize - iViewOffset.QuadPart;
                if ( iViewSize > m_iViewSize ) {
                    iViewSize = m_iViewSize;
                }

                UnmapViewOfFile(m_pData);
                m_pData = (char*)MapViewOfFile(m_hView, FILE_MAP_READ, iViewOffset.HighPart, iViewOffset.LowPart, iViewSize);
                m_iPageId = view_page_id;

            }

            if ( m_pData == NULL ) {
                cout << "InStream Error m_pData is NULL " << endl;
                return 0;
            }

            return m_pData[view_page_offset];
        }


    private:
        HANDLE          m_hFile;
        HANDLE          m_hView;
        uint64_t        m_iFileSize;
        const char*     m_pData;
        uint64_t        m_iPageId;
        uint32_t        m_iViewSize;
};


class OutStream {

    public:
        OutStream(void) {

            m_hFile      = INVALID_HANDLE_VALUE;
            m_iDataLen   = 0;
            m_pOutBuffer = NULL;
        }


        bool Init(string oOutFileName) {

            m_pOutBuffer = new char [IO_BUFF_LEN + 32];
            if ( m_pOutBuffer == NULL ) {
                cout << "OutStream Error m_pOutBuffer is failed" << endl;
                return false;
            }

            m_hFile = CreateFile(oOutFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
            if ( m_hFile == INVALID_HANDLE_VALUE ) {
                cout << "OutStream Error Init failed" << endl;
                return false;
            }

            return true;
        }


        void Close(void) {

            if ( m_pOutBuffer != NULL ) {
                if ( m_iDataLen > 0 ) {
                    DWORD iIoCnt;
                    WriteFile(m_hFile, m_pOutBuffer, m_iDataLen, &iIoCnt, NULL);
                    m_iDataLen = 0;
                }
            }
        }


       ~OutStream() {

           if ( m_hFile != INVALID_HANDLE_VALUE ) {
               DWORD iIoCnt;
               if ( m_iDataLen > 0 ) {
                   WriteFile(m_hFile, m_pOutBuffer, m_iDataLen, &iIoCnt, NULL);
                   m_iDataLen = 0;
               }
               CloseHandle(m_hFile);
               m_hFile = INVALID_HANDLE_VALUE;
               m_iDataLen = 0;
           }

           if ( m_pOutBuffer != NULL ) {
               delete m_pOutBuffer;
               m_pOutBuffer = NULL;
           }

        }


        void operator+= (const string& data) {

            for ( int i = 0; i < data.size(); i++ ) {

                m_pOutBuffer [m_iDataLen] = data[i];
                m_iDataLen++;

                if ( m_iDataLen >= IO_BUFF_LEN ) {
                    DWORD iIoCnt;
                    WriteFile(m_hFile, m_pOutBuffer, m_iDataLen, &iIoCnt, NULL);
                    m_iDataLen = 0;
                }
            }

            m_pOutBuffer [m_iDataLen] = 0;
        }


        void operator+= (const char* data) {

            while ( (*data) != 0 ) {

                m_pOutBuffer [m_iDataLen] = *data;
                data++;
                m_iDataLen++;

                if ( m_iDataLen >= IO_BUFF_LEN ) {
                    DWORD iIoCnt;
                    WriteFile(m_hFile, m_pOutBuffer, m_iDataLen, &iIoCnt, NULL);
                    m_iDataLen = 0;
                }

            }

            m_pOutBuffer [m_iDataLen] = 0;
        }


        void operator+= (const char data) {

            m_pOutBuffer[m_iDataLen] = data; 
            m_iDataLen++;
            if ( m_iDataLen >= IO_BUFF_LEN ) {
                DWORD iIoCnt;
                WriteFile(m_hFile, m_pOutBuffer, m_iDataLen, &iIoCnt, NULL);
                m_iDataLen = 0;
            }

            m_pOutBuffer [m_iDataLen] = 0;
        }


    private:
        HANDLE          m_hFile;
        char*           m_pOutBuffer;
        DWORD           m_iDataLen;
};


string          oInFileName;
string          oOutFileName;
vector<string>  oSkipList;


bool IsDelim(char ch) {

    bool retVal = false;

    switch ( ch ) {
        case  ' ':
        case '\r':
        case '\n':
        case '\t':
            retVal = true;
    }

    return retVal;
}


void ProcessParams(const string& nodeName, const string& paramName, const string& paramVal, bool singleComa, OutStream& oOutStr) {

    bool isNameValid = false;
    bool isRejected  = false;

    if ( nodeName == "node" ) {
        isNameValid = true;
    } else
    if ( nodeName == "way" ) {
        isNameValid = true;
    } else
    if ( nodeName == "relation" ) {
        isNameValid = true;
    } else
    if ( nodeName == "member" ) {
        isNameValid = true;
    } else
    if ( nodeName == "tag" ) {
        isNameValid = true;
    } else
    if ( nodeName == "nd" ) {
        isNameValid = true;
    }

    if ( isNameValid ) {
        for ( size_t i = 0; i < oSkipList.size(); i++ ) {
            if ( oSkipList [i] == paramName ) {
                isRejected = true;
                break;
            }
        }
    }

    if ( ! isRejected ) {
        oOutStr += " ";
        oOutStr += paramName;
        if ( singleComa ) {
            oOutStr += "=\'";
            oOutStr += paramVal;
            oOutStr += "\' ";
        } else {
            oOutStr += "=\"";
            oOutStr += paramVal;
            oOutStr += "\"";
        }
    }

    return;
}


int main (int argc, char* argv[] ) {

    if ( argc != 4 ) {
        cout << "Usage:    osm-filter InFile=<file_name> OutFile=<file_name> Skip=tag1,tag2,tag3 " << endl;
        cout << "Example:  osm-filter InFile=src.osm OutFile=dst.osm Skip=timestamp,uid,user,changeset" << endl;
        return -1;
    }

    for ( int i = 1; i < argc; i++ ) {

        istringstream f(argv[i]);
        string param;
        string val;

        getline(f, param, '=');
        getline(f, val,   ' ');

        if ( param == "InFile" ) {
            oInFileName = val;
        } else 
        if ( param == "OutFile" ) {
            oOutFileName = val;
        } else
        if ( param == "Skip" ) {
            istringstream z(val);
            string s;
            while ( getline(z, s, ',') ) {
                oSkipList.push_back(s);
            }

        }

    }

    if ( oInFileName == "" ) {
        cout << "Input file is not specified" << endl;
        return -2;
    }

    if ( oOutFileName == "" ) {
        cout << "Output file name is not specified" << endl;
        return -3;
    }

    if ( oSkipList.size() == 0 ) {
        cout << "Skip list is not specified" << endl;
        return -4;
    }

    InStream    oInStr;
    OutStream   oOutStr;
    XML_STATE   iXmlParserState;
    uint64_t    Len;
    char        ch;
    string      nodeName;
    string      paramName;
    string      paramVal;
    bool        singleComa;


    oInStr.Init(oInFileName);
    oOutStr.Init(oOutFileName);

    Len = oInStr.GetSize();

    iXmlParserState = XML_STATE_IDLE;
    singleComa = false;

    uint64_t i;
    for ( i = 0; i < Len; i++ ) {

        ch = oInStr[i];

        if ( iXmlParserState == XML_STATE_IDLE ) {
            // "<"
            oOutStr += ch;
            if ( ch == '<' ) {
                iXmlParserState = XML_STATE_NODE_NAME;
            }
        }   else
        if ( iXmlParserState == XML_STATE_NODE_NAME ) {
            // "<node"
            if ( IsDelim(ch) ) {
                iXmlParserState = XML_STATE_NODE_NAME_END;
            } else 
            if ( ch == '>' ) {
                // "</node>"
                iXmlParserState = XML_STATE_IDLE;
                oOutStr += ch;
                nodeName = "";
            } else {
                oOutStr += ch;
                nodeName += ch;
            }
        }   else
        if ( iXmlParserState == XML_STATE_NODE_NAME_END ) {
            // "<node "
            if ( IsDelim(ch) ) {
                // oOutStr += ch;
            } else
            if ( ch == '>' ) {
                iXmlParserState = XML_STATE_IDLE;
                oOutStr += ch;
                nodeName = "";
            } else
            if ( ch == '/' ) {
                iXmlParserState = XML_STATE_NODE_END;
                oOutStr += ch;
            } else
            if ( ch == '?' ) {
                oOutStr += ch;
                iXmlParserState = XML_STATE_NODE_END;
            } else {
                iXmlParserState = XML_STATE_PARAM_NAME;
                paramName = ch;
            }
        }   else
        if ( iXmlParserState == XML_STATE_PARAM_NAME ) {
            // "<node param"
            if ( IsDelim(ch) ) {
                // "<node param "
                iXmlParserState = XML_STATE_PARAM_NAME_END;
            } else
            if ( ch == '=' ) {
                // "<node param="
                iXmlParserState = XML_STATE_PARAM_EQU;
            } else {
                paramName += ch;
            }
        }   else
        if ( iXmlParserState == XML_STATE_PARAM_NAME_END ) {
            // "<node param "
            if ( IsDelim(ch) ) {
                // Just skip it.
            } else 
            if ( ch == '=' ) {
                // "<node param ="
                iXmlParserState = XML_STATE_PARAM_EQU;
            } else {
                // Error. Not expected.
                cout << "Parsing failed at: " << i << " (pos: " << __LINE__ << ")" << endl;
                break;
            }
        }   else
        if ( iXmlParserState == XML_STATE_PARAM_EQU ) {
            // "<node param="
            if ( IsDelim(ch) ) {
                // "<node param= "
                // Just skip it.
            } else
            if (ch == '\"') {
                // "<node param=""
                iXmlParserState = XML_STATE_PARAM_VAL;
                singleComa = false;
            } else
            if (ch == '\'') {
                // "<node param=""
                iXmlParserState = XML_STATE_PARAM_VAL;
                singleComa = true;
            } else {
                // Error. Not expected.
                cout << "Parsing failed at: " << i << " (pos: " << __LINE__ << ")" << endl;
                break;
            }
        }   else
        if ( iXmlParserState == XML_STATE_PARAM_VAL ) {
            if ( (ch == '\"') || (ch == '\'') ){
                // "<node param="val""
                iXmlParserState = XML_STATE_NODE_NAME_END;
                ProcessParams(nodeName, paramName, paramVal, singleComa, oOutStr);
                paramName = "";
                paramVal  = "";
            } else
            if ( (ch == '\r') || (ch == '\n') ) {
                // Error. Not expected.
                cout << "Parsing failed at: " << i << " (pos: " << __LINE__ << ")" << endl;
                break;
            } else {
                paramVal += ch;
            }
        }   else
        if ( iXmlParserState == XML_STATE_NODE_END ) {
            if ( ch == '>' ) {
                iXmlParserState = XML_STATE_IDLE;
                oOutStr += ch;
                nodeName = "";
            } else {
                // Error. Not expected.
                cout << "Parsing failed at: " << i << " (pos: " << __LINE__ << ")" << endl;
                break;
            }
        }

    }

    oOutStr.Close();

    cout << "Done." << endl;

    return 0;
}




