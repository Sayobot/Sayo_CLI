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


const char *SeriesModel[DEVICE_IDENTIFICATION_CODE_MAX + 1] = {
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

string O2Protocol::SearchDrivers(unsigned short vid, unsigned short pid)
{
	int devn = 0;
	struct hid_device_info *devs, *cur_dev;

	devs = hid_enumerate(vid, pid);

	cur_dev = devs;

	std::unique_ptr<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
	Json::Value root, value,value2;
	/*rapidjson::StringBuffer strBuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
	writer.StartObject();
	writer.Key("data");
	writer.StartArray();*/

	while (cur_dev) {
		//if (cur_dev->usage_page > 0xFF)
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
			if (handle)
			{
				this->EpOut.format.id = 2;
				this->EpOut.format.cmd= 0;
				this->EpOut.format.data_len = 0;
				OUT_CHECKSUM = 2;
				hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
				if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 2000) > 0)
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
								for (unsigned i = 0; i < (this->EpIn.format.data_len - 8);i++)
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
						value["mode"] == "unknown";
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
	hid_free_enumeration(cur_dev);
	root["status"] = devn ?0:-1;
	if (devn == 0)
		root["message"] = u8"未找到设备，请检查是否插了或者已被连接？";
	root["devices"] = devn;
	return root.toStyledString();
}

string O2Protocol::Connect(string p,int session)
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
		root["message"] = u8"打开设备失败了，你插好了吗？重新搜索一下试试？";
		root["status"] = -1;
	}

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
	hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
	if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 1000) > 0)
	{
		if (this->EpIn.format.cmd != 0)
		{
			root["error_code"] = this->EpIn.format.cmd;
			root["status"] = -1;
			root["message"] = u8"保存失败了，，，，要不要再试一次？";
		}
		else
		{
			root["status"] = 0;
			root["message"] = u8"恭喜！保存成功了！";
		}
	}
	else
	{
		root["status"] = -1;
		root["message"] = u8"响应超时……可能设备已断开";
	}
	return root.toStyledString();
}

int O2Protocol::Check()
{
	this->EpOut.format.id = 2;
	this->EpOut.format.cmd = 0;
	this->EpOut.format.data_len = 0;
	this->EpOut.udata[3] = 2;
	hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
	if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
	Json::Value root, value,value2;
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
			this->EpOut.format.data.key.number = key_number ++;

			OUT_CHECKSUM = 0;
			for (int i = 0; i < this->EpOut.format.data_len + 3; i++)
				OUT_CHECKSUM += this->EpOut.data[i];
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
			root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
					root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
			root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
			for (int v = 0; v < data["data"][i]["values"].size(); v++)
			{
				this->EpOut.format.data.api.data[v] = data["data"][i]["values"][v].asInt();
			}
			this->EpOut.format.data_len = data["data"][i]["values"].size() + 3;
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
			{
				OUT_CHECKSUM += this->EpOut.data[k];
				//printf_s("%hhx ", this->EpOut.data[k]);
			}
			//printf_s("\n ");
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
					root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
		hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
		if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 1000) > 0)
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
				root["message"] = "不支持的指令";
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
						hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
						if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 1000) > 0)
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
							hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
							if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 1000) > 0)
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
					root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
				printf_s("%02hhx ", this->EpOut.data[k]);
			}
			printf_s("\n ");
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
					root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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

