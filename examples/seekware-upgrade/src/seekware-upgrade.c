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
 * Project:     Seek Thermal SDK
 * Purpose:     Demonstrates how to upgrade firmware on a Seek Thermal Camera
 * Author:      Seek Thermal, Inc.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <string.h>
#include <seekware/seekware.h>

#define NUM_CAMS            1

int main(int argc, char *argv[])
{
   printf("seekware-upgrade - uploads new firmware to a Seek camera\n");
    
   if(strcmp(argv[argc -1],"--help") == 0 || strcmp(argv[argc -1],"seekware-upgrade") == 0 || strcmp(argv[argc -1],"./seekware-upgrade") == 0 || argc > 4){
       printf("Usage: seekware-upgrade [UPGRADE FILE]\n");
       return 1;
   }
    
    sw_retcode status;
    psw camera = NULL;

    psw camera_list[NUM_CAMS];
    int num_found = 0;
    status = Seekware_Find(camera_list, NUM_CAMS, &num_found);
    if (status == SW_RETCODE_NONE) {
        if (num_found == 0) {
            printf("Cannot find any cameras...exiting\n");
            return 0;
        }
        else {
            printf("Found %d cameras ...\n", num_found);
            camera = camera_list[0];
        }
   } else {
        printf("Error finding camera...exiting\n");
        return status;
   }
   
   if(Seekware_Open(camera) != SW_RETCODE_NONE){
       printf("Cannot open Seek camera...exiting\n");
   }
   printf("Opened Camera...\n");
   printf("Model Number:%s\n",camera->modelNumber);
   printf("SerialNumber: %s\n", camera->serialNumber);
   printf("Manufacture Date: %s\n", camera->manufactureDate);
   printf("Current Firmware Version: %u.%u.%u.%u\n",camera->fw_version_major,camera->fw_version_minor,camera->fw_build_major,camera->fw_build_minor);
   printf("Updating Firmware...please wait...\n");

   if(Seekware_UploadFirmware(camera,argv[argc -1]) == SW_RETCODE_NONE){
        printf("New Firmware Version: %u.%u.%u.%u\n",camera->fw_version_major,camera->fw_version_minor,camera->fw_build_major,camera->fw_build_minor);
        printf("\033[1;32mFirmware upgrade successful!\033[0m\n");
   } else{
        printf("\033[1;31mFirmware upgrade failed!\033[0m\n");
   }

   if(Seekware_Close(camera) != SW_RETCODE_NONE){
       return 1;
   }
   return 0;
}
