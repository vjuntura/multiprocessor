__kernel void simple_openCL(  __global int *mat1,
                       __global int *mat2,
                       __global int *sum,
                       const unsigned int n)
{
    //Get the index of the work-item
    int index = get_global_id(0);

    //Make sure we do not go out of bounds
    if (index < n)
        sum[index] = mat1[index] + mat2[index];
}
