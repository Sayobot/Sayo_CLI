#include "o2_protocol.h"
#include "script.h"

#ifndef WIN32
#define sscanf_s sscanf
#define printf_s printf
#define sprintf_s sprintf
#define strcpy_s strcpy
#define Sleep(MS) usleep(MS * 1000)
#endif
#define OUT_CHECKSUM this->EpOut.udata[this->EpOut.format.data_len + 3]


const char* SeriesModel[DEVICE_IDENTIFICATION_CODE_MAX + 1] = {
	"?",
	"??",
	"O2(Standard)2+4Key(s)",
	"O2C(Standard)2+3Key(s)",
	"O2C(S)3+5Key(s)",
	"O2C(T_ES)2+3TouchKey(s)",
	"O2C(T_QS)Touchpad",
	"O2C(MINI)2Key(s)+1TouchKey(s)",
	"O2C(M1T4K)4TouchKey(s)"

};

typedef struct times
{
	int Year;
	int Mon;
	int Day;
	int Hour;
	int Min;
	int Second;
}Times;

string O2Protocol::SearchDrivers(unsigned short vid, unsigned short pid)
{
	int devn = 0;
	struct hid_device_info* devs, * cur_dev;

	devs = hid_enumerate(vid, pid);

	cur_dev = devs;

	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	/*rapidjson::StringBuffer strBuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
	writer.StartObject();
	writer.Key("data");
	writer.StartArray();*/

	try
	{
		while (cur_dev) {
			if (cur_dev->usage_page > 0xFF && cur_dev->usage == 1)
			{
				printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
				printf("\n");
				printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
				printf("  Product:      %ls\n", cur_dev->product_string);
				printf("  Release:      %hx\n", cur_dev->release_number);
				printf("  Interface:    %d\n", cur_dev->interface_number);
				printf("  Usage (page): 0x%hx (0x%hx)\n", cur_dev->usage, cur_dev->usage_page);
				printf("\n");
				value["vendor_id"] = cur_dev->vendor_id;
				value["product_id"] = cur_dev->product_id;
				if (cur_dev->product_id == 2)
					value["series"] = "O2 STM32";
				else if (cur_dev->product_id == 3)
					value["series"] = "O2C WCH";
				else
					value["series"] = "unknown";
				this->handle = hid_open_path(cur_dev->path);
				if (this->handle)
				{
					printf("  output_report_length: %d\n", 64);
					printf("  input_report_length: %d\n", 64);
					this->EpOut.format.id = 2;
					this->EpOut.format.cmd = 0;
					this->EpOut.format.data_len = 4;
					time_t tick = time(0);
					struct tm tm;
					char s[100];
					Times standard;
					tm = *localtime(&tick);
					strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);
					//printf("%d: %s\n", (int)tick, s);
					standard.Year = atoi(s);
					standard.Mon = atoi(s + 5);
					standard.Day = atoi(s + 8);
					standard.Hour = atoi(s + 11);
					standard.Min = atoi(s + 14);
					standard.Second = atoi(s + 17);
					this->EpOut.format.data.udata[0] = standard.Day;
					this->EpOut.format.data.udata[1] = standard.Hour;
					this->EpOut.format.data.udata[2] = standard.Min;
					this->EpOut.format.data.udata[3] = standard.Second;

					OUT_CHECKSUM = 0;
					for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
						OUT_CHECKSUM += this->EpOut.data[i];
					hid_write(this->handle, this->EpOut.udata, 64);
					if (hid_read_timeout(this->handle, this->EpIn.udata, 64, 2000) > 0)
					{
						value["available"] = true;
						if (this->EpIn.format.cmd == 0)
						{
							value["mode"] = "app";
							value["versoin"] = this->EpIn.format.data.data[0] * 256 + this->EpIn.format.data.data[1];
							if (this->EpIn.format.data_len > 2)
							{
								unsigned short model = this->EpIn.format.data.info.modelH * 256 + this->EpIn.format.data.info.modelL;

								value["model_code"] = model;
								if (model >= DEVICE_IDENTIFICATION_CODE_MIN && model <= DEVICE_IDENTIFICATION_CODE_MAX)
								{
									value["model"] = SeriesModel[model];
								}
								else
								{
									value["model"] = "unknown " + to_string(model);
								}
								if (this->EpIn.format.data_len > 8)
								{
									value2.clear();
									for (unsigned i = 0; i < (this->EpIn.format.data_len - 8); i++)
									{
										value2[i] = this->EpIn.format.data.udata[i + 8];
									}
									value["support_list"] = value2;
								}
							}
						}
						else if (this->EpIn.format.cmd == 2)
						{
							this->EpOut.format.cmd = 1;
							hid_write(this->handle, this->EpOut.udata, 64);
							if (hid_read_timeout(this->handle, this->EpIn.udata, 64, 5) > 0)
							{
								unsigned short model = this->EpIn.format.data.info.modelH * 256 + this->EpIn.format.data.info.modelL;
								if (model >= DEVICE_IDENTIFICATION_CODE_MIN && model <= DEVICE_IDENTIFICATION_CODE_MAX)
								{
									value["mode"] = (string)"bootloader" + SeriesModel[model];
								}
								else
								{
									value["mode"] = "bootloader unknown";
								}
							}
							value["versoin"] = this->EpIn.format.data.data;
						}
						else
						{
							value["mode"] = "unknown";
						}



					}
					else
					{
						value["available"] = false;
					}
					hid_close(this->handle);

				}
				else
				{
					value["available"] = false;
				}
				value["path"] = cur_dev->path;
				value["release_number"] = cur_dev->release_number;
				value["interface_number"] = cur_dev->interface_number;
				if (cur_dev->serial_number)
					value["serial_number"] = ws2s(cur_dev->serial_number);
				else
					value["serial_number"] = Json::nullValue;

				if (cur_dev->manufacturer_string)
					value["manufacturer_string"] = ws2s(cur_dev->manufacturer_string);
				else
					value["manufacturer_string"] = Json::nullValue;

				if (cur_dev->product_string)
					value["product_string"] = unicodeToUTF8((wstring)cur_dev->product_string).data();
				else
					value["product_string"] = Json::nullValue;

				value["usage_page"] = cur_dev->usage_page;
				value["usage"] = cur_dev->usage;
				value["interface_number"] = cur_dev->interface_number;
				root["data"].append(value);
				value.clear();
				devn++;
			}
			cur_dev = cur_dev->next;
		}
	}
	catch (...)
	{
		cout << "search error" << endl;
	}
	hid_free_enumeration(cur_dev);
	root["status"] = devn ? 0 : -1;
	if (devn == 0)
		root["message"] = u8"deviceNotFound_msg";
	root["devices"] = devn;
	return root.toStyledString();
}