string O2Protocol::LightingV2(const Json::Value& data)
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
					unsigned indexValue = 0;
					value["code"] = this->EpIn.format.data.lamp.type & 0x3F;
					value["number"] = this->EpIn.format.data.lamp.number;
					if (rgb_format[this->EpIn.format.data.lamp.type & 0x3F])
					{
						sprintf_s(colhex, "#%02x%02x%02x", this->EpIn.format.data.lamp.data.col.col_r, this->EpIn.format.data.lamp.data.col.col_g, this->EpIn.format.data.lamp.data.col.col_b);
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
			root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
				printf_s("%02hhx ", this->EpOut.data[k]);
			}
			printf_s("\n ");
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
					root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
	unsigned char buf[2048];
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
			{
				if (this->EpIn.format.cmd == 0)
				{
#ifdef WIN32
					memcpy_s(&buf[addr],2048, this->EpIn.format.data.data, this->EpIn.format.data_len);
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
		if (addr == 0)
		{
			root["status"] = -1;
			root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
		}
		else
		{
			int num = 0;
			int i = 0;
			for (i=0; i < addr;)
			{
				value.clear();
				value2.clear();
				value["code"] = buf[i];
				this->script[num].code = buf[i];
				value["number"] = num;
				i++;
				switch (this->script[num].code)
				{
				case S_NOP:
					CV0;
					break;
				case S_JMP:
					CV2;
					break;
				case S_SJMP:
					CV1;
					break;
				case S_AJMP:
					CV1;
					break;
				case S_SLEEP_X256:
					CV1;
					break;
				case S_SLEEP:
					CV1;
					break;
				case S_SLEEP_RAND_X256:
					CV1;
					break;
				case S_SLEEP_RAND:
					CV1;
					break;
				case S_SLEEP_X256_VAL:
					CV1;
					break;
				case S_SLEEP_VAL:
					CV1;
					break;
				case S_SLEEP_RAND_X8_VAL:
					CV1;
					break;
				case S_SLEEP_RAND_VAL:
					CV1;
					break;

				case S_SK:
				case S_GK:
				case S_MK:
				case S_MU:
				case S_SK_VAL:
				case S_GK_VAL:
				case S_MK_VAL:
				case S_MU_VAL:
				case S_USK:
				case S_UGK:
				case S_UMK:
				case S_UMU:
				case S_USK_VAL:
				case S_UGK_VAL:
				case S_UMK_VAL:
				case S_UMU_VAL:
					CV1;
					break;


				case S_DJNZ_VAL:
					CV3;
					break;

				case S_UPDATE:
					CV0;
					break;

				case S_WHILE_UP:
					CV0;
					break;
				case S_WHILE_DOWN:
					CV0;
					break;
				case S_IF_UP_EXIT:
					CV0;
					break;
				case S_IF_DOWN_EXIT:
					CV0;
					break;
				case S_IF_KA_EXIT:
					CV0;
					break;

				case S_RES:
					CV0;
					break;
				case S_EXIT:
					CV0;
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
		unsigned num = data["data"][0]["number"].asInt();;
		unsigned i = 0;

		this->script[num].code = data["data"][0]["code"].asInt();
		this->script[num].value[0] = data["data"][0]["values"][0].asInt();
		this->script[num].value[1] = data["data"][0]["values"][1].asInt();
		this->script[num].value[2] = data["data"][0]["values"][2].asInt();
		this->script[num].value[3] = data["data"][0]["values"][3].asInt();



		for (unsigned step = 0; this->script[step].code != 0; step++)
		{
			buf[i] = this->script[step].code;
			i++;
			switch (this->script[step].code)
			{
			case S_NOP:
				WCV0;
				break;
			case S_JMP:
				WCV2;
				break;
			case S_SJMP:
				WCV1;
				break;
			case S_AJMP:
				WCV1;
				break;
			case S_SLEEP_X256:
				WCV1;
				break;
			case S_SLEEP:
				WCV1;
				break;
			case S_SLEEP_RAND_X256:
				WCV1;
				break;
			case S_SLEEP_RAND:
				WCV1;
				break;
			case S_SLEEP_X256_VAL:
				WCV1;
				break;
			case S_SLEEP_VAL:
				WCV1;
				break;
			case S_SLEEP_RAND_X8_VAL:
				WCV1;
				break;
			case S_SLEEP_RAND_VAL:
				WCV1;
				break;

			case S_SK:
			case S_GK:
			case S_MK:
			case S_MU:
			case S_SK_VAL:
			case S_GK_VAL:
			case S_MK_VAL:
			case S_MU_VAL:
			case S_USK:
			case S_UGK:
			case S_UMK:
			case S_UMU:
			case S_USK_VAL:
			case S_UGK_VAL:
			case S_UMK_VAL:
			case S_UMU_VAL:
				WCV1;
				break;


			case S_DJNZ_VAL:
				WCV3;
				break;

			case S_UPDATE:
				WCV0;
				break;

			case S_WHILE_UP:
				WCV0;
				break;
			case S_WHILE_DOWN:
				WCV0;
				break;
			case S_IF_UP_EXIT:
				WCV0;
				break;
			case S_IF_DOWN_EXIT:
				WCV0;
				break;
			case S_IF_KA_EXIT:
				WCV0;
				break;

			case S_RES:
				WCV0;
				break;
			case S_EXIT:
				WCV0;
				break;
			default:
				WCV0;
				break;
			}
		}
		buf[i++] = 0xff;
		buf[i++] = 0xff;
		addr = i;
		cout << addr <<endl;
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			Sleep(5);
			if ( (err = hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50)) > 0)
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
				cout << err<< " time_out" << endl;
				i = 0;
				break;
			}
		}
		if (i < addr)
		{
			root["status"] = -1;
			root["message"] = u8"写入失败！可能是长度过长，精简一点吧！";
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
		hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
		if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
			root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
		hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
		if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
			root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
		hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
		if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 1000) > 0)
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
				root["message"] = u8"设备不支持改名操作";
			}
		}
	}
	else if (data["method"].asString() == "write" && data["data"][0]["values"][0].isString())
	{
		memset(this->EpOut.udata, 0, 64);
		int old = 0;
		this->EpOut.format.id = 2;
		this->EpOut.format.cmd = 0x08;
		wstring ws = UTF8ToUnicode(data["data"][0]["values"][0].asString());
		unsigned len = ws.length();
		memcpy(&(this->EpOut.format.data.data[1]), ws.data(),30);
			//MultiByteToWideChar(CP_UTF8, 0, data["data"][0]["values"][0].asString().data(), -1, (LPWSTR)&(this->EpOut.format.data.data[1]), 30);
			this->EpOut.format.data_len = 32;
			this->EpOut.format.data.ok_pwd.pattern = 1;
			value.clear();
			OUT_CHECKSUM = 0;
			for (int k = 0; k < this->EpOut.format.data_len + 3; k++)
			{
				OUT_CHECKSUM += this->EpOut.data[k];
				printf_s("%02hhx ", this->EpOut.data[k]);
			}
			printf_s("\n");
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 100) > 0)
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
		for (int i = 0;true; i++)
		{
			this->EpOut.format.data.data[1] = i;
			value.clear();

			OUT_CHECKSUM = 0;
			for (int c = 0; c < this->EpOut.format.data_len + 3; c++)
				OUT_CHECKSUM += this->EpOut.data[c];
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 1000) > 0)
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 100) > 0)
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
					root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
				}
			}
			else
			{
				root["status"] = -1;
				root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
			}
		}
	}
	else if (data["method"].asString() == "write"&& data["data"][0]["values"][0].isString())
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 100) > 0)
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 100) > 0)
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
					root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
				}
			}
			else
			{
				root["status"] = -1;
				root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
			root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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
			hid_write(this->handle, this->EpOut.udata, HID_DATA_LENGTH);
			if (hid_read_timeout(this->handle, this->EpIn.udata, HID_DATA_LENGTH, 50) > 0)
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
					root["message"] = u8"什么都没读取到呢，可能是设备不支持或已断开连接";
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



