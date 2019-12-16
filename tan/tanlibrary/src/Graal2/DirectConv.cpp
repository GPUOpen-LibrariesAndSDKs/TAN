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

#include "IIRLowLat.h"
#include "IIRLowLat_OCL.h"
#include "Reverberate_OCL.h"
#include "DirectConv_OCL.h"
#include "amdFHT.h"



int ReverbCPUConv(ProjPlan *plan, void * conv_data, int conv_data_ln, void * kern_data, void *in_data, int in_data_sz, int start_pos, int conv_window) {
	int ret = 0;
	__FLOAT__ * out = (__FLOAT__ *)conv_data;
	__FLOAT__ * krn = (__FLOAT__ *)kern_data;
	__FLOAT__ * sig = (__FLOAT__ *)in_data;
	for( int i = 0; i < conv_data_ln; i++ ) {
		out[i] = 0;
		for (int j = 0, l = start_pos; l < start_pos + conv_window; l++) {
			out[i] += sig[j] * krn[l];
			j--;
			j = ( j < 0 ) ?  in_data_sz - 1 : j; 
		}
	}
	return(ret);
}


int ReverbDCCPUProcessing(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int n_samples, int flags) {
int ret = 0;
DirectConvPass* DCPass = NULL;
    if ( plan && input && output && (DCPass = (DirectConvPass*)plan->DirectConvpass) && DCPass->channels) {
		for( int i = 0; i < plan->conv_NumChannels; i++ ) {

			DirectCPUConvChannel* chnl_conv = &DCPass->channels[i];
			__FLOAT__ *new_data = input[i];
				// the prev block of incoming data
			__FLOAT__ *prev_data = 	(__FLOAT__ *)chnl_conv->data_window;
				// the first block of the conv kerenel
			__FLOAT__ * conv = (__FLOAT__*)chnl_conv->conv_window;
			__FLOAT__ * out_data = output[i];
			__FLOAT__ * in_data = new_data;
			int oi, ci, ii;
	//		#pragma omp parallel for private(ci,ii)
			for (oi = 0; oi < plan->frame_ln; oi++ ) {
				// output data already filled with the tail part of conv
	//			__FLOAT__ out = 0;
				double out = 0;
				
				in_data = new_data;
				for(ci = 0, ii = oi; ii >=0 && oi < n_samples && ci < n_samples; ci++, ii-- ) {
//					out += (double)in_data[ii] * (double)conv[ci];
					out += in_data[ii] * conv[ci];
				}
				ii = plan->frame_ln - 1;
				in_data = prev_data;
				for(; oi < n_samples && ci < n_samples; ci++, ii-- ) {
//					out += (double)in_data[ii] * (double)conv[ci];
					out += in_data[ii] * conv[ci];
				}
				out_data[oi] += (__FLOAT__)out;
			}
  // move new into previous
			memcpy(prev_data, new_data, plan->frame_ln * sizeof(__FLOAT__));

		} // per channel
	}
	return ret;
}

