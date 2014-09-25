all: readco readVtec readRTCM3
CC       = gcc
CXX      = g++
DEFINES  = -DNO_RTCM3_MAIN
CFLAGS   = -ggdb  $(DEFINES)
CXXFLAGS = -ggdb  $(DEFINES)

clock_orbit_rtcm.o: lib/clock_orbit_rtcm.c
	$(CC) -c $(CFLAGS) -o $@ $< -I lib

rtcm3torinex.o: lib/rtcm3torinex.c
	$(CC) -c $(CFLAGS) -o $@ $< -I lib  


readco.o: readco.c clock_orbit_rtcm.o
	$(CC) -c $(CFLAGS) $< -I lib

readVtec.o: readVtec.c
	$(CC) -c $(CFLAGS) $< -I lib

readRTCM3.o: readRTCM3.cpp
	$(CXX) -c $(CXXFLAGS) $< -I lib
	

readco: readco.o clock_orbit_rtcm.o
	$(CC) $(CFLAGS) -lm -o $@ readco.o clock_orbit_rtcm.o

readVtec: readVtec.o clock_orbit_rtcm.o
	$(CC) $(CFLAGS) -lm -o $@ readVtec.o clock_orbit_rtcm.o

readRTCM3: readRTCM3.o clock_orbit_rtcm.o rtcm3torinex.o
	$(CXX) $(CXXFLAGS) -lm -o $@ readRTCM3.o clock_orbit_rtcm.o rtcm3torinex.o
	

clean:
	rm *.o
	rm readco readVtec readRTCM3