#include "Sayo_control_CLI.h"
#include "http.h"
#include "o2_protocol.h"
#include <cstdio>
#include <json/json.h>

#ifdef WIN32
#include "HOOK/hook_keyboard.h"
#endif
unsigned short a_server_port = 7296;
int a_debug = 0;

O2Protocol O2link[50];
string exePath;

int main(int argc, char **argv) {
#ifdef WIN32
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
#else
    exePath = argv[0];
    exePath = exePath.substr(0, exePath.find_last_of('/') + 1);
#endif

    fstream fs_read;
    fs_read.open("config.ini", ios::in);
    if (fs_read) {
        string temp;
        while (!fs_read.eof()) {
            getline(fs_read, temp);
            if (temp[0] == '#' || temp.length() < 3)
                continue;
            else {
                string str[2];
                std::istringstream is(temp);
                is >> str[0] >> str[1];
                if (str[0] == "SERVER_PORT")
                    a_server_port = (unsigned short) strtol(str[1].data(), 0, 10);
                else
                    cout << "不可识别的配置条目\n" << temp.data() << endl;
            }
        }
    }

    for (int i = 1; i < argc; i++) {
        string sa = argv[i];
        if (sa == "-p") {
            a_server_port = (unsigned short) strtol(argv[i + 1], 0, 0);
            i++;
            continue;
        }
        if (sa == "--debug") {
            a_debug = 1;
            continue;
        }
        if (sa == "-h" || sa == "--help") {
            cout << STR_HELP << endl;
            return 0;
        }
    }
    for (int i = 0; i < 10; i++) {
        O2link[i].path = NULL;
    }
    std::thread t1(httpServer);
    t1.detach();

    while (1) {
#ifdef WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
    }
    return 0;
}


int httpServer() {
#ifndef WIN32
    int ss = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(a_server_port);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(ss, (struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(ss, 20) == -1) {
        perror("listen");
        exit(1);
    }

    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    char buffer[1024];
    cout << "open http://127.0.0.1:" << a_server_port << " to setting..." << endl;
#ifdef __APPLE__
    string open_web = "open http://127.0.0.1:" + to_string(a_server_port);
    system(open_web.c_str());
#endif
    while (1) {
        using namespace std;
        int conn;
        conn = accept(ss, (struct sockaddr *) &client_addr, &length);
        std::thread _01(http, conn, client_addr.sin_addr.s_addr);
        _01.detach();
    }
    close(ss);
    return 0;

#else
    //初始化WSA
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0) {
        cout << "WSA error !" << endl;
        return -1;
    }

    //创建套接字
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (slisten == INVALID_SOCKET) {
        cout << "socket error !" << endl;
        return 0;
    }

    //绑定IP和端口
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(a_server_port);
    sin.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(slisten, (LPSOCKADDR) &sin, sizeof(sin)) == SOCKET_ERROR) {
        cout << "bind error !" << endl;
        ;
    }

    //开始监听
    if (listen(slisten, 5) == SOCKET_ERROR) {
        cout << "listen error !" << endl;
        return 0;
    }

    //循环接收数据
    sockaddr_in remoteAddr;
    int nAddrlen = sizeof(remoteAddr);
    cout << "HTTP Waiting for connection..." << endl;
    cout << "设定过程中请不要关闭本程序！" << endl;
    cout << "设定过程中请不要关闭本程序！！" << endl;
    cout << "设定过程中请不要关闭本程序！！！" << endl;
    cout << "open http://127.0.0.1:" << a_server_port << " to setting..." << endl;
    wchar_t url[64];
    swprintf(url, 64, L"http://127.0.0.1:%d", a_server_port);
    ShellExecute(0, 0, url, 0, 0, 0);
    SOCKET sClient;
    while (true) {
        using namespace std;
        sClient = accept(slisten, (SOCKADDR *) &remoteAddr, &nAddrlen);
        std::thread _01(http, sClient, remoteAddr.sin_addr.S_un.S_addr);
        _01.detach();
    }
#ifdef WIN32
    closesocket(slisten);
    WSACleanup();
#else
#endif
    return 0;

#endif
}


