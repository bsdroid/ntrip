Configuration
*************
General procedure
=================
As a default, configuration files for running BNC on Unix/Linux/Mac OS X systems are saved in directory ``$HOME/.config/BKG``. On Windows systems, they are typically saved in directory ``C:/Documents and Settings/Username/.config/BKG``. The default configuration filename is ``BNC.bnc``.

The default filename ``BNC.bnc`` can be changed and the file content can easily be edited. On Graphical User Interfaces (GUI) it is possible to Drag \& Drop a configuration file icon to start BNC (not on Mac OS X systems). It is also possible to start and configure BNC via command line. Some configuration options can be changed on-the-fly. See annexed Command Line Help for a complete set of configuration options. 

BNC maintains configuration options at three different levels: 

(1) GUI, input fields level
(2) Active configuration level
(3) Configuration file, disk level

.. only:: html
 
  .. _fig_6:
  .. figure:: figures/fig_6.png
     :scale: 100 %

  ====== =============================================================================================================================
  Left   BNC in graphics mode; active configuration options are introduced through GUI input fields and finally saved on disk
  Middle BNC in 'no window' mode; active configuration options are read from disk
  Right  BNC in 'no window' mode without configuration file; default configuration options can be overwritten via command line options
  ====== =============================================================================================================================
  
     Management of configuration options in BNC.
 
.. only:: latex
  
   .. raw:: latex
     
     \phantomsection
     \label{chapter4:fig-6}
     \begin{figure}[tbph]
     \centering
     \includegraphics{fig_6.png}
     \label{chapter4:fig-6}
	 \begin{tabular}{l|l}
     \hline
     Left & BNC in graphics mode; active configuration options are introduced through GUI input\\
	      & fields and finally saved on disk \\
     Middle & BNC in `no window' mode; active configuration options are read from disk\\
     Right & BNC in `no window' mode without configuration file; default configuration options can be\\
           & overwritten via command line options\\
	 \hline
	 \end{tabular}
	 \caption{Management of configuration options in BNC.}
	 \end{figure}

Configuration options are usually specified using GUI input fields 1 after launching BNC. When hitting the 'Start' button, configuration options are transferred one level down to become BNC's active configuration 2, allowing the program to begin its operation. Pushing the 'Stop' button ends data processing so that the user can finally terminate BNC through 'File'->'Quit'->'Save Options' which saves processing options in a configuration file to disk 3. It is important to understand that: 

* Active configuration options (2) are independent from GUI input fields and configuration file content.
* Hence changing configuration options at GUI level (1) while BNC is already processing data does not influence a running job.
* Editing configuration options at disk level (3) while BNC is already processing data does also not influence a running job. However, there are two exceptions which force BNC to update certain active options on-the-fly:

  * Pushing the 'Reread \& Save Configuration' button lets BNC immediately reread its configuration from GUI input fields to make them active configuration options. Then BNC saves them on disk.
  * Specifying the 'Reread configuration' option lets BNC reread its configuration from disk at pre-defined intervals.

* A specific BNC configuration can be started in 'no window' mode from scratch without a configuration file if options for the active configuration level (2) are provided via command line.
 
Examples configuration files
============================
BNC comes with a number of configuration examples which can be used on all operating systems. Copy the complete directory ``Example_Configs`` which comes with the software to your disc. It includes sub-directories ``Input`` and ``Output``. There are several ways to start BNC using one of the example configurations: 

* On graphical systems (except for Mac systems), you may use the computer mouse to 'drag' a configuration file icon and 'drop' it on top of BNC's program icon. 
* You could also start BNC using a command line for naming a specific configuration file (suggested e.g. for Mac systems):  ``/Applications/bnc.app/Contents/MacOS/bnc --conf <configFileName>``
* On non-graphical systems or when running BNC in batch mode in the background you may start the program using a command line with a configuration file option in 'no window' mode (example for Windows systems): ``bnc.exe --conf <configFileName> --nw``

Although it's not a must, we suggest that you always create BNC configuration files with filename extension ``.bnc``. 

We furthermore suggest for convenience reasons that you configure your system to automatically start BNC when you double-click a file with the filename extension ``.bnc``. The following describes what to do on MS Windows systems to associate the BNC program to such configuration files: 

