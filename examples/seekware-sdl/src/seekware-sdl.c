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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Project:     Seek Thermal SDK Demo
 * Purpose:     Demonstrates how to image Seek Thermal Cameras with SDL2
 * Author:      Seek Thermal, Inc.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include <seekware/seekware.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#define inline __inline
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#ifdef __APPLE__
#include <SDL.h>
#endif

#ifdef __linux__
#include <sys/utsname.h>
#include <SDL2/SDL.h>
#endif

#define NUM_CAMS           5
#define SCALE_FACTOR       2
#define DRAW_RETICLE       true

typedef enum display_mode_t {
    DISPLAY_ARGB  = 0,
    DISPLAY_THERMAL = 1,
    DISPLAY_FILTERED = 2
} display_mode_t;

typedef struct display_lut_t {
    const char * name;
    sw_display_lut value;
}display_lut_t;

static display_lut_t lut_names[] = {
    { "white",  SW_LUT_WHITE    },
    { "black",  SW_LUT_BLACK    },
    { "iron",   SW_LUT_IRON     },
    { "cool",   SW_LUT_COOL     },
    { "amber",  SW_LUT_AMBER    },
    { "indigo", SW_LUT_INDIGO   },
    { "tyrian", SW_LUT_TYRIAN   },
    { "glory",  SW_LUT_GLORY    },
    { "envy",   SW_LUT_ENVY     }
};

bool exit_requested = false;
display_mode_t display_mode = DISPLAY_ARGB;
sw_display_lut current_lut = SW_LUT_TYRIAN;

static inline void print_time(void) {
	static time_t t = 0;
	static struct tm* timeptr = NULL;
	time(&t);
    timeptr = localtime(&t);
    printf("\n%s", asctime(timeptr));
}

static inline void print_fw_info(psw camera) {
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
    printf("Thermography Version: %i\n", therm_ver);

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
static void signal_callback(int signum) {
    printf("\nExit requested!\n");
    exit_requested  = true;
}

static void help(const char * name) {
    printf(
        "Usage: %s [option]...\n"
        "Valid options are:\n"
        "   -h | --help                             Prints help.\n"
        "   -lut | --lut <l>                        Sets the given LUT for drawing a color image. The following values can be set (default is \"black\"):\n"
        "   -display-thermal | --display-thermal    Demonstrates how to draw a grayscale image using fixed point U16 thermography data from the camera\n"
        "   -display-thermal | --display-thermal    Demonstrates how to draw a grayscale image using filtered, pre-AGC, image data from the camera:\n"
        "                         ", name
    );

    for (int j = 0; lut_names[j].name; ++j) {
        if (j) {
            printf(", ");
        }
        printf("%s", lut_names[j].name);
    }
    printf("\n");
}

static int parse_cmdline(int argc, char **argv) {
    for (int i=1; i<argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            help(argv[0]);
            return 0;
        }
        else if (!strcmp(argv[i], "-lut") || !strcmp(argv[i], "--lut")) {
            char* lut_name = argv[++i];
            bool found = false;
            for (int j = 0; lut_names[j].name; ++j) {
                if (!strcasecmp(lut_name, lut_names[j].name)) {
                    current_lut = lut_names[j].value;
                    found = true;
                    break;
                }
            }
            if (!found) {
                fprintf(stderr, "ERROR: Unknown parameter \"%s\" for \"-lut\"\n", lut_name);
                help(argv[0]);
                return 1;
            }
        }
        else if (!strcmp(argv[i], "-display-thermal") || !strcmp(argv[i], "---display-thermal")) {
            display_mode = DISPLAY_THERMAL;
        }
        else if (!strcmp(argv[i], "-display-filtered") || !strcmp(argv[i], "---display-filtered")) {
            display_mode = DISPLAY_THERMAL;
        }
        else {
            fprintf(stderr, "Unknown parameter: \"%s\"\n", argv[i]);
            help(argv[0]);
            return 1;
        }
    }
    return 1;
}

