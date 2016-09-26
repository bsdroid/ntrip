.. index:: Annex

.. 
   for html
  
  .. |G:CWPX_?PP| replace:: G:CWPX_?  

.. 
   for latex
   
.. |G:CWPX_?| replace:: G:CWPX\_\?
  
Annex
*****

.. index:: Revision History

Revision History
================

.. tabularcolumns:: |p{0.1\linewidth}|p{0.15\linewidth}|p{0.67\linewidth}|

======== ============= =============================================================
Dec 2006 Version 1.0b  [Add] First Beta Binaries published based on Qt 4.2.3.
Jan 2007 Version 1.1b  [Add] Observables C2, S1, and S2
\                      [Add] Virtual reference station access
\                      [Bug] RTCM2 decoder time tag fixed
\                      [Mod] Small letters for public RINEX skeleton files
\                      [Add] Online help through Shift+F1
Apr 2007 Version 1.2b  [Bug] Output only through IP port
\                      [Bug] Method 'reconnecting' now thread-save
\                      [Add] ZERO decoder added
\                      [Mod] Download public RINEX skeletons once per day
\                      [Mod] Upgrade to Qt Version 4.2.3
\                      [Mod] Replace 'system' call for RINEX script by 'QProcess'
\                      [Add] HTTP Host directive for skeleton file download
\                      [Add] Percent encoding for user IDs and passwords
\                      [Bug] Exit execution of calling thread for RTCM3 streams
\                      [Bug] Signal-slot mechanism for threads
May 2007 Version 1.3   [Add] Source code published.
Jul 2007 Version 1.4   [Bug] Skip messages from proxy server
\                      [Bug] Call RINEX script through 'nohup'
Apr 2008 Version 1.5   [Add] Handle ephemeris from RTCM Version 3 streams
\                      [Add] Upgrade to Qt Version 4.3.2
\                      [Add] Optional RINEX v3 output
\                      [Add] SBAS support
\                      [Bug] RINEX skeleton download following stream outage
\                      [Add] Handle ephemeris from RTIGS streams
\                      [Add] Monitor stream failure/recovery and latency
\                      [Mod] Redesign of main window
\                      [Bug] Freezing of About window on Mac OS X
\                      [Bug] Fixed problem with PRN 32 in RTCM v2 decoder
\                      [Bug] Fix for Trimble 4000SSI receivers in RTCM v2 decoder
\                      [Mod] Major revision of input buffer in RTCM v2 decoder
Dec 2008 Version 1.6   [Mod] Fill blank columns in RINEX v3 with 0.000
\                      [Add] RTCM v3 decoder for orbit and clock corrections
\                      [Add] Check RTCM v3 streams for incoming message types
\                      [Add] Decode RTCM v2 message types 3, 20, 21, and 22
\                      [Add] Loss of lock and lock time indicator
\                      [Bug] Rounding error in RTCM v3 decoder concerning GLONASS height
\                      [Mod] Accept GLONASS in RTCM v3 when transmitted first
\                      [Add] Leap second 1 January 2009
\                      [Add] Offline mode, read data from file
\                      [Add] Output antenna descriptor, coordinates and eccentricities from RTCM v3
\                      [Add] Reconfiguration on-the-fly
\                      [Mod] Binary output of synchronized observations
\                      [Add] Binary output of unsynchronized observations
\                      [Bug] Fixed problem with joined RTCM v3 blocks
Dec 2008 Version 1.6.1 [Mod] HTTP GET when no proxy in front
Nov 2009 Version 1.7   [Bug] RINEX Navigation file format
\                      [Add] Upgrade to Qt Version 4.5.2
\                      [Add] Support of Ntrip v2
\                      [Add] Rover support via serial port
\                      [Add] Show broadcaster table from www.rtcm-ntrip.org
\                      [Add] Enable/disable panel widgets
\                      [Add] User defined configuration filename
\                      [Mod] Switch to configuration files in ini-Format
\                      [Add] Daily logfile rotation
\                      [Add] Read from TCP/IP port, by-pass Ntrip transport protocol
\                      [Add] Save NMEA sentences coming from rover
\                      [Add] Auto start
\                      [Add] Drag and drop ini files
\                      [Add] Read from serial port, by-pass Ntrip transport protocol
\                      [Mod] Update of SSR messages following RTCM 091-2009-SC104-542
\                      [Add] Read from UPD port, by-pass Ntrip transport protocol
\                      [Mod] Output format of Broadcast Corrections
\                      [Add] Throughput plot
\                      [Add] Latency plot
Nov 2009 Version 1.8   [Mod] On-the-fly reconfiguration of latency and throughput plots
Feb 2010 Version 2.0   [Mod] Change sign of Broadcast Corrections
\                      [Add] Real-time PPP option
Jun 2010 Version 2.1   [Bug] SSR GLONASS message generation
\                      [Add] PPP in post processing mode
\                      [Mod] Update of SSR messages following draft dated 2010-04-12
\                      [Mod] Generating error message when observation epoch is wrong
Jul 2010 Version 2.2   [Bug] GLONASS ephemeris time
Aug 2010 Version 2.3   [Mod] Internal format for saving raw streams
\                      [Bug] Outlier detection in GLONASS ambiguity resolution
\                      [Mod] Format of PPP logs in logfile
\                      [Bug] Complete acceleration terms for GLONASS ephemeris
\                      [Bug] Handling ephemeris IOD's in PPP mode
Dec 2010 Version 2.4   [Add] Output of averaged positions when in PPP mode
\                      [Mod] Use always the latest received set of Broadcast Ephemeris
\                      [Add] QuickStart PPP option
\                      [Mod] Improvement of data sharing efficiency among different threads
\                      [Mod] Design of PPP panel section
\                      [Add] Sigmas for observations and parameters
\                      [Add] Stream distribution map
\                      [Bug] GPS Ephemeris in RINEX v3 format
Feb 2011 Version 2.5   [Add] PPP option for sync of clock observations and corrections
\                      [Add] Drafted RTCM v3 Galileo ephemeris messages 1045
\                      [Add] Drafted RTCM v3 Multiple Signal Messages
\                      [Add] Optional specification of sigmas for coordinates and troposphere in PPP
\                      [Add] Include Galileo in SPP
\                      [Add] Include Galileo observations in output via IP port
\                      [Add] Include Galileo observations in output via RINEX v3 files
\                      [Mod] Interface format for feeding a real-time engine with observations
\                      [Add] Correct observations for Antenna Phase Center offsets
\                      [Add] Combine orbit/clock correction streams
\                      [Add] Specify corrections mountpoint in PPP panel
Apr 2011 Version 2.6   [Add] Complete integration of BNS in BNC
\                      [Add] SP3 and Clock RINEX output
\                      [Add] PPP in post processing Mode
\                      [Add] Some RINEX editing & QC functionality
\                      [Add] Threshold for orbit outliers in combination solution
\                      [Add] Real-time engine becomes orbit/clock server instead of client
\                      [Mod] 'EOE' added to orbit/clock stream from engine
\                      [Add] Correction for antenna eccentricities
\                      [Add] Quick start mode for PPP
\                      [Mod] Design of format for feeding engine changed to follow RINEX v3
\                      [Mod] Implementation of SSR message encoding modified according to standard
\                      [Add] SSL/TLS Support of Ntrip Version 2
\                      [Mod] Switch to Qt version 4.7.3
\                      [Add] RINEX editing, concatenation and quality check
\                      [Add] Reading all configuration options from command line
\                      [Mod] RTCM v3 Galileo Broadcast Ephemeris message 1045
\                      [Mod] Change default configuration file suffix from 'ini' to 'bnc'
\                      [Add] Specific rates for orbits and clocks in streams and SP3/RNX files
\                      [Add] Version 2.6 published, May 2012
Sep 2012 Version 2.7   [Bug] Bug in L5 decoding fixed
\                      [Bug] Bug in on-the-fly configuration fixed
\                      [Add] Clock RINEX file header extended
\                      [Add] Decoding/converting BeiDou and QZSS added
\                      [Add] Work on RINEX v2 and v3 quality check started
\                      [Mod] Source code completely re-arranged
\                      [Add] QWT and QWTPOLAR graphics libraries added
\                      [Add] RINEX QC through multipath analysis sky plot
\                      [Add] RINEX QC through signal-to-noise ratio sky plot
\                      [Add] RINEX QC through satellite availability plot
\                      [Add] RINEX QC through satellite elevation plot
\                      [Add] RINEX QC through PDOP plot
\                      [Bug] Short periodic outages in PPP time series when 'Sync Corr' set to zero
\                      |Add] Log observation types contained in RTCM Version 3 MSM streams
\                      [Add] Reading RINEX v3 observation type header records from RINEX skeleton
\                      [Add] Logfile for RINEX file editing and concatenation
\                      [Add] Save PNG plot files on disk
\                      [Mod] Plot stream distribution map from Ntrip Broadcaster source-table
\                      [Add] Plot stream distribution map from selected sources
\                      [Add] Version 2.7 published
Sep 2012 Version 2.8   [Mod] Started work on new version in Sep 2012
\                      [Bug] Epoch special event flag in RINEX concatenation
\                      [Bug] Limit RINEX v2 records length to 80 characters
\                      [Bug] SSR message update interval indicator
\                      [Bug] Fixed SSR stream encoding and upload
\                      [Add] Concatenate RINEX v3 navigation files containing Galileo ephemeris
\                      [Mod] Plausibility check of GLONASS ephemeris
\                      [Add] Correcting clocks for scale factor involved in transformation
\                      [Mod] Orbit/clock interpolation in SSR stream encoding and upload to caster
\                      [Add] Version 2.8 published, Mar 2013
Mar 2013 Version 2.9   [Add] Started work on new version in Mar 2013
\                      [Bug] SSR stream upload buffering disabled
\                      [Mod] Format for feeding a connected GNSS engine
\                      [Mod] RTNET format for receiving data from a connected GNSS engine
\                      [Add] Include Galileo in SPP
\                      [Add] RINEX QC multipath an SNR sky plots for GLONASS and Galileo
\                      [Add] Bias estimation for GLONASS clocks in PPP
\                      [Add] Trace positions on GM or OSM maps
\                      [Add] Version 2.9 published, Jul 2013
Aug 2013 Version 2.10  [Add] Started work on new version in Aug 2013
\                      [Bug] Clock RINEX und SP3 file generation on Windows systems
\                      [Bug] Broadcast Ephemeris generation
\                      [Add] Transformation ITRF2008 to NAD83 and DREF91
\                      [Add] CodeBias added to RTNET stream format
\                      [Bug] GPS L2 in 'Feed Engine' output
\                      [Mod] Made C1 in BeiDou default observation type instead of C2
\                      [Add] Feed engine output sorted per stream
\                      [Add] Feed engine output filename change on-the-fly
\                      [Add] 'Append files' option for RINEX observation files
\                      [Mod] Broadcast Correction ASCII file output for message 1058 & 1064 modified
\                      [Bug] GPS L2 phase data in RINEX2
\                      [Bug] GLONASS frequency numbers
\                      [Add] RTCM v3 Galileo Broadcast Ephemeris message 1046
\                      [Add] Reset ambiguities in PPP when orbit/clock correction IDs change
\                      [Add] Satellite clock offsets are reset in adjustment for combination when orbit/clock correction IDs change
\                      [Add] Version 2.10 published in Dec 2013
Dec 2013 Version 2.11  [Add] Started work on new version in Dec 2013
\                      [Mod] SIRGAS transformation parameters adjusted
\                      [Mod] ANTEX file updated
\                      [Mod] RTCM SSR messages updated
\                      [Bug] GLONASS code biases
\                      [Mod] Maximum number of GNSS observations increased
\                      [Mod] Loss of lock handling changed
\                      [Add] Raw stream output through TCP/IP port
\                      [Add] Version 2.11.0 published in Sep 2014
Sep 2014 Version 2.12  [Add] Started work on new version in Sep 2014
\                      [Mod] RINEX file concatenation
\                      [Add] Observation code selection in RINEX file editing
\                      [Mod] Routine handling of data input and output in RINEX format re-written
\                      [Mod] QC routines re-written with the goal of handling all signal types
\                      [Add] Machine-readable output of RINEX QC
\                      [Add] PPP client functionality for parallel processing of an arbitrary number of stations in separate threads
\                      [Bug] Receiver antenna PCO in ionosphere-free PPP mode
\                      [Add] NMEA output for any station processed in PPP mode
\                      [Add] PPP processing of any number of linear combinations of GNSS measurements selected by user
\                      [Add] Encoding/Decoding RTCM SSR I messages for Galileo, BDS, SBAS and QZSS
\                      [Add] Encoding/Decoding RTCM SSR phase bias messages
\                      [Add] Encoding/Decoding RTCM SSR ionospheric model messages, single-layer model for total electron content
\                      [Add] RTCM SSR I messages for Galileo, BDS, SBAS and QZSS support from RTNET interface
\                      [Add] RTCM SSR II messages (phase biases and SSR ionospheric model) support from RTNET interface
\                      [Add] Computation of VTEC and STEC from SSR ionospheric model messages for usage in PPP mode
\                      [Add] Handle old-fashioned SNR values in RINEX
\                      [Mod] SNR and MP visualization depending on RINEX observation attribute
\                      [Bug] Saastamoinen tropospheric correction for very high elevation receivers
\                      [Add] Comparison of SP3 files
\                      [Add] Encoding/Decoding of RTCM v3 proposal for Galileo Broadcast Ephemeris message 1046
\                      [Add] Encoding/Decoding of RTCM v3 QZSS Broadcast Ephemeris message 1044
\                      [Add] Encoding/Decoding of RTCM v3 SBAS Broadcast Ephemeris message 1043
\                      [Add] Encoding/Decoding of RTCM v3 BDS Broadcast Ephemeris message 63
\                      [Add] RINEX v3 support of Galileo, BDS, SBAS and QZSS Broadcast Ephemerides
\                      [Add] Consideration of the aspect that Galileo NAV message can be provided for the same epoch but with different flags (I/NAV, F/NAV, DVS)
\                      [Bug] VRS support in sending NMEA in Auto/Manual mode to Ntrip Broadcaster
\                      [Add] Forwarding NMEA GNGGA to Ntrip Broadcaster
\                      [Bug] Stream failure/recovery reports
\                      [Add] Compute IODs for BDS and SBAS from CRC over Broadcast Ephemeris and clock parameters
\                      [Mod] PPP default options
\                      [Add] Example configuration for SP3 file comparison
\                      [Add] Choose between code and phase observations when in PPP SSR I mode
\                      [Bug] Reset time series plot when restarting PPP in post processing mode
\                      [Add] Broadcast ephemeris check regarding allowed age of data sets
\                      [Add] Code bias usage for PPP SSR I mode
\                      [Add] Code bias, phase bias and VTEC usage in extended PPP mode
\                      [Mod] Consideration of the full antenna PCO vector in all PPP modes
\                      [Add] Allow GPS-only and GLONASS-only RINEX v2 Navigation files
\                      [Mod] SSR clock correction converted to seconds to be consistent with broadcast values
\                      [Add] Support Galileo I/NAV Broadcast Ephemeris
\                      [Add] Extended RINEX v3 filenames
\                      [Add] Stream's country added to configuration string 'mountPoints'
\                      [Add] Distinction of GEO/MEO satellites during BDS velocity determination 
\                      [Bug] Velocity determination for geostationary BDS satellites
\                      [Add] Set TOE from BDS week and second
\                      [Add] Use BDS observations and ephemerides in PPP SSR I mode
\                      [Add] Considering that yaw angle restricted to -180 to +180 deg
\                      [Mod] Read local RINEX skeleton files
\                      [Add] Update interval for VTEC in RTNET stream format
\                      [Bug] SBAS IODN
\                      [Bug] Galileo week number
\                      [Add] Phase shift records in RINEX v3 headers
\                      [Add] Output GLONASS slot numbers from scanning stream content
\                      [Add] Decoder interface for PPP SSR I+II messages for Galileo/QZSS/SBAS/BDS
\                      [Mod] Renaming BDS first frequency from '1' to '2'
\                      [Add] RINEX QC, receiver/antenna information editable
\                      [Add] Support of new RINEX header lines regarding phase shifts, GLONAQSS slots and GLONASS biases during file merging
\                      [Add] Switch to port 443 for skeleton file download from https website
\                      [Mod] Default observation types for RINEX v3 files
\                      [Bug] RTCM v2 decoder
\                      [Add] SINEX Troposphere file output
\                      [Add] Comments with respect to RINEX v3 to RINEX v2 observation file conversion [Add] String for Operating System in logfile output
\                      [Add] Full integration of 'rtcm3torinex'
\                      [Add] Extended command line help
\                      [Add] Version 2.12.0 published in April 2016
Apr 2016 Version 2.13  [Add] Started work on new version in Apr 2016
======== ============= =============================================================

