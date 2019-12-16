//
// MIT license
//
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#define __FLOAT__              float
#define __FLOAT2__             float2

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/

void FHTmad(__FLOAT__ * z_k, __FLOAT__ * z_N_k, __FLOAT__ x_k, __FLOAT__ x_N_k, __FLOAT__ y_k, __FLOAT__ y_N_k) {
	*z_k = x_k * ( y_k + y_N_k ) + x_N_k * ( y_k - y_N_k);
	*z_N_k = x_N_k * ( y_k + y_N_k ) - x_k * ( y_k - y_N_k);
}
/////////////////////////////////////////////////////
//
// global size is total number of processed paires (N/2 * 2)  -1 
/////////////////////////////////////////////////////

__kernel 
void FHTMultAddHead(__global const __FLOAT__ * IR,
						__global const __FLOAT__ * FHTStage,
						__global __FLOAT__ * FHTStore,
						__global __FLOAT__ * Accum,
                        __global const int* channels_map,
						int accum_chnl_stride,
						int chnl_stride,				  
						int IR_bin_shift,
						int n_loop_bins,
						int total_n_bins,
                        int current_bin //  % __NUMBER_BINS__
					  ) 
{
	int block_sz = 	get_global_size(0) * 2;
	int k = get_global_id(0);
    int N_k = block_sz - k;
	int chunk_id = get_group_id(1);
	int chnl_id = channels_map[get_group_id(2)];
	 

	uint sample_offset = mul24(chnl_id, block_sz);
	uint channel_off = chnl_stride * chnl_id;
	uint accum_offset = mul24(chunk_id, block_sz);
	uint accum_chnl_offset = mul24(accum_chnl_stride, chnl_id);
	int bin_id = ((current_bin - chunk_id) < 0) ? total_n_bins + (current_bin - chunk_id) : (current_bin - chunk_id);
	int ir_id = chunk_id + IR_bin_shift;
	int IR_offset = ir_id * block_sz;
	int store_offset = bin_id * block_sz;

	__FLOAT__ ir_k, ir_N_k;
	__FLOAT__ fht_k, fht_N_k;
	__FLOAT__ t_accum_k, t_accum_N_k;
	__FLOAT__ accum_k = 0, accum_N_k = 0;

	int i = 0;

	N_k = (k == 0 ) ? block_sz/2 : N_k;


	if ( chunk_id == 0 ) {
		ir_k = IR[channel_off + IR_offset + k];
		ir_N_k = IR[channel_off + IR_offset + N_k];
		fht_k = FHTStage[sample_offset + k];
		fht_N_k = FHTStage[sample_offset + N_k];
		FHTStore[channel_off + store_offset +k] = fht_k;
		FHTStore[channel_off + store_offset + N_k] = fht_N_k;	

		if ( k == 0 ) {
			t_accum_k = ir_k * fht_k * (__FLOAT__)2.;
			t_accum_N_k = ir_N_k * fht_N_k  * (__FLOAT__)2.;
// everything else
		} else {

			FHTmad(&t_accum_k, &t_accum_N_k, ir_k, ir_N_k, fht_k, fht_N_k);
		}
		accum_k += t_accum_k;
		accum_N_k += t_accum_N_k;				
		i = 1;
		ir_id += get_global_size(1);
		bin_id -= get_global_size(1);		    
	}


	for(; i < n_loop_bins && ir_id < total_n_bins; i++, ir_id += get_global_size(1), bin_id -= get_global_size(1) ) {
		bin_id = (bin_id < 0) ? total_n_bins + bin_id : bin_id;
		IR_offset = ir_id * block_sz;
		store_offset = bin_id * block_sz;
		ir_k = IR[channel_off + IR_offset + k];
		ir_N_k = IR[channel_off + IR_offset + N_k];
		fht_k = FHTStore[channel_off + store_offset +k];
		fht_N_k = FHTStore[channel_off + store_offset + N_k];
		if ( k == 0 ) {
			t_accum_k = ir_k * fht_k * (__FLOAT__)2.;
			t_accum_N_k = ir_N_k * fht_N_k  * (__FLOAT__)2.;
// everything else
		} else {
			FHTmad(&t_accum_k, &t_accum_N_k, ir_k, ir_N_k, fht_k, fht_N_k);
		}
		accum_k += t_accum_k;
		accum_N_k += t_accum_N_k;	
	}

	Accum[accum_chnl_offset + accum_offset + k] = accum_k;
	Accum[accum_chnl_offset + accum_offset + N_k] = accum_N_k;
}
__kernel 
void FHTMultAddHeadXFade(__global const __FLOAT__ * IR1,
						__global const __FLOAT__ * FHTStage,
						__global __FLOAT__ * FHTStore,
						__global __FLOAT__ * Accum1,
                        __global const int* channels_map,
						int accum_chnl_stride,
						int chnl_stride,				  
						int IR_bin_shift,
						int n_loop_bins,
						int total_n_bins,
                        int current_bin, //  % __NUMBER_BINS__
                        __global const __FLOAT__ * IR2,
                        __global __FLOAT__ * Accum2
					  ) 
{
	int block_sz = 	get_global_size(0) * 2;
	int k = get_global_id(0);
    int N_k = block_sz - k;
	int chunk_id = get_group_id(1);
	int chnl_id = channels_map[get_group_id(2)];
	 

	uint sample_offset = mul24(chnl_id, block_sz);
	uint channel_off = chnl_stride * chnl_id;
	uint accum_offset = mul24(chunk_id, block_sz);
	uint accum_chnl_offset = mul24(accum_chnl_stride, chnl_id);
	int bin_id = ((current_bin - chunk_id) < 0) ? total_n_bins + (current_bin - chunk_id) : (current_bin - chunk_id);
	int ir_id = chunk_id + IR_bin_shift;
	int IR_offset = ir_id * block_sz;
	int store_offset = bin_id * block_sz;

	__FLOAT__ ir1_k, ir1_N_k, ir2_k, ir2_N_k;
	__FLOAT__ fht_k, fht_N_k;
	__FLOAT__ t_accum1_k, t_accum1_N_k, t_accum2_k, t_accum2_N_k;
	__FLOAT__ accum1_k = 0, accum1_N_k = 0, accum2_k = 0, accum2_N_k = 0;

	int i = 0;

	N_k = (k == 0 ) ? block_sz/2 : N_k;


	if ( chunk_id == 0 ) {
		ir1_k = IR1[channel_off + IR_offset + k];
		ir1_N_k = IR1[channel_off + IR_offset + N_k];
        ir2_k = IR2[channel_off + IR_offset + k];
        ir2_N_k = IR2[channel_off + IR_offset + N_k];
		fht_k = FHTStage[sample_offset + k];
		fht_N_k = FHTStage[sample_offset + N_k];
		FHTStore[channel_off + store_offset +k] = fht_k;
		FHTStore[channel_off + store_offset + N_k] = fht_N_k;	

		if ( k == 0 ) {
			t_accum1_k = ir1_k * fht_k * (__FLOAT__)2.;
			t_accum1_N_k = ir1_N_k * fht_N_k  * (__FLOAT__)2.;
            t_accum2_k = ir2_k * fht_k * (__FLOAT__)2.;
            t_accum2_N_k = ir2_N_k * fht_N_k  * (__FLOAT__)2.;
// everything else
		} else {

			FHTmad(&t_accum1_k, &t_accum1_N_k, ir1_k, ir1_N_k, fht_k, fht_N_k);
            FHTmad(&t_accum2_k, &t_accum2_N_k, ir2_k, ir2_N_k, fht_k, fht_N_k);
		}
		accum1_k += t_accum1_k;
		accum1_N_k += t_accum1_N_k;				
        accum2_k += t_accum2_k;
        accum2_N_k += t_accum2_N_k;
		i = 1;
		ir_id += get_global_size(1);
		bin_id -= get_global_size(1);		    
	}


	for(; i < n_loop_bins && ir_id < total_n_bins; i++, ir_id += get_global_size(1), bin_id -= get_global_size(1) ) {
		bin_id = (bin_id < 0) ? total_n_bins + bin_id : bin_id;
		IR_offset = ir_id * block_sz;
		store_offset = bin_id * block_sz;
		ir1_k = IR1[channel_off + IR_offset + k];
		ir1_N_k = IR1[channel_off + IR_offset + N_k];
        ir2_k = IR2[channel_off + IR_offset + k];
        ir2_N_k = IR2[channel_off + IR_offset + N_k];
		fht_k = FHTStore[channel_off + store_offset +k];
		fht_N_k = FHTStore[channel_off + store_offset + N_k];
		if ( k == 0 ) {
			t_accum1_k = ir1_k * fht_k * (__FLOAT__)2.;
			t_accum1_N_k = ir1_N_k * fht_N_k  * (__FLOAT__)2.;
            t_accum2_k = ir2_k * fht_k * (__FLOAT__)2.;
            t_accum2_N_k = ir2_N_k * fht_N_k  * (__FLOAT__)2.;
// everything else
		} else {
			FHTmad(&t_accum1_k, &t_accum1_N_k, ir1_k, ir1_N_k, fht_k, fht_N_k);
            FHTmad(&t_accum2_k, &t_accum2_N_k, ir2_k, ir2_N_k, fht_k, fht_N_k);
		}
		accum1_k += t_accum1_k;
		accum1_N_k += t_accum1_N_k;	
        accum2_k += t_accum2_k;
        accum2_N_k += t_accum2_N_k;
	}

	Accum1[accum_chnl_offset + accum_offset + k] = accum1_k;
	Accum1[accum_chnl_offset + accum_offset + N_k] = accum1_N_k;
    Accum2[accum_chnl_offset + accum_offset + k] = accum2_k;
    Accum2[accum_chnl_offset + accum_offset + N_k] = accum2_N_k;
}




