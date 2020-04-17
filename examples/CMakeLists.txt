include(ExternalProject)

cmake_minimum_required(VERSION 3.10)
set(DEFAULT_BUILD_TYPE Debug)

project(seek_examples)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
	set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
else()
	include(GNUInstallDirs)
	if(APPLE)
		set(CMAKE_OSX_DEPLOYMENT_TARGET 10.8)
	endif()
endif()

#Configure for Release/Debug
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	if(NOT MSVC)
		add_compile_options(-O3 -fPIC -fno-builtin -fno-exceptions -ftree-vectorize -fabi-version=1 -fomit-frame-pointer ${CFLAGS})
		add_link_options( -s ${LDFLAGS} -Wl,--as-needed )
	else()
		add_compile_options(-s W4 /PDBSTRIPPED)
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(NOT MSVC)
		add_compile_options(-O0 -g3 -fPIC -Wall -Wextra -pedantic -Wshadow -Wabi ${CFLAGS})
		add_link_options( ${LDFLAGS} )
	else()
		add_compile_options(W4)
	endif()
endif()

#Download Dependencies
if(MSVC)
	#libusb
	ExternalProject_Add(libusb-1.0
		PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/depends
		URL https://github.com/libusb/libusb/releases/download/v1.0.22/libusb-1.0.22.7z
		URL_HASH SHA256=671F1A420757B4480E7FADC8313D6FB3CBB75CA00934C417C1EFA6E77FB8779B
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	add_library(usb-1.0 SHARED IMPORTED GLOBAL)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set_target_properties(usb-1.0 PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/libusb-1.0/MS64/dll/libusb-1.0.lib)
		set_target_properties(usb-1.0 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/libusb-1.0/MS64/dll/libusb-1.0.dll)
	endif()
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set_target_properties(usb-1.0 PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/libusb-1.0/MS32/dll/libusb-1.0.lib)
		set_target_properties(usb-1.0 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/libusb-1.0/MS32/dll/libusb-1.0.dll)
	endif()
	set(LIBUSB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/libusb-1.0/include)
	file(MAKE_DIRECTORY ${LIBUSB_INCLUDE})
	target_include_directories(usb-1.0 INTERFACE ${LIBUSB_INCLUDE})

	#SDL2
	ExternalProject_Add(SDL
		PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/depends
		URL https://www.libsdl.org/release/SDL2-devel-2.0.10-VC.zip
		URL_HASH SHA256=F1930CF5033725B0E7829C7368F70F65A993BC946CA1197C36888CCECBF07891
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	add_library(SDL2 SHARED IMPORTED)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set_target_properties(SDL2 PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/lib/x64/SDL2.lib)
		set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/lib/x64/SDL2.dll)
	endif()
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set_target_properties(SDL2 PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/lib/x86/SDL2.lib)
		set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/lib/x86/SDL2.dll)
	endif()
	set(SDL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/depends/src/SDL/include)
	file(MAKE_DIRECTORY ${SDL_INCLUDE})
	target_include_directories(SDL2 INTERFACE ${SDL_INCLUDE})
endif()

#seekware
if(MSVC)
	if(NOT TARGET seekware)
		add_library(seekware SHARED IMPORTED)
		target_include_directories(seekware INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set_target_properties(seekware PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/../lib/x86_64-windows/seekware.lib)
			set_target_properties(seekware PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../lib/x86_64-windows/seekware.dll)
		endif()
		if(CMAKE_SIZEOF_VOID_P EQUAL 4)
			set_target_properties(seekware PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/../lib/x86_32-windows/seekware.lib)
			set_target_properties(seekware PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../lib/x86_32-windows/seekware.dll)
		endif()
	endif()
	add_dependencies(seekware libusb-1.0)
endif()

#seekware-simple
if(NOT TARGET seekware-simple)
	add_subdirectory(seekware-simple)
endif()

#seekware-upgrade
if(NOT TARGET seekware-upgrade)
	add_subdirectory(seekware-upgrade)
endif()

#seekware-sdl
if(NOT seekware-sdl)
	add_subdirectory(seekware-sdl)
	if(MSVC)
		add_dependencies(seekware-sdl SDL)
	endif()
endif()

#seekware-fbdev
if(UNIX AND NOT APPLE)
	if(NOT TARGET seekware-fbdev)
		add_subdirectory(seekware-fbdev)
	endif()
endif()