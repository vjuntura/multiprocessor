#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SOURCE_SIZE (0x100000)

int main(int argc, const char * argv[]) {
    // VARIABLES
    
    cl_int error;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;

    const char* filename = "test.png";
    const char* out_filename = "testi.png";

    unsigned char* image = 0;
    unsigned width, height;
    unsigned bitdepth = 8;
    lodepng_decode32_file(&image, &width, &height, filename);
    const size_t INPUT_SIZE = width * height;
    
    // Initializa the ouput array
    unsigned char *output_image = (unsigned char*) malloc(INPUT_SIZE * sizeof(unsigned char));
    
    // Select the platform
    error = clGetPlatformIDs(1, &platform, NULL);
    
    // Select the device
    error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, NULL);
    
    // Create the context
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
    
    // Create the command
    queue = clCreateCommandQueue(context, device, 0, &error);
    
    // Load the source code containing the kernel
    FILE *fp;
    char fileName[] = "./grayscale.cl";
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
    
    // Create the grayscale program
    program = clCreateProgramWithSource(context, 1, (const char **) &source_str, (const size_t *) &source_size, &error);
    
    // Build the grayscale kernel program
    error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    
    // Create the OpenCL kernel
    kernel = clCreateKernel(program, "grayscale", &error);

    // Create the input buffers
    cl_mem inbuff = clCreateBuffer(context, CL_MEM_READ_ONLY, 4*INPUT_SIZE * sizeof(unsigned char), NULL, NULL);
    
    // Create the output buffer
    cl_mem outbuff = clCreateBuffer(context, CL_MEM_READ_WRITE, INPUT_SIZE * sizeof(unsigned char), NULL, NULL);
    
    // Write the data to the input buffers
    error = clEnqueueWriteBuffer(queue, inbuff, CL_TRUE, 0, 4*INPUT_SIZE * sizeof(unsigned char), image, 0, NULL, NULL);
    
    // Set the kernel's parameters
    error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inbuff);
    error = clSetKernelArg(kernel, 1, sizeof(cl_mem), &outbuff);
    
    // Execute the OpenCL kernel
    size_t globalSize, localSize;
    localSize = 64;
    globalSize = ceil(INPUT_SIZE/(float)localSize)*localSize;
    error = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
    
    // Wait for the command queue to be finished
    error = clFinish(queue);
    
    // Read the device's buffer
    error = clEnqueueReadBuffer(queue, outbuff, CL_TRUE, 0, INPUT_SIZE * sizeof(unsigned char), output_image, 0, NULL, NULL);
    
    //error = lodepng_encode32_file(out_filename, output_image, width, height);
    error = lodepng_encode_file(out_filename, output_image, width, height, LCT_GREY, bitdepth);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
    
    // Free the resources
    error = clReleaseKernel(kernel);
    error = clReleaseProgram(program);
    error = clReleaseMemObject(outbuff);
    error = clReleaseCommandQueue(queue);
    error = clReleaseContext(context);
    
    return 0;
}