__kernel
void FHTMultAddHeadDevidedXFade(__global const __FLOAT__ * IR1,
                           __global const __FLOAT__ * FHTStage,
                           __global __FLOAT__ * FHTStore,
                           __global __FLOAT__ * Accum1,
                           __global __FLOAT__ * IFHTStage1,
                           __global const int* channels_map,
                           int chnl_stride,
                           int IR_bin_shift,
                           int total_n_bins,
                           int current_bin, //  % __NUMBER_BINS__
                           int loop_offset,
                           int this_loop_sz,
                           int first_cycle,
                           int last_cycle,
                           __global const __FLOAT__ * IR2,
                           __global __FLOAT__ * Accum2,
                           __global __FLOAT__ * IFHTStage2
)
{
    int block_sz = get_global_size(0) * 2;
    int k = get_global_id(0);
    int N_k = block_sz - k;
    int chnl_id = channels_map[get_group_id(1)];

    uint sample_offset = mul24(chnl_id, block_sz);
    uint channel_off = chnl_stride * chnl_id;
    uint accum_chnl_offset = mul24(block_sz, chnl_id);
    int bin_id = ((current_bin - loop_offset) < 0) ? total_n_bins + (current_bin - loop_offset) : (current_bin - loop_offset);
    int ir_id = loop_offset + IR_bin_shift;
    int IR_offset = ir_id * block_sz;
    int store_offset = bin_id * block_sz;
    __FLOAT__ ir1_k, ir1_N_k, ir2_k, ir2_N_k;
    __FLOAT__ fht_k, fht_N_k;
    __FLOAT__ t_accum1_k, t_accum1_N_k, t_accum2_k, t_accum2_N_k;
    __FLOAT__ accum1_k = 0, accum1_N_k = 0, accum2_k = 0, accum2_N_k = 0;

    N_k = (k == 0) ? block_sz / 2 : N_k;

    if (first_cycle)
    {
        Accum1[accum_chnl_offset + k] = 0;// zeroing the previous results
        Accum1[accum_chnl_offset + N_k] = 0;// zeroing the previous results
        Accum2[accum_chnl_offset + k] = 0;// zeroing the previous results
        Accum2[accum_chnl_offset + N_k] = 0;// zeroing the previous results


        fht_k = FHTStage[sample_offset + k];
        fht_N_k = FHTStage[sample_offset + N_k];
        int current_store_offset = current_bin * block_sz;
        FHTStore[channel_off + current_store_offset + k] = fht_k;// storing the new frame''s FHT in the FHTStore, the store_offset is current_bin * block_sz
        FHTStore[channel_off + current_store_offset + N_k] = fht_N_k;// storing the new frame''s FHT in the FHTStore
    }


    for (; (ir_id < total_n_bins) && (this_loop_sz > 0); this_loop_sz--, ir_id += 1, bin_id -= 1)

    {

        bin_id = (bin_id < 0) ? total_n_bins + bin_id : bin_id;
        IR_offset = ir_id * block_sz;
        store_offset = bin_id * block_sz;
        ir1_k = IR1[channel_off + IR_offset + k];
        ir1_N_k = IR1[channel_off + IR_offset + N_k];
        ir2_k = IR2[channel_off + IR_offset + k];
        ir2_N_k = IR2[channel_off + IR_offset + N_k];
        fht_k = FHTStore[channel_off + store_offset + k];
        fht_N_k = FHTStore[channel_off + store_offset + N_k];
        if (k == 0)
        {
            t_accum1_k = ir1_k * fht_k * (__FLOAT__)2.;
            t_accum1_N_k = ir1_N_k * fht_N_k  * (__FLOAT__)2.;
            t_accum2_k = ir2_k * fht_k * (__FLOAT__)2.;
            t_accum2_N_k = ir2_N_k * fht_N_k  * (__FLOAT__)2.;
            // everything else
        }
        else
        {
            FHTmad(&t_accum1_k, &t_accum1_N_k, ir1_k, ir1_N_k, fht_k, fht_N_k);
            FHTmad(&t_accum2_k, &t_accum2_N_k, ir2_k, ir2_N_k, fht_k, fht_N_k);
        }
        accum1_k += t_accum1_k;
        accum1_N_k += t_accum1_N_k;
        accum2_k += t_accum2_k;
        accum2_N_k += t_accum2_N_k;
    }

    Accum1[accum_chnl_offset + k] += accum1_k;
    Accum1[accum_chnl_offset + N_k] += accum1_N_k;
    Accum2[accum_chnl_offset + k] += accum2_k;
    Accum2[accum_chnl_offset + N_k] += accum2_N_k;

    if (last_cycle)
    { // end of looping
        IFHTStage1[accum_chnl_offset + k] = Accum1[accum_chnl_offset + k];// Pushing the accum results in the IFHT buffers at the start of loop
        IFHTStage1[accum_chnl_offset + N_k] = Accum1[accum_chnl_offset + N_k];// Pushing the accum results in the IFHT buffers at the start of loop
        IFHTStage2[accum_chnl_offset + k] = Accum2[accum_chnl_offset + k];// Pushing the accum results in the IFHT buffers at the start of loop
        IFHTStage2[accum_chnl_offset + N_k] = Accum2[accum_chnl_offset + N_k];// Pushing the accum results in the IFHT buffers at the start of loop
    }


}