string O2Protocol::Connect(string p, int session)
{
	this->handle = hid_open_path(p.data());
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value;
	root.clear();
	if (this->handle != NULL)
	{
		hid_set_nonblocking(handle, 1);
		this->connected = 1;
		root["session"] = session;
		root["status"] = 0;
	}
	else
	{
		root["message"] = u8"deviceNotFound_msg";
		root["status"] = -1;
	}
	memset(this->script_step_len, 1, sizeof(this->script_step_len));

	this->script_step_len[S_EXIT_NOT_CLEAN] = 1;
	this->script_step_len[S_NOP] = 1;
	this->script_step_len[S_JMP] = 3;
	this->script_step_len[S_SJMP] = 2;
	this->script_step_len[S_AJMP] = 2;
	this->script_step_len[S_SLEEP_X256] = 2;
	this->script_step_len[S_SLEEP] = 2;
	this->script_step_len[S_SLEEP_RAND_X256] = 2;
	this->script_step_len[S_SLEEP_RAND] = 2;
	this->script_step_len[S_SLEEP_X256_VAL] = 2;
	this->script_step_len[S_SLEEP_VAL] = 2;
	this->script_step_len[S_SLEEP_RAND_X8_VAL] = 2;
	this->script_step_len[S_SLEEP_RAND_VAL] = 2;
	this->script_step_len[S_SK] = 2;
	this->script_step_len[S_GK] = 2;
	this->script_step_len[S_MK] = 2;
	this->script_step_len[S_MU] = 2;
	this->script_step_len[S_SK_VAL] = 2;
	this->script_step_len[S_GK_VAL] = 2;
	this->script_step_len[S_MK_VAL] = 2;
	this->script_step_len[S_MU_VAL] = 2;
	this->script_step_len[S_USK] = 2;
	this->script_step_len[S_UGK] = 2;
	this->script_step_len[S_UMK] = 2;
	this->script_step_len[S_UMU] = 2;
	this->script_step_len[S_USK_VAL] = 2;
	this->script_step_len[S_UGK_VAL] = 2;
	this->script_step_len[S_UMK_VAL] = 2;
	this->script_step_len[S_UMU_VAL] = 2;
	this->script_step_len[S_UPDATE] = 1;
	this->script_step_len[S_MO_XYZ] = 3;
	this->script_step_len[S_MO_XYZ_VAL] = 3;
	this->script_step_len[S_GA_XYZ] = 5;
	this->script_step_len[S_GA_XYZ_VAL] = 5;
	this->script_step_len[S_TB_XY] = 5;
	this->script_step_len[S_TB_XY_VAL] = 3;
	this->script_step_len[S_GAK] = 2;
	this->script_step_len[S_GAK_VAL] = 2;
	this->script_step_len[S_UGAK] = 2;
	this->script_step_len[S_UGAK_VAL] = 2;
	this->script_step_len[S_C2K] = 1;
	this->script_step_len[S_U2K] = 1;
	this->script_step_len[S_C2K_RAND] = 1;
	this->script_step_len[S_JFC] = 2;
	this->script_step_len[S_JFNC] = 2;
	this->script_step_len[S_JFZ] = 3;
	this->script_step_len[S_JFNZ] = 3;
	this->script_step_len[S_DJFNZ] = 3;
	this->script_step_len[S_CJFNE] = 4;
	this->script_step_len[S_JC] = 3;
	this->script_step_len[S_JNC] = 3;
	this->script_step_len[S_JZ] = 4;
	this->script_step_len[S_JNZ] = 4;
	this->script_step_len[S_DJNZ] = 4;
	this->script_step_len[S_CJNE] = 5;
	this->script_step_len[S_CALL] = 3;
	this->script_step_len[S_RET] = 1;
	this->script_step_len[S_ANL] = 3;
	this->script_step_len[S_ANLD] = 3;
	this->script_step_len[S_ADD] = 2;
	this->script_step_len[S_ADDD] = 2;
	this->script_step_len[S_SUB] = 2;
	this->script_step_len[S_SUBD] = 2;
	this->script_step_len[S_ORL] = 2;
	this->script_step_len[S_ORLD] = 2;
	this->script_step_len[S_DEC] = 2;
	this->script_step_len[S_INC] = 2;
	this->script_step_len[S_MUL] = 1;
	this->script_step_len[S_DIV] = 1;
	this->script_step_len[S_XRL] = 3;
	this->script_step_len[S_XRLD] = 3;
	this->script_step_len[S_RL] = 3;
	this->script_step_len[S_RLD] = 3;
	this->script_step_len[S_RR] = 3;
	this->script_step_len[S_RRD] = 3;
	this->script_step_len[S_CLR] = 2;
	this->script_step_len[S_CPL] = 2;
	this->script_step_len[S_XCH] = 3;
	this->script_step_len[S_PUSH] = 2;
	this->script_step_len[S_POP] = 2;
	this->script_step_len[S_MOV] = 3;
	this->script_step_len[S_MOVD] = 3;
	this->script_step_len[S_MOV_PC2REG] = 2;
	this->script_step_len[S_VALUE_RELOAD] = 2;
	this->script_step_len[S_MODE_JOG] = 1;
	this->script_step_len[S_LED_CTRL] = 2;
	this->script_step_len[S_LED_COL] = 4;
	this->script_step_len[S_WHILE_UPDATE] = 1;
	this->script_step_len[S_WHILE_UP] = 1;
	this->script_step_len[S_WHILE_DOWN] = 1;
	this->script_step_len[S_IF_UP_EXIT] = 1;
	this->script_step_len[S_IF_DOWN_EXIT] = 1;
	this->script_step_len[S_IF_KA_EXIT] = 1;
	this->script_step_len[S_RES] = 1;
	return root.toStyledString();
}

string O2Protocol::Disconnect()
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root;
	hid_close(this->handle);
	this->connected = 0;
	root["status"] = 0;
	return root.toStyledString();
}

string O2Protocol::Save()
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value;
	root.clear();
	root["status"] = 0;
	this->EpOut.format.id = 2;
	this->EpOut.format.cmd = 4;
	this->EpOut.format.data_len = 2;
	this->EpOut.format.data.data[0] = 0x72;
	this->EpOut.format.data.data[1] = 0x96;

	OUT_CHECKSUM = 0;
	for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
		OUT_CHECKSUM += this->EpOut.data[i];
	hid_write(this->handle, this->EpOut.udata, 64);
	if (hid_read_timeout(this->handle, this->EpIn.udata, 64, 2000) > 0)
	{
		if (this->EpIn.format.cmd != 0)
		{
			root["error_code"] = this->EpIn.format.cmd;
			root["status"] = -1;
			root["message"] = u8"saveFailed_msg";
		}
		else
		{
			root["status"] = 0;
			root["message"] = u8"successfullySaved_msg";
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"responseTimeout_msg";
	}
	return root.toStyledString();
}

int O2Protocol::Check()
{
	this->EpOut.format.id = 2;
	this->EpOut.format.cmd = 0;
	this->EpOut.format.data_len = 0;
	this->EpOut.udata[3] = 2;
	hid_write(this->handle, this->EpOut.udata, 64);
	if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
	{
		return 0;
	}
	else
	{
		this->connected = 0;
		return -1;
	}
}

