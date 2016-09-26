.. index: BNC installation

Installation
************

.. index: BNC installation - pre-compiled builds (MS Windows, Linux, Mac OS)

Pre-compiled builds
===================

Precompiled builds of BNC are available for MS Windows, Linux, and Mac OS X systems. They can be downloaded for installation from http://igs.bkg.bund.de/ntrip/download. Please ensure that you always use the latest released version of the program.

.. rubric:: MS Windows Build

A dynamically compiled shared library build for Mircrosoft Windows systems is provided as Microsoft Installer (MSI) file. MSI files are used for installation, storage, and removal of programs. The BNC files are contained in a MSI package, which is used with the program's client-side installer service, an .EXE file, to open and install the program. We used the MinGW Version 4.4.0 compiler to create BNC for Windows. After installation your `bnc.exe` file shows up e.g. under 'All Programs'. 

.. rubric:: Linux Build

Static library and shared library builds for BNC are provided for a selection of Linux distributions. Download the ZIP archive for a version which fits to your Linux system, unzip the archive and run the included BNC binary. A static build would be sufficient in case you do not want BNC to plot PPP results with Google Map (GM) or OpenStreetMap (OSM) maps in the background. GM/OSM usage requires BNC builds from shared libraries. 

.. rubric:: Mac OS X Build

A shared library ``Disk iMaGe`` (DMG) file is provided for BNC on OS X systems; it also comes in a ZIP archive. The DMG file format is used in the Mac for distributing software. Mac install packages appear as a virtual disk drive. After download, when the DMG file icon is double clicked, the virtual drive is 'mounted' on the desktop. Install BNC by dragging the ``bnc.app`` icon to your ``Applications`` folder. To start BNC, double click on ``Applications/bnc.app``. You could also start BNC via Command Line Interface (CLI) using command ``Applications/bnc.app/Contents/MacOS/bnc``. 

.. index:: BNC compilation
 
Compilation
===========

BNC has been written as Open Source and published under GNU General Public License (GPL). The latest source code can be checked out from Subversion repository http://software.rtcm-ntrip.org/svn/trunk/BNC. A ZIP archive available from http://igs.bkg.bund.de/ntrip/download provides the source code for BNC Version 2.12, developed using Qt Version 4.8.5. 

The following describes how you can produce your own builds of BNC on MS Windows, Linux, and Mac systems. It is likely that BNC can also be compiled on other systems where a GNU compiler and Qt Version 4.8.5 or any later version is installed. 

.. index:: Static libraries, Shared libraries

Static versus Shared Libraries
------------------------------

You can produce static or shared library builds of BNC. Static builds are sufficient in case you do not want BNC to produce track maps on top of Google Map (GM) or OpenStreetMap (OSM). GM/OSM usage would require the QtWebKit library which can only be part of BNC builds from shared Qt libraries. Hence, having a shared library Qt installation available is a precondition for producing a shared library build of BNC. 

.. index:: BNC installation - MS Windows, MS Windows

MS Windows Systems, Shared Library
----------------------------------

This explains how to install a shared Qt 4.8.5 library on MS Windows systems to then create your own shared build of BNC. 

Supposing that 'Secure Socket Layer (SSL)' is not available on you system, you should install OpenSSL libraries in ``C:\OpenSSL-Win32``. They are available e.g. from http://igs.bkg.bund.de/root_ftp/NTRIP/software/Win32OpenSSL-1_0_1e.exe. See http://slproweb.com/products/Win32OpenSSL.html for other SSL resources. Ignore possibly occurring comments about missing components during installation.

1. Download MinGW compiler Version 4.4.0 e.g. from http://igs.bkg.bund.de/root_ftp/NTRIP/software/MinGW-gcc440_1.zip.

2. Unzip the ZIP archive and move its contents to a directory ``C:\MinGW``. Now you can do either (4) or (5, 6, 8, 9, 10). Following (4) is suggested.

3. Download file ``qt-win-opensource-4.8.5-mingw.exe`` (317 MB) e.g. from https://download.qt.io/archive/qt/4.8/4.8.5/.

4. Execute this file to install a pre-compiled shared Qt library.

5. Download file ``qt-everywhere-opensource-src-4.8.5.zip`` (269 MB) e.g. from https://download.qt.io/archive/qt/4.8/4.8.5/.

6. Unzip the ZIP archive and move the contents of the contained directory into a directory ``C:\Qt\4.8.5``.

7. Create somewhere a file ``QtEnv.bat`` with the following content 

.. code-block:: console
  
   set QTDIR=C:\Qt\4.8.5
   set PATH=%PATH%;C:\MinGW\bin;C:\Qt\4.8.5\bin
   set QMAKESPEC=C:\Qt\4.8.5\mkspecs\win32-g++

8. Open a command line window and execute file ``QtEnv.bat``.

