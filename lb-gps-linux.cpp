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
      cout << "Leo Bodnar GPS Clock" << endl;
      
      int fd;
      int i, res, desc_size = 0;
      u_int8_t buf[60];

      struct hidraw_devinfo info;

      GPSSettings *currentSettings = new GPSSettings;

   /* Open the Device with non-blocking reads. In real life,
      don't use a hard coded path; use libudev instead. 
   */
      if (argc == 1)
      {
            printf("usage: lb-gps-clock /dev/hidraw?? [--n31] [--n2_ls] [--n2_hs] [--n1_hs] [--nc1_ls] [--nc2_ls]\n");
            printf("      --gps:        integer within the range of 1 to 5000000\n");
            printf("      --n31:        integer within the range 1 to 2^19\n");
            printf("      --n2_ls:      even integer within the range 2 to 2^20\n");
            printf("      --n2_hs:      from the range [4,5,6,7,8,9,10,11]\n");
            printf("      --n1_hs:      from the range [4,5,6,7,8,9,10,11]\n");
            printf("      --nc1_ls:     even integer within the range 2 to 2^20\n");
            printf("      --nc2_ls:     even integer within  the range 2 to 2^20\n");
            printf("      --bw:         bandwidth integer within  the range 0 to 15\n");
            return -1;
      }

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
            if (info.vendor != VID_LB_USB || (info.product != PID_GPS_CLOCK && info.product != PID_MINI_GPS_CLOCK)) {
                  perror("Not a valid GPS Clock Device");
                  printf("Device Info:\n");
                  printf("\tvendor: 0x%04hx\n", info.vendor);
                  printf("\tproduct: 0x%04hx\n", info.product);
                  return -1;//Device not valid
            }
      }

      /* Get Raw Name */
      res = ioctl(fd, HIDIOCGRAWNAME(256), buf);

      if (res < 0) {
            perror("HIDIOCGRAWNAME");
      }
      else {
            printf("Connected To: %s\n", buf);
      }

      /* Get Feature */
      buf[0] = 0x9; /* Report Number */
      res = ioctl(fd, HIDIOCGFEATURE(256), buf);

      if (res < 0) {
            perror("HIDIOCGFEATURE");
      } else {
            currentSettings->setParamsFromReadBuffer(buf,res);
            printf("\nCurrent Settings:\n");
            currentSettings->printParameters();
            printf("\n");
      }

      /* Get Raw Name */
      res = ioctl(fd, HIDIOCGRAWNAME(256), buf);

      if (res < 0) {
            perror("HIDIOCGRAWNAME");
      }
      else {
            currentSettings->GPSSettings::processCommandLineArguments(argc, argv);
            printf("\nNew Settings:\n");
            currentSettings->GPSSettings::printParameters();
            currentSettings->GPSSettings::verifyParameters();
            currentSettings->GPSSettings::getSendBuffer(buf,60);

            /* Set Feature */
            res = ioctl(fd, HIDIOCSFEATURE(60), buf);
            if (res < 0) perror("HIDIOCSFEATURE");
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