string O2Protocol::Buttons(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	root.clear();
	root["status"] = 0;
	if (data["method"].asString() == "read")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 6;
		this->EpOut.format.data_len = 2;
		this->EpOut.format.data.key.pattern = 0;
		int key_number = 0;
		while (1)
		{
			value.clear();
			this->EpOut.format.data.key.number = key_number++;

			OUT_CHECKSUM = 0;
			for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
				OUT_CHECKSUM += this->EpOut.data[i];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = this->EpIn.format.data.key.type;
					value["number"] = this->EpIn.format.data.key.number;
					value2[0] = this->EpIn.format.data.key.data.keyboard.plain_0;
					value2[1] = this->EpIn.format.data.key.data.keyboard.plain_1;
					value2[2] = this->EpIn.format.data.key.data.keyboard.plain_2;
					value2[3] = this->EpIn.format.data.key.data.keyboard.plain_3;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (key_number == 0)
		{
			root["status"] = -1;
			root["message"] = u8"functionNotSupported_mag";
		}
	}
	else if (data["method"].asString() == "write")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 6;
		this->EpOut.format.data_len = 8;
		this->EpOut.format.data.key.pattern = 1;
		for (unsigned i = 0; i < data["data"].size(); i++)
		{
			value.clear();
			this->EpOut.format.data.key.number = data["data"][i]["number"].asInt();
			this->EpOut.format.data.key.type = data["data"][i]["code"].asInt();
			this->EpOut.format.data.key.data.keyboard.plain_0 = data["data"][i]["values"][0].asInt();
			this->EpOut.format.data.key.data.keyboard.plain_1 = data["data"][i]["values"][1].asInt();
			this->EpOut.format.data.key.data.keyboard.plain_2 = data["data"][i]["values"][2].asInt();
			this->EpOut.format.data.key.data.keyboard.plain_3 = data["data"][i]["values"][3].asInt();
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
				OUT_CHECKSUM += this->EpOut.data[k];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = this->EpIn.format.data.key.type;
					value["number"] = this->EpIn.format.data.key.number;
					value2[0] = this->EpIn.format.data.key.data.keyboard.plain_0;
					value2[1] = this->EpIn.format.data.key.data.keyboard.plain_1;
					value2[2] = this->EpIn.format.data.key.data.keyboard.plain_2;
					value2[3] = this->EpIn.format.data.key.data.keyboard.plain_3;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					root["status"] = -1;
					root["message"] = u8"functionNotSupported_mag";
					break;
				}
			}
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？";
	}
	return root.toStyledString();
}

string O2Protocol::Key_map(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, vkey_number, vinfo, vkey_code, vkey_values;
	root.clear();
	root["status"] = 0;
	if (data["method"].asString() == "read")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 22;
		this->EpOut.format.data_len = 2;
		this->EpOut.format.data.key.pattern = 0;
		int key_number = 0;
		while (1)
		{
			vkey_number.clear();
			this->EpOut.format.data.key.number = key_number++;

			OUT_CHECKSUM = 0;
			for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
				OUT_CHECKSUM += this->EpOut.data[i];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					vkey_number["code"] = this->EpIn.format.data.bottom.mode;
					vkey_number["number"] = this->EpIn.format.data.bottom.number;
					vinfo.clear();
					vinfo["key_shape_r"] = this->EpIn.format.data.bottom.shape_r;
					vinfo["key_shape_x"] = this->EpIn.format.data.bottom.shape_x;
					vinfo["key_shape_y"] = this->EpIn.format.data.bottom.shape_y;
					vinfo["key_site_x"] = this->EpIn.format.data.bottom.site_x;
					vinfo["key_site_y"] = this->EpIn.format.data.bottom.site_y;
					vinfo["key_type"] = this->EpIn.format.data.bottom.type;
					vkey_number["info"] = vinfo;
					for (unsigned i = 0; i < (this->EpIn.format.data_len - 16) / 6; i++)
					{
						vkey_code.clear();

						vkey_code["mode"] = this->EpIn.format.data.bottom.key_lay[i].mode;
						vkey_values.clear();
						vkey_values[0] = this->EpIn.format.data.bottom.key_lay[i].plain_0;
						vkey_values[1] = this->EpIn.format.data.bottom.key_lay[i].plain_1;
						vkey_values[2] = this->EpIn.format.data.bottom.key_lay[i].plain_2;
						vkey_values[3] = this->EpIn.format.data.bottom.key_lay[i].plain_3;
						vkey_code["values"] = vkey_values;
						vkey_number["key_data"].append(vkey_code);
					}
					root["data"].append(vkey_number);
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (key_number == 0)
		{
			root["status"] = -1;
			root["message"] = u8"functionNotSupported_mag";
		}
	}
	else if (data["method"].asString() == "write")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 22;
		this->EpOut.format.data_len = 16 + data["data"][0]["key_data"].size() * 6;
		this->EpOut.format.data.key.pattern = 1;
		for (unsigned key_num = 0; key_num < data["data"].size(); key_num++)
		{
			this->EpOut.format.data.key.number = data["data"][key_num]["number"].asInt();
			for (unsigned i = 0; i < data["data"][key_num]["key_data"].size(); i++)
			{
				this->EpOut.format.data.bottom.key_lay[i].mode = data["data"][key_num]["key_data"][i]["mode"].asInt();
				this->EpOut.format.data.bottom.key_lay[i].plain_0 = data["data"][key_num]["key_data"][i]["values"][0].asInt();
				this->EpOut.format.data.bottom.key_lay[i].plain_1 = data["data"][key_num]["key_data"][i]["values"][1].asInt();
				this->EpOut.format.data.bottom.key_lay[i].plain_2 = data["data"][key_num]["key_data"][i]["values"][2].asInt();
				this->EpOut.format.data.bottom.key_lay[i].plain_3 = data["data"][key_num]["key_data"][i]["values"][3].asInt();
			}
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
			{
				OUT_CHECKSUM += this->EpOut.data[k];
				//printf_s("%02hx ", this->EpOut.data[k]);
			}
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					vkey_number["code"] = this->EpIn.format.data.bottom.mode;
					vkey_number["number"] = this->EpIn.format.data.bottom.number;
					vinfo.clear();
					vinfo["key_shape_r"] = this->EpIn.format.data.bottom.shape_r;
					vinfo["key_shape_x"] = this->EpIn.format.data.bottom.shape_x;
					vinfo["key_shape_y"] = this->EpIn.format.data.bottom.shape_y;
					vinfo["key_site_x"] = this->EpIn.format.data.bottom.site_x;
					vinfo["key_site_y"] = this->EpIn.format.data.bottom.site_y;
					vkey_number["info"] = vinfo;
					for (unsigned i = 0; i < (this->EpIn.format.data_len - 16) / 6; i++)
					{
						vkey_code.clear();

						vkey_code["mode"] = this->EpIn.format.data.bottom.key_lay[i].mode;
						vkey_values.clear();
						vkey_values[0] = this->EpIn.format.data.bottom.key_lay[i].plain_0;
						vkey_values[1] = this->EpIn.format.data.bottom.key_lay[i].plain_1;
						vkey_values[2] = this->EpIn.format.data.bottom.key_lay[i].plain_2;
						vkey_values[3] = this->EpIn.format.data.bottom.key_lay[i].plain_3;
						vkey_code["values"] = vkey_values;
						vkey_number["key_data"].append(vkey_code);
					}
					root["data"].append(vkey_number);
					vkey_number.clear();
				}
				else
				{
					root["status"] = -1;
					root["message"] = u8"functionNotSupported_mag";
					break;
				}
			}
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？";
	}
	return root.toStyledString();
}

