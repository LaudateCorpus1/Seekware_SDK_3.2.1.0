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
 * Project:     Seek Thermal SDK demo
 * Purpose:     Demonstrates how to image Seek Thermal Cameras with the Linux Framebuffer
 * Author:      Seek Thermal, Inc.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sched.h>
#include <seekware/seekware.h>
#include <linux/fb.h>

#define UNUSED(x) x __attribute__((unused))

#define NUM_CAMS                4
#define NUM_VIRTUAL_SCREENS     1 //Controls how many virtual screens should be used for imaging (1 or 2) Use 2 to reduce image tearing.
#define SCREEN_OFFSET           0 //Controls how many virutal screens to pan before launching the main imaging loop.
                                  //This is useful when you only have a single fbdev you need to pan away from the active fbcon before imaging.

    static bool exit_requested = false;
    static int zero = 0;
    static int bits_per_pixel = 0;
    static const char * fbdev_name = "/dev/fb0";
    static uint8_t* fbdev_pixels = NULL;
    static struct fb_var_screeninfo vinfo;
    static struct fb_fix_screeninfo finfo;

/// Pixel format in the Seekware ARGB8888 image buffer
union imgdata {
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
    uint32_t raw;
} __attribute__((packed));

/// Pixel format in the framebuffer device
union pixel {
    struct {
        uint16_t    b:5;
        uint16_t    g:6;
        uint16_t    r:5;
    } p16;
    struct {
        uint32_t    b:8;
        uint32_t    g:8;
        uint32_t    r:8;
    } p24;
    struct {
        uint32_t    b:8;
        uint32_t    g:8;
        uint32_t    r:8;
        uint32_t    a:8;
    } p32;
} __attribute__((packed));

static void signal_callback(int UNUSED(signum)) {
    printf("\nExit requested.\n");
    exit_requested  = true;
}

static inline int put_pixel_direct(int x, int y, const union imgdata* value)
{
    union pixel* pixel = (union pixel*)(fbdev_pixels + (x + vinfo.xoffset) * bits_per_pixel + (y + vinfo.yoffset) * finfo.line_length);

    switch (vinfo.bits_per_pixel) {
        case 16:
            pixel->p16.r = value->r;
            pixel->p16.g = value->g;
            pixel->p16.b = value->b;
        break;
        case 24:
            pixel->p24.r = value->r;
            pixel->p24.g = value->g;
            pixel->p24.b = value->b;
        break;
        case 32:
            pixel->p32.r = value->r;
            pixel->p32.g = value->g;
            pixel->p32.b = value->b;
        break;
        default:
            return -1;
        break;
    };

    return 0;
}