__kernel 
void FHTMultAddHeadDevided(__global const __FLOAT__ * IR,
						__global const __FLOAT__ * FHTStage,
						__global __FLOAT__ * FHTStore,
						__global __FLOAT__ * Accum,
					    __global __FLOAT__ * IFHTStage,
                        __global const int* channels_map,
						int chnl_stride,				  
						int IR_bin_shift,
						int total_n_bins,
                        int current_bin, //  % __NUMBER_BINS__
						int loop_offset,
						int this_loop_sz,
                        int first_cycle,
                        int last_cycle
					  ) 
{
	int block_sz = 	get_global_size(0) * 2;
	int k = get_global_id(0);
    int N_k = block_sz - k;
    int chnl_id = channels_map[get_group_id(1)];

	uint sample_offset = mul24(chnl_id, block_sz);
	uint channel_off = chnl_stride * chnl_id;
	uint accum_chnl_offset = mul24(block_sz, chnl_id);
	int bin_id = ((current_bin - loop_offset) < 0) ? total_n_bins + (current_bin - loop_offset) : (current_bin - loop_offset);
	int ir_id = loop_offset + IR_bin_shift;
	int IR_offset = ir_id * block_sz;
	int store_offset = bin_id * block_sz;
	__FLOAT__ ir_k, ir_N_k;
	__FLOAT__ fht_k, fht_N_k;
	__FLOAT__ t_accum_k, t_accum_N_k;
	__FLOAT__ accum_k = 0, accum_N_k = 0;

	N_k = (k == 0 ) ? block_sz/2 : N_k;

	if (first_cycle) {
		Accum[accum_chnl_offset  + k] = 0;// zeroing the previous results
		Accum[accum_chnl_offset + N_k] = 0;// zeroing the previous results


		fht_k = FHTStage[sample_offset + k];
		fht_N_k = FHTStage[sample_offset + N_k];
        int current_store_offset = current_bin * block_sz;
		FHTStore[channel_off + current_store_offset +k] = fht_k;// storing the new frame''s FHT in the FHTStore, the store_offset is current_bin * block_sz
		FHTStore[channel_off + current_store_offset + N_k] = fht_N_k;// storing the new frame''s FHT in the FHTStore
	}


	for(;  (ir_id < total_n_bins) && (this_loop_sz > 0); this_loop_sz--, ir_id += 1, bin_id -= 1)

	{
		
		bin_id = (bin_id < 0) ? total_n_bins + bin_id : bin_id;
		IR_offset = ir_id * block_sz;
		store_offset = bin_id * block_sz;
		ir_k = IR[channel_off + IR_offset + k];
		ir_N_k = IR[channel_off + IR_offset + N_k];
		fht_k = FHTStore[channel_off + store_offset +k];
		fht_N_k = FHTStore[channel_off + store_offset + N_k];
		if ( k == 0 ) {
			t_accum_k = ir_k * fht_k * (__FLOAT__)2.;
			t_accum_N_k = ir_N_k * fht_N_k  * (__FLOAT__)2.;
// everything else
		} else {
			FHTmad(&t_accum_k, &t_accum_N_k, ir_k, ir_N_k, fht_k, fht_N_k);
		}
		accum_k += t_accum_k;
		accum_N_k += t_accum_N_k;	
	}

	Accum[accum_chnl_offset + k] += accum_k;
	Accum[accum_chnl_offset + N_k] += accum_N_k;

	if(last_cycle) { // end of looping
		IFHTStage[accum_chnl_offset + k ] = Accum[accum_chnl_offset + k];// Pushing the accum results in the IFHT buffers at the start of loop
		IFHTStage[accum_chnl_offset + N_k]  = Accum[accum_chnl_offset + N_k];// Pushing the accum results in the IFHT buffers at the start of loop
	}


}

