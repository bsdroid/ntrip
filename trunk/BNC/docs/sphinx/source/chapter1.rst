Introduction
************
Purpose
=======
The BKG Ntrip Client (BNC) is a program for simultaneously retrieving, decoding, converting and processing or analyzing real-time GNSS data streams applying the 'Networked Transport of RTCM via Internet Protocol' (Ntrip) standard. It has been developed within the framework of the IAG sub-commission for Europe (EUREF) and the International GNSS Service (IGS). Although meant to be a real-time tool, it comes with some post processing functionality. It can be used for data coming from Ntrip Broadcasters like 

* http://www.euref-ip.net/home
* http://www.igs-ip.net/home
* http://products.igs-ip.net/home
* http://mgex.igs-ip.net/home

or similar caster installation. 

BNC has been written under GNU General Public License (GPL). Source code is available from Subversion software archive http://software.rtcm-ntrip.org/svn/trunk/BNC. Precompiled binaries of BNC are available for MS Windows, Linux, and Mac OS X systems. They can be downloaded from http://igs.bkg.bund.de/ntrip/download.

Promoting Open RTCM Standards for streaming GNSS data over the Internet has been a major aspect in developing BNC as Open Source real-time software. Basically, the tool enables the test, validation and further evolution of new RTCM messages for precise satellite navigation. With high-level source code at hand, it also allows university education to catch up with comprehensive state-of-the-art positioning and potentially contributes fresh ideas which are free from any licensing. 

BNC was designed to serve the following purposes

* Retrieve real-time GNSS data streams available through Ntrip transport protocol;
* Retrieve real-time GNSS data streams via TCP directly from an IP address without using the Ntrip transport protocol;
* Retrieve real-time GNSS data streams from a local UDP or serial port without using the Ntrip transport protocol;
* Plot stream distribution map from Ntrip Broadcaster source-tables;
* Generate RINEX Observation and Navigation files to support near real-time GNSS post processing applications;
* Edit or concatenate RINEX files or carry out RINEX Quality Checks (QC);
* Convert RINEX Version 2 to RINEX Version 3 and vice versa;
* Compare SP3 files containing satellite orbit and clock data;
* Generate orbit and clock corrections to Broadcast Ephemeris through an IP port to
  
  * support real-time Precise Point Positioning on GNSS rovers;
  * support the (outside) combination of such streams as coming simultaneously from various correction providers;

* Generate ephemeris and synchronized or unsynchronized observations epoch by epoch through an IP port to support real-time GNSS network engines;
* Feed a stream into a GNSS receiver via serial communication link;
* Monitor the performance of a network of real-time GNSS data streams to generate advisory notes in case of outages or corrupted streams;
* Scan RTCM streams for incoming antenna information, observation types, message types and repetition rates and latencies and GLONASS slot numbers and frequency channels;
* Carry out real-time Precise Point Positioning to determine GNSS rover positions;
* Enable multi-station Precise Point Positioning for simultaneous processing of observations from a whole network of receivers;
* Plot positions derived via PPP from RTCM streams or RINEX files on maps from Google Map or OpenStreetMap;
* Simultaneously process several Broadcast Correction streams to produce, encode and upload combined Broadcast Corrections;
* Estimate real-time tropospheric zenith path delays and save them in SINEX troposphere file format;
* Read GNSS orbits and clocks in a plain ASCII format from an IP port. They can be produced by a real-time GNSS engine such as RTNET and should be referenced to the IGS Earth-Centered-Earth-Fixed (ECEF) reference system. BNC will then

  * Convert the IGS Earth-Centered-Earth-Fixed orbits and clocks into Broadcast Corrections with radial, along-track and out-of-plane components;
  * Upload Broadcast Corrections as an RTCM Version 3 stream to an Ntrip Broadcaster;
  * Refer the orbit and clock corrections to a specific reference system;
  * Log the Broadcast Corrections as Clock RINEX files for further processing using other tools than BNC;
  * Log the Broadcast Corrections as SP3 files for further processing using other tools than BNC;

  * Upload a Broadcast Ephemeris stream in RTCM Version 3 format;

BNC supports the following GNSS stream formats and message types: 

* RTCM Version 2 message types; 
* RTCM Version 3 'conventional' message types;
* RTCM Version 3 message types for Broadcast Ephemeris;
* RTCM Version 3 'State Space Representation' (SSR) messages;
* RTCM Version 3 'Multiple Signal Messages' (MSM) and 'High Precision Multiple Signal Messages' (HP MSM);
* RTNET, a plain ASCII format defined within BNC to receive orbits and clocks from a serving GNSS engine. 

BNC supports the following GNSS file formats: 