string O2Protocol::Api(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	root.clear();
	root["status"] = 0;
	if (data["method"].asString() == "read")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = data["cmd_code"].asInt();
		this->EpOut.format.data_len = 2;
		this->EpOut.format.data.api.pattern = 0;
		int number = 0;
		while (this->EpOut.format.cmd)
		{
			value.clear();
			this->EpOut.format.data.api.number = number++;

			OUT_CHECKSUM = 0;
			for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
				OUT_CHECKSUM += this->EpOut.data[i];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = this->EpIn.format.data.api.mode;
					value["number"] = this->EpIn.format.data.api.number;
					for (int i = 0; i < this->EpIn.format.data_len - 3; i++)
					{
						value2[i] = this->EpIn.format.data.api.data[i];
					}
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (number == 0)
		{
			root["status"] = -1;
			root["message"] = u8"functionNotSupported_mag";
		}
	}
	else if (data["method"].asString() == "write")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = data["cmd_code"].asInt();
		this->EpOut.format.data.key.pattern = 1;
		for (unsigned i = 0; i < data["data"].size(); i++)
		{
			value.clear();
			this->EpOut.format.data.api.number = data["data"][i]["number"].asInt();
			this->EpOut.format.data.api.mode = data["data"][i]["code"].asInt();
			this->EpOut.format.data_len = (data["data"][i]["values"].size() > 57 ? 57 : data["data"][i]["values"].size()) + 3;
			for (int v = 0; v < this->EpOut.format.data_len - 3; v++)
			{
				this->EpOut.format.data.api.data[v] = data["data"][i]["values"][v].asInt();
			}
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
			{
				OUT_CHECKSUM += this->EpOut.data[k];
				//printf_s("%hhx ", this->EpOut.data[k]);
			}
			//printf_s("\n ");
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = this->EpIn.format.data.api.mode;
					value["number"] = this->EpIn.format.data.api.number;
					for (int v = 0; v < this->EpIn.format.data_len - 3; v++)
					{
						value2[v] = this->EpIn.format.data.api.data[v];
					}
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					root["status"] = -1;
					root["message"] = u8"functionNotSupported_mag";
					break;
				}
			}
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？";
	}
	return root.toStyledString();
}

string O2Protocol::String_gbk(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	root.clear();
	root["status"] = 0;
	if (data["method"].asString() == "read")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 12;
		this->EpOut.format.data_len = 2;
		this->EpOut.format.data.api.pattern = 0;
		int number = 0;
		while (this->EpOut.format.cmd)
		{
			value.clear();
			this->EpOut.format.data.api.number = number++;

			OUT_CHECKSUM = 0;
			for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
				OUT_CHECKSUM += this->EpOut.data[i];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = this->EpIn.format.data.api.mode;
					value["number"] = this->EpIn.format.data.api.number;
					this->EpIn.format.data.api.data[56] = 0;
					unsigned gbk_i = 0;
					char buf[64];

					for (unsigned data_i = 0; data_i < 56; data_i += 2)
					{
						if (this->EpIn.format.data.api.data[data_i] == 0)
						{
							buf[gbk_i] = this->EpIn.format.data.api.data[data_i + 1];
							gbk_i++;
						}
						else
						{
							buf[gbk_i] = this->EpIn.format.data.api.data[data_i + 1];
							buf[gbk_i + 1] = this->EpIn.format.data.api.data[data_i];
							gbk_i += 2;
						}
					}
					this->EpIn.format.data.api.data[gbk_i] = 0;

					string str_gbk = buf;
					value2[0] = ANSIToUTF8(str_gbk);
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (number == 0)
		{
			root["status"] = -1;
			root["message"] = u8"functionNotSupported_mag";
		}
	}
	else if (data["method"].asString() == "write")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = data["cmd_code"].asInt();
		this->EpOut.format.data.key.pattern = 1;
		for (unsigned i = 0; i < data["data"].size(); i++)
		{
			value.clear();
			this->EpOut.format.data.api.number = data["data"][i]["number"].asInt();
			this->EpOut.format.data.api.mode = data["data"][i]["code"].asInt();
			this->EpOut.format.data_len = 56 + 3;
			string str_gbk = UTF8ToANSI(data["data"][i]["values"][0].asString());
			unsigned gbk_i = 0;
			for (unsigned data_i = 0; data_i < 56; data_i += 2)
			{
				if (gbk_i < str_gbk.length())
				{
					if (str_gbk[gbk_i] > 0)
					{
						this->EpOut.format.data.api.data[data_i] = 0;
						this->EpOut.format.data.api.data[data_i + 1] = str_gbk[gbk_i];
						gbk_i++;
					}
					else
					{
						this->EpOut.format.data.api.data[data_i] = str_gbk[gbk_i + 1];
						this->EpOut.format.data.api.data[data_i + 1] = str_gbk[gbk_i];
						gbk_i += 2;
					}
				}
				else
				{
					this->EpOut.format.data.api.data[data_i] = 0;
					this->EpOut.format.data.api.data[data_i + 1] = 0;
				}
			}
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
			{
				OUT_CHECKSUM += this->EpOut.data[k];
				//printf_s("%hhx ", this->EpOut.data[k]);
			}
			//printf_s("\n ");
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = this->EpIn.format.data.api.mode;
					value["number"] = this->EpIn.format.data.api.number;
					for (int v = 0; v < this->EpIn.format.data_len - 3; v++)
					{
						value2[v] = this->EpIn.format.data.api.data[v];
					}
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					root["status"] = -1;
					root["message"] = u8"functionNotSupported_mag";
					break;
				}
			}
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？";
	}
	return root.toStyledString();
}