#. Right-click a file that has the extension ``.bnc`` and then click 'Open'. If the 'Open' command is not available, click 'Open With' or double-click the file.
#. Windows displays a dialog box that says that the system cannot open this file. The dialog box offers several options for selecting a program.
#. Click 'Select the program from a list', and then click 'OK'.
#. The 'Open With' dialog box is displayed. Click 'Browse', locate and then click the BNC program, and then click 'Open'.
#. Click to select the 'Always use the selected program to open this kind of file' check box.
#. Click 'OK'.

Some of the presented example configurations contain a user ID 'Example' with a password 'Configs' for accessing a few GNSS streams from public Ntrip Broadcasters. This free generic account is arranged for convenience reasons only. Please be so kind as to replace the generic account details as well as the place holder's 'User' and 'Pass' by the personal user ID and password you receive following an online registration through http://register.rtcm-ntrip.org. 

Note that the account for an Ntrip Broadcaster is usually limited to pulling a specified maximum number of streams at the same time. As running some of the example configurations requires pulling several streams, it is suggested to make sure that you do not exceed your account's limits. 

Make also sure that sub-directories 'Input' and 'Output' which are part of the example configurations exist on your system or adjust the affected example configuration options according to your needs. 

Some BNC options require Antenna Phase Center variations as made available from IGS through so-called ANTEX files at ftp://igs.org/pub/station/general. An example ANTEX file ``igs08.atx`` is part of the BNC package for convenience. 

The example configurations assume that no proxy protects your BNC host. Should a proxy be operated in front of BNC then you need to introduce its name or IP and port number in the 'Network' panel. 

List of example configuration files
===================================
You should be able to run all configuration file examples without changing contained options. However, configuration 'Upload.bnc' is an exception because it requires an input stream from a connected network engine. 

1. Configuration File ``RinexObs.bnc``

.. 
  
  Purpose: Convert RTCM streams to RINEX Observation files. The configuration pulls streams from Ntrip Broadcasters using Ntrip Version 1 to generate 15min 1Hz RINEX Version 3 Observation files. See http://igs.bkg.bund.de/ntrip/observations for observation stream resources.  

2. Configuration File ``RinexEph.bnc``
  
..

  Purpose: Convert a RTCM stream with navigation messages to RINEX Navigation files. The configuration pulls a RTCM Version 3 stream with Broadcast Ephemeris coming from the real-time EUREF and IGS networks and saves hourly RINEX Version 3 Navigation files. See http://igs.bkg.bund.de/ntrip/ephemeris for further real-time Broadcast Ephemeris resources. 

3. Configuration File ``BrdcCorr.bnc`` 

..
  
  Purpose: Save Broadcast Corrections from RTCM SSR messages in hourly plain ASCII files. See http://igs.bkg.bund.de/ntrip/orbits for various real-time IGS or EUREF orbit/clock correction products. 

4. Configuration File ``RinexConcat.bnc`` 

..  
  
  Purpose: Concatenate several RINEX Version 3 files to produce one compiled file and edit the marker name in the file header. The sampling interval is set to 30 seconds. See section 'RINEX Editing \& QC' in the documentation for examples on how to call BNC from command line in 'no window' mode for RINEX file editing, concatenation and quality check. 

5. Configuration File ``RinexQC.bnc`` 
  
..  
  
  Purpose: Check the quality of a RINEX Version 3 file by means of a multipath analysis. Results are saved on disk in terms of a plot in PNG format. See section 'RINEX Editing \& QC' in the documentation for examples on how to call BNC from command line in 'no window' mode for RINEX file editing, concatenation and quality check. 

6. Configuration File ``RTK.bnc`` 
  
..  
  
  Purpose: Feed a serially connected receiver with observations from a nearby reference station for conventional RTK. The stream is scanned for RTCM messages. Message type numbers and latencies of incoming observations are reported in BNC's logfile. 

7. Configuration File ``FeedEngine.bnc`` 
  
..  
  
  Purpose: Feed a real-time GNSS engine with observations from remote reference stations. The configuration pulls a single stream from an Ntrip Broadcaster. You could also pull several streams from different casters. Incoming observations are decoded, synchronized, output through a local IP port and also saved into a file. Failure and recovery thresholds are specified to inform about outages. 

