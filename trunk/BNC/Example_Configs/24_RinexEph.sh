#!/bin/bash

../bnc -nw -conf /dev/null \
       -key mountPoints "//Example:Configs@products.igs-ip.net:80/RTCM3EPH RTCM_3 DEU 50.09 8.66 no 2" \
       -key ephPath Output \
       -key logFile Output/RinexEph.log \
       -key ephIntr 1 hour \
       -key ephV3 2 \

psID=`echo $!`
sleep 30
kill $psID