string O2Protocol::Bootloader(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	root.clear();
	root["status"] = 0;

	this->EpOut.format.id = 2;
	this->EpOut.format.cmd = data["cmd_code"].asInt();
	this->EpOut.format.data_len = 0;
	value.clear();
	OUT_CHECKSUM = 0;
	for (int c = 0; c < this->EpOut.format.data_len + 3; c++)
		OUT_CHECKSUM += this->EpOut.data[c];
	hid_write(this->handle, this->EpOut.udata, 64);
	if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
	{
		if (this->EpIn.format.cmd == 0)
		{
			value["code"] = 0;
			value["number"] = 0;
			for (int i = 0; i < this->EpIn.format.data_len - 3; i++)
			{
				value2[i] = this->EpIn.format.data.api.data[i];
			}
			value["values"] = value2;
			root["data"].append(value);
		}
		else if (this->EpIn.format.cmd == 2)
		{
			value["code"] = 0;
			value["number"] = 0;
			this->EpIn.format.data.data[this->EpIn.format.data_len] = 0;
			value2[0] = (char*)this->EpIn.format.data.data;
			value["values"] = value2;
			root["data"].append(value);
		}
		else
		{
			root["status"] = -1;
			root["message"] = "functionNotSupported_mag";
		}
	}
	return root.toStyledString();
}
/*
string O2Protocol::Firmware_write(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, format;
	root.clear();
	root["status"] = 0;

	this->EpOut.format.id = 2;
	string formatJson;
	HttpGet((char *)data["data"][0]["uri"].asString().data(), formatJson);
	Json::Reader reader;
	reader.parse(formatJson, format);
	if (format.size())
	{
		DWORD firmwareSize = 0;
		DWORD firmwareWriteSize = 0;
		char * firmwareBuff = NULL;
		if (HttpGet(format["firmware_uri"].asString().data(), firmwareBuff, firmwareSize) == 0)
		{
			if (firmwareSize)
			{
				for (unsigned i = 0; i < format["data"].size(); i++)
				{
					this->EpOut.format.cmd = format["data"][i]["code"].asInt();
					this->EpOut.format.data_len = format["data"][i]["data_len"].asInt();
					if (format["data"][i]["type"].asString() == "cmd")
					{
						DWORD dataLen = 0;
						for (unsigned f = 0; f < format["data"][i]["format_data"].size(); f++)
						{
							if (format["data"][i]["format_data"][f]["type"].asString() == "const")
							{
								for (unsigned v = 0; v < format["data"][i]["format_data"][f]["data"].size(); v++)
								{
									this->EpOut.format.data.udata[dataLen++] = format["data"][i]["format_data"][f]["data"][v].asInt();
								}
							}
							else if (format["data"][i]["format_data"][f]["type"].asString() == "firmware_len")
							{
								DWORD addr = firmwareSize + format["data"][i]["format_data"][f]["offset"].asInt();
								if (format["data"][i]["format_data"][f]["word"].asString() == "big")
								{
									for (int a = 0; a < format["data"][i]["format_data"][f]["len"].asInt(); a++)
									{
										this->EpOut.format.data.udata[dataLen++] = addr >> ((format["data"][i]["format_data"][f]["len"].asInt() - a - 1) * 8);
									}
								}
								else
								{
									for (int a = format["data"][i]["format_data"][f]["len"].asInt(); a; a++)
									{
										this->EpOut.format.data.udata[dataLen++] = addr >> ((format["data"][i]["format_data"][f]["len"].asInt() - a) * 8);
									}
								}
							}
						}
						OUT_CHECKSUM = 0;
						for (int c = 0; c < this->EpOut.format.data_len + 3; c++)
							OUT_CHECKSUM += this->EpOut.data[c];
						hid_write(this->handle, this->EpOut.udata, 64);
						if (hid_read_timeout(this->handle, this->EpIn.udata, 64, 1000) > 0)
						{
							if (this->EpIn.format.data_len != 0)
							{

								root["status"] = -1;
							}
						}
					}
					else if (format["data"][i]["type"].asString() == "data")
					{
						while (firmwareWriteSize < (firmwareSize - 16))
						{
							DWORD dataLen = 0;
							for (unsigned f = 0; f < format["data"][i]["format_data"].size(); f++)
							{
								if (format["data"][i]["format_data"][f]["type"].asString() == "addr")
								{
									DWORD addr = firmwareWriteSize + format["data"][i]["format_data"][f]["offset"].asInt();
									if (format["data"][i]["format_data"][f]["word"].asString() == "big")
									{
										for (int a = 0; a < format["data"][i]["format_data"][f]["len"].asInt(); a++)
										{
											this->EpOut.format.data.udata[dataLen++] = addr >> ((format["data"][i]["format_data"][f]["len"].asInt() - a - 1) * 8);
										}
									}
									else
									{
										for (int a = format["data"][i]["format_data"][f]["len"].asInt(); a; a++)
										{
											this->EpOut.format.data.udata[dataLen++] = addr >> ((format["data"][i]["format_data"][f]["len"].asInt() - a) * 8);
										}
									}
								}
								else if (format["data"][i]["format_data"][f]["type"].asString() == "const")
								{
									for (unsigned v = 0; v < format["data"][i]["format_data"][f]["data"].size(); v++)
									{
										this->EpOut.format.data.udata[dataLen++] = format["data"][i]["format_data"][f]["data"][v].asInt();
									}
								}
								else if (format["data"][i]["format_data"][f]["type"].asString() == "firmware_data")
								{
									memcpy_s(&this->EpOut.format.data.udata[dataLen], 60 - dataLen, &firmwareBuff[firmwareWriteSize], this->EpOut.format.data.udata[dataLen++] = format["data"][i]["format_data"][f]["len"].asInt());
									dataLen += format["data"][i]["format_data"][f]["len"].asInt();
									firmwareWriteSize += format["data"][i]["format_data"][f]["len"].asInt();
								}
							}
							OUT_CHECKSUM = 0;
							for (int c = 0; c < this->EpOut.format.data_len + 3; c++)
								OUT_CHECKSUM += this->EpOut.data[c];
							hid_write(this->handle, this->EpOut.udata, 64);
							if (hid_read_timeout(this->handle, this->EpIn.udata, 64, 1000) > 0)
							{
								if (this->EpIn.format.data_len != 0)
								{

									root["status"] = -1;
								}
							}
						}
					}

				}
			}
		}
	}
	return root.toStyledString();
}
*/
const bool rgb_format[64] = {
	true,		//0
	true,		//1
	true,		//2
	true,		//3
	true,		//4
	true,		//5
	true,		//6
	true,		//7
	false,		//8
	false,		//9
	false,		//0x0a
	false,		//0x0b
	false,		//0x0c
	false,		//0x0d
	false,		//0x0e
	false,		//0x0f
	false,		//0x10
	false,		//0x11
	false,		//0x12
	false,		//0x13
	true,		//0x14
	true,		//0x15
	true,		//0x16
	false,		//0x17
	false,		//0x18
	false,		//0x19
	false,		//0x1a
	false,		//0x1b
	false,		//0x1c
	true,		//0x1d
	false,		//0x1e
	false,		//0x1f
	false,		//0x20
	true,		//0x21
	false,		//0x22
	false,		//0x23
	false,		//0x24
	true,		//0x25
	true,		//0x26
	false,		//0x27
	false,		//0x28
	false,		//0x29
	true,		//0x2a
	true,		//0x2b
	false,		//0x2c
	false,		//0x2d
	false,		//0x2e
	true,		//0x2f
	true,		//0x30
	false,		//0x31
	false,		//0x32
	false,		//0x33
	true,		//0x34
	true,		//0x35
	false,		//0x36
	false,		//0x37
	false,		//0x38
	true,		//0x39
	false,		//0x3a
	false,		//0x3b
	false,		//0x3c
	false,		//0x3d
	false,		//0x3e
	false,		//0x3f
};

