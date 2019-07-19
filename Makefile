CFLAGS=-g
LBPROGRAMS=lb-gps-linux lb-gps-linux-status
PROGRAMS=gpsdo-status
HIDLIB=-L. -lhidapi-hidraw -Wl,-rpath,.

all: gpsdo-status lb-gps-utils

gpsdo-status: gpsdo-status.cpp libhidapi-hidraw.so
	gcc ${CFLAGS} -o gpsdo-status gpsdo-status.cpp -I. -lstdc++ ${HIDLIB}

libhidapi-hidraw.so:
	ln -s libhidapi-hidraw.so.0 libhidapi-hidraw.so

lb-gps-utils: lb-gps-linux-status.cpp lb-gps-linux.cpp GPSSettings.cpp
	gcc ${CFLAGS} -o lb-gps-linux-status lb-gps-linux-status.cpp -I. -lstdc++
	gcc ${CFLAGS} -o lb-gps-linux lb-gps-linux.cpp GPSSettings.cpp -I. -lstdc++

all-clean:
	rm ${LBPROGRAMS} ${PROGRAMS} libhidapi-hidraw.so

clean:
	rm ${PROGRAMS}