#if 0
int ReverbFFTCPUProcessing(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int n_samples, int flags) {
int ret = 0;
DirectConvPass* DCPass = NULL;
    if ( plan && input && output && (DCPass = (DirectConvPass*)plan->DirectConvpass) && DCPass->channels) {

		#pragma omp parallel for
		for( int i = 0; i < plan->conv_NumChannels; i++ ) {

			DirectCPUConvChannel* chnl_conv = &DCPass->channels[i];
			__FLOAT__ *new_data = input[i];
				// the prev block of incoming data
			__FLOAT__ *prev_data = 	(__FLOAT__ *)chnl_conv->data_window;
				// the first block of the conv kerenel
			__FLOAT__ *FFT_window = (__FLOAT__ *)chnl_conv->FFT_data_window;
			__FLOAT__ *FFT_IR = (__FLOAT__*)chnl_conv->FFT_conv_window;
// move data into FFT in-place windows
			memset(FFT_window, 0, plan->frame_ln * 2 * sizeof(__FLOAT__) * 2);
			for(int j = 0; j < plan->frame_ln; j++) {
				FFT_window[j*2] = new_data[j];
			}
			for(int j = 0; j < plan->frame_ln; j++) {
				FFT_window[plan->frame_ln * 2 + j*2] = prev_data[j];
			}


#if 0

			va_compex * in = (va_compex *)FFT_window;
			va_compex *out = (va_compex * )malloc(sizeof(va_compex) *  plan->frame_ln * 2);
			ReferenceCPU_D(out, in, HN, 1, 0);
			free(out);
			'
#endif
// FFT		
			switch(plan->frame_ln) {
			case 64:
				vacfft64(FFT_window, 0 );
				break;
			case 256:
				vacfft256(FFT_window, 0 );
				break;
			case 1024:
				vacfft1024(FFT_window, 0 );
				break;
			}


// Complex MAD
		// (ac-bd),(ad+bc)
			for( int j = 0; j < plan->frame_ln *2; j++ ){
				__FLOAT__ in_rl = FFT_window[j*2];
				__FLOAT__ in_img = FFT_window[j*2 + 1];
				__FLOAT__ real = FFT_window[j*2] *FFT_IR[j*2] -  FFT_window[j*2 + 1] *FFT_IR[j*2 + 1];
				// imagine
				FFT_window[j*2 + 1] = FFT_window[j*2] *FFT_IR[j*2 + 1] +  FFT_window[j*2 + 1] *FFT_IR[j*2];
				FFT_window[j*2] = real;
#if 0
				if ( i == 0 /*j < 4 || (j>= 64 && j < 68)*/) {
					printf("CPU cmad: %d %d  %f %f   %f %f   %f %f \n", i, j, in_rl, in_img, FFT_IR[j*2], FFT_IR[j*2 + 1], FFT_window[j*2], FFT_window[j*2 + 1]);
				}
#endif
			}


// IFFT
			switch(plan->frame_ln) {
			case 64:
				ivacfft64(FFT_window, 0 );
				break;
			case 256:
				ivacfft256(FFT_window, 0 );
				break;
			case 1024:
				ivacfft1024(FFT_window, 0 );
				break;
			}
// ssum
			__FLOAT__ * out_data = output[i];
			for(int j = 0; j < plan->frame_ln; j++ ) {
				out_data[j] += (FFT_window[j*2] / ((__FLOAT__) plan->frame_ln *2));
			}

  // move new into previous
			memcpy(prev_data, new_data, plan->frame_ln * sizeof(__FLOAT__));

		} // per channel
	}
	return ret;
}

#endif


void ReverbDirFHTCPUProc(FHT_FUNC fht_routine, __FLOAT__ * FHT_window, __FLOAT__ *new_data, __FLOAT__ *prev_data, __FLOAT__ *tsincos, short * bitrvrs, int N) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
// direct FHT
	fht_routine(tsincos, FHT_window);
}

int ReverbFHTCPUProcessing(ProjPlan *plan, __FLOAT__ ** input, __FLOAT__ ** output, int n_samples, int flags) {
int ret = 0;
DirectConvPass* DCPass = NULL;
// test 
#if 0
	if ( plan->frame_ln == 64 ) {
		ReverbDCCPUProcessing(plan, input, output, n_samples, flags);
		return(ret);
	}
#endif

    if ( plan && input && output && (DCPass = (DirectConvPass*)plan->CPUConvpass) && DCPass->channels) {
		short * bitrvrs = (short*)DCPass->FHT_bitrvrs;
		FHT_FUNC fht_routine = (FHT_FUNC)DCPass->FHT_routine;
		__FLOAT__ *tsincos = (__FLOAT__ *)DCPass->FHT_sincos;
//		#pragma omp parallel for
		for( int i = 0; i < plan->conv_NumChannels; i++ ) {
			int N = plan->frame_ln * 2;

			DirectCPUConvChannel* chnl_conv = &DCPass->channels[i];
			__FLOAT__ *new_data = input[i];
				// the prev block of incoming data
			__FLOAT__ *prev_data = 	(__FLOAT__ *)chnl_conv->data_window;
				// the first block of the conv kerenel
			__FLOAT__ *FHT_window = (__FLOAT__ *)chnl_conv->FFT_data_window;
			__FLOAT__ *FHT_window2 = (__FLOAT__*)chnl_conv->FHT_data2_window;
			__FLOAT__ *FHT_IR = (__FLOAT__*)chnl_conv->FFT_conv_window;
#if 0
// move data into FFT in-place windows
			ReverbDirFHTCPUProc(fht_routine, FHT_window, new_data, prev_data, tsincos, bitrvrs, N);

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
			FHT_window2[bitrvrs[0]] = FHT_window[0] * FHT_IR[0] *2;
			for( int k = 1; k < N/2; k++ ){
				__FLOAT__ x_k = FHT_window[k];
				__FLOAT__ x_N_k = FHT_window[N-k]; 
				__FLOAT__ y_k = FHT_IR[k];
				__FLOAT__ y_N_k = FHT_IR[N-k];
// write with reverse
				FHT_window2[bitrvrs[k]] = (x_k * (y_k + y_N_k) + x_N_k * (y_k - y_N_k)) ;
				FHT_window2[bitrvrs[N-k]] = (x_N_k * (y_k + y_N_k) - x_k * (y_k - y_N_k)) ;
			}
			FHT_window2[bitrvrs[N/2]] = FHT_window[(N/2)] * FHT_IR[(N/2)] * 2;
// Inverse FHT
			fht_routine(tsincos, FHT_window2);
#else
			((FHT_DIRFUNC)DCPass->FHT_proc[0])(FHT_window, new_data, prev_data, tsincos, bitrvrs);
			((FHT_INVFUNC)DCPass->FHT_proc[1])(FHT_window2,FHT_window, FHT_IR, tsincos, bitrvrs, NULL);
#endif
// ssum
			__FLOAT__ * out_data = output[i];
			for(int j = 0; j < N/2; j++ ) {
				out_data[j] += (FHT_window2[j] / ((__FLOAT__) N * 2));
			}

  // move new into previous
			memcpy(prev_data, new_data, plan->frame_ln * sizeof(__FLOAT__));

		} // per channel
	}
	return ret;
}




