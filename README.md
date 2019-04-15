## Prerequisites ##

gcc g++

make is optional depending on how you want to build it.

## Build Instructions Using Make ##

run ```make```

## Build Instructions Using GCC ##

For the register setting utility:
gcc -o lb-gps lb-gps-linux.cpp GPSSettings.cpp -lstdc++

For the status utility:
gcc -o lb-gps-status lb-gps-linux-status.cpp -lstdc++

## Usage for lb-gps ##
```
usage: lb-gps /dev/hidraw?? [--n31] [--n2_ls] [--n2_hs] [--n1_hs] [--nc1_ls] [--nc2_ls] [--bw]
      --gps:        integer within the range of 1 to 5000000
      --n31:        integer within the range 1 to 2^19
      --n2_ls:      even integer within the range 2 to 2^20
      --n2_hs:      from the range [4,5,6,7,8,9,10,11]
      --n1_hs:      from the range [4,5,6,7,8,9,10,11]
      --nc1_ls:     even integer within the range 2 to 2^20
      --nc2_ls:     even integer within the range 2 to 2^20
      --bw:         bandwidth integer within the range 0 to 15
```

## Usage for lb-gps-status ##
```
usage: lb-gps-status /dev/hidraw??
```
The command shows current lock status, and returns the following values:
For Sat unlocked, return value is 1, for PLL unlocked the value is 2, for both unlocked the value is 3. 0 is returned if both are locked.