.. index:: RTCM standards overview


RTCM Standards
==============

The Radio Technical Commission for Maritime Services (RTCM) is an international non-profit scientific, professional and educational organization. Special Committees provide a forum in which governmental and non-governmental members work together to develop technical standards and consensus recommendations in regard to issues of particular concern. RTCM is engaged in the development of international standards for maritime radionavigation and radiocommunication systems. The output documents and reports prepared by RTCM Committees are published as RTCM Recommended Standards. Topics concerning Differential Global Navigation Satellite Systems (DGNSS) are handled by the Special Committee SC 104. 

Personal copies of RTCM Recommended Standards can be ordered through http://www.rtcm.org/orderinfo.php.

.. index:: Ntrip Version 1
 
Ntrip Version 1
---------------

'Networked Transport of RTCM via Internet Protocol' Version 1.0 (Ntrip) stands for an application-level protocol streaming Global Navigation Satellite System (GNSS) data over the Internet. Ntrip is a generic, stateless protocol based on the Hypertext Transfer Protocol HTTP/1.1. The HTTP objects are enhanced to GNSS data streams. 

Ntrip Version 1 is an RTCM standard designed for disseminating differential correction data (e.g. in the RTCM-104 format) or other kinds of GNSS streaming data to stationary or mobile users over the Internet, allowing simultaneous PC, Laptop, PDA, or receiver connections to a broadcasting host. Ntrip supports wireless Internet access through Mobile IP Networks like GSM, GPRS, EDGE, or UMTS. 

