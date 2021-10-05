#pragma once

// Headers needed for sleeping.
#include <json/json.h>
#include "hidapi.h"
#ifdef _WIN32
#include <windows.h>
#pragma comment (lib,"setupapi.lib")
#pragma comment(lib, "./hidapi-master/windows/Release/hidapi.lib") 
#else
#include <unistd.h>
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
#endif
#include "tools.h"



#define DEVICE_IDENTIFICATION_CODE_MIN 0x0002
#define DEVICE_IDENTIFICATION_CODE_O2  0x0002
#define DEVICE_IDENTIFICATION_CODE_O2C  0x0003
#define DEVICE_IDENTIFICATION_CODE_O2S  0x0004
#define DEVICE_IDENTIFICATION_CODE_O2T_ES  0x0005
#define DEVICE_IDENTIFICATION_CODE_O2T_QS  0x0006
#define DEVICE_IDENTIFICATION_CODE_O2MINI  0x0007
#define DEVICE_IDENTIFICATION_CODE_M1T4K  0x0008
#define DEVICE_IDENTIFICATION_CODE_MAX 0x0008


#define HID_TIMEOUT 1000

#pragma pack (1)
struct o2_hid_data
{
	union
	{
		struct {
			UCHAR id;
			UCHAR cmd;
			UCHAR data_len;
			union
			{
				struct {
					UCHAR versionH;
					UCHAR versionL;
					UCHAR modelH;
					UCHAR modelL;
				}info;
				struct {
					UCHAR addr_h;
					UCHAR addr_l;
					char data[58];
				}config;
				struct {
					UCHAR addr_h;
					UCHAR addr_l;
					char data[58];
				}script;

				struct {
					UCHAR pattern;
					UCHAR number;
					char name[32];
					UCHAR retain_2[26];
				}script_sw;

				struct {
					UCHAR pattern;
					UCHAR number;
					UCHAR type;
					UCHAR retain_1;
					union {
						struct {
							UCHAR plain_0;
							UCHAR plain_1;
							UCHAR plain_2;
							UCHAR plain_3;
						}keyboard;
						//struct {}mouse;
						//struct {}multimedia;
						UCHAR data[4];
					}data;
					UCHAR retain_2[52];
				}key;

				struct {
					UCHAR pattern;
					UCHAR number;
					UCHAR mode;
					UCHAR data[57];
				}api;

				struct {
					UCHAR pattern;
					UCHAR number;
					UCHAR mode;
					UCHAR type;

					USHORT site_x;
					USHORT site_y;

					USHORT retain;
					USHORT shape_x;

					USHORT shape_y;
					USHORT shape_r;

					struct {
						UCHAR mode;
						UCHAR retain;
						UCHAR plain_0;
						UCHAR plain_1;
						UCHAR plain_2;
						UCHAR plain_3;
					}key_lay[5];
					UCHAR retain_2[14];
				}bottom;



				struct {
					UCHAR pattern;
					UCHAR number;
					UCHAR type;
					UCHAR event;
					UCHAR lamp_cmd_len;
					union
					{
						struct {
							UCHAR col_r;
							UCHAR col_g;
							UCHAR col_b;
							UCHAR interval_time;
						}col;
						struct {
							UCHAR number;
							UCHAR interval_time;
							UCHAR retain_1;
							UCHAR retain_2;
						}sb;
						struct {
							UCHAR interval_time;
							UCHAR retain_1;
							UCHAR retain_2;
							UCHAR retain_3;
						}t;
						UCHAR data[4];
					} data;
					UCHAR retain[51];
				}lamp;
				struct {
					UCHAR pattern;
					char data[59];
				}name_of_device;
				struct {
					UCHAR pattern;
					UCHAR number;
					char pwd[58];
				}ok_pwd;
				char data[60];
				unsigned char udata[60];
			}data;
			UCHAR check_sum;
		}format;
		unsigned char udata[64];
		char data[64];
	};
};

#pragma pack ()

#define MAX_STR 255


struct SCRIPT_TMP
{
	UCHAR code = 0;
	USHORT value[4] = { 0 };
	UCHAR step;

};

class O2Protocol
{
public:
	char* path;
	int connected = 0;
	string SearchDrivers(unsigned short vid = 0x8089, unsigned short pid = 0x0);
	string Connect(string path, int session);
	string Disconnect();
	string Save();
	int Check();
	string Buttons(const Json::Value& data);
	string Key_map(const Json::Value& data);
	string Lighting(const Json::Value& data);
	string Script(const Json::Value& data);
	string Script_sw(const Json::Value& data);
	string Dev_id(const Json::Value& data);
	string Dev_name(const Json::Value& data);
	string Ok_pwd(const Json::Value& data);
	string Api(const Json::Value& data);
	string String_gbk(const Json::Value& data);
	string Bootloader(const Json::Value& data);
	//string Firmware_write(const Json::Value& data);
private:
	bool bootloaderMode = 0;
	hid_device* handle;
	o2_hid_data EpIn;
	o2_hid_data EpOut;
	int  test12 = 0;
	SCRIPT_TMP script[6144];
	unsigned char script_step_len[256];
};
