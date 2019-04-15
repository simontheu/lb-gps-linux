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

#include "GPSSettings.h"
    
    void GPSSettings::getSendBuffer (u_int8_t buf[], int length) {

        //Clear the buffer to 0
        memset(buf, 0x0, length);

        //Adjust parameters before applying to buffer
        uint32_t tempN31 = N31 - 1;
        uint32_t tempN2_HS = N2_HS - 4;
        uint32_t tempN2_LS = N2_LS - 1;
        uint8_t tempN1_HS = N1_HS - 4;
        uint32_t tempNC1_LS = NC1_LS - 1;//  = 10 -1;
        uint32_t tempNC2_LS = NC2_LS - 1;
        uint32_t tempBW = bandwidth;
        buf[0] = 0; //reportID; // Report Number
        buf[1] = 0x04;// reportTag for set clock settings

        
        memcpy(&buf[0x2], &GPSFrequency,           3 * sizeof(u_int8_t));
        memcpy(&buf[0x5], &tempN31,                3 * sizeof(u_int8_t));
        memcpy(&buf[0x8], &tempN2_HS,              1 * sizeof(u_int8_t));
        memcpy(&buf[0x9], &tempN2_LS,              3 * sizeof(u_int8_t));
        memcpy(&buf[0xC], &tempN1_HS,              1 * sizeof(u_int8_t));
        memcpy(&buf[0xD], &tempNC1_LS,             3 * sizeof(u_int8_t));
        memcpy(&buf[0x10], &tempNC2_LS,            3 * sizeof(u_int8_t));

        memcpy(&buf[0x13], &phase,             sizeof(uint8_t));
        memcpy(&buf[0x14], &tempBW,         sizeof(uint8_t));

    }

    void GPSSettings::setParamsFromReadBuffer(u_int8_t buf[], int length) {

        out1Enabled = buf[0] & 0x01;
        out2Enabled = buf[0] & 0x02 >> 1;
        driveStrength = buf[1] > 3 ? buf[1] : 3; //Limit at 3
        
        GPSFrequency = (buf[4] << 16) + (buf[3] << 8) + buf[2];
        
        N31 = ((buf[7] << 16) + (buf[6] << 8) + buf[5]) + 1;
        N2_HS = buf[8] + 4;
        N2_LS = ((buf[11] << 16) + (buf[10] << 8) + buf[9]) + 1;
        N1_HS = buf[12] + 4;
        NC1_LS = ((buf[15] << 16) + (buf[14] << 8) + buf[13]) + 1;
        NC2_LS = ((buf[18] << 16) + (buf[17] << 8) + buf[16]) + 1;
        bandwidth = buf[20];
    }

    void GPSSettings::printParameters() {
        double VCO = (double(GPSFrequency) / double(N31)) * double(N2_HS) * double(N2_LS);
        double Out1 = VCO / double(N1_HS) / double(NC1_LS);
        double Out2 = VCO / double(N1_HS) / double(NC2_LS);
        printf("    GPS Frequency = %u\n", GPSFrequency);
        printf("    N31           = %u\n", N31);
        printf("    N2_HS         = %u\n", N2_HS);
        printf("    N2_LS         = %u\n", N2_LS);
        printf("    N1_HS         = %u\n", N1_HS);
        printf("    NC1_LS        = %u\n", NC1_LS);
        printf("    NC2_LS        = %u\n\n", NC2_LS);
        printf("    VCO           = %f Hz\n\n", VCO);
        printf("    Clock Out 1   = %f Hz\n", Out1);
        printf("    Clock Out 2   = %f Hz\n", Out2);
        printf("    Bandwidth     = %u\n", bandwidth);
        
    }

    int GPSSettings::processCommandLineArguments(int argc, char **argv)
    {
      int c;

      while (1)
      {
            static struct option long_options[] =
            {
                  /* These options set a flag. */
                  {"verbose", no_argument,       &verbose_flag, 1},
                  {"brief",   no_argument,       &verbose_flag, 0},
                  /* These options don’t set a flag.
                        We distinguish them by their indices. */
                  {"gps",    required_argument, 0, 'a'},
                  {"n31",     required_argument, 0, 'b'},
                  {"n2_hs",   required_argument, 0, 'c'},
                  {"n2_ls",  required_argument, 0, 'd'},
                  {"n1_hs",  required_argument, 0, 'e'},
                  {"nc1_ls",    required_argument, 0, 'f'},
                  {"nc2_ls",    required_argument, 0, 'g'},
                  {"drive",    required_argument, 0, 'h'},
                  {"bw",    required_argument, 0, 'i'},
                  {0, 0, 0, 0}
            };
            /* getopt_long stores the option index here. */
            int option_index = 0;

            c = getopt_long (argc, argv, "abc:d:f:",
                        long_options, &option_index);

            /* Detect the end of the options. */
            if (c == -1)
            break;

            switch (c)
            {
                case 0:
                    /* If this option set a flag, do nothing else now. */
                    if (long_options[option_index].flag != 0)
                            break;
                    printf ("option %s", long_options[option_index].name);
                    if (optarg)
                            printf (" with arg %s", optarg);
                    printf ("\n");
                    break;
                
                case 'a'://N31
                    GPSFrequency = atoi(optarg);
                    break;

                case 'b'://N31
                    N31 = atoi(optarg);
                    break;

                case 'c'://N2_HS
                    N2_HS = atoi(optarg);
                    break;

                case 'd'://N2_LS
                    N2_LS = atoi(optarg);
                    break;

                case 'e'://N1_HS
                    N1_HS = atoi(optarg);
                    break;

                case 'f'://NC1_LS
                    NC1_LS = atoi(optarg);
                    break;
                
                case 'g'://NC2_LS
                    NC2_LS = atoi(optarg);
                    break;
                
                case 'h'://Drive
                    driveStrength = atoi(optarg);
                    break;
                
                case 'i'://Bandwith
                    bandwidth = atoi(optarg);
                    break;

                case '?':
                    /* getopt_long already printed an error message. */
                    break;

                default:
                    abort ();
            }
        } 
    }


    void GPSSettings::verifyParameters()
    {
        double VCO = (double(GPSFrequency) / double(N31)) * double(N2_HS) * double(N2_LS);
        if (N31 < kN31LowerLimit || N31 > kN31UpperLimit) {
            printf("\e[0;33m[Warning]\e[0m Invalid Parameter: N31\n");
            printf("N31 Should be in the range from %i to %i\n", kN31LowerLimit, kN31UpperLimit);
        }
        if (bandwidth < kBandwidthLowerLimit || bandwidth > kBandwidthUpperLimit) {
            printf("\e[0;33m[Warning]\e[0m Invalid Parameter: bw\n");
            printf("Bandwidth should be in the range from %i to %i\n", kBandwidthLowerLimit, kBandwidthUpperLimit);
        }
        if (VCO < kVCOLowerLimit || VCO > kVCOUpperLimit) {
            printf("\e[0;33m[Warning]\e[0m Invalid Parameters\n");            
            printf("The VCO should fall between %fHz to %fHz\n", kVCOLowerLimit, kVCOUpperLimit);
        }
    }