Ntrip is implemented in three system software components: Ntrip Clients, Ntrip Servers and Ntrip Broadcasters. The Ntrip Broadcaster is the actual HTTP server program whereas Ntrip Client and Ntrip Server are acting as HTTP clients. 


Ntrip is an open none-proprietary protocol. Major characteristics of Ntrip's dissemination technique are:

* Based on the popular HTTP streaming standard; comparatively easy to implement when having limited client and server platform resources available;
* Application not limited to one particular plain or coded stream content; ability to distribute any kind of GNSS data;
* Potential to support mass usage; disseminating hundreds of streams simultaneously for thousands of users possible when applying modified Internet Radio broadcasting software;
* Considering security needs; stream providers and users do not necessarily get into contact, streams often not blocked by firewalls or proxy servers protecting Local Area Networks;
* Enables streaming over mobile IP networks because of using TCP/IP.

The Ntrip Broadcaster maintains a source-table containing information on available Ntrip streams, networks of Ntrip streams and Ntrip Broadcasters. The source-table is sent to an Ntrip Client on request. Source-table records are dedicated to one of the following: Data Streams (record type STR), Casters (record type CAS), or Networks of streams (record type NET). 

Source-table records of type STR contain the following data fields: 'mountpoint', 'identifier', 'format', 'format-details', 'carrier', 'nav-system', 'network', 'country', 'latitude', 'longitude', 'nmea', 'solution', 'generator', 'compr-encryp', 'authentication', 'fee', 'bitrate', 'misc'. 

