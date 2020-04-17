Copyright (c) [2020] [Seek Thermal, Inc.] 
See LICENSE.txt for the full licensing terms of this SDK.

This SDK package includes unmodified copies of the following third party open source software components: libusb, SDL2. 
Each of these components have their own license. Please see the relevant sections of LICENSE.txt and CREDITS.txt.

-----------------------------------------------------------

In this package you will find:
--Build variants of the Seekware library fo x86, armv8, and armv7 hosts running both Linux and Windows.
--The public interface definition of the Seek SDK library.
--Several sample apps that demonstrate functionality of the Seekware library. Source code and pre-compiled binaries are included. A MSVC project can be generated for the sources using CMake. See section ("Building with CMake" below).
--A Signed device driver for Windows hosts, udev rules for Linux hosts.

Driver Installation:
Windows 10:
	1) Disconnect all Seek Cameras from the host PC.
	2) Navigate to driver/win32 in the SDK package.
	3) Right click on seekusb.inf and select "Install"
	4) If installation is successful Seek cameras will be identified in Windows Device Manager under Universal Serial Bus devices as "Seek Thermal Camera"
Window 7, 8, 8.1:
	1) Disconnect all Seek Cameras from the host PC.
	2) Open Windows Device Manager and locate the "PIR Thermal Camera" entry under "Other Devices"
	3) Right Click on PIR Thermal Camera and select "Update Driver"
	4) Select the option "Browse for drivers on your computer"
	5) Set the path in "Search for drivers in this location" to drivers/win32 in the SDK package.
	6) If installation is successful Seek cameras will be identified in Windows Device Manager under Universal Serial Bus devices as "Seek Thermal Camera"
NOTE: Some Seek Cameras will enumerate as both a "Seek Thermal Camera" and "Seek Thermal Camera (iOS)" with the version 4.0+ driver. This is normal behavior on Windows hosts.
Linux:
	1) Disconnect all Seek Cameras from the host PC.
	2) Install the udev rules provided in the SDK package:
		sudo cp driver/udev/10-seekthermal.rules /etc/udev/rules.d
		sudo udevadm control --reload
NOTE: If your Linux system is built without udev, you can still use Seek Cameras with the SDK, however, please note the following:
	1) The application that uses libseekware.so must run as root so that libusb-1.0.so can issue commands to the camera.
	2) Older Seek Camera firmware may need to be power cycled (disconnected and reconnected) if there is >20s between closing and re-opening the device. 

Installing the SDK Libraries on Linux:
	1) Follow the driver installation instructions above.
	2) Download the SDK dependencies:
		sudo apt-get install libusb-1.0.0-dev
		sudo apt-get install libsdl2-dev
	2) Copy libseekware.so.3.* to your preferred install location and update your linker cache. (Previous versions of this SDK used /usr/lib by default)
	3) Copy the public header files in include/seekware headers to your preferred install location. (Previous versions of this SDK used /usr/include by default)
	4) Run the pre-compiled sample apps in bin/<ARCH>, or use CMake to compile the examples.

Building with CMake:
The SDK package includes CMake scripts that will automatically generate Unix makefiles or a MSVC project for the SDK sample apps.
The CMakelists included in this SDK package require version 3.10+.
The latest version of CMake can be downloaded here: https://cmake.org/download/ or installed via your package manager on Linux (sudo apt-get install cmake)
Windows:
	1) Open the CMake GUI tool
	2) Click "Browse Source" and navigate to the examples folder in the SDK package.
	3) Click "Browse Build" and navigate to your desired build folder.
	4) Click "Generate"
	5) Follow the on-screen dialogue for selecting the version of Visual Studio and compiler you wish to target.
	6) When generation is complete, Click "Open Project".
	7) The generated MSVC solution will contain projects for each SDK sample app, each SDK dependency, and the source code to a C# wrapper for accessing the C dll from managed code.
	