string O2Protocol::Lighting(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	char colhex[8];
	root.clear();
	root["status"] = 0;
	if (data["method"].asString() == "read")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 7;
		this->EpOut.format.data_len = 2;
		this->EpOut.format.data.key.pattern = 0;
		int led_number = 0;
		while (1)
		{
			value.clear();
			this->EpOut.format.data.key.number = led_number++;

			OUT_CHECKSUM = 0;
			for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
				OUT_CHECKSUM += this->EpOut.data[i];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					unsigned indexValue = 0;
					value["code"] = this->EpIn.format.data.lamp.type & 0x3F;
					value["number"] = this->EpIn.format.data.lamp.number;
					if (rgb_format[this->EpIn.format.data.lamp.type & 0x3F])
					{
#ifdef WIN32
						sprintf_s(colhex, "#%02x%02x%02x", this->EpIn.format.data.lamp.data.col.col_r, this->EpIn.format.data.lamp.data.col.col_g, this->EpIn.format.data.lamp.data.col.col_b);
#else
						sprintf(colhex, "#%02x%02x%02x", this->EpIn.format.data.lamp.data.col.col_r, this->EpIn.format.data.lamp.data.col.col_g, this->EpIn.format.data.lamp.data.col.col_b);
#endif
						value2[indexValue++] = colhex;
					}
					else
					{
						value2[indexValue++] = this->EpIn.format.data.lamp.data.col.col_r;
						value2[indexValue++] = this->EpIn.format.data.lamp.data.col.col_g;
						value2[indexValue++] = this->EpIn.format.data.lamp.data.col.col_b;
					}
					value2[indexValue++] = this->EpIn.format.data.lamp.type & 0xC0;
					value2[indexValue++] = this->EpIn.format.data.lamp.event;
					value2[indexValue++] = this->EpIn.format.data.lamp.data.col.interval_time;
					value2[indexValue++] = this->EpIn.format.data.lamp.lamp_cmd_len;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (led_number == 0)
		{
			root["status"] = -1;
			root["message"] = u8"functionNotSupported_mag";
		}
	}
	else if (data["method"].asString() == "write")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 7;
		this->EpOut.format.data_len = 9;
		this->EpOut.format.data.key.pattern = 1;
		for (unsigned num = 0; num < data["data"].size(); num++)
		{
			unsigned indexValue = 0;
			value.clear();
			this->EpOut.format.data.lamp.number = data["data"][num]["number"].asInt();
			if (rgb_format[data["data"][num]["code"].asInt()])
			{
				int icol = 0;
				sscanf_s(data["data"][num]["values"][indexValue++].asString().data(), "#%x", &icol);
				this->EpOut.format.data.lamp.data.col.col_r = icol >> 16;
				this->EpOut.format.data.lamp.data.col.col_g = icol >> 8;
				this->EpOut.format.data.lamp.data.col.col_b = icol;
			}
			else
			{
				this->EpOut.format.data.lamp.data.data[0] = data["data"][num]["values"][indexValue++].asInt();
				this->EpOut.format.data.lamp.data.data[1] = data["data"][num]["values"][indexValue++].asInt();
				this->EpOut.format.data.lamp.data.data[2] = data["data"][num]["values"][indexValue++].asInt();

			}
			this->EpOut.format.data.lamp.type = data["data"][num]["code"].asInt() | data["data"][num]["values"][indexValue++].asInt();
			this->EpOut.format.data.lamp.event = data["data"][num]["values"][indexValue++].asInt();
			this->EpOut.format.data.lamp.data.col.interval_time = data["data"][num]["values"][indexValue++].asInt();
			this->EpOut.format.data.lamp.lamp_cmd_len = data["data"][num]["values"][indexValue++].asInt();
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
			{
				OUT_CHECKSUM += this->EpOut.data[k];
				//printf_s("%02hhx ", this->EpOut.data[k]);
			}
			//printf_s("\n ");
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					indexValue = 0;
					value["code"] = this->EpIn.format.data.lamp.type & 0x3F;
					value["number"] = this->EpIn.format.data.lamp.number;
					if (rgb_format[this->EpIn.format.data.lamp.type & 0x3F])
					{
						sprintf_s(colhex, "#%02hhX%02hhX%02hhX", this->EpIn.format.data.lamp.data.col.col_r, this->EpIn.format.data.lamp.data.col.col_g, this->EpIn.format.data.lamp.data.col.col_b);
						value2[indexValue++] = colhex;
					}
					else
					{
						value2[indexValue++] = this->EpIn.format.data.lamp.data.data[0];
						value2[indexValue++] = this->EpIn.format.data.lamp.data.data[1];
						value2[indexValue++] = this->EpIn.format.data.lamp.data.data[2];
					}
					value2[indexValue++] = this->EpIn.format.data.lamp.type & 0xC0;
					value2[indexValue++] = this->EpIn.format.data.lamp.event;
					value2[indexValue++] = this->EpIn.format.data.lamp.data.col.interval_time;
					value2[indexValue++] = this->EpIn.format.data.lamp.lamp_cmd_len;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					root["status"] = -1;
					root["message"] = u8"functionNotSupported_mag";
					break;
				}
			}
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？";
	}
	return root.toStyledString();
}

