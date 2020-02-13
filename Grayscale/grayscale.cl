__kernel void grayscale(__global unsigned char* input, __global unsigned char* output){

    int n = get_global_id(0)*4;

    unsigned char gray;
    gray = (char) (0.2126 * input[n] + 0.7152 * input[1 + n] +
                    0.0722 * input[2 + n]);
    output[n/4] = gray;
}


__kernel void blur(__global unsigned char* input, __global unsigned char* output, unsigned int width,unsigned int height)
{
    int x = get_global_id(0);
    int tmp;

    if (x % width < 2 || x % width >= width-2 || x < 2*width || x > width * height - 2 * width)
    {
        output[x]  = input[x];
    }
    else
    {
        tmp = 0;
        for (int i = -2; i < 3; i++)
        {
            for (int j = -2; j < 3; j++)
            {
                tmp += input[i*width + x+j];
            }
        }
        output[x] = tmp * 0.04;
    }
}