NOTE: CMake can also be accessed from the command line. You may issue to the following commands to achieve the same result as above:
	cd examples
	md build
	cd build
	cmake -G "Visual Studio 16 2019" ..\
	cmake --build . --config Debug

Linux:
	1) Follow the instructions above for installing the SDK on Linux hosts.
	2) Run cmake and make
		cd apps
		mkdir build
		cd build
		cmake ../
		make

libseekware.so Dependencies:

x86
+-------------+-------------------------+-----------------------+-------------------+-------------------+
|             |   x86_64-none-linux-gnu |   i686-none-linux-gnu |   x86-windows     |   x64-windows     |
+-------------+-------------------------+-----------------------+-------------------+-------------------+
|   libc      |   GLIBC_2.3.2+          |   GLIBC_2.3.2+        |   -               |   -               |
+-------------+-------------------------+-----------------------+-------------------+-------------------+
|   libm      |   GLIBC_2.2.5+          |   GLIBC_2.1+          |   -               |   -               |
+-------------+-------------------------+-----------------------+-------------------+-------------------+
|   libstdc++ |   GLIBCXX_3.4.19+       |   GLIBCXX_3.4.19+     |   -               |   -               |
+-------------+-------------------------+-----------------------+-------------------+-------------------+
|   libgcc    |   -                     |   GLIBC_2.0+          |   -               |   -               |
+-------------+-------------------------+-----------------------+-------------------+-------------------+
|   kernel32  |   -                     |   -                   |   Windows 7+      |   Windows 7+      |
+-------------+-------------------------+-----------------------+-------------------+-------------------+
|   MSVCR120  |   -                     |   -                   |   Visual C++ 2013 |   Visual C++ 2013 |
+-------------+-------------------------+-----------------------+-------------------+-------------------+
|   MSVCP120  |   -                     |   -                   |   Visual C++ 2013 |   Visual C++ 2013 |
+-------------+-------------------------+-----------------------+-------------------+-------------------+
|   libusb    |   1.0.22+               |   1.0.22+             |   1.0.22+         |   1.0.22+         |
+-------------+-------------------------+-----------------------+-------------------+-------------------+

armv8
+------------+-------------------------+-----------------------------+
|            | aarch64-none-linux-gnu  | aarch64-none-linux-muslgnu  |
+------------+-------------------------+-----------------------------+
| libc       | GLIBC_2.17+             | musl  1.1.16+               |
+------------+-------------------------+-----------------------------+
| libm       | GLIBC_2.17+             | -                           |
+------------+-------------------------+-----------------------------+
| libstdc++  | GLIBCXX_3.4.19+         | GLIBCXX_3.4.19+             |
+------------+-------------------------+-----------------------------+
| libgcc     | -                       | -                           |
+------------+-------------------------+-----------------------------+
| libusb     | 1.0.22+                 | 1.0.22+                     |
+------------+-------------------------+-----------------------------+

armv7 (hard float)
+------------+---------------------------+-----------------------------+---------------------------------+-------------------------------+
|            | arm-none-linux-gnueabihf  | arm-linaro-linux-gnueabihf  | arm-none-linux-uclibcgnueabihf  | arm-none-linux-muslgnueabihf  |
+------------+---------------------------+-----------------------------+---------------------------------+-------------------------------+
| libc       | GLIBC_2.4+                | GLIBC_2.4+                  | uClibc 0.9.33.2+                | musl  1.1.16+                 |
+------------+---------------------------+-----------------------------+---------------------------------+-------------------------------+
| libm       | GLIBC_2.4+                | GLIBC_2.4+                  | uClibc 0.9.33.2+                | -                             |
+------------+---------------------------+-----------------------------+---------------------------------+-------------------------------+
| libstdc++  | GLIBCXX_3.4.19+           | GLIBCXX_3.4.11+             | GLIBCXX_3.4.19+                 | GLIBCXX_3.4.19+               |
+------------+---------------------------+-----------------------------+---------------------------------+-------------------------------+
| libgcc     | GCC_3.5+                  | GCC_3.5+                    | GCC_3.5+                        | GCC_3.5+                      |
+------------+---------------------------+-----------------------------+---------------------------------+-------------------------------+
| libusb     | 1.0.22+                   | 1.0.22+                     | 1.0.22+                         | 1.0.22                        |
+------------+---------------------------+-----------------------------+---------------------------------+-------------------------------+