//Perform a simple min/max linear stretch to transform U16 grayscale image data to ARGB8888
//For advanced AGC options that are highly customizable, please see the AGC settings listed in the Seekware User Guide.
void simple_agc(uint16_t* u16_input, size_t elements_in, uint32_t* argb_output, size_t elements_out) {
    if(u16_input == NULL || argb_output == NULL){
        return;
    }

    size_t i = 0;
    uint16_t min = 0;
    uint16_t max = 0;
    uint16_t pixel_in = 0;

    //Find min and max of the input
    for (i = 0; i < elements_in; ++i){
        pixel_in = u16_input[i];
        if(pixel_in > max){
            max = pixel_in;
        }
        if(pixel_in < min) {
            min = pixel_in;
        }
    }
    
    //Find relative intensity based on min/max. Assign output RGB channels to computed 8bit luminance.
    uint16_t delta = max-min;
    uint32_t luminance = 0;
    float relative_intensity = 0.0f;
    if(delta > 0){
        for (i = 0; i < elements_out; ++i){
            relative_intensity = (float)(u16_input[i]-min)/delta;
            luminance = (uint32_t)(relative_intensity * 255.0f);
            argb_output[i] =  0xFF000000 | luminance << 16 |  luminance << 8 | luminance;
        }
    }
}

int main(int argc, char ** argv) {

	int x0 = 0;
	int y0 = 0;
	int offset = 2;
    int window_texture_pitch = 0;
	float spot = 0.0f;
	size_t frame_count = 0;
	size_t frame_pixels = 0;
	uint16_t* thermography_data = NULL;
	uint16_t* filtered_data = NULL;
    uint32_t* window_texture_data = NULL;

    bool camera_running = false;
	psw camera = NULL;
	psw camera_list[NUM_CAMS];
    sw_retcode status = SW_RETCODE_NONE;

	SDL_Event event;
	SDL_Window* window = NULL;
	SDL_Renderer* window_renderer = NULL;
	SDL_Texture* window_texture = NULL;

    signal(SIGINT, signal_callback);
    signal(SIGTERM, signal_callback);

    printf("seekware-sdl: Seek Thermal imaging tool for SDL2\n\n");

	sw_sdk_info sdk_info;
	Seekware_GetSdkInfo(NULL, &sdk_info);
	printf("SDK Version: %u.%u\n\n", sdk_info.sdk_version_major, sdk_info.sdk_version_minor);

/* * * * * * * * * * * * * Find Seek Cameras * * * * * * * * * * * * * * */

    int num_cameras_found = 0;
    Seekware_Find(camera_list, NUM_CAMS, &num_cameras_found);
    if (num_cameras_found == 0) {
        printf("Cannot find any cameras.\n");
        goto cleanup;
    }

/* * * * * * * * * * * * * Initialize Seek SDK * * * * * * * * * * * * * * */

    //Open the first Seek Camera found by Seekware_Find
    for(int i = 0; i < num_cameras_found; ++i){
        camera = camera_list[i];
        status = Seekware_Open(camera);
        if (status == SW_RETCODE_OPENEX) {
            continue;   
        }
        if(status != SW_RETCODE_NONE){
            fprintf(stderr, "Could not open camera (%d)\n", status);
            goto cleanup;
        }
        camera_running = true;
        break;
    }

	frame_pixels = (size_t)camera->frame_cols * (size_t)camera->frame_rows;

    printf("::Seek Camera Info::\n");
    print_fw_info(camera);

    // Set the default display lut value
    if (strcmp(camera->modelNumber, "2179") == 0) {
        current_lut = SW_LUT_BLACK_NEW;
        //If you would like to increase the contrast of the image on an M series stater kit (model# 2179)
        //Uncomment below to adjust the Seekware HISTEQ settings
        // float value = 0.001f;
        // if(Seekware_SetSettingEx(camera,SETTING_HISTEQ_PLATEAU_VALUE,&value, sizeof(float)) != SW_RETCODE_NONE){
        //     return 1;
        // }
        // value = 2.0f;
        // if(Seekware_SetSettingEx(camera,SETTING_HISTEQ_GAIN_LIMIT,&value, sizeof(float)) != SW_RETCODE_NONE){
        //     return 1;
        // }
    } else {
    	current_lut = SW_LUT_TYRIAN_NEW;
    }
    
    // Parse the command line to additional settings
    if (parse_cmdline(argc, argv) == 0) {
		goto cleanup;
    }

    if(Seekware_SetSettingEx(camera, SETTING_ACTIVE_LUT, &current_lut, sizeof(current_lut)) != SW_RETCODE_NONE) {
        fprintf(stderr, "Invalid LUT index\n");
		goto cleanup;
    }

    // Allocate memory to store thermography data returned from the Seek camera
    if(display_mode == DISPLAY_THERMAL){
        thermography_data = (uint16_t*) malloc(frame_pixels * sizeof(uint16_t));
    } else if(display_mode == DISPLAY_FILTERED){
        filtered_data = (uint16_t*) malloc(frame_pixels * sizeof(uint16_t));
    }

/* * * * * * * * * * * * * Initialize SDL * * * * * * * * * * * * * * */

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0) {
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        printf("Display driver: %s\n", SDL_GetVideoDriver(0));
    } else {
        perror("Error: Cannot initialize SDL2");
		goto cleanup;
    }

    // Initialize an SDL window:
    window = SDL_CreateWindow(
        "seekware-sdl",                         // window title
        100,                                    // initial x position
        100,                                    // initial y position
        camera->frame_cols * SCALE_FACTOR,		// width, in pixels
        camera->frame_rows * SCALE_FACTOR,		// height, in pixels
        SDL_WINDOW_SHOWN                        // present window on creation
    );
    if (window == NULL) {
        fprintf(stdout, "Could not create SDL window: %s\n", SDL_GetError());
		goto cleanup;
    }
    #if SDL_VERSION_ATLEAST(2,0,5)