Source-table records of type NET contain the following data fields: 'identifier', 'operator', 'authentication', 'fee', 'web-net', 'web-str', 'web-reg', 'misc'. 

Source-table records of type CAS contain the following data fields: 'host', 'port', 'identifier', 'operator', 'nmea', 'country', 'latitude', 'longitude', 'misc'. 

.. index:: Ntrip Version 2
 
Ntrip Version 2
---------------

The major changes of Ntrip Version 2 compared to Version 1.0 are: 

* Cleared and fixed design problems and HTTP protocol violations;
* Replaced nonstandard directives;
* Chunked transfer encoding;
* Improvements in header records;
* Source-table filtering;
* RTSP communication.

Ntrip Version 2 allows to communicate either in TCP/IP mode or in RTSP/RTP mode or in UDP mode whereas Version 1 is limited to TCP/IP only. It furthermore allows using the Transport Layer Security (TLS) and its predecessor, Secure Sockets Layer (SSL) cryptographic protocols for secure Ntrip communication over the Internet. 

.. index:: RTCM Version 2

RTCM Version 2
--------------

Transmitting GNSS carrier phase data can be done through RTCM Version 2 messages. Please note that only RTCM Version 2.2 and 2.3 streams may include GLONASS data. Messages that may be of interest here are: 

* Type 1 message is the range correction message and is the primary message in code-phase differential positioning (DGPS). It is computed in the base receiver by computing the error in the range measurement for each tracked SV. 
*  Type 2 message is automatically generated when a new set of satellite ephemeris is downloaded to the base receiver. It is the computed difference between the old ephemeris and the new ephemeris. Type 2 messages are used when the base station is transmitting Type 1 messages. 
*  Type 3 and 22 messages are the base station position and the antenna offset. Type 3 and 22 are used in RTK processing to perform antenna reduction. 
*  Type 6 message is a null frame filler message that is provided for data links that require continuous transmission of data, even if there are no corrections to send. As many Type 6 messages are sent as required to fill in the gap between two correction messages (type 1). Message 6 is not sent in burst mode. 
*  Type 9 message serves the same purpose as Type 1, but does not require a complete satellite set. As a result, Type 9 messages require a more stable clock than a station transmitting Type 1 's, because the satellite corrections have different time references. 
*  Type 16 message is simply a text message entered by the user that is transmitted from the base station to the rover. It is used with code-phase differential. 
*  Type 18 and 20 messages are RTK uncorrected carrier phase data and carrier phase corrections. 
*  Type 19 and 21 messages are the uncorrected pseudo-range measurements and pseudo-range corrections used in RTK. 
*  Type 23 message provides the information on the antenna type used on the reference station. 
*  Type 24 message carries the coordinates of the installed antenna's ARP in the GNSS coordinate system coordinates. 

.. index:: RTCM Version 3

RTCM Version 3
--------------

RTCM Version 3 has been developed as a more efficient alternative to RTCM Version 2. Service providers and vendors have asked for a standard that would be more efficient, easy to use, and more easily adaptable to new situations. The main complaint was that the Version 2 parity scheme was wasteful of bandwidth. Another complaint was that the parity is not independent from word to word. Still another was that even with so many bits devoted to parity, the actual integrity of the message was not as high as it should be. Plus, 30-bit words are awkward to handle. The Version 3 standard is intended to correct these weaknesses. 

RTCM Version 3 defines a number of message types. Messages that may be of interest here are:

* Type 1001, GPS L1 code and phase.
* Type 1002, GPS L1 code and phase and ambiguities and carrier-to-noise ratio.
* Type 1003, GPS L1 and L2 code and phase.
* Type 1004, GPS L1 and L2 code and phase and ambiguities and carrier-to-noise ratio.
* Type 1005, Station coordinates XYZ for antenna reference point.
* Type 1006, Station coordinates XYZ for antenna reference point and antenna height.
* Type 1007, Antenna descriptor and ID.
* Type 1008, Antenna serial number.
* Type 1009, GLONASS L1 code and phase.
* Type 1010, GLONASS L1 code and phase and ambiguities and carrier-to-noise ratio.
* Type 1011, GLONASS L1 and L2 code and phase.
* Type 1012, GLONASS L1 and L2 code and phase and ambiguities and carrier-to-noise ratio.
* Type 1013, Modified Julian Date, leap second, configured message types and interval.
* Type 1014 and 1017, Network RTK (MAK) messages.
* Type 1019, GPS ephemeris.
* Type 1020, GLONASS ephemeris.
* Type 1043, SBAS ephemeris.
* Type 1044, QZSS ephemeris.
* Type 1045, Galileo F/NAV ephemeris.
* Type 1046, Galileo I/NAV ephemeris.
* Type 63, BeiDou ephemeris, tentative.
* Type 4088 and 4095, Proprietary messages. 

The following are so-called 'State Space Representation' (SSR) messages:

* Type 1057, GPS orbit corrections to Broadcast Ephemeris
* Type 1058, GPS clock corrections to Broadcast Ephemeris
* Type 1059, GPS code biases
* Type 1060, Combined orbit and clock corrections to GPS Broadcast Ephemeris
* Type 1061, GPS User Range Accuracy (URA)
* Type 1062, High-rate GPS clock corrections to Broadcast Ephemeris
* Type 1063, GLONASS orbit corrections to Broadcast Ephemeris
* Type 1064, GLONASS clock corrections to Broadcast Ephemeris
* Type 1065, GLONASS code biases
* Type 1066, Combined orbit and clock corrections to GLONASS Broadcast Ephemeris
* Type 1067, GLONASS User Range Accuracy (URA)
* Type 1068, High-rate GLONASS clock corrections to Broadcast Ephemeris
 
* Type 1240, Galileo orbit corrections to Broadcast Ephemeris
* Type 1241, Galileo clock corrections to Broadcast Ephemeris
* Type 1242, Galileo code biases
* Type 1243, Combined orbit and clock corrections to Galileo Broadcast Ephemeris
* Type 1244, Galileo User Range Accuracy (URA)
* Type 1245, High-rate Galileo clock corrections to Broadcast Ephemeris