__kernel 
void FHTMultAddTail(__global __FLOAT__ * Accum,
					__global __FLOAT__ * IFFTStage,
                    __global const int* channels_map,
						int accum_chnl_stride,
 						int n_loop_bins,
						int total_n_bins
					  ) 
{

	int k = get_global_id(0);
	int chunk_id = get_group_id(1);
    int chnl_id = channels_map[get_group_id(2)];
	int block_sz = get_global_size(0);

	int channel_off = mul24(accum_chnl_stride, chnl_id);
	int accum_offset = mul24(chunk_id, block_sz);
	int accum_chnl_offset = channel_off;

	__FLOAT__ accum = 0;
	for(int i = 0, bin_id = chunk_id; i < n_loop_bins && bin_id < total_n_bins; i++, bin_id += get_global_size(1)) {

	    accum += Accum[channel_off + mul24(bin_id, block_sz) + k];
	}
// last stage ?
	if ( get_global_size(1) == 1 ) {
		IFFTStage[mul24(block_sz,chnl_id) + k ] = accum;
	} else {
		Accum[accum_chnl_offset + accum_offset + k] = accum;
	}
}

__kernel
void FHTMultAddTailXFade(__global __FLOAT__ * Accum1,
                    __global __FLOAT__ * IFFTStage1,
                    __global const int* channels_map,
                    int accum_chnl_stride,
                    int n_loop_bins,
                    int total_n_bins,
                    __global __FLOAT__ * Accum2,
                    __global __FLOAT__ * IFFTStage2
)
{

    int k = get_global_id(0);
    int chunk_id = get_group_id(1);
    int chnl_id = channels_map[get_group_id(2)];
    int block_sz = get_global_size(0);

    int channel_off = mul24(accum_chnl_stride, chnl_id);
    int accum_offset = mul24(chunk_id, block_sz);
    int accum_chnl_offset = channel_off;

    __FLOAT__ accum1 = 0;
    __FLOAT__ accum2 = 0;
    for (int i = 0, bin_id = chunk_id; i < n_loop_bins && bin_id < total_n_bins; i++, bin_id += get_global_size(1))
    {
        accum1 += Accum1[channel_off + mul24(bin_id, block_sz) + k];
        accum2 += Accum2[channel_off + mul24(bin_id, block_sz) + k];
    }
    // last stage ?
    if (get_global_size(1) == 1)
    {
        IFFTStage1[mul24(block_sz, chnl_id) + k] = accum1;
        IFFTStage2[mul24(block_sz, chnl_id) + k] = accum2;
    }
    else
    {
        Accum1[accum_chnl_offset + accum_offset + k] = accum1;
        Accum2[accum_chnl_offset + accum_offset + k] = accum2;
    }
}


