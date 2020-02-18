__kernel void grayscale(__global unsigned char* input, __global unsigned char* output) {

    int n = get_global_id(0)*4;

    unsigned char gray;
    gray = (char) (0.2126 * input[n] + 0.7152 * input[1 + n] +
                    0.0722 * input[2 + n]);
    output[n/4] = gray;
}


__kernel void blurbox(__global unsigned char* in, __global unsigned char* out,
                        unsigned int w, unsigned int h) {
    int n = get_global_id(0);
    int temp;

    //Exclude borders
    if (n % w < 2 || n % w >= w - 2 || n < 2 * w ||
        n > w * h - 2 * w) {
        out[n] = in[n];
    }
    //Do box blur for all other pixels
    else {
        temp = 0;
        for (int i = -2; i < 3; i++) {
            for (int j = -2; j < 3; j++) {
                temp += in[i * w + n + j];
            }
        }
        //Multiply by 1/25 because we have 5x5
        out[n] = temp * 0.04;
    }
}
