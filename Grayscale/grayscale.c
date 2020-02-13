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
    cl_kernel kernel_gray;
    cl_kernel kernel_blur;

    const char* filename = "lena.png";
    const char* out_filename = "grayblurlena.png";

    unsigned char* image = 0;
    unsigned width, height;
    unsigned bitdepth = 8;
    error = lodepng_decode32_file(&image, &width, &height, filename);
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
    kernel_gray = clCreateKernel(program, "grayscale", &error);
    kernel_blur = clCreateKernel(program, "blur", &error);

    // Create the input buffer
    cl_mem inbuff = clCreateBuffer(context, CL_MEM_READ_ONLY, 4*INPUT_SIZE * sizeof(unsigned char), NULL, NULL);

    // Create the temp buffers
    cl_mem tempbuff = clCreateBuffer(context, CL_MEM_READ_ONLY, INPUT_SIZE * sizeof(unsigned char), NULL, NULL);
    
    // Create the output buffer
    cl_mem outbuff = clCreateBuffer(context, CL_MEM_READ_WRITE, INPUT_SIZE * sizeof(unsigned char), NULL, NULL);
    
    // Write the data to the input buffers
    error = clEnqueueWriteBuffer(queue, inbuff, CL_TRUE, 0, 4*INPUT_SIZE * sizeof(unsigned char), image, 0, NULL, NULL);
    
    // Set the kernel's parameters
    error = clSetKernelArg(kernel_gray, 0, sizeof(cl_mem), &inbuff);
    error = clSetKernelArg(kernel_gray, 1, sizeof(cl_mem), &tempbuff);
    
    // Execute the OpenCL kernel grayscale
    size_t globalSize, localSize;
    localSize = 64;
    globalSize = ceil(INPUT_SIZE/(float)localSize)*localSize;
    error = clEnqueueNDRangeKernel(queue, kernel_gray, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);

    // Set Kernel Arguments for blur		
    error = clSetKernelArg(kernel_blur, 0, sizeof(cl_mem), (void *)&tempbuff);		
    error = clSetKernelArg(kernel_blur, 1, sizeof(cl_mem), (void *)&outbuff);		
    error = clSetKernelArg(kernel_blur, 2, sizeof(int), (void *)&width);		
    error = clSetKernelArg(kernel_blur, 3, sizeof(int), (void *)&height);

    // Execute the OpenCL kernel
    error = clEnqueueNDRangeKernel(queue, kernel_blur, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
    
    // Wait for the command queue to be finished
    error = clFinish(queue);
    
    // Read the device's buffer
    error = clEnqueueReadBuffer(queue, outbuff, CL_TRUE, 0, INPUT_SIZE * sizeof(unsigned char), output_image, 0, NULL, NULL);
    
    //error = lodepng_encode32_file(out_filename, output_image, width, height);
    error = lodepng_encode_file(out_filename, output_image, width, height, LCT_GREY, bitdepth);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
    
    // Free the resources
    error = clReleaseKernel(kernel_gray);
    error = clReleaseKernel(kernel_blur);
    error = clReleaseProgram(program);
    error = clReleaseMemObject(inbuff);
    error = clReleaseMemObject(tempbuff);
    error = clReleaseMemObject(outbuff);
    error = clReleaseCommandQueue(queue);
    error = clReleaseContext(context);

    free(image);
    free(output_image);
    
    return 0;
}