armv7 (soft float)
+------------+-------------------------+---------------------------+-------------------------------+-----------------------------+
|            | arm-none-linux-gnueabi  | arm-linaro-linux-gnueabi  | arm-none-linux-uclibcgnueabi  | arm-none-linux-muslgnueabi  |
+------------+-------------------------+---------------------------+-------------------------------+-----------------------------+
| libc       | GLIBC_2.4+              | GLIBC_2.4+                | uClibc 0.9.33.2+              | musl  1.1.16+               |
+------------+-------------------------+---------------------------+-------------------------------+-----------------------------+
| libm       | GLIBC_2.4+              | GLIBC_2.4+                | uClibc 0.9.33.2+              | -                           |
+------------+-------------------------+---------------------------+-------------------------------+-----------------------------+
| libstdc++  | GLIBCXX_3.4.19+         | GLIBCXX_3.4.11+           | GLIBCXX_3.4.19+               | GLIBCXX_3.4.19+             |
+------------+-------------------------+---------------------------+-------------------------------+-----------------------------+
| libgcc     | GCC_3.5+                | GCC_3.5+                  | GCC_3.5+                      | GCC_3.5+                    |
+------------+-------------------------+---------------------------+-------------------------------+-----------------------------+
| libusb     | 1.0.22+                 | 1.0.22+                   | 1.0.22+                       | 1.0.22+                     |
+------------+-------------------------+---------------------------+-------------------------------+-----------------------------+

Sample App Dependencies:
+---------------+-------------------+----------------+------------------+--------------------+
|               |   seekware-simple |   seekware-sdl |   seekware-fbdev |   seekware-upgrade |
+---------------+-------------------+----------------+------------------+--------------------+
|   libseekware |   2.9+            |   2.9+         |   2.9+           |   2.9+             |
+---------------+-------------------+----------------+------------------+--------------------+
|   libSDL2     |   -               |   2.0.5+       |   -              |   -                |
+---------------+-------------------+----------------+------------------+--------------------+
|   OS          |   Linux 3.2+      |   Linux 3.2+   |   Linux 3.2+     |   -                |
|		|   Windows 7+	    |	Windows 7+   |   Windows7+	|   -	             |
+---------------+-------------------+----------------+------------------+--------------------+

Release Notes:
v3.2
	-- Resolves an issue where Seekware_Open could hang on Windows hosts.
	-- Adds support for C2X Mosaic cores

v3.1
	-- Added new API functions (Seekware_Stop and Seekware_Start) that suspend or resume background frame processing. When frame processing it stopped, the camera will enter low power mode, but remain open.
	-- Fixed an issue t¬hat prevented selecting a user defined color lut (SW_LUT_USER0 - SW_LUT_USER4) with both Seekware_SetSetting and Seekware_SetSettingEx.
	-- Added support for 2850 J-series cores.

v3.0
	-- Added cross platform support for Windows. Moving forward, Linux and Windows builds of this SDK will maintain feature parity by sharing a common API.
	-- This release is ABI compatible with Seekware Linux 2.21
	-- This release is NOT ABI compatible with Seekware Windows 2.8.1
v2.21
	-- Fixed an issue where certain J-series cores would report inaccurate thermography for the first 5 frames. 
v2.20
	-- Added support for 2808 J-series cores.
v2.19
	-- Fixed an issue with error reporting during certain firmware upgrade failures.
	-- libseekware.so no longer requires librt.so for arm-linaro-linux-gnueabi and arm-linaro-linux-gnueabihf targets. For more detail on the dependencies of libseekware.so for each build variant see the tables included above.

