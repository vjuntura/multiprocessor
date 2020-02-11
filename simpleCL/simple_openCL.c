#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#define MEM_SIZE (10000)
#define MAX_SOURCE_SIZE (0x100000)

int main() {

    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_mem memobj = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    //pointers
    int *mat1;
    int *mat2;
    int *sum;
    unsigned int n = MEM_SIZE;

    mat1 = (int*)malloc(MEM_SIZE * sizeof(int));
    mat2 = (int*)malloc(MEM_SIZE * sizeof(int));
    sum = (int*)malloc(MEM_SIZE * sizeof(int));

    //init srand
    srand(time(NULL));

    //init mat arrays with random int
    int i, j;
    for(i=0; i<100; i++) {
       for(j=0;j<100;j++) {
          mat1[i*100+j] = i+j;//rand() % 50;
          mat2[i*100+j] = i+j;//rand() % 50;
       }
    }

    // buffers
    cl_mem matbuf1;
    cl_mem matbuf2;
    cl_mem sumbuf;

    // Load the source code containing the kernel
    FILE *fp;
    char fileName[] = "./simple_openCL.cl";
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
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, &ret_num_devices);

    // Print device name
    char* device_name;
    size_t device_size;
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &device_size);
    device_name = (char*) malloc(device_size);
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, device_size, device_name, NULL);
    printf("Device: %s\n", device_name);
    free(device_name);

    // Create OpenCL context
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

    // Create Command Queue
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    // Create Kernel Program from the source
    program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
    (const size_t *)&source_size, &ret);

    // Build Kernel Program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    // Create OpenCL Kernel
    kernel = clCreateKernel(program, "simple_openCL", &ret);

    // Create memory buffers on the device for each matrix
    matbuf1 = clCreateBuffer(context, CL_MEM_READ_ONLY, MEM_SIZE * sizeof(int), NULL, NULL);
    matbuf2 = clCreateBuffer(context, CL_MEM_READ_ONLY, MEM_SIZE * sizeof(int), NULL, NULL);
    sumbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, MEM_SIZE * sizeof(int), NULL, NULL);

    // Copy Buffers to the device
    ret = clEnqueueWriteBuffer(command_queue, matbuf1, CL_TRUE, 0,
                                   MEM_SIZE * sizeof(int), mat1, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, matbuf2, CL_TRUE, 0,
                                   MEM_SIZE * sizeof(int), mat2, 0, NULL, NULL);

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &matbuf1);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &matbuf2);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), &sumbuf);
    ret = clSetKernelArg(kernel, 3, sizeof(unsigned int), &n);


    //Measure execution time
    clock_t begin = clock();

    // Execute the OpenCL kernel on the list
    size_t globalSize, localSize;
    localSize = 64;
    globalSize = ceil(n/(float)localSize)*localSize;
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &globalSize, &localSize,
                                                              0, NULL, NULL);

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    // Read the cl memory C_clmem on device to the host variable C
    ret = clEnqueueReadBuffer(command_queue, sumbuf, CL_TRUE, 0,
                              MEM_SIZE * sizeof(int), sum, 0, NULL, NULL );

    //Print result sum
    int totalsum = 0;
    for(i=0; i<n; i++)
        totalsum += sum[i];
    printf("Final result: %d\n", totalsum);
    printf("Excecution time %f\n", time_spent);

    // Clean up and wait for all the comands to complete.
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);

    // Finally release all OpenCL allocated objects and host buffers.
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(matbuf1);
    ret = clReleaseMemObject(matbuf2);
    ret = clReleaseMemObject(sumbuf);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    free(source_str);
    free(mat1);
    free(mat2);
    free(sum);

    return 0;
}
