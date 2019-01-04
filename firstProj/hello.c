#include <stdio.h>
#include <stdlib.h>
 
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
 
#define MAX_SOURCE_SIZE (0x100000)
 
int main()
{
cl_device_id device_id = NULL;
cl_context context = NULL;
cl_command_queue command_queue = NULL;
cl_mem Amobj = NULL;
cl_mem Bmobj = NULL;
cl_mem Cmobj = NULL;
cl_program program = NULL;
cl_kernel kernel = NULL;
cl_platform_id platform_id = NULL;
cl_uint ret_num_devices;
cl_uint ret_num_platforms;
cl_int ret;
 
int i,j;
float* A;
float* B;
float* C;

A = (float*)malloc(4*4*sizeof(float));
B = (float*)malloc(4*4*sizeof(float));
C = (float*)malloc(4*4*sizeof(float));
 
FILE* fp;
char fileName[] = "./hello.cl";
char* source_str;
size_t source_size;
 
/* Load the source code containing the kernel*/
fp = fopen(fileName, "r");
if (!fp) {
fprintf(stderr, "Failed to load kernel.\n");
exit(1);
}
source_str = (char*)malloc(MAX_SOURCE_SIZE);
source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
fclose(fp);
 
/* initialize the input data */
for (i=0;i<4;i++){
    for (j=0;j<4;j++){
        A[i*4+j] = i*4+j;
        B[i*4+j] = i*4+j;
    }
}

/* Get Platform and Device Info */
ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
 
/* Create OpenCL context */
context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
 
/* Create Command Queue */
command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
 
/* Create Memory Buffer */
Amobj = clCreateBuffer(context, CL_MEM_READ_WRITE,4*4*sizeof(float), NULL, &ret);
Bmobj = clCreateBuffer(context, CL_MEM_READ_WRITE,4*4*sizeof(float), NULL, &ret);
Cmobj = clCreateBuffer(context, CL_MEM_READ_WRITE,4*4*sizeof(float), NULL, &ret);

/* Copy input data to memory buffer */
ret = clEnqueueWriteBuffer(command_queue, Amobj, CL_TRUE, 0, 4*4*sizeof(float), A, 0, NULL, NULL); 
ret = clEnqueueWriteBuffer(command_queue, Bmobj, CL_TRUE, 0, 4*4*sizeof(float), B, 0, NULL, NULL); 

/* Create Kernel Program from the source */
program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
(const size_t *)&source_size, &ret);
 
/* Build Kernel Program */
ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
 
/* Create OpenCL Kernel */
kernel = clCreateKernel(program, "hello", &ret);
 
/* Set OpenCL Kernel Parameters */
ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&Amobj);
ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&Bmobj);
ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&Cmobj);

size_t global_item_size = 4;
size_t local_item_size = 1; 

/* Execute OpenCL Kernel */
ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size,&local_item_size, 0, NULL,NULL);

/* transfer results to host */
ret = clEnqueueReadBuffer(command_queue, Cmobj, CL_TRUE, 0, 4*4*sizeof(float),C,0,NULL,NULL);

/* Display Results */
for (i=0; i<4;i++) {
    for (j=0;j<4;j++) {
        printf("%7.2f ", C[i*4+j]);
    }
    printf("\n");
}
 
/* Finalization */
ret = clFlush(command_queue);
ret = clFinish(command_queue);
ret = clReleaseKernel(kernel);
ret = clReleaseProgram(program);
ret = clReleaseMemObject(Amobj);
ret = clReleaseMemObject(Bmobj);
ret = clReleaseMemObject(Cmobj);
ret = clReleaseCommandQueue(command_queue);
ret = clReleaseContext(context);
 
free(source_str);
free(A);
free(B);
free(C);
 
return 0;
}
