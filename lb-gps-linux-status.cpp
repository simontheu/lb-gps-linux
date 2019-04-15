#include <iostream>
using namespace std;

/* Linux */
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <getopt.h>

#include <time.h>

//Leo GPS Clock
#include "GPSSettings.h"

/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 */
#ifndef HIDIOCSFEATURE
#warning Please have your distro update the userspace kernel headers
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif

#define HIDIOCGRAWNAME(len)     _IOC(_IOC_READ, 'H', 0x04, len)

const char *bus_str(int bus);

int main(int argc, char **argv)
{
      cout << "Leo Bodnar GPS Clock Status" << endl;
      
      int fd;
      int i, res, desc_size = 0;
      u_int8_t buf[60];

      struct hidraw_devinfo info;

   /* Open the Device with non-blocking reads. In real life,
      don't use a hard coded path; use libudev instead. 
   */


      printf("Opening device %s\n", argv[1]);

      fd = open(argv[1], O_RDWR|O_NONBLOCK);

      if (fd < 0) 
      {
            perror("Unable to open device");
            return 1;
      }

      //Device connected, setup report structs
      memset(&info, 0x0, sizeof(info));

      // Get Raw Info
      res = ioctl(fd, HIDIOCGRAWINFO, &info);
      
      if (res < 0) 
      {
            perror("HIDIOCGRAWINFO");
      } 
      else
      {
            printf("Device Info:\n");
            printf("\tvendor: 0x%04hx\n", info.vendor);
            printf("\tproduct: 0x%04hx\n", info.product);
            if (info.vendor != VID_LB_USB || (info.product != PID_GPS_CLOCK && info.product != PID_MINI_GPS_CLOCK)) {
                perror("Not a valid GPS Clock Device");
                return -1;//Device not valid
            }
      }

      /* Get Raw Name */
      res = ioctl(fd, HIDIOCGRAWNAME(256), buf);

      if (res < 0)
            perror("HIDIOCGRAWNAME");
      else
            printf("Device Name: %s\n", buf);
            /* Get Status */
            int timeout = 0;
            while (timeout < 1000){
                int report_len = read(fd, buf, sizeof (buf));
                printf(".");
                if (report_len < 1)  {
                    printf(".");
                    //Create some delay before trying again.
                    for (i = 0; i< 10000000; i++) {
                        report_len = i*i;
                    }
                    timeout++;
                } else {
                    printf("\n");
                    printf("Loss of Signal Count: %i\n", buf[0]);
                    if (buf[1] & 0x01) {
                        printf("Sat Status: Unlocked\n");
                    } else {
                        printf("Sat Status: Locked\n");
                    }
                    if (buf[1] & 0x02) {
                        printf("PLL Status: Unlocked\n");
                    } else {
                        printf("PLL Status: Locked\n");
                    }
                    return buf[1] & 0x03;//Return locked status
                }
            }

      close(fd);

      return 0;
      }

const char * bus_str(int bus)
{
   switch (bus) 
   {
      case BUS_USB:       return "USB";       break;
      case BUS_HIL:       return "HIL";       break;
      case BUS_BLUETOOTH: return "Bluetooth"; break;
      case BUS_VIRTUAL:   return "Virtual";   break;
      default: return "Other"; break;
   }
}
