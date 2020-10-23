#include "tools.h"


const unsigned long table[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832,
    0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0, 0xfd62f97a,
    0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
    0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab,
    0xb6662d3d, 0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
    0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
    0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525,
    0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
    0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76,
    0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
    0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
    0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7,
    0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
    0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330,
    0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

unsigned long getCRC(const char *buf, int nLength) {
    if (nLength == 0) {
        nLength = strlen(buf);
    }
    if (nLength < 1) return 0xffffffff;

    unsigned long crc = 0;

    for (int i = 0; i != nLength; ++i) {
        crc = table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }

    crc = crc ^ 0xffffffff;

    return crc;
}


//����ֲ�汾 wstring => string
std::string ws2s(const std::wstring &ws) {
    std::string curLocale = setlocale(LC_ALL, "");
    const wchar_t *_Source = ws.c_str();
    size_t _Dsize = ws.length() * 4 + 1;
    char *_Dest = new char[_Dsize];
    memset(_Dest, 0, _Dsize);
    wcstombs(_Dest, _Source, _Dsize);
    std::string result = _Dest;
    delete[] _Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}

//����ֲ�汾 string => wstring
std::wstring s2ws(const std::string &s) {
    std::string curLocale = setlocale(LC_ALL, "");
    const char *_Source = s.c_str();
    size_t _Dsize = s.length() + 1;
    wchar_t *_Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    // mbstowcs(_Dest, _Source, _Dsize);
    mbstowcs(_Dest, _Source, _Dsize);
    std::wstring result = _Dest;
    delete[] _Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}

void unicodeToUTF8(const wstring &src, string &result) {
#ifdef WIN32
    int n = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, 0, 0, 0, 0);
    result.resize(n);
    ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, (char *) result.c_str(), n, 0, 0);
#else
    size_t InPutStrLen = src.length();
    unsigned long *InPutStr = NULL;
    char *OutPutStr = NULL;
    try {
        InPutStr = new unsigned long[InPutStrLen + 1];
        OutPutStr = new char[InPutStrLen * 8];
    } catch (std::bad_alloc) {
        std::cerr << "bad_alloc" << endl;
        return;
    }
    for (size_t l = 0; l < InPutStrLen; l++) {
        InPutStr[l] = src[l];
    }

    int i = 0, offset = 0;
    for (i = 0; i < InPutStrLen; i++) {
        if (InPutStr[i] <= 0x0000007f) {
            OutPutStr[offset++] = (char) (InPutStr[i] & 0x0000007f);
        }

        else if (InPutStr[i] >= 0x00000080 && InPutStr[i] <= 0x000007ff) {
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x000007c0) >> 6) | 0x000000e0);
            OutPutStr[offset++] = (char) ((InPutStr[i] & 0x0000003f) | 0x00000080);
        }

        else if (InPutStr[i] >= 0x00000800 && InPutStr[i] <= 0x0000ffff) {
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x0000f000) >> 12) | 0x000000e0);
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x00000fc0) >> 6) | 0x00000080);
            OutPutStr[offset++] = (char) ((InPutStr[i] & 0x0000003f) | 0x00000080);
        } else if (InPutStr[i] >= 0x00010000 && InPutStr[i] <= 0x0010ffff) {
            OutPutStr[offset++] = (char) ((((InPutStr[i] & 0x001c0000) >> 16) | 0x000000f0));
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x0003f000) >> 12) | 0x00000080);
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x00000fc0) >> 6) | 0x00000080);
            OutPutStr[offset++] = (char) ((InPutStr[i] & 0x0000003f) | 0x00000080);
        }
    }
    result = OutPutStr;
    delete[] InPutStr;
    delete[] OutPutStr;
    return;
#endif
}

string unicodeToUTF8(const wstring &src) {
#ifdef WIN32
    int n = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, 0, 0, 0, 0);
    string result;
    result.resize(n);
    ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, (char *) result.c_str(), n, 0, 0);
    return result;