v2.18
	-- Improved reliability during firmware upgrades.
	-- Minor improvements to the seekware-upgrade sample app.
	-- Added the ability to call Seekware_GetSdkInfo with id=NULL for accessing the Seekware SDK version without a connected device.
	-- SETTING_AGC_MODE will default to Legacy HistEQ for all J series cores.

v2.17
	-- Added new build variants for armv7 and armv8 targets that support uclibc and musl libc.
	-- Added a new build variant, arm-linaro-linux-gnueabihf that lowers the minimum requirement for libstdc++ to GLIBCXX_3.4.11+ from GLIBCXX_3.4.19+
	-- Added support for 2470 J3 cores.

v2.16
	-- Fixed a bug where the Seek Device could not reopen after previously closing
v2.15
	-- Added new settings for HistEQ and Linear Min/Max (
		SETTING_AGC_MODE,
		SETTING_HISTEQ_BIN_COUNT,
		SETTING_HISTEQ_INPUT_BIT_DEPTH,
		SETTING_HISTEQ_OUTPUT_BIT_DEPTH,
		SETTING_HISTEQ_HIST_WIDTH_COUNTS,
		SETTING_HISTEQ_PLATEAU_VALUE,
		SETTING_HISTEQ_GAIN_LIMIT,
		SETTING_HISTEQ_GAIN_LIMIT_FACTOR_ENABLE,
		SETTING_HISTEQ_GAIN_LIMIT_FACTOR,
		SETTING_HISTEQ_GAIN_LIMIT_FACTOR_XMAX,
		SETTING_HISTEQ_GAIN_LIMIT_FACTOR_YMIN,
		SETTING_HISTEQ_ALPHA_TIME,
		SETTING_HISTEQ_TRIM_LEFT,
		SETTING_HISTEQ_TRIM_RIGHT,
		SETTING_LINMINMAX_MODE,
		SETTING_LINMINMAX_MIN_LOCK,
		SETTING_LINMINMAX_MAX_LOCK,
		SETTING_LINMINMAX_ACTIVE_MIN_VALUE,
		SETTING_LINMINMAX_ACTIVE_MAX_VALUE
	)
	-- Performance improvements to the Seekware imaging pipeline.
	-- Image processing improvements for automotive products.
	-- Added support for Microcore Starter Kits.
	-- Added new sample apps seekware-sdl and seekware-fbdev, which replace seekware-test.
	-- Added new sw_retcode value ( SW_RETCODE_DISCONNECTED)
	-- Added support for soft-float ARM7 targets.M
	
v2.14
	-- Added new settings (
		SETTING_SHARPENING,
		SETTING_ENABLE_TIMESTAMP,
		SETTING_RESET_TIMESTAMP,
		SETTING_TRIGGER_SHUTTER
	) that are used as parameters of Seekware_GetSettingEx and Seekware_SetSettingEx .
	-- Added Seekware_GetImageEx telemetry lines to display timestamp value.
	-- Fixed a bug where EnvTemp was not updating correctly.
	-- Fixed a bug where SETTING_SMOOTHING was not being properly enabled.
	-- Removed ProcessDisplayImage function documentation and set/get features to control it. A future release will include AGC control for the Seekware_GetImage display buffer.
	-- Added new sw_retcode values ( SW_RETCODE_NOTSUPPORTED and SW_RETCODE_INVALIDSETUP ).
	-- Renamed FEATURE_MINMAX to SETTING_MINMAX.
	
v2.13
	-- Improved transient correction functionality.
v2.12
	-- Added ProcessDisplayImage function documentation and set/get features to control it.
v2.10
	-- Fixed a bug where some seek devices would report a usb error during Seekware_Open
	-- Fixed a bug where some seek devices would report incorrect min and max temperature values.
v2.9
	-- Added new API functions ( Seekware_GetThermographyImage and Seekware_GetDisplayImage ) that each return a frame of either fixed point thermography values or ARGB display values.
	-- Added new features (FEATURE_MINMAX and FEATURE_OEM) that are used as parameters of Seekware_GetSettingEx and Seekware_SetSettingEx.
