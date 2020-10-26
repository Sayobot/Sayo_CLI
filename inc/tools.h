#pragma once


#include <cstring> // for strcat()
#include <fstream>
#include <iostream>
#include <locale>
#include <stdio.h>
#include <string>
#include <thread>
#include <time.h>
#include <codecvt>

#ifdef WIN32
#include <Windows.h>
#include <codecvt>
#include <direct.h>
#include <io.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#else
#ifdef __APPLE__

#include <sys/uio.h>

#else
#include <sys/io.h>
#endif

#include <sys/stat.h>

#define UINT64 (unsigned long long)
#define UINT32 (unsigned long)
#define DWORD (unsigned long int)
#endif


using std::bad_alloc;
using std::cout;
using std::endl;
using std::fstream;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::thread;
using std::to_string;
using std::wcout;
using std::wifstream;
using std::wstring;


void unicodeToUTF8(const wstring &src, string &result);

string unicodeToUTF8(const wstring &src);


std::string UnicodeToUTF8(const std::wstring &wstr);

std::wstring UTF8ToUnicode(const std::string &str);

std::string UnicodeToANSI(const std::wstring &wstr);

std::wstring ANSIToUnicode(const std::string &str);

std::string UTF8ToANSI(const std::string &str);

std::string ANSIToUTF8(const std::string &str);

unsigned long str2addr(const char *ip_addr);

void UpChar(char *str);

int UpChar(string &astr);

int UrlDecode(char *url);

std::string ws2s(const std::wstring &ws);

std::wstring s2ws(const std::string &s);

unsigned long getCRC(const char *buf, int nLength = 0);


unsigned int unicodestrtoul(char *str, int radix, int maxCount = 4);

size_t find_first_of(char *str, char c);

size_t find_last_of(char *str, char c);

size_t findstr(const char *src, const char *str);

unsigned long long str2uint64(const char *str, int radix, unsigned int maxCount = 999);

unsigned long long str2time_t(const char *date);

size_t checkstr(std::string str);