* Type 1246, QZSS orbit corrections to Broadcast Ephemeris
* Type 1247, QZSS clock corrections to Broadcast Ephemeris
* Type 1248, QZSS code biases
* Type 1249, Combined orbit and clock corrections to QZSS Broadcast Ephemeris
* Type 1250, QZSS User Range Accuracy (URA)
* Type 1251, High-rate QZSS clock corrections to Broadcast Ephemeris

* Type 1252, SBAS orbit corrections to Broadcast Ephemeris
* Type 1253, SBAS clock corrections to Broadcast Ephemeris
* Type 1254, SBAS code biases
* Type 1255, Combined orbit and clock corrections to SBAS Broadcast Ephemeris
* Type 1256, SBAS User Range Accuracy (URA)
* Type 1257, High-rate SBAS clock corrections to Broadcast Ephemeris

* Type 1258, BDS orbit corrections to Broadcast Ephemeris
* Type 1259, BDS clock corrections to Broadcast Ephemeris
* Type 1260, BDS code biases
* Type 1261, Combined orbit and clock corrections to BDS Broadcast Ephemeris
* Type 1262, BDS User Range Accuracy (URA)
* Type 1263, High-rate BDS clock corrections to Broadcast Ephemeris\\ 

* Type 1264 SSR Ionosphere VTEC Spherical Harmonics
* Type 1265 SSR GPS Satellite Phase Bias
* Type 1266 SSR Satellite GLONASS Phase Bias
* Type 1267 SSR Satellite Galileo Phase Bias
* Type 1268 SSR Satellite QZSS Phase Bias
* Type 1269 SSR Satellite SBAS Phase Bias
* Type 1270 SSR Satellite BDS Phase Bias

The following are so-called 'Multiple Signal Messages' (MSM):

* Type 1071, Compact GPS pseudo-ranges
* Type 1072, Compact GPS carrier phases
* Type 1073, Compact GPS pseudo-ranges and carrier phases
* Type 1074, Full GPS pseudo-ranges and carrier phases plus signal strength
* Type 1075, Full GPS pseudo-ranges, carrier phases, Doppler and signal strength
* Type 1076, Full GPS pseudo-ranges and carrier phases plus signal strength (high resolution)
* Type 1077, Full GPS pseudo-ranges, carrier phases, Doppler and signal strength (high resolution)

* Type 1081, Compact GLONASS pseudo-ranges
* Type 1082, Compact GLONASS carrier phases
* Type 1083, Compact GLONASS pseudo-ranges and carrier phases
* Type 1084, Full GLONASS pseudo-ranges and carrier phases plus signal strength
* Type 1085, Full GLONASS pseudo-ranges, carrier phases, Doppler and signal strength
* Type 1086, Full GLONASS pseudo-ranges and carrier phases plus signal strength (high resolution)
* Type 1087, Full GLONASS pseudo-ranges, carrier phases, Doppler and signal strength (high resolution)

* Type 1091, Compact Galileo pseudo-ranges
* Type 1092, Compact Galileo carrier phases
* Type 1093, Compact Galileo pseudo-ranges and carrier phases
* Type 1094, Full Galileo pseudo-ranges and carrier phases plus signal strength
* Type 1095, Full Galileo pseudo-ranges, carrier phases, Doppler and signal strength
* Type 1096, Full Galileo pseudo-ranges and carrier phases plus signal strength (high resolution)
* Type 1097, Full Galileo pseudo-ranges, carrier phases, Doppler and signal strength (high resolution)
 
* Type 1121, Compact BeiDou pseudo-ranges
* Type 1122, Compact BeiDou carrier phases
* Type 1123, Compact BeiDou pseudo-ranges and carrier phases
* Type 1124, Full BeiDou pseudo-ranges and carrier phases plus signal strength
* Type 1125, Full BeiDou pseudo-ranges, carrier phases, Doppler and signal strength
* Type 1126, Full BeiDou pseudo-ranges and carrier phases plus signal strength (high resolution)
* Type 1127, Full BeiDou pseudo-ranges, carrier phases, Doppler and signal strength (high resolution)

* Type 1111, Compact QZSS pseudo-ranges
* Type 1112, Compact QZSS carrier phases
* Type 1113, Compact QZSS pseudo-ranges and carrier phases
* Type 1114, Full QZSS pseudo-ranges and carrier phases plus signal strength
* Type 1115, Full QZSS pseudo-ranges, carrier phases, Doppler and signal strength
* Type 1116, Full QZSS pseudo-ranges and carrier phases plus signal strength (high resolution)
* Type 1117, Full QZSS pseudo-ranges, carrier phases, Doppler and signal strength (high resolution)

The following are proposed 'Multiple Signal Messages' (MSM) under discussion for standardization:

* Type 1101, Compact SBAS pseudo-ranges
* Type 1102, Compact SBAS carrier phases
* Type 1103, Compact SBAS pseudo-ranges and carrier phases
* Type 1104, Full SBAS pseudo-ranges and carrier phases plus signal strength
* Type 1105, Full SBAS pseudo-ranges, carrier phases, Doppler and signal strength
* Type 1106, Full SBAS pseudo-ranges and carrier phases plus signal strength (high resolution)
* Type 1107, Full SBAS pseudo-ranges, carrier phases, Doppler and signal strength (high resolution)\\ 

.. index:: Command Line Help

Command Line Help
=================

Command line option ``--help`` provides a complete list of all configuration parameters which can be specified via BNC's Command Line Interface (CLI). Note that command line options overrule configuration options specified in the configuration file. The following is the output produced when running BNC with command line option '--help': 

Usage
-----

.. code-block:: bash

   bnc --help (MS Windows: bnc.exe --help | more)
       --nw
       --version (MS Windows: bnc.exe --version | more)
       --display {name}
       --conf {confFileName}
       --file {rawFileName}
       --key  {keyName} {keyValue}

.. index:: Command Line Help - Network Panel keys
	   
Network Panel keys
------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}|

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
proxyHost                Proxy host, name or IP address [character string]
proxyPort                Proxy port [integer number]
sslCaCertPath            Full path to SSL certificates [character string]
sslIgnoreErrors          Ignore SSL authorization errors [integer number: 0=no,2=yes]
======================== ================================================================================================================

.. index:: Command Line Help - General Panel keys

