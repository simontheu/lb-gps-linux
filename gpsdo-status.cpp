/* (used hidtest.cpp from https://github.com/signal11/hidapi as raw material)

   gpsdo-status - display basic (or --verbose) status information from Leo Bodnar Electronics Ltd.
   Precision Frequency Reference (GPS Clock) or Mini Precision GPS Reference Clock ( https://www.leobodnar.com/shop/ )

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

   CSylvain  2019-04-16 Copyright ©2019, all rights not already granted above are reserved.
*/
/* [original hidtest.cpp comment header; reformatted]
 Windows HID simplification by Alan Ott, Signal 11 Software
 8/22/2009; Copyright 2009
 
 The contents of this file may be used by anyone for any reason without any conditions
 and may be used as a starting point for your own applications which use HIDAPI.
*/

#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "GPSSettings.h"
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
	unsigned char buf[256];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif

	struct hid_device_info *devs, *cur_dev;

	int c;
	while (1) {
		static struct option long_options[] =
			{
				/* first option sets a flag */
				{"verbose", no_argument, &verbose_flag, 1},
				{"help", no_argument, NULL, 'h'},
				{0, 0, 0, 0}
			};

		int idxopt = 0;	/* getopt_long stores the option index here. */
	
		c = getopt_long(argc, argv, "vh", long_options, &idxopt);

		if (c == -1) /* Detect the end of the options. */
            break;

		switch (c) {
		case 0: // do nothing more once this option sets a flag
			if (long_options[idxopt].flag != 0) break;
			if (optarg) fprintf(stderr, "option %s should have no argument. option ignored.\n",
								long_options[idxopt].name);
			break;
		case 'v':
			verbose_flag = 1;
			break;
		case 'h':
		case '?': // getopt_long emits an error message before returning this value
		default:
			fprintf(stderr, "Usage:\n   %s (--v[erbose] | --h[elp])\n", basename(argv[0]));
			return -1;
		}
	}
	
	if (hid_init()) {
		fprintf(stderr, "\n** unable to initialize HID interface **\n");
		return -1;
	}

	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;	
	while (cur_dev) {
		if (cur_dev->vendor_id == VID_LB_USB) {
			printf("--- Device Found  (id %04hx:%04hx)\n\n", cur_dev->vendor_id, cur_dev->product_id);

			printf("  Manufacturer:  %ls\n", cur_dev->manufacturer_string);
			printf("  Product:       %ls", cur_dev->product_string);

			if (verbose_flag) {
				switch (cur_dev->product_id) {
				case PID_GPS_CLOCK:
					printf(" [Dual output]\n");
					break;
				case PID_MINI_GPS_CLOCK:
					printf(" [Single output]\n");
					break;
				default:
					printf(" [type unknown]\n");
				}
			}
			else printf("\n");
			
			printf("  Serial number: %ls\n", cur_dev->serial_number);
			printf("  Firmware:      %hx.%hx\n", // Major.Minor
				   ((cur_dev->release_number & 0xff00)>>8), (cur_dev->release_number & 0xff));
			
			if (verbose_flag) {
				printf("\n  device path:  %s\n", cur_dev->path);
				printf(  "  Interface:    %d\n\n",  cur_dev->interface_number);
			}
			
			break;
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	// Open the device using the VID, PID, and optionally the Serial number.
	handle = hid_open(VID_LB_USB, cur_dev->product_id, NULL);
	if (!handle) {
		fprintf(stderr, "unable to open device\n");
 		return -1;
	}

	// Set the hid_read() function to be non-blocking.
	// hid_set_nonblocking(handle, 1);

	memset(buf,0x00,sizeof(buf));
	res = 0;
	while (res == 0) {
		res = hid_read(handle, buf, sizeof(buf));
		if (res == 0)
			printf("waiting...\n");
		if (res < 0)
			printf("hid_read() unsuccessful\n");
		// Read requested state. when hid_read() has been set to be
		// non-blocking by the call to hid_set_nonblocking() above.
		// This part of loop demonstrates the non-blocking nature of hid_read().
		// -->> use next part when hid_read() is non-blocking
		/*		#ifdef WIN32
		Sleep(500);
		#else
		usleep(500*1000);
		#endif
		*/
	}

	if (verbose_flag) {
		printf("  Data read (%i bytes):\n    ", res);
		// Print out the returned buffer.
		for (int i = 0; i < res; i++) {
			if (i && !(i % 32)) printf("\n    ");
			printf("%02hhx ", buf[i]);
		}
		printf("\n");
	}
	
	printf("\n  Loss of signal count: %i\n", buf[0]);
	printf(  "  Sat Status: %s", ((buf[1] & 0x01) ? "Unlocked" : "Locked"));
	printf("\n  PLL Status: %s", ((buf[1] & 0x02) ? "Unlocked" : "Locked"));
	printf("\n");

	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x9; // request Report ID 0x9
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res == -1) {
		fprintf(stderr, "unable to request Report ID 0x9\n");
		return -1;
	}
	
	if (verbose_flag) {
		printf("\n  Data read (%i bytes):\n    ", res);
		// Print out the returned buffer.
		for (int i = 0; i < res; i++) {
			if (i && !(i % 32)) printf("\n    ");
			printf("%02hhx ", buf[i]);
		}
		printf("\n");
	}
	
	unsigned int N1_HS = buf[12] + 4;
	unsigned int NC1_LS = (buf[15]<<16) + (buf[14]<<8) + buf[13] + 1;
	unsigned int NC2_LS = (buf[18]<<16) + (buf[17]<<8) + buf[16] + 1;
	unsigned int N31 = (buf[7]<<16) + (buf[6]<<8) + buf[5] + 1;
	unsigned int N2_HS = buf[8] + 4;
	unsigned int N2_LS = (buf[11]<<16) + (buf[10]<<8) + buf[9] + 1;

	unsigned int GPSfreq = (buf[4]<<16) + (buf[3]<<8) + buf[2];
	
	double VCO = (double(GPSfreq) / double(N31)) * double(N2_HS) * double(N2_LS);
	
	double Out1 = VCO / double(N1_HS) / double(NC1_LS);
	double Out2 = VCO / double(N1_HS) / double(NC2_LS);

	printf("\n  Output 1 = %s", ((buf[0] & 0x01) ? "enabled" : "disabled"));
	printf("\n  Output 2 = %s", (((buf[0] & 0x02)>>1) ? "enabled" : "disabled"));
	printf("\n\n");
	
	printf(  "  Output 1 = %.0f Hz\n", Out1);
	printf(  "  Output 2 = %.0f Hz\n\n", Out2);
	printf(  "  Output drive = Level %u\n\n", buf[1]); // 0,1,2, or 3 [?] -> 8,16,24,32mA
	
	if (verbose_flag) {
		printf("  Bandwidth     = %u\n", buf[20]);
		printf("  GPS Ref freq. = %u Hz\n", GPSfreq);
		printf("  N31     = %u\n", N31);
		printf("  N2_HS   = %u\n", N2_HS);
		printf("  N2_LS   = %u\n", N2_LS);
		printf("  N1_HS   = %u\n", N1_HS);
		printf("  NC1_LS  = %u\n", NC1_LS);
		printf("  NC2_LS  = %u\n", NC2_LS);
		printf("  VCO     = %.0f Hz\n\n", VCO);
	}
	
	hid_close(handle); // todo: iterate on all GPS devices found

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause"); // console might be in a window. wait for user.
#endif

	return buf[1] & 0x03; // return locked statuses (normally 0)
}