9. Go to directory Qt directory and configure Qt using command
   
.. code-block:: console
    
   cd Qt\4.8.5
   configure -fast -webkit -release -nomake examples -nomake tutorial 
             -openssl -I C:\OpenSSL-Win32\include

10. Compile Qt using command ``mingw32-make``. This may take quite a long time. Don't worry if the compilation process runs into a problem after some time. It is likely that the libraries you require are already generated at that time. Should you want to reconfiguring Qt following steps (8)-(10) you first need to clean the previous configuration using command ``mingw32-make confclean``. Run command ``mingw32-make clean`` to delete previously compiled source code.

11. Download latest BNC from SVN repository http://software.rtcm-ntrip.org/svn/trunk/BNC.

12. Open command line window and execute file ``QtEnv.bat``, see (7).

13. Go to directory BNC and enter command ``qmake bnc.pro``.

14. Enter command ``mingw32-make``.

15. Find binary file ``bnc.exe`` in directory named ``src``.

16. Extend the Windows environment variable PATH by ``C:\Qt\4.8.5\bin``.

Steps (11)-(15) can be repeated whenever a BNC update becomes available. Running ``bnc.exe`` on a windows system requires (1) when using the NTRIP Version 2s option for stream transfer over TLS/SSL. 

.. index:: BNC installation - Linux systems, Linux

Linux Systems
-------------

On Linux systems you may use the following procedure to install a shared Qt version 4.8.5 library: 

Download file ``qt-everywhere-opensource-src-4.8.5.tar.gz`` (230 MB) available from https://download.qt.io/archive/qt/4.8/4.8.5/. Unzip file, extract tar archive and change to directory ``qt-everywhere-opensource-src-4.8.5``. Run commands 

.. code-block:: console

  ./configure -fast -webkit -nomake examples -nomake tutorial 
              -prefix /usr/local/Trolltech/Qt-4.8.5
  gmake
  gmake install

Qt will be installed into directory ``/usr/local/Trolltech/Qt-4.8.5``. To reconfigure, run ``gmake confclean`` and ``configure``. Note that the ``-prefix`` option allows you to specify a directory for saving the Qt libraries. This ensures that you do not run into conflicts with other Qt installations on your host. Note further that the following two lines

.. code-block:: console
  
  export QTDIR="/usr/local/Trolltech/Qt-4.8.5"
  export PATH="$QTDIR/bin:$PATH"

need to be added either to ``$HOME/.bash/profile`` or ``$HOME/.bashrc``. Once that is done, logout/login and start using Qt 4.8.5. 

To compile the BNC program, you first download the source code from SVN repository http://software.rtcm-ntrip.org/svn/trunk/BNC. Go to directory BNC and run the following commands: 

.. code-block:: console
  
  qmake bnc.pro
  make

You will find a build of BNC in directory BNC. 

.. index:: BNC installation - Mac OS X systems, Mac OS

Mac OS X Systems
----------------

Xcode and Qt installation
^^^^^^^^^^^^^^^^^^^^^^^^^^
Xcode and Qt are required to compile BNC on OS X. Both tools are freely available. Xcode can be downloaded from the App Store or the Apple Developer Connection website. Once installed, run Xcode, go to 'Preferences->Downloads' and install the Command Line Tools component. Qt can be downloaded from the Qt Project website. We suggest installing version 4.8.4 or higher. The Qt libraries for Mac can be downloaded from http://www.qt.io/download. Once downloaded, mount the disk image, run the Qt.mpkg package and follow instructions from the installation wizard. 

Compilation of bnc
^^^^^^^^^^^^^^^^^^
The version of qmake supplied in the Qt binary package is configured to use the macx-xcode specification. This can be overridden with the ``-spec macx-g++`` option which makes it possible to use ``qmake`` to create a ``Makefile`` to be used by ``make``. 

From the directory where bnc.pro is located, run ``qmake`` to create the ``Makefile`` and then ``make`` to compile the binary:

.. code-block:: console

   qmake -spec macx-g++ bnc.pro
   make

Refer to the following webpage for further information: http://doc.qt.io/qt-4.8/qmake-platform-notes.html. 

Bundle Deployment
^^^^^^^^^^^^^^^^^
When distributing BNC it is necessary to bundle in all related Qt resources in the package. The Mac Deployment Tool has been designed to automate the process of creating a deployable application bundle that contains the Qt libraries as private frameworks. To use it, issue the following commands where ``bnc.app`` is located. 

.. code-block:: console

   macdeployqt bnc.app -dmg

Refer to the following webpage for further information: http://doc.qt.io/qt-4.8/deployment-mac.html. 

Once a DMG file for BNC is created, you can double click it and install BNC by dragging the ``bnc.app`` icon to your ``Applications`` folder. To start BNC, double click on ``Applications/bnc.app``.