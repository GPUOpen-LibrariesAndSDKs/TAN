
__kernel void SimpleCopy(
    __global float *src, 
    __global float *dst
    )
{
	int id = get_global_id(0);
	dst[id] = src[id];
}

