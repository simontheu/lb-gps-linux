//USB ID Defines
#define VID_LB_USB 0x1dd2
#define PID_GPS_CLOCK 0x2210
#define PID_MINI_GPS_CLOCK 0x2211

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
    
    //Sets instance parameters based on buffer
    void setParamsFromReadBuffer(u_int8_t buf[], int length);

    //Populates buffer based on instance settings
    void getSendBuffer (u_int8_t buf[], int length);

    //Prints instance parameters to the console
    void printParameters() ;

    //Sets instance parameters based on CLI parameters
    int processCommandLineArguments(int argc, char **argv);

    //Checks current instance parameters and reports if any are outside of the window.
    void verifyParameters();

} ;
