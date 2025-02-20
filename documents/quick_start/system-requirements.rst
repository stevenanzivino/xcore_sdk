.. _sdk-system-requirements-label:

###################
System Requirements
###################

The XCore SDK are officially supported on the following platforms. Unofficial support is mentioned where appropriate.


.. tab:: Windows

   
   Official support is provided for Windows 10 with Windows Subsystem for Linux (WSL).  See `Windows Subsystem for Linux Installation Guide for Windows 10 <https://docs.microsoft.com/en-us/windows/wsl/install-win10>`__ to install WSL.  

   The SDK should also work using other Windows GNU development environments like MinGW or Cygwin.

.. tab:: Mac

   Operating systems macOS 10.14 (Mojave) and newer are supported. Intel processors only.  Older operating systems are likely to also work, though they are not supported.

   A standard C/C++ compiler is required to build applications and libraries on the host PC.  Mac users may use the Xcode command line tools.

.. tab:: Linux

   Officical support is provided for CentOS 7.6.  The SDK also works on many modern Linux distros including Fedora and Ubuntu.

.. _sdk-prerequisites-label:

*************
Prerequisites
*************

`XTC Tools 15.0.6 <https://www.xmos.com/software/tools/>`_ or newer and `CMake 3.14 <https://cmake.org/download/>`_ or newer are required for building the example applications.  If necessary, download and follow the installation instructions for those components.
