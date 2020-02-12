__kernel void grayscale(__global unsigned char* input, __global unsigned char* output){

    int n = get_global_id(0)*4;

    unsigned char gray;
    gray = (char) (0.2126 * input[n] + 0.7152 * input[1 + n] +
                    0.0722 * input[2 + n]);
    output[n/4] = gray;


}