int DirectConvCPUSetup(ProjPlan * plan) {
	int err = CL_SUCCESS;
	DirectConvPass* DCPass = NULL;
	uint init_flags = SchedGetInitFlags(GetScheduler(plan));
    if ( (init_flags & __INIT_FLAG_HETEROGEN__) && !plan->DirectConvpass ) {
		DCPass = (DirectConvPass*)malloc(sizeof(DirectConvPass));
		if ( !DCPass) {
			printf("DirectConvCPUSetup: not enough system memory\n");
			err = -1;
			return (err);
		}
		memset(DCPass, 0, sizeof(DirectConvPass));
		plan->DirectConvpass = DCPass;

		plan->CPUConvpass = malloc(sizeof(DirectConvPass));
		memset(plan->CPUConvpass, 0, sizeof(DirectConvPass));
	}
	return err;
}

int SetupCPUDCKernel(ProjPlan * plan, void * convKernel) {
int err = CL_SUCCESS;
DirectConvPass* DCPass = (DirectConvPass*)plan->DirectConvpass;
	if (DCPass ) {

		size_t cpu_channel_pass_sz = plan->conv_NumChannels * sizeof(DirectCPUConvChannel);
		DCPass->channels = (DirectCPUConvChannel*)malloc(cpu_channel_pass_sz);
		if ( DCPass->channels ) {
			memset(DCPass->channels, 0, cpu_channel_pass_sz);
			for( int i = 0; i < plan->conv_NumChannels; i++ ) {
				DirectCPUConvChannel* chnl_conv = &DCPass->channels[i];
				// here we will keep prev block of incoming data
				chnl_conv->data_window = malloc(plan->frame_ln * sizeof(__FLOAT__));
				// here we will keep the first block of the conv kerenel
				chnl_conv->conv_window = malloc(plan->frame_ln * sizeof(__FLOAT__));
				if ( chnl_conv->data_window && chnl_conv->conv_window ) {
					memset(chnl_conv->data_window, 0, plan->frame_ln * sizeof(__FLOAT__));

				// copy the firts block of reverb filter
				// already prepared for us
					memcpy(chnl_conv->conv_window, (__FLOAT__*)convKernel + i * plan->frame_ln, plan->frame_ln * sizeof(__FLOAT__));
				}
			}
#if 0
// prepare channels
			if ( plan->conv_BitsPerSample == 16 && plan->conv_NumChannels <= 2 ) {
				DirectCPUConvChannel* chnl0 = &DCPass->channels[0];
				DirectCPUConvChannel* chnl1 = &DCPass->channels[1];
				uint * reverb = (uint*)plan->conv_kernel_sys;
				for ( int s = 0; s < plan->conv_kernel_ln; s++ ) {
	
					uint data = reverb[s];
					int left = ((int)(data & 0xffff)  << 16) >> 16;
					int right = (int)data >> 16;

					if ( s <  plan->frame_ln) {
						((__FLOAT__*)chnl0->conv_window)[s] = (__FLOAT__)left;
						((__FLOAT__*)chnl1->conv_window)[s] = (__FLOAT__)right;
					}

					chnl0->kernel_sum += left;
					chnl1->kernel_sum += right;
				}
			} else {
				printf ("ERROR: currently only processing stereo with 16 bit depth\n");
			}

#endif
		}


	}
	return err;
}