.. raw:: latex

    \newpage

8. Configuration File ``PPP.bnc``
  
..  
  
  Purpose: Precise Point Positioning from observations of a rover receiver. The configuration reads RTCM Version 3 observations, a Broadcast Ephemeris stream and a stream with Broadcast Corrections. Positions are saved in the logfile. 

9. Configuration File ``PPPNet.bnc``
  
..  
  
  Purpose: Precise Point Positioning for several rovers or receivers from an entire network of reference stations in one BNC job. The possible maximum number of PPP solutions per job depends on the processing power of the hosting computer. This example configuration reads two RTCM Version 3 observation streams, a Broadcast Ephemeris stream and a stream with Broadcast Corrections. PPP Results for the two stations are saved in PPP logfiles. 

10. Configuration File ``PPPQuickStart.bnc``
  
..  
  
  Purpose: Precise Point Positioning in Quick-Start mode from observations of a static receiver with precisely known position. The configuration reads RTCM Version 3 observations, Broadcast Corrections and a Broadcast Ephemeris stream. Positions are saved in NMEA format on disc. They are also output through IP port for real-time visualization with tools like RTKPLOT. Positions are saved in the logfile. 

11. Configuration File ``PPPPostProc.bnc``
  
..  
  
  Purpose: Precise Point Positioning in post processing mode. BNC reads RINEX Version 3 Observation and Navigation files and a Broadcast Correction file. PPP processing options are set to support the Quick-Start mode. The output is saved in a specific post processing logfile and contains coordinates derived over time following the implemented PPP filter algorithm. 

12. Configuration File ``PPPGoogleMaps.bnc``
  
..  
  
  Purpose: Track BNC's point positioning solutions using Google Maps or OpenStreetMap as background. BNC reads a RINEX Observation file and a RINEX Navigation file to carry out a 'Standard Point Positioning' solution in post processing mode. Although this is not a real-time application, it requires the BNC host to be connected to the Internet. Specify a computation speed, then hit button 'Open Map' to open the track map, then hit 'Start' to visualize receiver positions on top of GM/OSM maps. 

13. Configuration File ``SPPQuickStartGal.bnc``
  
..  
  
  Purpose: Single Point Positioning in Quick-Start mode from observations of a static receiver with quite precisely known position. The configuration uses GPS, GLONASS and Galileo observations and a Broadcast Ephemeris stream. 

14. Configuration File ``SaveSp3.bnc``
  
..  
  
  Purpose: Produces SP3 files from a Broadcast Ephemeris stream and a Broadcast Correction stream. The Broadcast Correction stream is formally introduced in BNC's 'Combine Corrections' table. Note that producing SP3 requires an ANTEX file because SP3 file content should be referred to CoM. 

15. Configuration File ``Sp3ETRF2000PPP.bnc`` 
  
..  
  
  Purpose: Produce SP3 files from a Broadcast Ephemeris stream and a stream carrying ETRF2000 Broadcast Corrections. The Broadcast Correction stream is formally introduced in BNC's 'Combine Corrections' table. The configuration leads to a SP3 file containing orbits also referred to ETRF2000. Pulling in addition observations from a reference station at precisely known ETRF2000 position allows comparing an 'INTERNAL' PPP solution with a known ETRF2000 reference coordinate. 

16. Configuration File ``Upload.bnc`` 
  
..  
  
  Purpose: Upload orbits and clocks from a real-time GNSS engine to an Ntrip Broadcaster. For that the configuration reads precise orbits and clocks in RTNET format. It also reads a stream carrying Broadcast Ephemeris. BNC converts the orbits and clocks into Broadcast Corrections and encodes them to RTCM Version 3 SSR messages to finally upload them to an Ntrip Broadcaster. The Broadcast Correction stream is referred to satellite Antenna Phase Center (APC) and reference system IGS08. Orbits are saved on disk in SP3 format and clocks are saved in Clock RINEX format. 

17. Configuration File ``Combi.bnc`` 
  
..  
  
  Purpose: Pull several streams carrying Broadcast Corrections and a Broadcast Ephemeris stream from an Ntrip Broadcaster to produce a combined Broadcast Correction stream. BNC encodes the combination product in RTCM Version 3 SSR messages and uploads that to an Ntrip Broadcaster. The Broadcast Correction stream is referred to satellite Antenna Phase Center (APC) and not to satellite Center of Mass (CoM). Its reference system is IGS08. Orbits are saved in SP3 format (referred to CoM) and clocks in Clock RINEX format. 