General Panel keys
------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}|

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
logFile                  Logfile, full path [character string]
rnxAppend                Append files [integer number: 0=no,2=yes]
onTheFlyInterval         Configuration reload interval [character string: 1 day|1 hour|5 min|1 min]
autoStart                Auto start [integer number: 0=no,2=yes]
rawOutFile               Raw output file, full path [character string]
======================== ================================================================================================================

.. index:: Command Line Help - RINEX Observations Panel keys

RINEX Observations Panel keys
-----------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}|

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
rnxPath                  Directory [character string]
rnxIntr                  File interval [character string: 1 min|2 min|5 min|10 min|15 min|30 min|1 hour|1 day]
rnxSampl                 File sampling rate [integer number of seconds: 0,5|10|15|20|25|30|35|40|45|50|55|60]
rnxSkel                  RINEX skeleton file extension [character string]
rnxOnlyWithSKL           Using RINEX skeleton file is mandatory [integer number: 0=no,2=yes]
rnxScript                File upload script, full path [character string]
rnxV2Priority            Priority of signal attributes [character string, list separated by blank character, example: |G:CWPX_?| R:CP]
rnxV3                    Produce version 3 file content [integer number: 0=no,2=yes]
rnxV3filenames           Produce version 3 filenames [integer number: 0=no,2=yes]
======================== ================================================================================================================

.. index:: Command Line Help - Ephemeris Panel keys

RINEX Ephemeris Panel keys
--------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}|

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
ephPath                  Directory [character string]
ephIntr                  File interval [character string: 1 min|2 min|5 min|10 min|15 min|30 min|1 hour|1 day]
ephOutPort               Output port [integer number]
ephV3                    Produce version 3 file content [integer number: 0=no,2=yes]
ephV3filenames           Produce version 3 filenames [integer number: 0=no,2=yes]
======================== ================================================================================================================

.. index:: Command Line Help - RINEX Editing and QC Panel keys

RINEX Editing and QC Panel keys
-------------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}|

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
reqcAction               Action specification [character string:  Blank|Edit/Concatenate|Analyze]
reqcObsFile              Input observations file(s), full path [character string, comma separated list in quotation marks]
reqcNavFile              Input navigation file(s), full path [character string, comma separated list in quotation marks]
reqcOutObsFile           Output observations file, full path [character string]
reqcOutNavFile           Output navigation file, full path [character string]
reqcOutLogFile           Output logfile, full path [character string]
reqcLogSummaryOnly       Output only summary of logfile [integer number: 0=no,2=yes]
reqcSkyPlotSignals       Observation signals [character string, list separated by blank character, example: C:2&7 E:1&5 G:1&2 J:1&2 R:1&2 S:1&5]
reqcPlotDir              QC plots directory [character string]
reqcRnxVersion           RINEX version [integer number: 2|3]
reqcSampling             RINEX output file sampling rate [integer number of seconds: 0|5|10|15|20|25|30|35|40|45|50|55|60]
reqcV2Priority           Version 2 priority of signal attributes [character string, list separated by blank character, example: |G:CWPX_?| R:CP]
reqcStartDateTime        Start time [character string, example: 1967-11-02T00:00:00]
reqcEndDateTime          Stop time [character string, example: 2099-01-01T00:00:00]
reqcRunBy                Operators name [character string]
eqcUseObsTypes           Use observation types [character string, list separated by blank character, example: G:C1C G:L1C R:C1C RC1P]
reqcComment              Additional comments [character string]
reqcOldMarkerName        Old marker name [character string]
reqcNewMarkerName        New marker name [character string]
reqcOldAntennaName       Old antenna name [character string]
reqcNewAntennaName       New antenna name [character string]
reqcOldAntennaNumber     Old antenna number [character string]
reqcNewAntennaNumber     New antenna number [character string]
reqcOldAntennadN         Old north eccentricity [character string]
reqcNewAntennadN         New north eccentricity [character string]
reqcOldAntennadE         Old east eccentricity [character string]
reqcNewAntennadE         New east eccentricity [character string]
reqcOldAntennadU         Old up eccentricity [character string]
reqcNewAntennadU         New up eccentricity [character string]
reqcOldReceiverName      Old receiver name [character string]
reqcNewReceiverName      New receiver name [character string]
reqcOldReceiverNumber    Old receiver number [character string]
reqcNewReceiverNumber    New receiver number [character string]
======================== ================================================================================================================

.. index:: Command Line Help - SP3 Comparison Panel keys

SP3 Comparison Panel keys
-------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
sp3CompFile              SP3 input files, full path [character string, comma separated list in quotation marks]
sp3CompExclude           Satellite exclusion list [character string, comma separated list in quotation marks, example: G04,G31,R]
sp3CompOutLogFile        Output logfile, full path [character string]
======================== ================================================================================================================

.. index:: Command Line Help - Broadcast Corrections Panel keys

Broadcast Corrections Panel keys
--------------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
corrPath                 Directory for saving files in ASCII format [character string]
corrIntr                 File interval [character string: 1 min|2 min|5 min|10 min|15 min|30 min|1 hour|1 day]
corrPort                 Output port [integer number]
======================== ================================================================================================================

.. index:: Command Line Help - Feed Engine Panel keys

Feed Engine Panel keys
----------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
outPort                  Output port, synchronized [integer number]
outWait                  Wait for full observation epoch [integer number of seconds: 1-30]
outSampl                 Sampling rate [integer number of seconds: 0|5|10|15|20|25|30|35|40|45|50|55|60]
outFile                  Output file, full path [character string]
outUPort                 Output port, unsynchronized [integer number]
======================== ================================================================================================================

.. index:: Command Line Help - Serial Output Panel keys