/////////////////////////////////////////////////////
//
// global size is total number of processed paires (N/2 * 2)  -1 
/////////////////////////////////////////////////////

__kernel 
void FHTMultAddAccum(__global const __FLOAT__ * IR,
						__global const __FLOAT__ * FHTStage,
						__global __FLOAT__ * Accum,
						__global __FLOAT__ * IFHTStage,
                        __global const int* channels_map,
						int chnl_stride,
						int block_sz,					  
						int IR_bin_shift,
						int first_interval,
						int total_n_bins,
                        int current_bin //  % __NUMBER_BINS__
					  ) 
{
	int k = get_global_id(0);
    int N_k = get_global_size(0)*2 - k;
	int rel_bin_id = get_group_id(1);
    int chnl_id = channels_map[get_group_id(2)];
	 

	uint channel_off = chnl_stride * chnl_id;
	int bin_id = ((current_bin + rel_bin_id) >= total_n_bins)? (current_bin + rel_bin_id) - total_n_bins : (current_bin + rel_bin_id);
	uint bin_offset = bin_id * block_sz;
	int IR_id = IR_bin_shift + rel_bin_id;
	uint IR_offset = IR_id * block_sz;
	uint sample_offset = chnl_id * block_sz;
	__FLOAT__ ir_k, ir_N_k;
	__FLOAT__ fht_k, fht_N_k;
	__FLOAT__ accum_k, accum_N_k;
	__FLOAT__ t_accum_k, t_accum_N_k;

	N_k = (k == 0 ) ? get_global_size(0) : N_k;

	ir_k = IR[channel_off + IR_offset + k];
	ir_N_k = IR[channel_off + IR_offset + N_k];
	fht_k = FHTStage[sample_offset + k];
	fht_N_k = FHTStage[sample_offset + N_k];
	accum_k = Accum[channel_off + bin_offset + k];
	accum_N_k = Accum[channel_off + bin_offset + N_k];

// 0 and PI/2
	if ( k == 0 ) {
		t_accum_k = ir_k * fht_k * (__FLOAT__)2.;
		t_accum_N_k = ir_N_k * fht_N_k  * (__FLOAT__)2.;
// everything else
	} else {
		FHTmad(&t_accum_k, &t_accum_N_k, ir_k, ir_N_k, fht_k, fht_N_k);
	}
	accum_k += t_accum_k;
	accum_N_k += t_accum_N_k;

	barrier(CLK_GLOBAL_MEM_FENCE); 

	if ( first_interval && bin_id == current_bin ) {
		IFHTStage[sample_offset + k] = accum_k;
		IFHTStage[sample_offset + N_k] = accum_N_k;
		accum_k = 0;//reseting accum buffer
		accum_N_k = 0;//reseting accum buffer
	}

	Accum[channel_off + bin_offset + k] = accum_k;
	Accum[channel_off + bin_offset + N_k] = accum_N_k;
}