18. Configuration File ``CombiPPP.bnc`` 
  
..  
  
  Purpose: This configuration equals the 'Combi.bnc' configuration. However, the combined Broadcast Corrections are in addition used for an 'INTERNAL' PPP solution based on observations from a static reference station with known precise coordinates. This allows a continuous quality check of the combination product through observing coordinate displacements. 

19. Configuration File ``UploadEph.bnc``
  
..  
  
  Purpose: Pull a number of streams from reference stations to get hold of contained Broadcast Ephemeris messages. They are encoded to RTCM Version 3 format and uploaded for the purpose of providing a Broadcast Ephemeris stream with an update rate of 5 seconds. 

20. Configuration File ``CompareSp3.bnc`` 
  
..  
  
  Purpose: Compare two SP3 files to calculate RMS values for orbit and clock differences. GPS satellite G05 and GLONASS satellite R18 are excluded from this comparison. Comparison results are saved in a logfile. 

21. Configuration File ``Empty.bnc``
  
..  
  
  Purpose: Provide an empty example configuration file for BNC which only contains default settings. 

Command Line configuration options
==================================
The following configuration examples make use of BNC's 'Command Line Interface' (CLI). Configuration options are exclusively specified via command line. No configuration file is used. Examples are provided as shell scripts for a Linux system. They call BNC in 'no window' batch mode (command line option ``-nw``). The scripts expect 'Example\_Configs' to be the current working directory. 

1. Shell Script ``RinexQC.sh``
 
.. 
 
  Purpose: Equals configuration file example ``RinexQC.bnc``, checks the quality of a RINEX Version 3 file by means of a multipath analysis. Virtual X-Server 'Xvfb' is operated while producing plot files in PNG format. BNC is offline. All results are saved on disk. 

2. Shell Script ``RinexConcat.sh`` 
 
.. 

  Purpose: Equals configuration file example ``RinexConcat.bnc``, concatenates several RINEX Version 3 files to produce one compiled file and edit the marker name in the file header. The sampling interval is set to 30 seconds. 

3. Shell Script ``RinexEph.sh``

..
 
  Purpose: Equals configuration file example ``RinexEph.bnc``, converts a RTCM stream with navigation messages to RINEX Navigation files. The configuration pulls a RTCM Version 3 stream with Broadcast Ephemeris coming from the real-time EUREF and IGS networks and saves hourly RINEX Version 3 Navigation files. BNC runs online until it's terminated after 10 seconds. See http://igs.bkg.bund.de/ntrip/ephemeris for further real-time Broadcast Ephemeris resources. 

4. Shell Script ``ScanLate.sh`` 

..
  
  Purpose: Scan an observation stream for contained RTCM message types, print observation latencies. The output is saved in a logfile. Latencies are reported every 10 seconds. BNC runs online until it's terminated after 20 seconds. 

5. Shell Script ``RinexObs.sh``

..
 
  Purpose: Equals configuration file example ``RinexObs.bnc``, converts RTCM streams to RINEX Observation files. The configuration pulls streams from two Ntrip Broadcasters using Ntrip Version 1 to generate 15min 1Hz RINEX Version 3 Observation files. See http://igs.bkg.bund.de/ntrip/observations for observation stream resources. BNC runs online until it's terminated after 30 seconds. 

Command Line configuration options overwriting Configuration File options
=========================================================================
For specific applications you may like to use your own set of standard configuration options from a configuration file and update some of its content via command line. When using a configuration file together with command line configuration options in one BNC call, the command line configuration options will always overrule options contained in the configuration file:

Shell script ``CompareSp3.sh``.

Purpose: Equals configuration file example ``CompareSp3.bnc``, compares two SP3 files to calculate RMS values for orbit and clock differences. However, instead of excluding GPS satellite G05 and GLONASS satellite R18 from the comparison as specified in ``CompareSp3.bnc``, GPS satellite G06 and all GLONASS satellites are excluded via command line option. BNC runs offline. Comparison results are saved in a logfile. 

