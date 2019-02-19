lb-gps-linux: lb-gps-linux-status.cpp lb-gps-linux.cpp GPSSettings.cpp
	gcc -o lb-gps-linux-status lb-gps-linux-status.cpp -I. -lstdc++
	gcc -o lb-gps-linux lb-gps-linux.cpp GPSSettings.cpp -I. -lstdc++


