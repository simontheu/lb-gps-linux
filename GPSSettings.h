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

/* Flag set by ‘--verbose’. */
static int verbose_flag;

//Limits for parameters
static const uint32_t kN31LowerLimit = 1;
static const uint32_t kN31UpperLimit = 524288;

class GPSSettings {
    public:
    
    uint8_t out1Enabled;// = 0
    uint8_t out2Enabled;// = 0;
    
    uint8_t driveStrength;// = 0;

    uint8_t reportID;// = 0x0; //Report ID Unused
    uint8_t reportTag;// = 0x4; //Change clock settings tag

    uint32_t GPSFrequency;//  = 0;  // 800Hz - 10 000 000Hz
    uint32_t N31;// = 2 -1;//1,2,4,5...2**21
    uint32_t N2_HS;// = 4 -4;
    uint32_t N2_LS;// = 6 -1; // 0 .. 0xFFF (0..4095)
    uint8_t N1_HS;// = 8 -4;
    uint32_t NC1_LS;//  = 10 -1;
    uint32_t NC2_LS;//  = 12 -1;

    uint8_t phase; //= 1;//?? Need to clarify this
    uint8_t bandwidth;// = 3;//0-15

    //HID Specific Settings
    struct hidraw_report_descriptor rpt_desc;
    struct hidraw_devinfo info;

    
    void getSendBuffer (u_int8_t buf[], int length) {

        //Clear the buffer to 0
        memset(buf, 0x0, length);

        //Adjust parameters before applying to buffer
        uint32_t tempN31 = N31 - 1;
        uint32_t tempN2_HS = N2_HS - 4;
        uint32_t tempN2_LS = N2_LS - 1;
        uint8_t tempN1_HS = N1_HS - 4;
        uint32_t tempNC1_LS = NC1_LS - 1;//  = 10 -1;
        uint32_t tempNC2_LS = NC2_LS - 1;

        buf[0] = 0; //reportID; // Report Number
        buf[1] = 0x04;// reportTag for set clock settings

        memcpy(&buf[0x2], &GPSFrequency,       2 * sizeof(u_int8_t));
        memcpy(&buf[0x5], &tempN31,                2 * sizeof(u_int8_t));
        memcpy(&buf[0x8], &tempN2_HS,              2 * sizeof(u_int8_t));
        memcpy(&buf[0x9], &tempN2_LS,              2 * sizeof(uint8_t));
        memcpy(&buf[0xC], &tempN1_HS,              sizeof(u_int8_t));
        memcpy(&buf[0xD], &tempNC1_LS,             2 * sizeof(u_int8_t));
        memcpy(&buf[0x10], &tempNC2_LS,            2 * sizeof(u_int8_t));

        memcpy(&buf[0x13], &phase,             sizeof(uint8_t));
        memcpy(&buf[0x14], &bandwidth,         sizeof(uint8_t));

    }

    void setParamsFromReadBuffer(u_int8_t buf[], int length) {

        this->out1Enabled = buf[0] & 0x01;
        this->out2Enabled = buf[0] & 0x02 >> 1;
        this->driveStrength = buf[1] > 3 ? buf[1] : 3; //Limit at 3
        
        this->GPSFrequency = (buf[4] << 16) + (buf[3] << 8) + buf[2];
        
        this->N31 = ((buf[7] << 16) + (buf[6] << 8) + buf[5]) + 1;
        this->N2_HS = buf[8] + 4;
        this->N2_LS = ((buf[11] << 16) + (buf[10] << 8) + buf[9]) + 1;
        this->N1_HS = buf[12] + 4;
        this->NC1_LS = ((buf[15] << 16) + (buf[14] << 8) + buf[13]) + 1;
        this->NC2_LS = ((buf[18] << 16) + (buf[17] << 8) + buf[16]) + 1;
    }

    void printParameters() {
        double VCO = (double(this->GPSFrequency) / double(this->N31)) * double(this->N2_HS) * double(this->N2_LS);
        double Out1 = VCO / double(this->N1_HS) / double(this->NC1_LS);
        double Out2 = VCO / double(this->N1_HS) / double(this->NC2_LS);
        printf("GPS Frequency = %u\n", this->GPSFrequency);
        printf("N31           = %u\n", this->N31);
        printf("N2_HS         = %u\n", this->N2_HS);
        printf("N2_LS         = %u\n", this->N2_LS);
        printf("N1_HS         = %u\n", this->N1_HS);
        printf("NC1_LS        = %u\n", this->NC1_LS);
        printf("NC2_LS        = %u\n\n", this->NC2_LS);
        printf("VCO           = %f Hz\n\n", VCO);
        printf("Clock Out 1   = %f Hz\n", Out1);
        printf("Clock Out 2   = %f Hz\n", Out2);
        
    }

    int processCommandLineArguments(int argc, char **argv)
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
                    this->GPSFrequency = atoi(optarg);
                    break;

                case 'b'://N31
                    this->N31 = atoi(optarg);
                    break;

                case 'c'://N2_HS
                    this->N2_HS = atoi(optarg);
                    break;

                case 'd'://N2_LS
                    this->N2_LS = atoi(optarg);
                    break;

                case 'e'://N1_HS
                    this->N1_HS = atoi(optarg);
                    break;

                case 'f'://NC1_LS
                    this->NC1_LS = atoi(optarg);
                    break;
                
                case 'g'://NC2_LS
                    this->NC2_LS = atoi(optarg);
                    break;
                
                case 'h'://Drive
                    this->driveStrength = atoi(optarg);
                    break;
                
                case 'i'://
                    //this->NC2_LS = atoi(optarg);
                    break;

                case '?':
                    /* getopt_long already printed an error message. */
                    break;

                default:
                    abort ();
            }
        } 
    }

    void verifyParameters()
    {
        if (this->N31 < kN31LowerLimit || this->N31 > kN31UpperLimit) {
            printf("\n[Warning] Invalid Parameter: N31\n");
            printf("N31 Should be in the range from %i to %i\n", kN31LowerLimit, kN31UpperLimit);
        }
    }

} ;
