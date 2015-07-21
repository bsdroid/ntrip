Configuration Examples

BNC comes with a number of configuration examples which can be used on all 
operating systems. You may use a statically linked BNC executable to run the
configuration examples. Configuration 'PPPGoogleMaps.bnc' is an exception
from this because it requires a shared library BNC build.

If not already done then copy the complete directory 'Example_Configs' to you
disc. It contains sub-directories 'Input' and 'Output'. There are several ways to
start BNC using one of the example configurations: 

* On graphical systems (except for Mac systems) you may use the computer mouse to 
  'drag' a configuration file icon and 'drop' it on top of BNC's program icon. 
* On non-graphical systems you may start BNC using a command line with the 
  following option for a configuration file (example for Windows systems):
  bnc.exe --conf <configFileName> --nw 

Although it's not a must, we suggest that you always create BNC configuration 
files with the file name extension '.bnc'. 

We furthermore suggest for convenience reasons that you configure your system 
to automatically start BNC when you double-click a file with the file name 
extension '.bnc'. The following describes what to do on Windows systems to 
associate the BNC program to such configuration files: 

1. Right-click a file that has the extension '.bnc' and then click 'Open'. If the 
   'Open' command is not available, click 'Open With' or double-click the file. 
2. Windows displays a dialog box that says that the system cannot open this file. 
   The dialog box offers several options for selecting a program. 
3. Click 'Select the program from a list', and then click 'OK'. 
4. The 'Open With' dialog box is displayed. Click 'Browse', locate and then click 
   the BNC program, and then click 'Open'. 
5. Click to select the 'Always use the selected program to open this kind of file' 
   check box. 
6. Click 'OK'. 

Some of the presented example configuration files contain a user ID 'Example' 
with a password 'Configs' for accessing a few GNSS streams from public Ntrip 
Broadcasters. This generic account is arranged for convenience reasons only. 
Please be so kind as to replace the generic account details as well as the 
place holders 'User' and 'Pass' by the personal user ID and password you 
receive following an online registration through 
http://register.rtcm-ntrip.org. 

Note that the account for an Ntrip Broadcaster is usually limited to pulling a 
specified maximum number of streams at the same time. As running some of the 
example configurations requires pulling several streams, it is suggested to 
make sure that you don't exceed your account's limits. 

Make also sure that sub-directories 'Input' and 'Output' which are part of the 
example configurations exist on your system or adjust the affected example 
configuration options according to your needs. 

Some BNC options require antenna phase center variations as made available from 
IGS through so-called ANTEX files at ftp://igs.org/pub/station/general. An 
example ANTEX file 'igs08.atx' is part of the BNC package for convenience. 

The example configurations assume that no proxy protects your BNC host. Should 
a proxy be operated in front of BNC then you need to introduce its IP and port 
in the 'Network' tab of the example configurations.

You should be able to run all configuration examples without changing their 
options. However, configurations 'Upload.bnc' and 'UploadPPP.bnc' are 
exceptions because they require an input stream from a connected GNSS network 
engine.

1. File 'RinexObs.bnc'
The purpose of this configuration is showing how to convert RTCM streams to 
RINEX. The configuration pulls two streams from Ntrip Broadcasters using 
Ntrip version 2 to generate 15min 1Hz RINEX Version 3 observation files. 
Note that network option 'Ignore SSL authorization errors' is set in order 
to allow pulling RINEX skeleton files via HTTPS when necessary. See 
http://igs.bkg.bund.de/ntrip/observations for observation stream resources. 

2. File 'RinexEph.bnc'
The purpose of this configuration is showing how to convert a RTCM stream 
carrying navigation messages to a RINEX Navigation files. The configuration 
pulls an RTCM Version 3 stream with Broadcast Ephemeris coming from the 
real-time EUREF and IGS networks. It saves hourly RINEX Version 3 Navigation 
files. See http://igs.bkg.bund.de/ntrip/ephemeris for further real-time 
Broadcast Ephemeris resources. 

3. File 'BrdcCorr.bnc'
The purpose of this configuration is to save Broadcast Corrections from RTCM 
SSR messages in a plain ASCII format as hourly files. See 
http://igs.bkg.bund.de/ntrip/orbits for further real-time IGS or EUREF 
orbit/clock products. 

4. File 'RinexConcat.bnc'
The purpose of this configuration is to concatenate RINEX Version 3 files to 
produce a concatenated file and edit the marker name in the file header. The 
sampling interval is set to 30 seconds. See section 'RINEX Editing & QC' in the 
documentation for examples on how to call BNC from command line in 'no window' 
mode for RINEX file editing, concatenation and quality checks. 

5. File 'RinexQC.bnc'
The purpose of this configuration is to check the quality of a RINEX Version 3 
file through a multipath analysis. The results is saved in disk in terms of a 
plot in PNG format. See section 'RINEX Editing & QC' in the documentation for 
examples on how to call BNC from command line in 'no window' mode for RINEX 
file editing, concatenation and quality checks. 

6. File 'RTK.bnc'
The purpose of this configuration is to feed a serial connected receiver with 
observations from a reference station for conventional RTK. The stream is 
scanned for RTCM messages. Message type numbers and latencies of incoming 
observation are reported in BNC's logfile. 

7. File 'FeedEngine.bnc'
The purpose of this configuration is to feed a real-time GNSS engine with 
observations from a remote reference stations. The configuration pulls a single 
stream from an NTRIP Broadcasters. It would of course be possible to pull 
several streams from different casters. Incoming observations are decoded, 
synchronized and output through a local IP port and saved into a file. Failure 
and recovery thresholds are specified to inform about outages. 