//        SDL_SetWindowResizable(window, SDL_TRUE);
    #endif

    //Initialize an SDL Renderer
    //If you would like to use software rendering, use SDL_RENDERER_SOFTWARE for the flags parameter of SDL_CreateRenderer
#ifdef __linux__
    struct utsname host_info;
    memset(&host_info, 0, sizeof(host_info));
    uname(&host_info);
    if(strstr(host_info.nodename, "raspberrypi") != NULL) {
        window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }else {
        window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }
#else
    window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
#endif
    if(window_renderer == NULL){
        fprintf(stdout, "Could not create SDL window renderer: %s\n", SDL_GetError());
		goto cleanup;
    }

    if(SDL_RenderSetLogicalSize(window_renderer, camera->frame_cols, camera->frame_rows) < 0){
        fprintf(stdout, "Could not set logical size of the SDL window renderer: %s\n", SDL_GetError());
		goto cleanup;
    }

    //Create a backing texture for the SDL window
    window_texture = SDL_CreateTexture(window_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, camera->frame_cols, camera->frame_rows);
    if(window_texture == NULL){
        fprintf(stdout, "Could not create SDL window texture: %s\n", SDL_GetError());
		goto cleanup;
    }
    
/* * * * * * * * * * * * * Imaging Loop * * * * * * * * * * * * * * */

    print_time(); printf(" Start! \n");

    do{
        //Lock the backing texture and get a pointer for accessing the texture memory directly
        if(SDL_LockTexture(window_texture, NULL, (void**)(&window_texture_data), &window_texture_pitch) != 0){
            fprintf(stdout, "Could not lock SDL window texture: %s\n", SDL_GetError());
            exit_requested = true;
            continue;
        }
        
		//Get data from the camera
        if(display_mode == DISPLAY_ARGB){
			// Pass a pointer to the texture directly into Seekware_GetImage for maximum performance
            status =  Seekware_GetDisplayImage(camera, window_texture_data, (uint32_t)frame_pixels);
        }
		if(display_mode == DISPLAY_THERMAL){
            status =  Seekware_GetThermographyImage(camera, thermography_data, (uint32_t)frame_pixels);
        }
		if(display_mode == DISPLAY_FILTERED) {
            status =  Seekware_GetImage(camera, filtered_data, NULL, NULL);
        }

		//Check for errors
        if(camera_running){
            if(status == SW_RETCODE_NOFRAME){
                print_time(); printf(" Seek Camera Timeout ...\n");
            }
            if(status == SW_RETCODE_DISCONNECTED){
                print_time(); printf(" Seek Camera Disconnected ...\n");
            }
            if(status != SW_RETCODE_NONE){
                print_time(); printf(" Seek Camera Error : %u ...\n", status);
                break;
            }
        }


		//Do AGC
		if (display_mode == DISPLAY_THERMAL) {
			simple_agc(thermography_data, frame_pixels, window_texture_data, frame_pixels);
		}
		if (display_mode == DISPLAY_FILTERED) {
			simple_agc(filtered_data, frame_pixels, window_texture_data, frame_pixels);
		}

        //Unlock texture
		SDL_UnlockTexture(window_texture);

        ++frame_count;

		//Load Texture
		if (SDL_RenderCopyEx(window_renderer, window_texture, NULL, NULL, 0, NULL, SDL_FLIP_NONE) < 0) {
			fprintf(stdout, "\n Could not copy window texture to window renderer: %s\n", SDL_GetError());
			break;
		}

		//Draw Reticle
#if DRAW_RETICLE
		x0 = camera->frame_cols / 2;
		y0 = camera->frame_rows / 2;
		SDL_SetRenderDrawColor(window_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderDrawLine(window_renderer, x0, y0 - offset, x0, y0 + offset);
		SDL_RenderDrawLine(window_renderer, x0 - offset, y0, x0 + offset, y0);
#endif

		//Blit
		SDL_RenderPresent(window_renderer);

        // Request additional temperature information from the Seek camera (optional)
        if (Seekware_GetSpot(camera, &spot, NULL, NULL) != SW_RETCODE_NONE) {
            fprintf(stderr, "\n Get Spot error!\n");
            break;
        }

	    //Update temperatures to stdout
        //On select cameras that do not support thermography, nan is returned for spot, min, and max
        if(!exit_requested){
            if (frame_count > 1) {
				static const int num_lines = 4;
				for (int i = 0; i < num_lines; i++) {
					printf("\033[A");
				}
			}
			printf("\r\33[2K--------------------------\n");
			printf("\33[2K\x1B[42m\x1B[37m spot:%*.1fC \x1B[0m\n", 3, spot);
			printf("\33[2K--------------------------\n\n");
            fflush(stdout);
        }

        //Check for SDL window events
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE){
                exit_requested = true;
            }
            if(event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                if(camera_running){
                    Seekware_Stop(camera);
                    camera_running = false;
                }else {
                    Seekware_Start(camera);
                    camera_running = true;
                }
            }
        }

	} while (!exit_requested);

/* * * * * * * * * * * * * Cleanup * * * * * * * * * * * * * * */
cleanup:

    printf("Exiting...\n");

	if (camera != NULL) {
		Seekware_Close(camera);
	}
    if(thermography_data != NULL){
        free(thermography_data);
    }
    if(filtered_data != NULL){
        free(filtered_data);
    }
	if(window_texture != NULL) {
		SDL_DestroyTexture(window_texture);
	}
	if(window_renderer != NULL) {
		SDL_DestroyRenderer(window_renderer);
	}
	if(window != NULL) {
		SDL_DestroyWindow(window);
	}

    SDL_Quit();
    return 0;
}

/* * * * * * * * * * * * * End - of - File * * * * * * * * * * * * * * */