string O2Protocol::Script(const Json::Value& data)
{
	unsigned char buf[8192];
	memset(buf, 0, 8192);
	unsigned addr = 0;
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	root.clear();
	value.clear();
	root["status"] = 0;
	if (data["method"].asString() == "read")
	{
		addr = 0;
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0xF0;
		this->EpOut.format.data_len = 2;
		while (1)
		{
			this->EpOut.format.data.script.addr_h = addr / 256;
			this->EpOut.format.data.script.addr_l = addr % 256;
			OUT_CHECKSUM = 0;
			for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
				OUT_CHECKSUM += this->EpOut.data[i];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
#ifdef WIN32
					memcpy_s(&buf[addr], 8192, this->EpIn.format.data.data, this->EpIn.format.data_len);
#else
					memcpy(&buf[addr], this->EpIn.format.data.data, this->EpIn.format.data_len);
#endif
					addr += this->EpIn.format.data_len;
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (addr > 1)
		{
			buf[addr - 1] = 0;
		}
		if (addr == 0)
		{
			root["status"] = -1;
			root["message"] = u8"functionNotSupported_mag";
		}
		else
		{
			int num = 0;
			int i = 0;
			for (i = 0; i < addr;)
			{
				value.clear();
				value2.clear();
				value["code"] = buf[i];
				this->script[num].code = buf[i];
				value["number"] = num;
				i++;
				switch (this->script_step_len[this->script[num].code])
				{
				case 1:
					CV0;
					break;
				case 2:
					CV1;
					break;
				case 3:
					CV2;
					break;
				case 4:
					CV3;
					break;
				case 5:
					if (S_TB_XY == this->script[num].code)
					{
						this->script[num].value[0] = *(unsigned short*)&buf[i]; i += 2;
						this->script[num].value[1] = *(unsigned short*)&buf[i]; i += 2;
					}
					else
					{
						CV4;
					}
					break;
				default:
					CV0;
					break;
				}
				value2[0] = this->script[num].value[0];
				value2[1] = this->script[num].value[1];
				value2[2] = this->script[num].value[2];
				value2[3] = this->script[num].value[3];
				value["values"] = value2;
				root["data"].append(value);
				num++;
			}
			value2[0] = 0;
			value2[1] = 0;
			value2[2] = 0;
			value2[3] = 0;
			for (; i < 256; i++)
			{
				value["code"] = 0;
				value["number"] = num++;
				value["values"] = value2;
				root["data"].append(value);
			}
		}
	}
	else if (data["method"].asString() == "write")
	{

		for (unsigned v = 0; v < data["data"].size(); v++)
		{
			unsigned num = data["data"][v]["number"].asInt();;
			this->script[num].code = data["data"][v]["code"].asInt();
			for (unsigned vs = 0; vs < data["data"][v]["values"].size(); vs++)
			{
				this->script[num].value[vs] = data["data"][v]["values"][vs].asInt();
			}
		}

		unsigned i = 0;


		for (unsigned step = 0; this->script[step].code != 0; step++)
		{
			buf[i] = this->script[step].code;
			i++;
			switch (this->script_step_len[this->script[step].code])
			{
			case 1:
				WCV0;
				break;
			case 2:
				WCV1;
				break;
			case 3:
				WCV2;
				break;
			case 4:
				WCV3;
				break;
			case 5:
				if (this->script[step].code)
				{
					*(unsigned short*)&buf[i] = this->script[step].value[0]; i += 2;
					*(unsigned short*)&buf[i] = this->script[step].value[1]; i += 2;
				}
				else
				{
					WCV4;
				}
				break;
			default:
				WCV0;
				break;
			}
		}
		if (i > 0 && buf[i - 1] != 0xff)
		{
			buf[i++] = 0xff;
			buf[i++] = 0xff;
		}
		addr = i;
		cout << addr << endl;
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0xF0;
		int err = 0;
		for (i = 0; i < addr;)
		{
			this->EpOut.format.data_len = addr - i > 54 ? 54 + 2 : addr - i + 2;
			this->EpOut.format.data.script.addr_h = i / 256;
			this->EpOut.format.data.script.addr_l = i % 256;
#ifdef WIN32
			memcpy_s(&this->EpOut.format.data.data[2], 54, &buf[i], addr - i > 54 ? 54 : addr - i);
#else
			memcpy(&this->EpOut.format.data.data[2], &buf[i], addr - i > 54 ? 54 : addr - i);
#endif // WIN32

			i += addr - i > 54 ? 54 : addr - i;
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
			{
				OUT_CHECKSUM += this->EpOut.data[k];
			}
			hid_write(this->handle, this->EpOut.udata, 64);
			Sleep(5);
			if ((err = hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT)) > 0)
			{
				this->EpIn.format.cmd;
				if (this->EpIn.format.cmd == 0)
				{

				}
				else
				{
					i = 0;
					break;
				}
			}
			else
			{
				printf_s("%ws\n", hid_error(this->handle));
				cout << err << " time_out" << endl;
				i = 0;
				break;
			}
		}
		if (i < addr)
		{
			root["status"] = -1;
			root["message"] = u8"writeLengthExceeds_msg";
		}
		else
		{
			root["status"] = 0;
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？";
	}
	return root.toStyledString();
}

string O2Protocol::Dev_id(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	root.clear();
	root["status"] = 0;
	cout << data["method"].asString().data() << endl;
	if (data["method"].asString() == "read")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0xfe;
		this->EpOut.format.data_len = 0;
		value.clear();
		OUT_CHECKSUM = 0;
		for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
			OUT_CHECKSUM += this->EpOut.data[i];
		hid_write(this->handle, this->EpOut.udata, 64);
		if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
		{
			if (this->EpIn.format.cmd == 0)
			{
				value["code"] = 0;
				value["number"] = 0;
				value2[0] = (int)this->EpIn.format.data.udata[0] + (int)this->EpIn.format.data.udata[1] * 256;
				value2[1] = (int)this->EpIn.format.data.udata[2] + (int)this->EpIn.format.data.udata[3] * 256;
				value["values"] = value2;
				root["data"].append(value);
			}
		}
		if (root["data"].size() < 1)
		{
			root["status"] = -1;
			root["message"] = u8"functionNotSupported_mag";
		}
	}
	else if (data["method"].asString() == "write")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0xfe;
		this->EpOut.format.data_len = 4;
		value.clear();
		this->EpOut.format.data.udata[0] = data["data"][0]["values"][0].asInt();
		this->EpOut.format.data.udata[1] = data["data"][0]["values"][0].asInt() / 256;
		this->EpOut.format.data.udata[2] = data["data"][0]["values"][1].asInt();
		this->EpOut.format.data.udata[3] = data["data"][0]["values"][1].asInt() / 256;
		OUT_CHECKSUM = 0;
		for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
			OUT_CHECKSUM += this->EpOut.data[i];
		hid_write(this->handle, this->EpOut.udata, 64);
		if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
		{
			if (this->EpIn.format.cmd == 0)
			{
				value["code"] = 0;
				value["number"] = 0;
				value2[0] = this->EpIn.format.data.data[0] + this->EpIn.format.data.data[1] * 256;
				value2[1] = this->EpIn.format.data.data[2] + this->EpIn.format.data.data[3] * 256;
				value["values"] = value2;
				root["data"].append(value);
			}
		}
		if (root["data"].size() < 1)
		{
			root["status"] = -1;
			root["message"] = u8"functionNotSupported_mag";
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？";
	}
	return root.toStyledString();
}

