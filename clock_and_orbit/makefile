all: readco readVtec

clock_orbit_rtcm.o: lib/clock_orbit_rtcm.c
	gcc -ggdb -c -o $@ $< -I lib

readco.o: readco.c
	gcc -ggdb -c $< -I lib
	
readVtec.o: readVtec.c
	gcc -ggdb -c $< -I lib

readco: readco.o clock_orbit_rtcm.o
	gcc -ggdb -lm -o $@ readco.o clock_orbit_rtcm.o
	
readVtec: readVtec.o clock_orbit_rtcm.o
	gcc -ggdb -lm -o $@ readVtec.o clock_orbit_rtcm.o

clean:
	rm *.o
	rm readco readVtec
