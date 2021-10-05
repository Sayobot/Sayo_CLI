/*******************************************************
 Windows HID simplification

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009, All Rights Reserved.

 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.
********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"

// Headers needed for sleeping.
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[255];
#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device* handle;
	int i;

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif

	struct hid_device_info* devs, * cur_dev;

	devs = hid_enumerate(0x8089, 0);
	cur_dev = devs;
	while (cur_dev) {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n", cur_dev->interface_number);
		printf("\n");
		handle = hid_open_path(cur_dev->path);
		if (handle)
			break;
	}
	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf, 0x00, sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;


	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
		if (!handle) {
			printf("unable to open device\n");
			return 1;
		}
	

	// Read the Manufacturer String

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 0);

	// Try to read from the device. There shoud be no
	// data here, but execution should not block.
	//res = hid_read(handle, buf, 64);


	char input[256];
	while (1)
	{
		printf("请输入16进制数按空格分开\n");
		scanf("%[^\n]", input);
		while (getchar() != '\n');
		sscanf(input, "%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx",
			&buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5], &buf[6], &buf[7],
			&buf[8], &buf[9], &buf[10], &buf[11], &buf[12], &buf[13], &buf[14], &buf[15],
			&buf[16], &buf[17], &buf[18], &buf[19], &buf[20], &buf[21], &buf[22], &buf[23],
			&buf[24], &buf[25], &buf[26], &buf[27], &buf[28], &buf[29], &buf[30], &buf[31],
			&buf[32], &buf[33], &buf[34], &buf[35], &buf[36], &buf[37], &buf[38], &buf[39], &buf[40], &buf[41]);
		printf("Sent :");
		// Request state (cmd 0x81). The first byte is the report number (0x1).
		//buf[0] = 0x02;
		//buf[1] = 0x01;
		//buf[2] = 0x02;
		//buf[3] = 0x00;
		//buf[4] = 0x01;
		unsigned char checksum = 0;
		int lengh = buf[2] + 3;
		for (i = 0; i < lengh; i++)
		{
			printf("%02hhx ", buf[i]);
		}
		printf("\n");
		for (i = 0; i < lengh; i++)
		{
			checksum += buf[i];
		}
		buf[lengh] = checksum;
		printf("%d ", hid_write(handle, buf, 64));

		// Read requested state. hid_read() has been set to be
		// non-blocking by the call to hid_set_nonblocking() above.
		// This loop demonstrates the non-blocking nature of hid_read().
		res = 0;
		res = hid_read_timeout(handle, buf, 64,2000);
		if (res < 0)
			printf("Unable to read()\n");
		const char* status = NULL;
		switch (buf[1])
		{
		case 0:
			status = "Successful!";
			break;
		case 1:
			break;
		case 2:
			status = "Tips:";
			break;
		case 3:
			status = "Data Error!";
			break;
		case 4:
			status = "Check failed!";
			break;
		default:
			status = "?????";
			break;
		}
		printf("%s\n", status);
		if (buf[2])
		{
			printf("Data read:\n   ");
			// Print out the returned buffer.
			lengh = buf[2] + 4;
			checksum = 0;
			if (buf[1] == 2)
			{
				for (i = 3; i < lengh - 1; i++)
				{
					printf("%c", buf[i]);
					checksum += buf[i];
				}
			}
			else
			{
				for (i = 0; i < lengh; i++)
				{
					printf("%02hhx ", buf[i]);
					checksum += buf[i];
				}
			}
		}
		printf("\n");

	}
	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause");
#endif

	return 0;
}