* RINEX Version 2.11 \& 3.03, Receiver Independent Exchange format for observations, navigation and meteorological data;
* SINEX Version 2.10, Solution Independent Exchange format for station position and velocity solutions;
* SINEX TRO Draft Version 2.00, Troposphere Solution Independent Exchange format for zenith path delay products;
* SP3 Version c format for orbit solutions;
* Clock RINEX Version 3.02 format for station and satellite clock solutions;
* ANTEX Version 1.4, Antenna Exchange format for Antenna Phase Center variations;
* NMEA Version 0813, National Marine Electronics Association format for satellite navigation data;

Note that BNC allows to by-pass decoding and conversion algorithms for incoming streams, leaves whatever is received untouched to save it in files or output it through local TCP/IP port. 

Authors
=======
The BKG Ntrip Client (BNC) with a Qt Graphical User Interface (GUI) and a Command Line Interface (CLI) has been developed for

 | Federal Agency for Cartography and Geodesy (BKG)
 | Department of Geodesy, Section Satellite Navigation
 | Frankfurt am Main, Germany

The software has been written by 
 
 | Prof. Dr. Leos Mervart
 | Czech Technical University (CTU)
 | Department of Geomatics
 | Prague, Czech Republic

Prof. Mervart started working on BNC in 2005. His sole responsibility for writing the program code ended February 2015. In March 2015, Dipl.-Ing. Andrea Stürze took over the responsibility for maintaining and further developing BNC's source code. 