#if 0
int SetupCPUFFTConvKernel(ProjPlan * plan, void * convKernel) {
int err = CL_SUCCESS;
DirectConvPass* DCPass = (DirectConvPass*)plan->DirectConvpass;
	if (DCPass ) {

		size_t cpu_channel_pass_sz = plan->conv_NumChannels * sizeof(DirectCPUConvChannel);
		DCPass->channels = (DirectCPUConvChannel*)malloc(cpu_channel_pass_sz);
		if ( DCPass->channels ) {
			memset(DCPass->channels, 0, cpu_channel_pass_sz);
			for( int i = 0; i < plan->conv_NumChannels; i++ ) {
				DirectCPUConvChannel* chnl_conv = &DCPass->channels[i];
				// here we will keep prev block of incoming data
				chnl_conv->data_window = malloc(plan->frame_ln * sizeof(__FLOAT__));
				chnl_conv->FFT_data_window =  malloc(plan->frame_ln * 2 * sizeof(__FLOAT__) * 2);
				// here we will keep the first block of the conv kerenel
				chnl_conv->FFT_conv_window = malloc(plan->frame_ln * 2 * sizeof(__FLOAT__) * 2);
				if ( chnl_conv->data_window && chnl_conv->FFT_conv_window &&chnl_conv->FFT_data_window ) {
					memset(chnl_conv->data_window, 0, plan->frame_ln * sizeof(__FLOAT__));
					memset(chnl_conv->FFT_data_window, 0, plan->frame_ln * 2 * sizeof(__FLOAT__) * 2);

				// copy the first block of reverb filter
				// already prepared for us
					memcpy(chnl_conv->FFT_conv_window, (__FLOAT__*)convKernel + i * plan->frame_ln*2*2, plan->frame_ln * 2 * sizeof(__FLOAT__) * 2);
				}
			}
		}

		switch(plan->frame_ln) {
			case 64:
				init_vacfft64();
				break;
			case 256:
				init_vacfft256();
				break;
			case 1024:
				init_vacfft1024();
				break;
		}
		uint init_flags = SchedGetInitFlags(GetScheduler(plan));

		for( int i = 0; (init_flags & __INIT_FLAG_HETEROGEN__) && i < plan->conv_NumChannels; i++ ) {

			DirectCPUConvChannel* chnl_conv = &DCPass->channels[i];

			__FLOAT__ *FFT_IR = (__FLOAT__*)chnl_conv->FFT_conv_window;
// FFT		
			switch(plan->frame_ln) {
			case 64:
				vacfft64(FFT_IR, 0 );
				break;
			case 256:
				vacfft256(FFT_IR, 0 );
				break;
			case 1024:
				vacfft1024(FFT_IR, 0 );
				break;
			}
		}

	}
	return err;
}

#endif