string O2Protocol::Dev_name(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	root.clear();
	root["status"] = 0;

	if (data["method"].asString() == "read")
	{
		int old = 0;
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0x08;
		this->EpOut.format.data_len = 1;
		this->EpOut.format.data.data[0] = 0;
		value.clear();
		OUT_CHECKSUM = 0;
		for (int c = 0; c < this->EpOut.format.data_len + 3; c++)
			OUT_CHECKSUM += this->EpOut.data[c];
		hid_write(this->handle, this->EpOut.udata, 64);
		if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
		{
			if (this->EpIn.format.cmd == 0)
			{
				value["code"] = 0;
				value["number"] = 0;
				this->EpIn.format.data.data[this->EpIn.format.data_len] = 0;
				value2[0] = unicodeToUTF8((wchar_t*)this->EpIn.format.data.data).data();
				value["values"] = value2;
				root["data"].append(value);
			}
			else
			{
				root["status"] = -1;
				root["message"] = u8"functionNotSupported_mag";
			}
		}
	}
	else if (data["method"].asString() == "write" && data["data"][0]["values"][0].isString())
	{
		memset(this->EpOut.udata, 0, 64);
		int old = 0;
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0x08;
		wstring ws = utf8ToUnicode(data["data"][0]["values"][0].asString());
		unsigned len = ws.length();
		memcpy(&(this->EpOut.format.data.data[1]), ws.data(), 30);
		//MultiByteToWideChar(CP_UTF8, 0, data["data"][0]["values"][0].asString().data(), -1, (LPWSTR)&(this->EpOut.format.data.data[1]), 30);
		this->EpOut.format.data_len = 32;
		this->EpOut.format.data.ok_pwd.pattern = 1;
		value.clear();
		OUT_CHECKSUM = 0;
		for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
		{
			OUT_CHECKSUM += this->EpOut.data[k];
			//printf_s("%02hhx ", this->EpOut.data[k]);
		}
		//printf_s("\n");
		hid_write(this->handle, this->EpOut.udata, 64);
		if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
		{
			if (this->EpIn.format.cmd == 0)
			{
				value["code"] = 0;
				value["number"] = 0;
				this->EpIn.format.data.data[this->EpIn.format.data_len] = 0;
				value2[0] = unicodeToUTF8((wchar_t*)this->EpIn.format.data.data).data();
				value["values"] = value2;
				root["data"].append(value);
			}
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？或者是数据格式错误？";
	}
	return root.toStyledString();
}

string O2Protocol::Ok_pwd(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	root.clear();
	root["status"] = 0;
	if (data["method"].asString() == "read")
	{
		int old = 0;
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0x0B;
		this->EpOut.format.data_len = 2;
		this->EpOut.format.data.data[0] = 0;
		for (int i = 0; true; i++)
		{
			this->EpOut.format.data.data[1] = i;
			value.clear();

			OUT_CHECKSUM = 0;
			for (int c = 0; c < this->EpOut.format.data_len + 3; c++)
				OUT_CHECKSUM += this->EpOut.data[c];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = 0;
					value["number"] = this->EpIn.format.data.ok_pwd.number;
					value2[0] = this->EpIn.format.data.ok_pwd.pwd;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					if (i == 0)
					{
						old = 1;
					}
					break;
				}
			}
			else
			{
				old = 1;
				break;
			}
		}
		if (old)
		{
			this->EpOut.format.id = 2;
			this->EpOut.format.cmd = 9;
			this->EpOut.format.data_len = 1;
			this->EpOut.format.data.data[0] = 0;
			value.clear();

			OUT_CHECKSUM = 0;
			for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
				OUT_CHECKSUM += this->EpOut.data[i];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = 0;
					value["number"] = 0;
					value2[0] = this->EpIn.format.data.data;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					root["status"] = -1;
					root["message"] = u8"functionNotSupported_mag";
				}
			}
			else
			{
				root["status"] = -1;
				root["message"] = u8"functionNotSupported_mag";
			}
		}
	}
	else if (data["method"].asString() == "write" && data["data"][0]["values"][0].isString())
	{
		int old = 0;
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0x0B;
		for (unsigned i = 0; i < data["data"].size(); i++)
		{
			this->EpOut.format.data_len = data["data"][i]["values"][0].asString().length() + 3;
			this->EpOut.format.data.ok_pwd.pattern = 1;
			this->EpOut.format.data.ok_pwd.number = data["data"][i]["number"].asInt();
			value.clear();
			strcpy_s(this->EpOut.format.data.ok_pwd.pwd, data["data"][i]["values"][0].asString().data());
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
				OUT_CHECKSUM += this->EpOut.data[k];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = 0;
					value["number"] = this->EpIn.format.data.ok_pwd.number;
					value2[0] = this->EpIn.format.data.ok_pwd.pwd;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					old = 1;
					break;
				}
			}
			else
			{
				old = 1;
				break;
			}
		}

		if (old)
		{
			this->EpOut.format.id = 2;
			this->EpOut.format.cmd = 9;
			this->EpOut.format.data_len = data["data"][0]["values"][0].asString().length() + 2;
			this->EpOut.format.data.name_of_device.pattern = 1;
			value.clear();
			strcpy_s(this->EpOut.format.data.name_of_device.data, data["data"][0]["values"][0].asString().data());
			this->EpOut.format.data.name_of_device.data[data["data"][0]["values"][0].asString().length()] = 0;
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
				OUT_CHECKSUM += this->EpOut.data[k];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = 0;
					value["number"] = 0;
					value2[0] = this->EpIn.format.data.data;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					root["status"] = -1;
					root["message"] = u8"functionNotSupported_mag";
				}
			}
			else
			{
				root["status"] = -1;
				root["message"] = u8"functionNotSupported_mag";
			}
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？或者是数据格式错误？";
	}
	return root.toStyledString();
}

string O2Protocol::Script_sw(const Json::Value& data)
{
	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value, value2;
	root.clear();
	root["status"] = 0;
	cout << data["method"].asString().data() << endl;
	if (data["method"].asString() == "read")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0xf1;
		this->EpOut.format.data_len = 2;
		this->EpOut.format.data.script_sw.pattern = 0;
		int script_number = 0;
		while (1)
		{
			value.clear();
			this->EpOut.format.data.script_sw.number = script_number++;

			OUT_CHECKSUM = 0;
			for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
				OUT_CHECKSUM += this->EpOut.data[i];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT * 2) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = this->EpIn.format.data.script_sw.pattern;
					value["number"] = this->EpIn.format.data.script_sw.number;
					value2[0] = this->EpIn.format.data.script_sw.name;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (script_number == 0)
		{
			root["status"] = -1;
			root["message"] = u8"functionNotSupported_mag";
		}
	}
	else if (data["method"].asString() == "write")
	{
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0xf1;
		this->EpOut.format.data_len = 34;
		this->EpOut.format.data.script_sw.pattern = 1;
		for (unsigned i = 0; i < data["data"].size(); i++)
		{
			value.clear();
			this->EpOut.format.data.script_sw.number = data["data"][i]["number"].asInt();
#ifdef WIN32
			memcpy_s(this->EpOut.format.data.script_sw.name, 32, data["data"][i]["values"][0].asString().data(), 32);
#else
			memcpy(this->EpOut.format.data.script_sw.name, data["data"][i]["values"][0].asString().data(), 32);
#endif
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
				OUT_CHECKSUM += this->EpOut.data[k];
			hid_write(this->handle, this->EpOut.udata, 64);
			if (hid_read_timeout(this->handle, this->EpIn.udata, 64, HID_TIMEOUT * 2) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					value["code"] = this->EpIn.format.data.script_sw.pattern;
					value["number"] = this->EpIn.format.data.script_sw.number;
					value2[0] = this->EpIn.format.data.script_sw.name;
					value["values"] = value2;
					root["data"].append(value);
				}
				else
				{
					root["status"] = -1;
					root["message"] = u8"functionNotSupported_mag";
					break;
				}
			}
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"read还是write？";
	}
	return root.toStyledString();
}



