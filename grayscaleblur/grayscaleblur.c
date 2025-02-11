//Compilation: gcc -I /opt/intel/system_studio_2020/opencl-sdk/include -L /opt/intel/system_studio_2020/opencl-sdk/lib64 -o grayblur grayscaleblur.c lodepng.c -Wl,-rpath,/opt/intel/system_studio_2020/opencl-sdk/lib64 -lOpenCL -lm -Wall

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_SOURCE_SIZE (0x100000)

int main() {
    //Measure execution time
    clock_t total_begin = clock();

    const char* filename = "test.png";
    const char* out_filename = "testi.png";

    unsigned char* image = 0;
    unsigned width, height;
    unsigned bitdepth = 8;

    //Decode
    unsigned error = lodepng_decode32_file(&image, &width, &height, filename);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

    //Calculate image size
    unsigned image_size = width * height;

    //Init pointers
    unsigned char* final_image;
    final_image = (unsigned char*)malloc(image_size * sizeof(unsigned char));


    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel_blur = NULL;
    cl_kernel kernel_gray = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    // buffers
    cl_mem inbuf;
    cl_mem tempbuf;
    cl_mem outbuf;

    // Load the source code containing the kernel
    FILE *fp;
    char fileName[] = "./grayscaleblur.cl";
    char *source_str;
    size_t source_size;

    fp = fopen(fileName, "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }

    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    // Get Platform and Device Info
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    printf("%d\n", ret); //1
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, &ret_num_devices);
    printf("%d\n", ret); //2

    // Create OpenCL context
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

    // Create Command Queue
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    // Create Kernel Program from the source
    program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
    (const size_t *)&source_size, &ret);

    // Build Kernel Program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    printf("%d\n", ret); //3

    // Create OpenCL Kernel
    kernel_gray = clCreateKernel(program, "grayscale", &ret);
    kernel_blur = clCreateKernel(program, "blurbox", &ret);

    // Create memory buffers on the device for each matrix
    inbuf = clCreateBuffer(context, CL_MEM_READ_ONLY, image_size * 4 * sizeof(unsigned char), NULL, NULL);
    tempbuf = clCreateBuffer(context, CL_MEM_READ_WRITE, image_size * sizeof(unsigned char), NULL, NULL);
    outbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, image_size * sizeof(unsigned char), NULL, NULL);

    // Copy Buffers to the device
    ret = clEnqueueWriteBuffer(command_queue, inbuf, CL_TRUE, 0,
                                   image_size * 4 * sizeof(unsigned char), image, 0, NULL, NULL);
    printf("%d\n", ret); //4


    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel_gray, 0, sizeof(cl_mem), &inbuf);
    printf("%d\n", ret); //5
    ret = clSetKernelArg(kernel_gray, 1, sizeof(cl_mem), &tempbuf);
    printf("%d\n", ret);//6


    // Execute the OpenCL kernel on the list
    size_t globalSize, localSize;
    localSize = 64;
    globalSize = ceil(image_size/(float)localSize)*localSize;

    //Measure execution time
    clock_t gray_begin = clock();

    ret = clEnqueueNDRangeKernel(command_queue, kernel_gray, 1, NULL, &globalSize, &localSize,
                                                              0, NULL, NULL);
    printf("%d\n", ret); //7

    //Calculate execution time
    clock_t gray_end = clock();
    double time_spent_gray = (double)(gray_end - gray_begin) / CLOCKS_PER_SEC;

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel_blur, 0, sizeof(cl_mem), &tempbuf);
    printf("%d\n", ret);//9
    ret = clSetKernelArg(kernel_blur, 1, sizeof(cl_mem), &outbuf);
    printf("%d\n", ret);//10
    ret = clSetKernelArg(kernel_blur, 2, sizeof(unsigned), &width);
    printf("%d\n", ret);//11
    ret = clSetKernelArg(kernel_blur, 3, sizeof(unsigned), &height);
    printf("%d\n", ret);//12

    //Measure execution time
    clock_t blur_begin = clock();

    // Execute the OpenCL kernel on the list
    ret = clEnqueueNDRangeKernel(command_queue, kernel_blur, 1, NULL, &globalSize, &localSize,
                                0, NULL, NULL);
    printf("%d\n", ret);//13

    //Calculate execution time
    clock_t blur_end = clock();
    double time_spent_blur = (double)(blur_end - blur_begin) / CLOCKS_PER_SEC;

    // Read the cl memory C_clmem on device to the host variable C
    ret = clEnqueueReadBuffer(command_queue, outbuf, CL_TRUE, 0,
                  image_size * sizeof(unsigned char), final_image, 0, NULL, NULL );
    printf("%d\n", ret);//14

    // Clean up and wait for all the comands to complete.
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);

    // Finally release all OpenCL allocated objects and host buffers.
    ret = clReleaseKernel(kernel_gray);
    ret = clReleaseKernel(kernel_blur);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(inbuf);
    ret = clReleaseMemObject(tempbuf);
    ret = clReleaseMemObject(outbuf);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    //Encode
    error = lodepng_encode_file(out_filename, final_image, width, height, LCT_GREY, bitdepth);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

    free(image);
    free(final_image);

    //Calculate execution time
    clock_t total_end = clock();
    double time_spent_total = (double)(total_end - total_begin) / CLOCKS_PER_SEC;

    // Print device info
    char* device_name;
    size_t device_size;
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &device_size);
    device_name = (char*) malloc(device_size);
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, device_size, device_name, NULL);
    printf("Device: %s\n", device_name);
    free(device_name);

    //Print device execution times
    printf("Total execution time: %f\n", time_spent_total);
    printf("Grayscale kernel execution time: %f\n", time_spent_gray);
    printf("Running average kernel execution time: %f\n", time_spent_blur);

}
