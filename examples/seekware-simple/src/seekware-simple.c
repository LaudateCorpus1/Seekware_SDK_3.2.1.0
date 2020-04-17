/*Copyright (c) [2019] [Seek Thermal, Inc.]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The Software may only be used in combination with Seek cores/products.

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Project:     Seek Thermal SDK Demo
 * Purpose:     Demonstrates how to communicate with Seek Thermal Cameras
 * Author:      Seek Thermal, Inc.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <winnt.h>
#define inline __inline
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <sys/time.h>
#endif

#include <seekware/seekware.h>

#define NUM_CAMS            	5
#define LOG_THERMOGRAPHY_DATA	true

bool exit_requested = false;

static inline double wall_clock_s(void) {
#ifdef _WIN32
	LARGE_INTEGER time;
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);
	return (double)time.QuadPart / freq.QuadPart;
#else
	struct timeval time;
	gettimeofday(&time, NULL);
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
#endif
}

static inline void print_fw_info(psw camera)
{
    sw_retcode status; 
    int therm_ver;

    printf("Model Number:%s\n",camera->modelNumber);
    printf("SerialNumber: %s\n", camera->serialNumber);
    printf("Manufacture Date: %s\n", camera->manufactureDate);

    printf("Firmware Version: %u.%u.%u.%u\n",
            camera->fw_version_major,
			camera->fw_version_minor,
            camera->fw_build_major,
			camera->fw_build_minor);

	status = Seekware_GetSettingEx(camera, SETTING_THERMOGRAPHY_VERSION, &therm_ver, sizeof(therm_ver));
	if (status != SW_RETCODE_NONE) {
		fprintf(stderr, "Error: Seek GetSetting returned %i\n", status);
	}
    printf("Themography Version: %i\n", therm_ver);

	sw_sdk_info sdk_info;
	Seekware_GetSdkInfo(NULL, &sdk_info);
	printf("Image Processing Version: %u.%u.%u.%u\n",
		sdk_info.lib_version_major,
		sdk_info.lib_version_minor,
		sdk_info.lib_build_major,
		sdk_info.lib_build_minor);

	printf("\n");
    fflush(stdout);
}

static void signal_callback(int signum)
{
    printf("Exit requested!\n");
    exit_requested = true;
}

int main(int argc, char * argv [])
{
	double start = 0.0f;
	double stop = 0.0f;
	double frametime = 0.0f;
	double framerate = 0.0f;

	float spot = 0;
	float min = 0;
	float max = 0;
	float timestamp_s = 0.0f;

	uint32_t field_count = 0;
	uint32_t enable = 1;
	uint64_t timestamp_us = 0;
	uint64_t frame_count = 0;

	size_t camera_pixels = 0;

	sw_retcode status;
	psw camera = NULL;
	psw camera_list[NUM_CAMS];

	float* thermography_data = NULL;
	uint16_t* filtered_data = NULL;

	signal(SIGINT, signal_callback);
	signal(SIGTERM, signal_callback);

	printf("seekware-simple - A simple data capture utility for Seek Thermal cameras\n\n");

	sw_sdk_info sdk_info;
	Seekware_GetSdkInfo(NULL, &sdk_info);
	printf("SDK Version: %u.%u\n\n", sdk_info.sdk_version_major, sdk_info.sdk_version_minor);

	int num_cameras_found = 0;
	status = Seekware_Find(camera_list, NUM_CAMS, &num_cameras_found);
	if (status != SW_RETCODE_NONE || num_cameras_found == 0) {
		printf("Cannot find any cameras...exiting\n");
		return 1;
	}

	camera = camera_list[0];
	status = Seekware_Open(camera);
	if (status != SW_RETCODE_NONE) {
		fprintf(stderr, "Cannot open camera : %d\n", status);
		goto cleanup;
	}

	// Must read firmware info AFTER the camera has been opened
	printf("::Camera Firmware Info::\n");
	print_fw_info(camera);

	camera_pixels = (size_t)camera->frame_cols * (size_t)camera->frame_rows;

	//Allocate buffers for holding thermal data
	thermography_data = (float*)malloc(camera_pixels * sizeof(float));
	if (thermography_data == NULL) {
		fprintf(stderr, "Cannot allocate thermography buffer!\n");
		goto cleanup;
	}

	filtered_data = (uint16_t*)malloc((camera_pixels + camera->frame_cols) * sizeof(uint16_t));
	if (filtered_data == NULL) {
		fprintf(stderr, "Cannot allocate filtered buffer!\n");
		goto cleanup;
	}

	Seekware_SetSettingEx(camera, SETTING_ENABLE_TIMESTAMP, &enable, sizeof(enable));
	Seekware_SetSettingEx(camera, SETTING_RESET_TIMESTAMP, &enable, sizeof(enable));

	start = wall_clock_s();

	/* * * * * * * * * * * * * Data Capture Loop * * * * * * * * * * * * * * */

	do {
		status = Seekware_GetImageEx(camera, filtered_data, thermography_data, NULL);
		if (status == SW_RETCODE_NOFRAME) {
			printf("Seek Camera Timeout ...\n");
		}
		if (status == SW_RETCODE_DISCONNECTED) {
			printf("Seek Camera Disconnected ...\n");
		}
		if (status != SW_RETCODE_NONE) {
			printf("Seek Camera Error : %u ...\n", status);
			break;
		}

		status = Seekware_GetSpot(camera, &spot, &min, &max);
		if (status != SW_RETCODE_NONE) {
			break;
		}

		++frame_count;

		// Calculate the frame rate
		stop = wall_clock_s();
		frametime = stop - start;
		framerate = (1 / frametime);
		start = wall_clock_s();

		//Writes every 10th thermography frame to a csv file.
#if LOG_THERMOGRAPHY_DATA
		if (frame_count % 10 == 0) {
			FILE* log = fopen("thermography.csv", "w");
			for (uint16_t i = 0; i < camera->frame_rows; ++i) {
				for (uint16_t j = 0; j < camera->frame_cols; ++j) {
					float value = thermography_data[(i * camera->frame_cols) + j];
					float rounded_value = roundf(10.0f * value) / 10.0f;
					fprintf(log, "%.1f,", rounded_value);
				}
				fputc('\n', log);
			}
			fclose(log);
		}
#endif
		 // Extract telemetry data
		 field_count = *(uint32_t*)&filtered_data[camera_pixels];
		 timestamp_us = *(uint64_t*)& filtered_data[camera_pixels + 5];
		 timestamp_s = (float)timestamp_us / 1.0e6f;

	    //Update metrics on stdout
        //On select cameras that do not support thermography, nan is returned for spot, min, and max
		if (!exit_requested) {
			if (frame_count > 1) {
				static const int num_lines = 17;
				for (int i = 0; i < num_lines; i++) {
					printf("\033[A");
				}
			}
			printf("\r\33[2K Frame Info:\n");
			printf("\33[2K--------------------------\n");
			printf("\33[2K frame_width:  %u\n",		 camera->frame_cols);
			printf("\33[2K field_height: %u\n",		 camera->frame_rows);
			printf("\33[2K frame_count:  %lu\n",	 frame_count);
			printf("\33[2K field_count:  %u\n",		 field_count);
			printf("\33[2K frame_rate:   %.1ffps\n", framerate);
			printf("\33[2K timestamp:    %.6fs\n",	 timestamp_s);
			printf("\33[2K--------------------------\n");

			printf("\n\33[2K Temperature Info:\n");
			printf("\33[2K--------------------------\n");
			printf("\33[2K\33\33[2K\x1B[41m\x1B[37m max: %*.1fC \x1B[0m\n", 3, max);
			printf("\33[2K\33\33[2K\x1B[42m\x1B[37m spot:%*.1fC \x1B[0m\n", 3, spot);
			printf("\33[2K\33\33[2K\x1B[44m\x1B[37m min: %*.1fC \x1B[0m\n", 3, min);
			printf("\33[2K--------------------------\n\n");

			fflush(stdout);
		}
	} while (!exit_requested);

/* * * * * * * * * * * * * Cleanup * * * * * * * * * * * * * * */
cleanup:
	
	printf("Exiting...\n");

	if (camera != NULL) {
		Seekware_Close(camera);
	}

	if (thermography_data != NULL) {
		free(thermography_data);
	}

	if (filtered_data != NULL){
		free(filtered_data);
	}

	return 0;
}