Documentation
=============
BNC provides context-sensitive help ( *What's This* ) related to specific objects. It furthermore comes with the here presented documentation, available as part of the software and as a PDF file. Responsible for offline documentation as well as online documentation at http://software.rtcm-ntrip.org/export/HEAD/ntrip/trunk/BNC/src/bnchelp.html and example configurations is Dr. Georg Weber.

Note that some figures presented in this documentation may show screen shots from earlier versions of BNC. If so, there is either no relevant change compared to the current appearance of the program or no change at all. 

Contact
=======
Feel free to send us comments, suggestions or bug reports. Any contribution would be appreciated. 

.. line-block::

  Federal Agency for Cartography and Geodesy (BKG)
  Department of Geodesy, Section Satellite Navigation
  Richard-Strauss-Allee 11
  60598 Frankfurt am Main, Germany
  Email: igs-ip@bkg.bund.de

Acknowledgements
================
* Oliver Montenbruck, German Space Operations Center, DLR, Oberpfaffenhofen, Germany published a RTCM Version 2 decoder unter GNU GPL which has been integrated in BNC. 
* Thomas Yan, Australian NSW Land and Property Information, proofread earlier versions of BNC's Help Contents. Up to Version 2.11 he also provides builds of BNC for Mac OS X systems. 
* Scott Glazier, OmniSTAR Australia, has been helpful in finding BNC bugs in version 1.5.  
* James Perlt, BKG, helped fixing bugs and redesigned BNC's main window in version 1.5. 
* André Hauschild, German Space Operations Center, DLR, revised the RTCM Version 2 decoder. 
* Zdenek Lukes, Czech Technical University Prague, Department of Geodesy, extended the RTCM Version 2 decoder to handle message types 3, 20, 21, and 22 and added the loss of lock indicator. 
* Jan Dousa, Geodetic Observatory Pecny, Czech Republic, helped with fixing bugs in version 2.5. 
* Denis Laurichesse, Centre National d'\'Etudes Spatiales (CNES), suggested synchronizing observations and clock corrections to reduce high frequency noise in PPP solutions. 
* Lennard Huisman, Kadaster Netherlands, and Rolf Dach, Astronomical Institute University of Bern, assisted in handling satellite clocks in transformations from ITRF to regional reference frames.

Looking Back
============
A basic function of BNC is streaming GNSS data over the open Internet using the Ntrip transport protocol. Employing IP streaming for satellite positioning goes back to the beginning of our century. Wolfgang Rupprecht has been the first person who developed TCP/IP server software under the acronym of DGPS-IP :cite:`rupprecht2000a` and published it under GNU General Public License (GPL). While connecting marine beacon receivers to PCs with permanent access to the Internet he transmitted DGPS corrections in an RTCM format to support Differential GPS positioning over North America. With approximately 200 bits/sec the bandwidth requirement for disseminating beacon data was comparatively small. Each stream was transmitted over a unique combination of IP address and port. Websites informed about existing streams and corresponding receiver positions. 

To cope with an increasing number of transmitting GNSS reference stations, the Federal Agency for Cartography and Geodesy (BKG) together with the Informatik Centrum Dortmund (ICD) in Germany developed a streaming protocol for satellite navigation data called 'Networked Transport of RTCM via Internet Protocol' (Ntrip). The protocol was built on top of the HTTP standard and included the provision of meta data describing the stream content. Any stream could now be globally transmitted over just one IP port: HTTP port 80. Stream availability and content details became part of the transport protocol. The concept was first published in 2003 :cite:`weber2004a`, :cite:`weber2005a` and was based on three software components, namely an NtripServer pushing data from a reference station to an NtripCaster and an NtripClient pulling data from the stream splitting caster to support a rover receiver. (Note that from a socket-programmers perspective NtripServer and NtripClient both act as clients; only the NtripCaster operates as socket-server.) Ntrip could essentially benefit from Internet Radio developments. It was the ICECAST multimedia server, which provided the bases for BKG's 'Professional Ntrip Broadcaster' with software published first in 2003 and of course again as Open Source under GPL. 

For BKG as a governmental agency, making Ntrip an Open Industry Standard has been an objective from the very beginning. The 'Radio Technical Commission for Maritime Services' (RTCM) accepted 'Ntrip Version 1' in 2004 as 'RTCM Recommended Standard' :cite:`weber2005b`. Nowadays there is almost no geodetic GNSS receiver which does not come with integrated NtripClient and NtripServer functionality as part of the firmware. Hundreds of NtripCaster implementations are operated world-wide for highly accurate satellite navigation through RTK networks. Thousands of reference stations upload observations via NtripServer to central computing facilities for any kind of NtripClient application. In 2011 'Ntrip Version 2' was released :cite:`rtcm-sc104_2011a` which cleared and fixed some design problems and HTTP protocol violations. It also supports TCP/IP via SSL and adds optional communication over RTSP/RTP and UDP. 

With the advent of Ntrip as an open streaming standard, BKG's interest turned towards taking advantage from free real-time access to GNSS observations. International Associations such as the IAG Reference Frame Sub Commissions for Africa (AFREF), Asia \& Pacific (APREF), Europe (EUREF), North America (NAREF) Latin America \& Caribbean (SIRGAS), and the International GNSS Service (IGS) maintain continental or even global GNSS networks with the majority of modern receivers supporting Ntrip stream upload. Through operating BKG's NtripCaster software, these networks became extremely valuable sources of real-time GNSS information. In 2005, this was the starting point for developing the 'BKG Ntrip Client' (BNC) as a multi-stream Open Source NtripClient that allows pulling hundreds of streams simultaneously from any number of NtripCaster installations world-wide. Decoding incoming RTCM streams and output observations epoch by epoch via IP port to feed a real-time GNSS network engine became BNC's first and foremost ability :cite:`weber2009a`. Converting decoded streams to short high-rate RINEX files to assist near real-time applications became a welcome by-product right from the start of this development. 

Adding real-time Precise Point Positioning (PPP) support to BNC began in 2010 as an important completion in view of developing an Open RTCM Standard for that. According to the State Space Representation (SSR) model, new Version 3 messages are proposed to provide e.g. satellite orbit and clock corrections and ionospheric corrections as well as biases for code and phase data. The ultimate goal for SSR standardization is to reach centimeter level accuracy within seconds as an alternative to Network RTK methods such as VRS, FKP, and MAC. Because of interoperability aspects, an Open Standard in this area is of particular interest for clients. Regarding stand-alone PPP in BNC, it is worth mentioning that the program is not and can never be in competition with a receiver manufacturer's proprietary solution. Only software or services that are part of a receiver firmware could have the potential of becoming a thread for commercial interests. However, implementing or not implementing an Open PPP approach in a firmware is and will always remain a manufacturer's decision. 

Implementing some post processing capability is essential for debugging real-time software in case of problems. So certain real-time options in BNC were complemented to work offline through reading data from files. Moreover, beginning in 2012, the software was extended to support Galileo, BeiDou, and QZSS besides GPS and GLONASS. With that, the Open Source tool BNC could be used for RINEX Version 3 file editing, concatenation and quality checks, a post processing functionality demanded by the IGS Multi-GNSS Experiment and not really covered at that time by UNAVCO's famous TEQC program with its limitation on GPS. 

Over the years, the BNC Subversion (SVN) software archive received over seven thousand commits made by 11 contributors representing about one hundred thirty thousand lines of code. The well-established, mature codebase is mostly written in C++ language. Its publication under GNU GPL is thought to be well-suited for test, validation and demonstration of new approaches in precise real-time satellite navigation when IP streaming is involved. Commissioned by a German governmental agency, the overall intention has been to push the development of RTCM Recommended Standards to the benefit of IAG institutions and services such as IGS and the interested public in general. 

.. only:: latex
  
  In February 2014 the overall responsibility at BKG for the concept and realization of BNC was handed over from Georg Weber to Axel Rülke. He is in charge now for guiding the application and further evolution of the software in view of appearing new satellite navigation systems and services. 

.. only:: html

  In February 2014 the overall responsibility at BKG for the concept and realization of BNC was handed over from Georg Weber to Axel R{\"u}lke. He is in charge now for guiding the application and further evolution of the software in view of appearing new satellite navigation systems and services. 