int http(int sClient, in_addr_t sClinentAddr) {
    std::random_device rd;
    if (sClient == -1) {
        cout << "accept error !" << endl;
        return 0;
    }
    char *recvbuffer = NULL;

    try {
        recvbuffer = new char[HTTP_BUF_SIZE];
    } catch (std::bad_alloc &memExp) {
        cout << "bad_alloc on Sayo_control_CLI::http::recvbuffer = new char[BUF_SIZE + 32]" << memExp.what() << endl;
        return -1;
    }

    REQUEST req;
    string filepath;
    GetGmtTime(req.accept_time, 32);
    //接收数据
    int ret = recv(sClient, recvbuffer, HTTP_BUF_SIZE - 1, 0);
    if (ret > 20) {
        recvbuffer[ret] = 0;
        AnalysisRequest(req, recvbuffer);
        if (req.type != REQ_ERROR) {
            req.response_code = 200;
            req.response_code_text = "OK";
            req.response_content_length = 0;
            if (a_debug) {
                cout << (sClinentAddr & 0xff) << "." << ((sClinentAddr & 0xff00) >> 8) << "."
                     << ((sClinentAddr & 0xff0000) >> 16) << "." << ((sClinentAddr & 0xff000000) >> 24) << "\t "
                     << req.uri << endl;
            }
            if (req.uri.find("/API/DEVICES") != string::npos) {
                req.content_type = "application/json;charset=utf-8";
                string cmd;
                Json::Reader reader;
                Json::Value root;
                if (reader.parse(req.body, root) && root.size()) {
                    cmd = root["cmd"].asString();
                    if (cmd == "search") {
                        unsigned short vid, pid;
                        vid = (unsigned short) root["vendor_id"].asInt();
                        pid = (unsigned short) root["product_id"].asInt();
                        req.response_body = O2link[0].SearchDrivers(vid, pid);
                        for (int i = 1; i < 50; i++) {
                            if (O2link[i].connected) {
                                O2link[i].Check();
                            }
                        }
                    } else if (cmd == "connect") {
                        int i = 1;
                        for (; i < 50; i++) {
                            if (O2link[i].connected == 0) break;
                        }
                        req.response_body = O2link[i].Connect(root["path"].asString(), i);
                    } else if (cmd == "disconnect") {
                        req.response_body = O2link[root["session"].asInt()].Disconnect();
                    }
                    else if (cmd == "buttons")
                    {
                        req.response_body = O2link[root["session"].asInt()].Buttons(root);
                    }
                    else if (cmd == "key_map")
                    {
                        req.response_body = O2link[root["session"].asInt()].Key_map(root);
                    }
                    else if (cmd == "lighting")
                    {
                        req.response_body = O2link[root["session"].asInt()].Lighting(root);
                    }
                    else if (cmd == "script")
                    {
                        req.response_body = O2link[root["session"].asInt()].Script(root);
                    }
                    else if (cmd == "script_sw")
                    {
                        req.response_body = O2link[root["session"].asInt()].Script_sw(root);
                    }
                    else if (cmd == "dev_id")
                    {
                        req.response_body = O2link[root["session"].asInt()].Dev_id(root);
                    }
                    else if (cmd == "dev_name")
                    {
                        req.response_body = O2link[root["session"].asInt()].Dev_name(root);
                    }
                    else if (cmd == "ok_pwd")
                    {
                        req.response_body = O2link[root["session"].asInt()].Ok_pwd(root);
                    }
                    else if (cmd == "save")
                    {
                        req.response_body = O2link[root["session"].asInt()].Save();
                    }
                    else if (cmd == "api_code")
                    {
                        req.response_body = O2link[root["session"].asInt()].Api(root);
                    }
//#ifdef WIN32
                    else if (cmd == "string_gbk")
                    {
                        req.response_body = O2link[root["session"].asInt()].String_gbk(root);
                    }
//#endif // WIN32
                    else if (cmd == "bootloader_code")
                    {
                        req.response_body = O2link[root["session"].asInt()].Bootloader(root);
                    }/*
                    else if (cmd == "firmware_write")
                    {
                        req.response_body = O2link[root["session"].asInt()].Firmware_write(root);
                    }*/
#ifdef WIN32
                    else if (cmd == "key_capture_install")
                    {
                        req.response_body = "{\"status\":0}";
                        cli_event |= CLIEVENT_HOOKKEY_INSTALL;
                        Sleep(5);
                    }
                    else if (cmd == "key_capture_uninstall")
                    {
                        req.response_body = "{\"status\":0}";
                        cli_event |= CLIEVENT_HOOKKEY_UNINSTALL;
                    }
                    else if (cmd == "get_key_event")
                    {
                        int timeout = 100;
                        if (root["timeout"].isInt())
                        {
                            timeout = root["timeout"].asInt();
                        }
                        root.clear();
                        EVENT_HIDKEY key;
                        int kr;
                        do
                        {
                            if ((kr = get_key_event(&key)) != 0)
                            {
                                if (kr > 0)
                                {
                                    root["status"] = 0;
                                    root["key_code"] = key.value;
                                    root["key_flags"] = key.flags;
                                    root["key_type"] = key.type;
                                    req.response_body = root.toStyledString();
                                }
                                else
                                {
                                    root["status"] = -1;
                                    root["key_code"] = "Please install the key_capture first";
                                    req.response_body = root.toStyledString();
                                }
                                break;
                            }
                            else
                            {
                                Sleep(1);

                                timeout--;
                            }
                        } while (timeout);
                        if (timeout == 0)
                        {
                            req.response_body = "{\"status\":-1,\"message\":\"timeout\"}";
                        }
                    }

#endif // WIN32
                    else
                    {
                        req.response_body = "{\"status\":-1,\"message\":\"No cmd parameters found\"}";
                    }
                }
                req.response_content_length = req.response_body.length();
            }
            else if (req.type == REQ_GET) {
                ifstream fs_read;
                if (req.uri == "/")
                {
                    req.uri = "/index.html";
                }
            web_home:
                filepath = exePath + "html" + req.uri;
                fs_read.open(filepath, ios::in | ios::binary);
                // if (req.uri.length() > 1)
                if (!fs_read) {
                    filepath = exePath + "html" + req.uri + "index.html";
                    fs_read.open(filepath, ios::in | ios::binary);
                    if (!fs_read) {
                        filepath = exePath + "html" + req.uri + "index.htm";
                        fs_read.open(filepath, ios::in | ios::binary);
                        if (!fs_read) {
                            if (req.uri.find_first_of('.') == string::npos)
                            {
                                req.uri = "/index.html";
                                goto web_home;
                            }
                            req.response_code = 404;
                            req.response_code_text = "Not Found";
                            req.response_body = "404 Not Found";
                            req.response_content_length = req.response_body.length();
                        }
                        else {
                            req.content_type = "text/html;charset=utf-8";
                        }
                    }
                    else {
                        req.content_type = "text/html;charset=utf-8";
                    }
                }
                if (fs_read) {
                    fs_read.seekg(0, fs_read.end);
                    req.response_content_length = fs_read.tellg();
                    fs_read.seekg(0, fs_read.beg);
                    size_t pos = req.uri.find_last_of('.');
                    if (pos != string::npos) {
                        string file_type = req.uri.substr(pos + 1);
                        if (file_type == "HTML" || file_type == "html")
                            req.content_type = "text/html;charset=utf-8";
                        else if (file_type == "js")
                            req.content_type = "application/javascript";
                        else if (file_type == "css")
                            req.content_type = "text/css";
                        else if (file_type == "xml")
                            req.content_type = "text/xml";
                        else if (file_type == "json")
                            req.content_type = "application/json;charset=utf-8";
                        else if (file_type == "svg")
                            req.content_type = "image/svg+xml";
                        else
                            req.content_type = "application/octet-stream";
                    }
                    req.sent_size = 0;
                    fs_read.close();
                }
                else {
                    req.content_type = "text/html;charset=utf-8";
                }
            }
#ifdef WIN32
            sprintf_s(recvbuffer, HTTP_BUF_SIZE, "\
HTTP/1.1 %d %s\r\n\
date: %s\r\n\
content-length: %d\r\n\
cache-control: public;max-age=300\r\n\
server: Sayobot_HTTP/0.0.5\r\n\
access-control-allow-origin: *\r\n\
content-type: %s\r\n\
connection: close\r\n\
\r\n\
",
                      req.response_code, req.response_code_text.data(), req.accept_time, req.response_content_length,
                      req.content_type.data());
#else
            sprintf(recvbuffer, "\
HTTP/1.1 %d %s\r\n\
date: %s\r\n\
content-length: %d\r\n\
cache-control: public;max-age=300\r\n\
server: Sayobot_HTTP/0.0.5\r\n\
access-control-allow-origin: *\r\n\
content-type: %s\r\n\
connection: close\r\n\
\r\n\
",
                    req.response_code, req.response_code_text.data(), req.accept_time, req.response_content_length,
                    req.content_type.data());

#endif
            // cout << recvbuffer << endl;
            send(sClient, recvbuffer, strlen(recvbuffer), 0);
            if (req.response_body.length() == 0 && req.response_content_length > 0) {
                ifstream _read(filepath, ios::in | ios::binary);
                if (_read) {
                    do {
                        _read.read(recvbuffer, req.response_content_length > HTTP_BUF_SIZE
                                                   ? HTTP_BUF_SIZE
                                                   : req.response_content_length);
                        send(sClient, recvbuffer,
                             req.response_content_length > HTTP_BUF_SIZE ? HTTP_BUF_SIZE : req.response_content_length,
                             0);
                        req.response_content_length -=
                            req.response_content_length > HTTP_BUF_SIZE ? HTTP_BUF_SIZE : req.response_content_length;
                    } while (!_read.eof() && req.response_content_length);
                    _read.close();
                }
            } else if (req.response_content_length > 0) {
                send(sClient, req.response_body.data(), req.response_content_length, 0);
            }
            close(sClient);


            delete[] recvbuffer;
            return 0;
        } else {
#ifdef WIN32
            sprintf_s(recvbuffer, HTTP_BUF_SIZE, "\
HTTP/1.1 400 Bad Request\r\n\
date: %s\r\n\
content-length: 15\r\n\
server: Sayobot_HTTP/0.0.5\r\n\
connection: close\r\n\
\r\n\
400 Bad Request\
",
                      req.accept_time);
#else

            sprintf(recvbuffer, "\
HTTP/1.1 400 Bad Request\r\n\
date: %s\r\n\
content-length: 15\r\n\
server: Sayobot_HTTP/0.0.5\r\n\
connection: close\r\n\
\r\n\
400 Bad Request\
",
                    req.accept_time);
#endif
            // cout << recvbuffer << endl;
            send(sClient, recvbuffer, strlen(recvbuffer), 0);
            close(sClient);
        }
    } else
        send(sClient, "", 1, 0);
    close(sClient);

    delete[] recvbuffer;
    return 0;
}