8. File 'PPP.bnc'
The purpose of this configuration is Precise Point Positioning from 
observations of a rover receiver. The configuration reads RTCM Version 3 
observations, a Broadcast Ephemeris stream and a stream with Broadcast 
Corrections. Positions are saved in the logfile. 

9. File 'PPPNet.bnc'
The purpose of this configuration is to demonstrate siumultaneous Precise 
Point Positioning for several rovers or several receivers from a network of 
reference stations in one BNC job. The possible maximum number of PPP solutions 
per job depends on the processing power of the hosting computer. This example 
configuration reads two RTCM Version 3 observation streams, a Broadcast 
Ephemeris stream and a stream with Broadcast Corrections. PPP Results for the 
two stations are saved in PPP logfiles. 

10. File 'PPPQuickStart.bnc'
The purpose of this configuration is Precise Point Positioning in Quick-Start 
mode from observations of a static receiver with precisely known position. The 
configuration reads RTCM Version 3 observations, Broadcast Corrections and a 
Broadcast Ephemeris stream. Positions are saved in NMEA format on disc. 
Positions are also output through IP port for real-time visualization with 
tools like RTKPLOT. Positions are also saved in the logfile. 

11. File 'PPPPostProc.bnc'
The purpose of this configuration is Precise Point Positioning in Post 
Processing mode. BNC reads a RINEX Observation and a RINEX Version 3 Navigation 
files and a Broadcast Corrections file. PPP processing options are set to 
support the Quick-Start mode. The output is saved in a specific Post Processing 
logfile and contains the coordinates derived over time following the 
implemented PPP filter algorithm. 

12. File 'PPPGoogleMaps.bnc'
The purpose of this configuration is to track BNC's point positioning
solution using Google Maps or Open StreetMap as background. BNC reads a
RINEX Observation file and a RINEX Navigation file to carry out a
'Standard Point Positioning' solution in post-processing mode. Although 
this is not a real-time application it requires the BNC host to be connected
to the Internet. Specify a computation speed, then hit button 'Open Map'
to open the track map, then hit 'Start' to visualize receiver positions
on top of GM/OSM maps.

13. File 'SPPQuickStartGal.bnc'
The purpose of this configuration is Single Point Positioning in Quick-Start 
mode from observations of a static receiver with precisely known position. The 
configuration uses GPS, GLONASS and Galileo observations and a Broadcast 
Ephemeris stream. 

14. File 'SaveSp3.bnc'
The purpose of this configuration is to produce SP3 files from a Broadcast 
Ephemeris stream and a Broadcast Corrections stream. The Broadcast Corrections 
stream is formally introduced in BNC's 'Combine Corrections' table. Note that 
producing SP3 requires an ANTEX file because SP3 file contents should be 
referred to CoM. 

15. File 'Sp3ETRF2000PPP.bnc'
The purpose of this configuration is to produce SP3 files from a Broadcast 
Ephemeris stream and a stream carrying ETRF2000 Broadcast Corrections. The 
Broadcast Corrections stream is formally introduced in BNC's 'Combine 
Corrections' table. This leads to an SP3 file containing orbits referred also 
to ETRF2000. Pulling in addition observations from a reference station at 
precisely known ETRF2000 position allows comparing an 'INTERNAL' PPP solution 
with ETRF2000 reference coordinates. 

16. File 'Upload.bnc'
The purpose of this configuration is to upload orbits and clocks from a 
real-time GNSS engine to an NTRIP Broadcaster. For that the configuration reads 
precise orbits and clocks in RTNET format. It also reads a stream carrying 
Broadcast Ephemeris. BNC converts the orbits and clocks into Broadcast 
Corrections and encodes them in RTCM Version 3 SSR messages to upload them to 
an NTRIP Broadcaster. The Broadcast Corrections stream is referred to satellite 
Antenna Phase Center (APC) and IGS08. Orbits are saved on disk in SP3 format 
and clocks in Clock RINEX format. 

17. File 'UploadPPP.bnc'
This configuration equals the 'Upload.bnc' configuration. However, the 
Broadcast Corrections are in addition used for an 'INTERNAL' PPP solution based 
on observations from a static reference station with known precise coordinates. 
This allows a continuous quality check of the Broadcast Corrections through 
observing coordinate displacements. 

18. File 'Combi.bnc'
The purpose of this configuration is to pull several streams carrying Broadcast 
Corrections and a Broadcast Ephemeris stream from an NTRIP Broadcaster to 
produce a combined Broadcast Corrections stream. BNC encodes the combination 
product in RTCM Version 3 SSR messages and uploads that to an Ntrip 
Broadcaster. The Broadcast Corrections stream is not referred to satellite 
Center of Mass (CoM). It is referred to IGS08. Orbits are saved in SP3 format 
and clocks in Clock RINEX format. 

19. File 'CombiPPP.bnc'
This configuration equals the 'Combi.bnc' configuration. However, the combined 
Broadcast Corrections are in addition used for an 'INTERNAL' PPP solutions 
based on observations from a static reference station with known precise 
coordinates. This allows a continuous quality check of the combination product 
through observing coordinate displacements. 

20. File 'UploadEph.bnc'
The purpose of this configuration is to pull a number of streams from reference 
stations to get hold of contained Broadcast Ephemeris messages. These are 
encoded then in a RTCM Version 3 stream which only provides Broadcast Ephemeris 
with an update rate of 5 seconds. 

21. File 'CompareSp3.bnc'
The purpose of this configuration is to compare two SP3 files to calculate 
RMS values for orbit and clock differences. GPS satellite G05 and GLONASS 
satellite R18 are excluded from this comparison. Comparison results are saved 
in a logfile. 

22. File 'Empty.bnc'
The purpose of this example is to provide an empty configuration file for BNC
which only contains the default settings.

Georg Weber, BKG
Frankfurt, August 2015
igs-ip@bkg.bund.de
