# probably works not with all compilers. Thought this should be easy
# fixable. There is nothing special at this source.

rtcm3torinex: rtcm3torinex.c
	$(CC) -Wall -W -O3 -lm $? -o $@

archive:
	zip -9 rtcm3torinex.zip rtcm3torinex.c rtcm3torinex.h rtcm3torinex.txt makefile
