#pragma once
#include <stdio.h>
//#include <Windows.h>
#include <string>
#include <time.h>

using std::string;

#define HTTP_BUF_SIZE 20480

enum REQUEST_CODE {
    REQ_ERROR = 0,
    REQ_GET,
    REQ_HEAD,
    REQ_POST,
    REQ_PUT,
    REQ_DELETE,
    REQ_CONNECT,
    REQ_OPTIONS,
    REQ_TRACE
};


struct REQUEST {
    char type = 0;
    char accept_time[32];
    char connection = 0;
    int response_code = 0;
    int content_cength = 0;
    char version = 0;
    char HostPort = 0;
    unsigned long response_content_length = 0;
    unsigned long sent_size = 0;
    string response_code_text;
    string response_body;
    string uri;
    string Accept;
    string Accept_Charset;
    string Accept_Language;
    string Authorization;
    string Cookie;
    string content_type;
    string Host;
    size_t If_Modified_Since;
    string Referer;
    string User_Agent;
    string body;
};

int GetGmtTime(char *szGmtTime, char l = 30);
int UrlDecode(char *url);
int AnalysisRequest(REQUEST &req, char *buffer);
/*
int InitializeReq(REQUEST &req);
int FreeReq(REQUEST &req);*/
