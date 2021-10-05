#include "http.h"
#include "tools.h"
#include <stdio.h>
#include <time.h>

const char *MONTHS[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

const char *WEEK[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

int GetGmtTime(char *szGmtTime, char l) {
    if (szGmtTime == NULL) {
        return -1;
    }
#ifdef WIN32
    __int64 rawTime;
    struct tm timeInfo;
    _time64(&rawTime);
    _gmtime64_s(&timeInfo, &rawTime);
    sprintf_s(szGmtTime, l, "%s, %02d %s %d %02d:%02d:%02d GMT", WEEK[timeInfo.tm_wday], timeInfo.tm_mday,
              MONTHS[timeInfo.tm_mon], timeInfo.tm_year + 1900, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
#else
    time_t rawTime;
    struct tm *timeInfo;
    time(&rawTime);
    timeInfo = gmtime(&rawTime);
    sprintf(szGmtTime, "%s, %02d %s %d %02d:%02d:%02d GMT", WEEK[timeInfo->tm_wday], timeInfo->tm_mday,
            MONTHS[timeInfo->tm_mon], timeInfo->tm_year + 1900, timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
#endif
    return (int) strlen(szGmtTime);
}


int UrlDecode(char *url) {
    if (url == NULL) return -1;

    int s = 0, e = 0;
    unsigned long code = 0;
    unsigned char p[13];
    memset(p, 0, sizeof(p));

    for (s = 0; url[s]; s++) {
        if (url[s] != '%') {
            url[e++] = url[s];
            continue;
        }
        s++;
        p[0] = url[s] - 48 - ((url[s] >= 'A') ? 7 : 0) - ((url[s] >= 'a') ? 32 : 0);
        s++;
        p[1] = url[s] - 48 - ((url[s] >= 'A') ? 7 : 0) - ((url[s] >= 'a') ? 32 : 0);
        code = (p[0] << 4) | p[1];
        p[0] = (p[0] << 4) | p[1];
        url[e++] = (char) code;
        memset(p, 0, sizeof(p));
    }
    url[e] = 0;
    return 0;
}

int geturi(char *dst, char *src) {
    char *p;
    p = strchr(src, ' ');
    if (p) {
        memcpy(dst, src, p - src);
        dst[p - src] = 0;
        return 0;
    }
    return -1;
}

int AnalysisRequest(REQUEST &req, char *const buffer) {
    char *p = (char *) buffer;
    char *strp = (char *) buffer;
    int i = 0;
    int step = 0;
    while (*p != 0) {
        if (*p == '\r') {
            if (*(++p) == '\n') {
                *(p - 1) = 0;
                // printf_s("%s\n", strp);
                if (step == 0) {
                    switch (strp[0]) {
                        case 'P':
                            if (strp[1] == 'O')
                                req.type = REQ_POST;
                            else
                                req.type = REQ_PUT;
                            break;
                        case 'G':
                            req.type = REQ_GET;
                            break;
                        case 'H':
                            req.type = REQ_HEAD;
                            break;
                        case 'D':
                            req.type = REQ_DELETE;
                            break;
                        case 'O':
                            req.type = REQ_OPTIONS;
                            break;
                        default:
                            req.type = REQ_ERROR;
                            break;
                    }
                    while (*(strp++) != ' ')
                        ;
                    geturi(buffer, strp);
                    UrlDecode(buffer);
                    UpChar(buffer);
                    req.uri = buffer;
                } else {
                    if (strp[0] >= 'A' && strp[0] <= 'Z') {
                        i = 0;
                        while (strp[i] != ':' && strp[i] != 0) {

                            if (strp[i] >= 'A' && strp[i] <= 'Z') strp[i] += 32;
                            i++;
                        }
                    }
                    switch (strp[0]) {
                        case 0:
                            if (req.type == REQ_POST) {
                                strp += 2;
                                req.body = strp;
                            }
                            break;
                        default:
                            break;
                    }
                }
                step++;
                strp = p + 1;
            }
        }
        p++;
    }
    return 0;
}

/*
int InitializeReq(REQUEST &req)
{
        if (&req == NULL)
                return -1;
        memset(&req, 0, sizeof(REQUEST));
        return 0;
};

int FreeReq(REQUEST &req)
{
        if (&req == NULL)
                return -1;
        if (req.Accept)
                delete[] req.Accept;
        if (req.Accept_Charset)
                delete[] req.Accept_Charset;
        if (req.Accept_Language)
                delete[] req.Accept_Language;
        if (req.Authorization)
                delete[] req.Authorization;
        if (req.User_Agent)
                delete[] req.User_Agent;
        if (req.Accept)
                delete[] req.Accept;
        if (req.Accept)
                delete[] req.Accept;
        if (req.Accept)
                delete[] req.Accept;
        if (req.Accept)
                delete[] req.Accept;
        if (req.Accept)
                delete[] req.Accept;
        if (req.Accept)
                delete[] req.Accept;
        if (req.Accept)
                delete[] req.Accept;
        if (req.Accept)
                delete[] req.Accept;
}*/