Serial Output Panel keys
------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
serialMountPoint         Mountpoint [character string]
serialPortName           Port name [character string]
serialBaudRate           Baud rate [integer number: 110|300|600|1200|2400|4800|9600|19200|38400|57600|115200]
serialFlowControl        Flow control [character string: OFF|XONXOFF|HARDWARE
serialDataBits           Data bits [integer number: 5|6|7|8]
serialParity             Parity [character string: NONE|ODD|EVEN|SPACE]
serialStopBits           Stop bits [integer number: 1|2]
serialAutoNMEA           NMEA specification [character string: no|Auto|Manual GPGGA|Manual GNGGA]
serialFileNMEA           NMEA filename, full path [character string]
serialHeightNMEA         Height [floating-point number]
serialHeightNMEASampling Sampling rate [integer number of seconds: 0|10|20|30|...|280|290|300]
======================== ================================================================================================================

.. index:: Command Line Help - Outages Panel keys

Outages Panel keys
------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
adviseObsRate            Stream observation rate [character string: 0.1 Hz|0.2 Hz|0.5 Hz|1 Hz|5 Hz]
adviseFail               Failure threshold [integer number of minutes: 0-60]
adviseReco               Recovery threshold [integer number of minutes: 0-60]
adviseScript             Advisory script, full path [character string]
======================== ================================================================================================================

.. index:: Command Line Help - Miscellaneous Panel keys

Miscellaneous Panel keys
------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
miscMount                Mountpoint [character string]
miscIntr                 Interval for logging latency [character string: Blank|2 sec|10 sec|1 min|5 min|15 min|1 hour|6 hours|1 day]
miscScanRTCM             Scan for RTCM message numbers [integer number: 0=no,2=yes]
miscPort                 Output port [integer number]
======================== ================================================================================================================

.. index:: Command Line Help - PPP Client Panel 1 keys

PPP Client Panel 1 keys
-----------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
PPP/dataSource           Data source [character string: Blank|Real-Time Streams|RINEX Files]
PPP/rinexObs             RINEX observation file, full path [character string]
PPP/rinexNav             RINEX navigation file, full path [character string]
PPP/corrMount            Corrections mountpoint [character string]
PPP/corrFile             Corrections file, full path [character string]
PPP/antexFile            ANTEX file, full path [character string]
PPP/crdFile              Coordinates file, full path [character string]
PPP/v3filenames          Produce version 3 filenames, [integer number: 0=no,2=yes]
PPP/logPath              Directory for PPP log files [character string]
PPP/nmeaPath             Directory for NMEA output files [character string]
PPP/snxtroPath           Directory for SINEX troposphere output files [character string]
PPP/snxtroIntr           SINEX troposphere file interval [character string: 1 min|2 min|5 min|10 min|15 min|30 min|1 hour|1 day]
PPP/snxtroSampl          SINEX troposphere file sampling rate [integer number of seconds: 0|30|60|90|120|150|180|210|240|270|300]
PPP/snxtroAc             SINEX troposphere Analysis Center [character string]
PPP/snxtroSol            SINEX troposphere solution ID [character string]
======================== ================================================================================================================

.. index:: Command Line Help - PPP Client Panel 2 keys

PPP Client Panel 2 keys
-----------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
PPP/staTable             Station specifications table [character string, semicolon separated list, each element in quotation marks, example:"FFMJ1,100.0,100.0,100.0,100.0,100.0,100.0,0.1,3e-6,7777;CUT07,100.0,100.0,100.0,100.0,100.0,100.0,0.1,3e-6,7778"]
======================== ================================================================================================================

.. index:: Command Line Help - PPP Client Panel 3 keys

PPP Client Panel 3 keys
-----------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
PPP/lcGPS                Select linear combination from GPS code or phase data [character string; P3|P3&L3]
PPP/lcGLONASS            Select linear combination from GLONASS code or phase data [character string: no|P3|L3|P3&L3]
PPP/lcGalileo            elect linear combination from Galileo code or phase data [character string: no|P3|L3|P3&L3]
PPP/lcBDS                Select linear combination from BDS code or phase data [character string: no|P3|L3|P3&L3]
PPP/sigmaC1              Sigma for code observations in meters [floating-point number]
PPP/sigmaL1              Sigma for phase observations in meters [floating-point number]
PPP/maxResC1             Maximal residuum for code observations in meters [floating-point number]
PPP/maxResL1             Maximal residuum for phase observations in meters [floating-point number]
PPP/eleWgtCode           Elevation dependent waiting of code observations [integer number: 0=no,2=yes]
PPP/eleWgtPhase          Elevation dependent waiting of phase observations [integer number: 0=no,2=yes]
PPP/minObs               Minimum number of observations [integer number: 4|5|6]
PPP/minEle               Minimum satellite elevation in degrees [integer number: 0-20]
PPP/corrWaitTime         Wait for clock corrections [integer number of seconds: no|1-20]
PPP/seedingTime          Seeding time span for Quick Start [integer number of seconds]
======================== ================================================================================================================

.. index:: Command Line Help - PPP Client Panel 4 keys

PPP Client Panel 4 keys
-----------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
PPP/plotCoordinates      Mountpoint for time series plot [character string]
PPP/audioResponse        Audio response threshold in meters [floating-point number]
PPP/useOpenStreetMap     OSM track map [character string: true|false]
PPP/useGoogleMap         Google track map [character string: true|false]
PPP/mapWinDotSize        Size of dots on map [integer number: 0-10]
PPP/mapWinDotColor       Color of dots and cross hair on map [character string: red|yellow]
PPP/mapSpeedSlider       Offline processing speed for mapping [integer number: 1-100]
======================== ================================================================================================================

.. index:: Command Line Help - Combine Corrections Panel keys

Combine Corrections Panel keys
------------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
cmbStreams               Correction streams table [character string, semicolon separated list, each element in quotation marks, example:"IGS01 ESA 1.0;IGS03 BKG 1.0"]
cmbMethodFilter          Combination approach [character string: Single-Epoch|Filter]
cmbMaxres                Clock outlier residuum threshold in meters [floating-point number]
cmbSampl                 Clock sampling rate [integer number of seconds: 10|20|30|40|50|60]
cmbUseGlonass            Use GLONASS in combination [integer number: 0=no,2=yes]
======================== ================================================================================================================

.. index:: Command Line Help - Upload Corrections Panel keys

Upload Corrections Panel keys
-----------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
uploadMountpointsOut     Upload corrections table [character string, semicolon separated list, each element in quotation marks, example: "www.igs-ip.net,2101,IGS01,pass,IGS08,0, /home/user/BNC$[GPSWD}.sp3, /home/user/BNC$[GPSWD}.clk,258,1,0; www.euref-ip.net,2101,EUREF01,pass,ETRF2000,0,,,258,2,0"]
uploadIntr               Length of SP3 and Clock RINEX file interval [character string: 1 min|2 min|5 min|10 min|15 min|30 min|1 hour|1 day]
uploadSamplRtcmEphCorr   Orbit corrections stream sampling rate [integer number of seconds: 0|5|10|15|20|25|30|35|40|45|50|55|60]
uploadSamplSp3           SP3 file sampling rate [integer number of minutes: 0-15]
uploadSamplClkRnx        Clock RINEX file sampling rate [integer number of seconds: 0|5|10|15|20|25|30|35|40|45|50|55|60]
======================== ================================================================================================================

.. index:: Command Line Help - Custom Trafo keys

Custom Trafo keys
-----------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
trafo_dx                 Translation X in meters [floating-point number]
trafo_dy                 Translation Y in meters [floating-point number]
trafo_dz                 Translation Z in meters [floating-point number]
trafo_dxr                Translation change X in meters per year [floating-point number]
trafo_dyr                Translation change Y in meters per year [floating-point number]
trafo_dzr                Translation change Z in meters per year [floating-point number]
trafo_ox                 Rotation X in arcsec [floating-point number]
trafo_oy                 Rotation Y in arcsec [floating-point number]
trafo_oz                 Rotation Z in arcsec [floating-point number]
trafo_oxr                Rotation change X in arcsec per year [floating-point number]
trafo_oyr                Rotation change Y in arcsec per year [floating-point number]
trafo_ozr                Rotation change Z in arcsec per year [floating-point number]
trafo_sc                 Scale [10^-9, floating-point number]
trafo_scr                Scale change [10^-9 per year, floating-point number]
trafo_t0                 Reference year [integer number]
======================== ================================================================================================================

.. index:: Command Line Help - Upload Ephemeris Panel keys

Upload Ephemeris Panel keys
---------------------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
uploadEphHost            Broadcaster host, name or IP address [character string]
uploadEphPort            Broadcaster port [integer number]
uploadEphMountpoint      Mountpoint [character string]
uploadEphPassword        Stream upload password [character string]
uploadEphSample          Stream upload sampling rate [integer number of seconds: 5|10|15|20|25|30|35|40|45|50|55|60]
======================== ================================================================================================================

.. index:: Command Line Help - Add Stream keys

Add Stream keys
---------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
mountPoints              Mountpoints [character string, semicolon separated list, example:
|                        "//user:pass@www.igs-ip.net:2101/FFMJ1 RTCM_3.1 DEU 50.09 8.66 no 2;
|                        //user:pass@www.igs-ip.net:2101/FFMJ2 RTCM_3.1 DEU 50.09 8.66 no 2"
ntripVersion             Ntrip Version [character string: 1|2|2s|R|U]
casterUrlList            Visited Broadcasters [character string, comma separated list]
======================== ================================================================================================================

.. index:: Command Line Help - Appearance keys
   
Appearance keys
---------------

.. tabularcolumns:: |p{0.3\linewidth}|p{0.62\linewidth}| 

======================== ================================================================================================================
**KeyName**              **Meaning**
======================== ================================================================================================================
startTab                 Index of top panel to be presented at start time [integer number: 0-17]
statusTab                Index of bottom panel to be presented at start time [integer number: 0-3]
font                     Font specification [character string in quotation marks, example: "Helvetica,14,-1,5,50,0,0,0,0,0"]
======================== ================================================================================================================

Example command lines
---------------------

The syntax of some command line configuration options slightly differs from that 
used in configuration files: Configuration file options which contain one or more
blank characters or contain a semicolon separated parameter list must be enclosed
by quotation marks when specified on command line.

.. index: Command lines - Examples
.. index: Example command lines

1. ``/home/weber/bin/bnc``
2. ``/Applications/bnc.app/Contents/MacOS/bnc``
3. ``/home/weber/bin/bnc --conf /home/weber/MyConfigFile.bnc``
4. ``bnc --conf /Users/weber/.config/BKG/BNC.bnc -nw``
5. ``bnc --conf /dev/null --key startTab 4 --key reqcAction Edit/Concatenate --key reqcObsFile AGAR.15O --key reqcOutObsFile AGAR_X.15O --key reqcRnxVersion 2 --key reqcSampling 30 --key reqcV2Priority CWPX_?``
6. ``bnc --key mountPoints "//user:pass@mgex.igs-ip.net:2101/CUT07 RTCM_3.0 ETH 9.03 38.74 no 2;//user:pass@www.igs-ip.net:2101/FFMJ1 RTCM_3.1 DEU 50.09 8.66 no 2"``
7. ``bnc --key cmbStreams "CLK11 BLG 1.0;CLK93 CNES 1.0"``
8. ``bnc --key uploadMountpointsOut "products.igs-ip.net,98756,TEST, letmein,IGS08,2,/Users/weber/BNC${GPSWD}.clk,,33,3,2; www.euref-ip.net,333,TEST2,aaaaa,NAD83,2,,,33,5,5"``
9. ``bnc --key PPP/staTable "FFMJ1,100.0,100.0,100.0,100.0,100.0,100.0, 0.1,3e-6,7777;CUT07,100.0,100.0,100.0,100.0,100.0,100.0,0.1,3e-6, 7778"``

.. index:: Further Reading							
							
Further Reading
===============

.. tabularcolumns:: |p{0.46\textwidth}|p{0.46\textwidth}| 

+---------------------------------------+-----------------------------------+
|Ntrip                                  |http://igs.bkg.bund.de/ntrip/index |
+---------------------------------------+-----------------------------------+
|EUREF-IP Ntrip Broadcaster             |http://www.euref-ip.net/home       |
+---------------------------------------+-----------------------------------+
|IGS-IP Ntrip Broadcaster               |http://www.igs-ip.net/home         |
+---------------------------------------+-----------------------------------+
|IGS products Ntrip Broadcaster         |http://products.igs-ip.net/home    |
+---------------------------------------+-----------------------------------+
|IGS M-GEX Ntrip Broadcaster            |http://mgex.igs-ip.net/home        |
+---------------------------------------+-----------------------------------+
|IGS Central Bureau Ntrip Broadcaster   |http://rt.igs.org                  |
+---------------------------------------+-----------------------------------+
|IGS Real-time Service                  |http://rts.igs.org                 |
+---------------------------------------+-----------------------------------+
|Distribution of IGS-IP streams         |http://www.igs.oma.be/real_time    |
+---------------------------------------+-----------------------------------+
|Completeness and latency of IGS-IP data|http://www.igs.oma.be/highrate/    |
+---------------------------------------+-----------------------------------+
|Ntrip Broadcaster overview             |http://www.rtcm-ntrip.org/home     |
+---------------------------------------+-----------------------------------+
|Ntrip Open Source software code        |http://software.rtcm-ntrip.org     |
+---------------------------------------+-----------------------------------+
|EUREF-IP Project                       |http://www.epncb.oma.be/euref_IP   |
+---------------------------------------+-----------------------------------+
|Radio Technical Commission for Maritime|http://www.rtcm.org                | 
|Services                               |                                   |
+---------------------------------------+-----------------------------------+