int SetupCPUFHTConvKernel(ProjPlan * plan, void * convKernel) {
int err = CL_SUCCESS;
DirectConvPass* DCPass = (DirectConvPass*)plan->CPUConvpass;
int n_channels = plan->conv_NumChannels;
	if (DCPass ) {
// add direct filter setup
		size_t direct_channel_stride = plan->frame_ln;
		__FLOAT__ *t_kernel = (__FLOAT__*)malloc(n_channels * direct_channel_stride * sizeof(__FLOAT__));
		assert(t_kernel);
		for(int i = 0; i < n_channels; i++) {
			memcpy(t_kernel + i *direct_channel_stride, (__FLOAT__*)convKernel +i * direct_channel_stride *2, direct_channel_stride * sizeof(__FLOAT__));
		}
		SetupCPUDCKernel(plan, t_kernel);
		free(t_kernel);

		size_t cpu_channel_pass_sz = plan->conv_NumChannels * sizeof(DirectCPUConvChannel);
		DCPass->channels = (DirectCPUConvChannel*)malloc(cpu_channel_pass_sz);
		int N = plan->frame_ln * 2;
		if ( DCPass->channels ) {
			memset(DCPass->channels, 0, cpu_channel_pass_sz);
			for( int i = 0; i < plan->conv_NumChannels; i++ ) {
				DirectCPUConvChannel* chnl_conv = &DCPass->channels[i];
				// here we will keep prev block of incoming data
				chnl_conv->data_window = malloc((N/2) * sizeof(__FLOAT__));
				chnl_conv->FFT_data_window =  malloc(N * sizeof(__FLOAT__));
				chnl_conv->FHT_data2_window =  malloc(N * sizeof(__FLOAT__));
				// here we will keep the first block of the conv kerenel
				chnl_conv->FFT_conv_window = malloc(N * sizeof(__FLOAT__) );
				if ( chnl_conv->data_window && chnl_conv->FFT_conv_window &&chnl_conv->FFT_data_window ) {
					memset(chnl_conv->data_window, 0, (N/2) * sizeof(__FLOAT__));
					memset(chnl_conv->FFT_data_window, 0, N * sizeof(__FLOAT__));
					memset(chnl_conv->FHT_data2_window, 0, N * sizeof(__FLOAT__));

				// copy the first block of reverb filter
				// already prepared for us
					memcpy(chnl_conv->FFT_conv_window, (__FLOAT__*)convKernel + i * N, N *sizeof(__FLOAT__) );


				}
			}
		}
		DCPass->FHT_bitrvrs = malloc(sizeof(short) * N);
		DCPass->FHT_sincos = malloc(sizeof(__FLOAT__) * (N/2));
//		FHTInit((__FLOAT__**)&DCPass->FHT_sincos, (short**)&DCPass->FHT_bitrvrs, (FHT_FUNC*)&DCPass->FHT_routine, N);
		FHTInit2(DCPass->FHT_proc, (__FLOAT__**)&DCPass->FHT_sincos, (short**)&DCPass->FHT_bitrvrs, N);
		
		uint init_flags = SchedGetInitFlags(GetScheduler(plan));

		for( int i = 0; i < plan->conv_NumChannels; i++ ) {

			DirectCPUConvChannel* chnl_conv = &DCPass->channels[i];

			__FLOAT__ *FHT_IR = (__FLOAT__*)chnl_conv->FFT_conv_window;

// FHT
			FHTWithReverse((__FLOAT__*)DCPass->FHT_sincos, (short*)DCPass->FHT_bitrvrs, FHT_IR, N);
		}

	}
	return err;
}



int DirectConvGPUSetup(ProjPlan * plan) {
int err = CL_SUCCESS;
	return err;
}


static
int DirectCPUConvDeinitInternal(ProjPlan * plan, DirectConvPass* DCPass) {
	int err = 0;
		if ( DCPass->channels ) {
			for( int i = 0; i < plan->conv_NumChannels; i++ ) {
				DirectCPUConvChannel* chnl_conv = &DCPass->channels[i];
				if ( chnl_conv->data_window ) {
					free(chnl_conv->data_window);
				}
				if (chnl_conv->conv_window ) {

					free(chnl_conv->conv_window);
				}
				if ( chnl_conv->FFT_conv_window ) {
					free( chnl_conv->FFT_conv_window);
				}
				if ( chnl_conv->FFT_data_window ) {
					free(chnl_conv->FFT_data_window);
				}
				if ( chnl_conv->FHT_data2_window ) {
					free(chnl_conv->FHT_data2_window);
				}
			}
			free(DCPass->channels);
			DCPass->channels = 0;
			if ( DCPass->FHT_bitrvrs ) {
				free(DCPass->FHT_bitrvrs);
			}
			if ( DCPass->FHT_sincos) {
				free(DCPass->FHT_sincos);
			}
		}
		return(err);
}

int DirectConvCPUDeinit(ProjPlan * plan) {
int err = CL_SUCCESS;
    if ( plan->DirectConvpass ) {
		DirectConvPass* DCPass = (DirectConvPass*)plan->DirectConvpass;
		err = DirectCPUConvDeinitInternal(plan, DCPass);


		free(plan->DirectConvpass);
		plan->DirectConvpass = NULL;
	}
	if ( plan->CPUConvpass ) {
		err = DirectCPUConvDeinitInternal(plan, (DirectConvPass*)plan->CPUConvpass);
		free(plan->CPUConvpass);
		plan->CPUConvpass = 0;
	}
	return err;
}

int DirectConvGPUDeinit(ProjPlan * plan) {
int err = CL_SUCCESS;
    if ( plan->DirectConvpass ) {
		DirectConvCPUDeinit(plan);
// GPU Deinit HERE

		free(plan->DirectConvpass);
		plan->DirectConvpass = NULL;
	}

	
	return err;
}