#else
    size_t InPutStrLen = src.length();
    unsigned long *InPutStr = NULL;
    char *OutPutStr = NULL;
    try {
        InPutStr = new unsigned long[InPutStrLen + 1];
        OutPutStr = new char[InPutStrLen * 8];
    } catch (std::bad_alloc) {
        std::cerr << "bad_alloc" << endl;
        return "";
    }
    for (size_t l = 0; l < InPutStrLen; l++) {
        InPutStr[l] = src[l];
    }

    int i = 0, offset = 0;
    for (i = 0; i < InPutStrLen; i++) {
        if (InPutStr[i] <= 0x0000007f) {
            OutPutStr[offset++] = (char) (InPutStr[i] & 0x0000007f);
        }

        else if (InPutStr[i] >= 0x00000080 && InPutStr[i] <= 0x000007ff) {
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x000007c0) >> 6) | 0x000000e0);
            OutPutStr[offset++] = (char) ((InPutStr[i] & 0x0000003f) | 0x00000080);
        }

        else if (InPutStr[i] >= 0x00000800 && InPutStr[i] <= 0x0000ffff) {
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x0000f000) >> 12) | 0x000000e0);
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x00000fc0) >> 6) | 0x00000080);
            OutPutStr[offset++] = (char) ((InPutStr[i] & 0x0000003f) | 0x00000080);
        } else if (InPutStr[i] >= 0x00010000 && InPutStr[i] <= 0x0010ffff) {
            OutPutStr[offset++] = (char) ((((InPutStr[i] & 0x001c0000) >> 16) | 0x000000f0));
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x0003f000) >> 12) | 0x00000080);
            OutPutStr[offset++] = (char) (((InPutStr[i] & 0x00000fc0) >> 6) | 0x00000080);
            OutPutStr[offset++] = (char) ((InPutStr[i] & 0x0000003f) | 0x00000080);
        }
    }
    string ret = OutPutStr;
    delete[] InPutStr;
    delete[] OutPutStr;
    return ret;
#endif
}

std::string UnicodeToUTF8(const std::wstring &wstr) {
    std::string ret; /*
     try {
             std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
             ret = wcv.to_bytes(wstr);
     }
     catch (const std::exception& e) {
             std::cerr << e.what() << std::endl;
     }*/
    return ret;
}

std::wstring UTF8ToUnicode(const std::string &str) {
    std::wstring ret;
    /*
    try {
            std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
            ret = wcv.from_bytes(str);
    }
    catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
    }*/
    return ret;
}

std::string UnicodeToANSI(const std::wstring &wstr) {
    std::string ret;
    std::mbstate_t state = {};
    const wchar_t *src = wstr.data();
    size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::unique_ptr<char[]> buff(new char[len + 1]);
        len = std::wcsrtombs(buff.get(), &src, len, &state);
        if (static_cast<size_t>(-1) != len) {
            ret.assign(buff.get(), len);
        }
    }
    return ret;
}

std::wstring ANSIToUnicode(const std::string &str) {
    std::wstring ret;
    std::mbstate_t state = {};
    const char *src = str.data();
    size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::unique_ptr<wchar_t[]> buff(new wchar_t[len + 1]);
        len = std::mbsrtowcs(buff.get(), &src, len, &state);
        if (static_cast<size_t>(-1) != len) {
            ret.assign(buff.get(), len);
        }
    }
    return ret;
}

std::string UTF8ToANSI(const std::string &str) { return UnicodeToANSI(UTF8ToUnicode(str)); }