int main(int argc, char ** argv)
{
    bool flip = false;
    int fbdev = -1;
    int screensize = 0;
    int num_cameras_found = 0;
    int current_lut = SW_LUT_TYRIAN_NEW;
    psw camera = NULL;
    psw camera_list[NUM_CAMS];
    sw_retcode status = SW_RETCODE_NONE;
    union imgdata* argb_data = NULL;
    
    signal(SIGINT, signal_callback);
    signal(SIGTERM, signal_callback);

    printf("seekware-fbdev: Seek Thermal imaging tool for the Linux framebuffer\n");

    // Look for connected cameras
    status = Seekware_Find(camera_list, NUM_CAMS, &num_cameras_found);
    if(status != SW_RETCODE_NONE){
        goto cleanup;
    }
    
    if (num_cameras_found == 0) {
        printf("Cannot find any cameras.\n");
        goto cleanup;
    }
    printf("Found %d cameras.\nPress Control-C to exit...\n", num_cameras_found);

    //Initialize the linux framebuffer
    fbdev = open(fbdev_name, O_RDWR);
    if (fbdev == -1) {
        char msg[100];
        snprintf(msg, sizeof msg, "Error: Cannot open framebuffer device '%s'", fbdev_name);
        perror(msg);
        goto cleanup;
    }

    if (ioctl(fbdev, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed framebuffer characteristics");
        goto cleanup;
    }

    if (ioctl(fbdev, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading virtual framebuffer characteristics");
        goto cleanup;
    }

    vinfo.yres_virtual = vinfo.yres * (SCREEN_OFFSET + NUM_VIRTUAL_SCREENS);
    vinfo.yoffset = (SCREEN_OFFSET * vinfo.yres);	

    if (ioctl(fbdev, FBIOPUT_VSCREENINFO, &vinfo) == -1) {
        perror("Error updating yres variable");
        goto cleanup;
    }

    bits_per_pixel = vinfo.bits_per_pixel / 8;
    screensize = vinfo.xres * vinfo.yres * bits_per_pixel;

    // Memory map the virutal frame buffer memory into the process memory space
    fbdev_pixels = (uint8_t *)mmap(NULL, screensize * (SCREEN_OFFSET + NUM_VIRTUAL_SCREENS), PROT_WRITE, MAP_SHARED, fbdev, 0);
    if ((void*)fbdev_pixels == MAP_FAILED) {
        perror("Error: failed to map framebuffer device to memory");
        goto cleanup;
    }

    camera = camera_list[0];
    status = Seekware_Open(camera);
    if (status != SW_RETCODE_NONE) {
        fprintf(stderr, "Could not open Camera(%d)\n", status);
        goto cleanup;
    }

    // Set the current lut value
    if (Seekware_SetSettingEx(camera, SETTING_ACTIVE_LUT, &current_lut, sizeof(current_lut)) != SW_RETCODE_NONE) {
        fprintf(stderr, "Invalid LUT index\n");
        goto cleanup;
    }

    argb_data = (union imgdata*) calloc(sizeof(union imgdata), camera->frame_cols * camera->frame_rows);
    
    // Main imaging loop
    do {
        // Grab an image from the camera
        status = Seekware_GetDisplayImage(camera, (uint32_t*)argb_data, camera->frame_cols * camera->frame_rows);
        if(status == SW_RETCODE_NOFRAME){
            printf("Seek Camera Timeout ...\n");
            continue;
        }
        if(status == SW_RETCODE_DISCONNECTED){
            printf("Seek Camera Disconnected ...\n");
        } 
        if(status != SW_RETCODE_NONE){
            printf("Seek Camera Error : %u ...\n", status);
            break;
        }

        //Wait for VSYNC (may not be implemented on some platforms)
        ioctl(fbdev, FBIO_WAITFORVSYNC, &zero);
        
        //Swap buffers
        if(NUM_VIRTUAL_SCREENS == 2){
            if(flip){
                vinfo.yoffset = vinfo.yres * (SCREEN_OFFSET + (NUM_VIRTUAL_SCREENS - 1));
                if(ioctl(fbdev, FBIOPAN_DISPLAY, &vinfo) == -1){
                    perror("Cannot pan fbdev!\n");
                    goto cleanup;
                }
                flip = false;
            } else if(!flip){
                vinfo.yoffset = vinfo.yres * SCREEN_OFFSET;
                if(ioctl(fbdev, FBIOPAN_DISPLAY, &vinfo) == -1){
         	        perror("Cannot pan fbdev!\n");
                    goto cleanup;
                }
                flip = true;
            }
        }

        //Write image data to the memmory mapped fbdev
        union imgdata* value = &argb_data[0];
        for (int y = 0; y < camera->frame_rows; ++y) {
            for (int x = 0; x < camera->frame_cols; ++x) {
                if (put_pixel_direct(x, y, value)) {
                    goto cleanup;
                }
                ++value;
            }
        }
    } while(!exit_requested);

cleanup:

    if(camera != NULL){
        Seekware_Close(camera);
        camera = NULL;
    }

    if(argb_data != NULL){
        free(argb_data);
        argb_data = NULL;
    }

    if(fbdev_pixels != NULL){
        munmap(fbdev_pixels, screensize * (SCREEN_OFFSET + NUM_VIRTUAL_SCREENS));
        fbdev_pixels = NULL;
    }
    if(fbdev > 0){
        vinfo.yres_virtual = vinfo.yres;
        vinfo.yoffset = 0;
        if (ioctl(fbdev, FBIOPUT_VSCREENINFO, &vinfo) == -1) {
            perror("Error updating yres variable");
        }
        close(fbdev);
        fbdev = -1;
    }

    return 0;
}

/* * * * * * * * * * * * * End - of - File * * * * * * * * * * * * * * */