std::string ANSIToUTF8(const std::string &str) { return UnicodeToUTF8(ANSIToUnicode(str)); }
unsigned long str2addr(const char *ip_addr) {
    unsigned long ip = 0;
    int i = 0, j = 3, l = 0;
    while (ip_addr[i]) {
        switch (ip_addr[i]) {
            case '.':
                i++;
                j--;
                break;
            default:
                ip |= ((l = atoi(&ip_addr[i])) << (j * 8));
                if (l > 99) {
                    i += 3;
                } else if (l > 9) {
                    i += 2;
                } else {
                    i++;
                }
                break;
        }
    }
    return ip;
}


size_t checkstr(std::string str) { return str.find_first_of("&@%<>|\",;()=+!\\*/*:^."); }


//Сдת����д
void UpChar(char *str) {
    int i = 0;
    while (str[i]) {
        if (str[i] >= 97) str[i] -= 32;
        i++;
    }
    return;
}
//Сдת����д
int UpChar(string &astr) {
    char *str;
    try {
        str = new char[astr.length() + 1];
    } catch (const std::bad_alloc &) {
        return -1;
    }
#ifdef WIN32
    sprintf_s(str, astr.length() + 1, "%s", astr.data());
#else
    sprintf(str, "%s", astr.data());
#endif
    int i = 0;
    while (str[i]) {
        if (str[i] >= 97) str[i] -= 32;
        i++;
    }
    astr = str;
    delete[] str;
    return 0;
}
unsigned int unicodestrtoul(char *str, int radix, int maxCount) { return (unsigned) str2uint64(str, radix, maxCount); }


size_t find_first_of(char *str, char c) {
    if (str == NULL) return string::npos;
    int i = 0;
    while (str[i]) {
        if (str[i] == c) return i;
        i++;
    }
    return string::npos;
}

size_t find_last_of(char *str, char c) {
    if (str == NULL) return string::npos;
    int i = (int) strlen(str);
    while (--i >= 0) {
        if (str[i] == c) return i;
    }
    return string::npos;
}

size_t findstr(const char *src, const char *str) {
    if (str == NULL) return string::npos;
    size_t i = 0;
    while (str[i]) {
        if (str[i] == str[0]) {
            size_t j = 1;
            while (str[j] && str[i + j] == str[j]) {
                j++;
            }
            if (str[j] == 0) return i;
        }
        i++;
    }
    return string::npos;
}


unsigned long long str2uint64(const char *str, int radix, unsigned int maxCount) {
    unsigned long long ret = 0;
    unsigned long long temp = 0;
    int i = 0;
    if (radix == 10) {
        while (maxCount--) {
            if (str[i] >= '0' && str[i] <= '9') {
                temp *= 10;
                if (temp < ret) {
                    ret = 0xFFFFFFFFFFFFFFFF;
                    break;
                }
                temp += (str[i] - 48);
                ret = temp;
                i++;
            } else {
                break;
            }
        }
    } else if (radix == 16) {
        while (str[i] && maxCount--) {
            ret <<= 4;
            switch (str[i]) {
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case '0':
                    ret += (str[i] - 48);
                    break;
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                    ret += (str[i] - 55);
                    break;
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                    ret += (str[i] - 87);
                    break;
                default:
                    return ret;
            }
            i++;
            if (i > 16) {
                ret = 0xFFFFFFFFFFFFFFFF;
                break;
            }
        }
    } else {
        return 0;
    }
    return ret;
}


unsigned long long str2time_t(const char *date) {
    tm t;
    int iY, iM, iD, iH, iMin, iS;
    memset(&t, 0, sizeof(t));
#ifdef WIN32
    sscanf_s(date, "%d-%d-%d %d:%d:%d", &iY, &iM, &iD, &iH, &iMin, &iS);
#else
    sscanf(date, "%d-%d-%d %d:%d:%d", &iY, &iM, &iD, &iH, &iMin, &iS);
#endif
    t.tm_year = iY - 1900;
    t.tm_mon = iM - 1;
    t.tm_mday = iD;
    t.tm_hour = iH;
    t.tm_min = iMin;
    t.tm_sec = iS;
    return mktime(&t);
}
