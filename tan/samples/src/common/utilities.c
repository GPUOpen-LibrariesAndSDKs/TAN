//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
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

#include <math.h>
#include <omp.h>

int DirectConv(float * out, float * in, int index, int block_sz, float * kernel, int kernel_sz, int n_blocks)
{
    int ret = 0;
    int b, j, c;
    int in_sz = n_blocks * block_sz;
#pragma omp parallel for private(j,c)
    for (b = 0; b < block_sz; b++) {
        out[b] = 0;
        double o = 0;
        // limited by real kernel length
        for (j = index * block_sz + b, c = 0; c < kernel_sz; c++, j--) {
            j = (j < 0) ? in_sz - 1 : j;

            o += in[j] * kernel[c];

        }

        out[b] = (float)o;
    }

    return ret;
}

#if 0
const bool use_pattern = false;

static
int InsertInput(void *input_hist, int index, void * data, int data_ln) {
    int err = 0;

    float * coeff = (float*)(data);
    float * in_hist = (float*)input_hist + index * data_ln;
    memcpy(in_hist, data, data_ln * sizeof(float));


    return err;
}

static int adder = 0;
static
int GenRandomInput(void * data, int input_data_ln, int data_ln) {
    int err = 0;

    float * coeff = (float*)(data);
    memset(coeff, 0, data_ln*sizeof(float));
    for (int i = 0; i < input_data_ln; i++) {
        /*
        if ( i > 0 )
        {
        coeff[i] = 0;
        continue;
        }
        */
        coeff[i] = (2.0f * (float)rand() / (float)RAND_MAX - 1.0f);
    }

    //	adder++;
    return err;
}

static
int GenPatternInput(void * data, int input_data_ln, int data_ln, int offset)
{
    float *out = (float*)data;
    memset(out, 0, data_ln*sizeof(float));

    for (int i = 0; i < input_data_ln; i++)
    {
        out[i] = i + offset;
    }

    return 0;
}

static
int GenRandomFilter(void * filter, int filter_ln) {
    int err = 0;

    float * coeff = (float*)(filter);
    double norm2 = 0;
    for(int i = 0; i < filter_ln; i++ ) {
//		if (i< filter_ln - 64 * 2 + 1)
        {
            coeff[i] = (2.f * (float)rand() / (float)RAND_MAX - 1.0f);
        }
#if 0		
        else
        {
            coeff[i] = 0;
        }
        
#endif
//		norm2 += coeff[i] * coeff[i];
    }

/*
    for(int i = 0; i < filter_ln; i++ ) {
        coeff[i] /= norm2;
    }
*/


    return err;
}


#if 0
void fillInput(	std::vector<float*> inputs,
    std::vector<float*> input_accum,
    std::vector<int> n_input_accum_blcks,
    std::vector<void *> kernels_ptrs,
    std::vector<int> kernel_len,
    std::vector<float*> outputs_v,
    int proc_block_sz,
    __int64 p_count,
    int verify)
{
    static int round = 0;
    for(int i = 0; i < inputs.size(); i++)
    {
        GenRandomInput(inputs[i], proc_block_sz);
        if ( verify == 1 ) 
        {
            int index = (int)(p_count % n_input_accum_blcks[i]);
            InsertInput(input_accum[i], index, inputs[i], proc_block_sz);
            DirectConv(outputs_v[i], input_accum[i], index, proc_block_sz,(float*)kernels_ptrs[i],kernel_len[i],n_input_accum_blcks[i]);
        }
    }
    round++;
}

#endif
/*-----------------------------------------------------------------------------------------------
conv kernel upload
------------------------------------------------------------------------------------------------*/

typedef struct _upload_thread2{
    __int64 *upload_count;
    double *uploading_time;
    int n_channels;
    int kernel_upload_buffers;
    graal::CGraalConv * fhtConv;
    std::vector<void *> *kernels_ptrs;
    std::vector<int> *kernel_len;
    std::vector<int> *upload_id;
    std::vector<int> *kernel_id;
    std::vector<cl_mem> *kernel_mems;
    int *eo_upload_signal;
} upload_thread2;

typedef struct _upload_thread{
    __int64 upload_count;
    double uploading_time;
    int method;
    int n_delays_onconv_switch;
    int n_channels;
    int n_sets;
    int curr_set;
    int prev_set;
    int onconv_switch_delay_counter;
    int kernel_upload_buffers;
    int single_threaded;
    graal::CGraalConv * fhtConv;
    std::vector<std:: vector<void *>> kernels_ptrs;
    std::vector<std:: vector<int>> kernel_len;
    std::vector<std::vector<cl_mem>> kernel_mems;
    std::vector<std::vector<int>> upload_id;
    std::vector<std::vector<int>> kernel_id;
    std::vector<int> eo_upload;
    std::vector<pthread_mutex_t> conv_updt_guard;  // prevent conv update while teh conv still in use with the process call
    int eo_thread;
} upload_thread;


static
void convUpload(
    __int64 &upload_count,
    double & uploading_time,
    int n_channels,
    int kernel_upload_buffers,
    graal::CGraalConv & fhtConv,
    std::vector<void *> &kernels_ptrs,
    std::vector<int> &kernel_len,
    std::vector<int> &upload_id,
    std::vector<int> &kernel_id,
    std::vector<cl_mem> &kernel_mems,
    pthread_mutex_t & conv_updt_guard
    )
{
    double t0, t1;

    if (kernel_upload_buffers == 2 )
    {
        fhtConv.getConvBuffers(n_channels,&upload_id[0], &kernel_id[0], (float**)&kernels_ptrs[0]);
    }

    for(int i = 0; i < n_channels; i++)
    {
        if (use_pattern)
            GenPatternInput(kernels_ptrs[i], kernel_len[i], kernel_len[i], 0);
        else
            GenRandomFilter(kernels_ptrs[i], kernel_len[i]);
    }

    if (kernel_upload_buffers == 1 )
    {
    // user managed OCL buffers
    // this is an emulation
    // a user manages OCL buffer and generate conv in time domain and upload into OCL buffers before hands
        fhtConv.getConvBuffers(n_channels,&upload_id[0], &kernel_id[0], &kernel_mems[0]);
        fhtConv.uploadConvHostPtrs(n_channels, &upload_id[0], &kernel_id[0], (const float**)&kernels_ptrs[0], &kernel_len[0], true);

    }

    if (kernel_upload_buffers == 3 )
    {
    // library managed OCL buffers

    // called to obtain ocl buffers from Graal before user upload conv into them 
        fhtConv.getConvBuffers(n_channels,&upload_id[0], &kernel_id[0], &kernel_mems[0]);
        fhtConv.uploadConvHostPtrs(n_channels, &upload_id[0], &kernel_id[0], (const float**)&kernels_ptrs[0], &kernel_len[0], true);

    }


    t0 = mach_absolute_time();

//	pthread_mutex_lock(&conv_updt_guard);
    if (kernel_upload_buffers == 0 )
    {
        // udate conv from host system ptrs
        fhtConv.updateConvHostPtrs(n_channels, &upload_id[0], &kernel_id[0], (const float**)&kernels_ptrs[0], &kernel_len[0], true);

    }
    else if ( kernel_upload_buffers == 1 )
    {
        // update from user managed OCL buffers 
        fhtConv.updateConv(n_channels, &upload_id[0], &kernel_id[0], &kernel_mems[0], &kernel_len[0], true);

//		fhtConv.finishUpdate();

    }
    else if ( kernel_upload_buffers == 2 )
    {
        // update from GPU-friendly host system ptrs
        fhtConv.updateConv(n_channels, &upload_id[0], &kernel_id[0], (const float**)&kernels_ptrs[0], &kernel_len[0], true);

//		fhtConv.finishUpdate();
    }
    else  //( kernel_upload_buffers == 3 )
    {
        // update from lib managed buffers
        fhtConv.updateConv(n_channels, &upload_id[0], &kernel_id[0], &kernel_len[0], true);

    }
//	pthread_mutex_unlock(&conv_updt_guard);


    t1 = mach_absolute_time(); 
    if ( upload_count > 0 ) 
    {
        uploading_time += subtractTimes(t1, t0);
    }


    upload_count++;

}



static
void * convUploadRoutine( void * _upload_ctl )
{

    int set = 0;

    upload_thread * upload_ctl = (upload_thread *)_upload_ctl;
    do {

        set = upload_ctl->curr_set;
        set++;
        set = (set >= upload_ctl->n_sets) ? 0 : set;
        pthread_mutex_lock(&upload_ctl->conv_updt_guard[set]);

        if (!upload_ctl->eo_upload[set])
        {
            for (int i = 0; i < upload_ctl->n_channels; i++)
            {
                upload_ctl->upload_id[set][i] = set;
            }
            convUpload(
                upload_ctl->upload_count,
                upload_ctl->uploading_time,
                upload_ctl->n_channels,
                upload_ctl->kernel_upload_buffers,
                *upload_ctl->fhtConv,
                upload_ctl->kernels_ptrs[set],
                upload_ctl->kernel_len[set],
                upload_ctl->upload_id[set],
                upload_ctl->kernel_id[set],
                upload_ctl->kernel_mems[set],
                upload_ctl->conv_updt_guard[set]
                );

            upload_ctl->eo_upload[set] = 1;
            upload_ctl->curr_set = set;


            printf("upload set %d\n", set);

        }
        pthread_mutex_unlock(&upload_ctl->conv_updt_guard[set]);


        if (!upload_ctl->single_threaded)
        {
            Sleep(100);
        }


    } while (!upload_ctl->eo_thread);

    return (NULL);
}


/*------------------------------------------------------------------------------------------------
processing loop
-------------------------------------------------------------------------------------------------*/
static
void processingLoop(
    upload_thread &upload_ctl,
    __int64 & n_actual_loops,
    double & processing_time,
    int ext_verify,
    int n_channels,
    int input_block_sz,
    int proc_block_sz,
    int process_dev_buffers,
    std::vector<std::vector<int>> &process_upload_id,
    std::vector<std::vector<int>> &process_kernel_id,
    std::vector<std::vector<float*>> &inputs,
    std::vector<std::vector<float*>> &outputs,
    std::vector<std::vector<int>> &n_input_accum_blcks,
    std::vector<std::vector<float*>> &input_accum,
    std::vector<std::vector<float*>> &outputs_v

    )
{
    double t0, t1;

    int sample_mismatch = 0;
    int set = upload_ctl.curr_set;
    int prev_set = upload_ctl.prev_set;

    graal::CGraalConv & fhtConv = *upload_ctl.fhtConv;

    std::vector<void *> &kernels_ptrs = upload_ctl.kernels_ptrs[set];
    std::vector<int> &kernel_len = upload_ctl.kernel_len[set];
    pthread_mutex_t & conv_updt_guard = upload_ctl.conv_updt_guard[set];


    for( int c = 0; c < n_channels && !sample_mismatch; c++)
    {
        if (use_pattern)
            GenPatternInput(inputs[0][c], input_block_sz, proc_block_sz, c * 10000);
        else
//			GenRandomInput(inputs[0][c], proc_block_sz);
            GenRandomInput(inputs[0][c], input_block_sz, proc_block_sz);

    }

    t0 = mach_absolute_time();

    int n_real_rounds = 0;
// precompute cpu version
    if (ext_verify) 
    {
        for (int c = 0; c < n_channels && !sample_mismatch; c++)
        {
            int uploadId = process_upload_id[set][c];
            int convId = process_kernel_id[set][c];
            n_real_rounds = fhtConv.getRoundCounter(0, convId);
            int index = (int)(n_real_rounds % n_input_accum_blcks[uploadId][convId]);
            InsertInput(input_accum[0][convId], index, inputs[0][c], proc_block_sz);
            if (set == prev_set)
            {
                DirectConv(outputs_v[set][c], input_accum[0][convId], index, proc_block_sz, (float*)kernels_ptrs[c], kernel_len[c], n_input_accum_blcks[uploadId][convId]);
            }
            else
            {
                switch (upload_ctl.method)
                {
                case graal::ALG_UNIFORMED:
                default:
                    DirectConv(outputs_v[prev_set][c], input_accum[0][convId], index, proc_block_sz, (float*)upload_ctl.kernels_ptrs[prev_set][c], upload_ctl.kernel_len[prev_set][c], n_input_accum_blcks[uploadId][convId]);
                    DirectConv(outputs_v[set][c], input_accum[0][convId], index, proc_block_sz, (float*)kernels_ptrs[c], kernel_len[c], n_input_accum_blcks[uploadId][convId]);

                    for (int i = 0; i < proc_block_sz; i++)
                    {
                        outputs_v[set][c][i] = (outputs_v[prev_set][c][i] * (float)i + outputs_v[set][c][i] * (float)(proc_block_sz - i)) / (float)proc_block_sz;
                    }
                    break;
                case graal::ALG_UNI_HEAD_TAIL:
                    if (upload_ctl.onconv_switch_delay_counter == 0)
                    {
                    // additional previous run to get toa second stage
                        DirectConv(outputs_v[set][c], input_accum[0][convId], index, proc_block_sz, (float*)upload_ctl.kernels_ptrs[prev_set][c], upload_ctl.kernel_len[prev_set][c], n_input_accum_blcks[uploadId][convId]);
                    }
                    if (upload_ctl.onconv_switch_delay_counter == 1)
                    {
                        DirectConv(outputs_v[prev_set][c], input_accum[0][convId], index, proc_block_sz, (float*)upload_ctl.kernels_ptrs[prev_set][c], upload_ctl.kernel_len[prev_set][c], n_input_accum_blcks[uploadId][convId]);
                        DirectConv(outputs_v[set][c], input_accum[0][convId], index, proc_block_sz, (float*)kernels_ptrs[c], kernel_len[c], n_input_accum_blcks[uploadId][convId]);
#if 1
                        for (int i = 0; i < proc_block_sz; i++)
                        {
                            outputs_v[set][c][i] = (outputs_v[prev_set][c][i] * (float)i + outputs_v[set][c][i] * (float)(proc_block_sz - i)) / (float)proc_block_sz;
                        }
#endif
                    }
                    break;
                }
                
            }
        }
    }

    pthread_mutex_lock(&conv_updt_guard);

    if (set == prev_set)
    {
        fhtConv.process(n_channels, &process_upload_id[set][0], &process_kernel_id[set][0], &inputs[0][0], &outputs[set][0]);
    }
    else
    {
        switch (upload_ctl.method)
        {
        case graal::ALG_UNIFORMED:
        default:
            // prev conv, do not advance time
            fhtConv.process(n_channels, &process_upload_id[prev_set][0], &process_kernel_id[prev_set][0], &inputs[0][0], &outputs[prev_set][0], 0, 0);
            // new conv, previous input advance time
            fhtConv.process(n_channels, &process_upload_id[set][0], &process_kernel_id[set][0], &inputs[0][0], &outputs[set][0], 1);
            for (int c = 0; c < n_channels && !sample_mismatch; c++)
            {
                for (int i = 0; i < proc_block_sz; i++)
                {
                    outputs[set][c][i] = (outputs[prev_set][c][i] * (float)i + outputs[set][c][i] * (float)(proc_block_sz - i)) / (float)proc_block_sz;
                }
            }

            // free conv slot for the next upload
            upload_ctl.prev_set = upload_ctl.curr_set;
            upload_ctl.eo_upload[prev_set] = 0;
            break;
        case graal::ALG_UNI_HEAD_TAIL:
            if (upload_ctl.onconv_switch_delay_counter == 0)
            {
                // previous conv run, return data, advance the internal Graal time
                fhtConv.process(n_channels, &process_upload_id[prev_set][0], &process_kernel_id[prev_set][0], &inputs[0][0], &outputs[set][0]);
                // new conv run, do not return data, skip the first (head ) stage, do not advance the timer : flash the previous switch state
                fhtConv.process(n_channels, &process_upload_id[set][0], &process_kernel_id[set][0], &inputs[0][0], &outputs[prev_set][0], 1, 0/*, 1*/);
#if 0
                if (ext_verify)
                {
                    int sample_mismatch = 0;
                    for (int c = 0; c < n_channels && !sample_mismatch; c++)
                    {
                        for (int i = 0; i < proc_block_sz/* && !sample_mismatch*/; i++)
                        {
                            float c_val = outputs_v[set][c][i];
                            float g_val = outputs[set][c][i];
                            if (!_isnan(c_val) && !_isnan(g_val) && abs(c_val - g_val) > 0.01)
                            {
                                printf("Prev mismatch s=%d r=%d rr=%d c=%d i=%d c_v=%f g_v=%f\n", set, (int)n_actual_loops, n_real_rounds, c, i, c_val, g_val);
                                sample_mismatch = 1;
                            }
                        }
                    }
                    if (sample_mismatch) exit(0);
                    //					print_interval = 2;
                }
#endif

            }
            if (upload_ctl.onconv_switch_delay_counter == 1)
            {
                // last previous conv run, return data, do not advance the internal Graal time (skip 2nd stage)
                fhtConv.process(n_channels, &process_upload_id[prev_set][0], &process_kernel_id[prev_set][0], &inputs[0][0], &outputs[prev_set][0], 0, 0);
                // new conv run, return data, previous input, advance the timer
                fhtConv.process(n_channels, &process_upload_id[set][0], &process_kernel_id[set][0], &inputs[0][0], &outputs[set][0],1);
#if 0
                if (ext_verify) 
                {
                    int sample_mismatch = 0;
                    for( int c = 0; c < n_channels && !sample_mismatch; c++)
                    {
                        for(int i = 0; i < proc_block_sz/* && !sample_mismatch*/; i++)
                        {
                            float c_val = outputs_v[prev_set][c][i];
                            float g_val = outputs[prev_set][c][i];
                            if (!_isnan(c_val) && !_isnan(g_val) && abs(c_val - g_val) > 0.01)
                            {
                                printf("Prev mismatch s=%d r=%d rr=%d c=%d i=%d c_v=%f g_v=%f\n", set, (int)n_actual_loops, n_real_rounds, c, i, c_val, g_val);
                                sample_mismatch = 1;
                            }
                        }
                    }
                    if (sample_mismatch) exit(0);
//					print_interval = 2;
                }
#endif


#if 1
                for (int c = 0; c < n_channels && !sample_mismatch; c++)
                {
                    for (int i = 0; i < proc_block_sz; i++)
                    {
                        outputs[set][c][i] = (outputs[prev_set][c][i] * (float)i + outputs[set][c][i] * (float)(proc_block_sz - i)) / (float)proc_block_sz;
                    }
                }
#endif
                // free conv slot for the next upload
                upload_ctl.prev_set = upload_ctl.curr_set;
                upload_ctl.eo_upload[prev_set] = 0;

            }
            upload_ctl.onconv_switch_delay_counter++;
            upload_ctl.onconv_switch_delay_counter = (upload_ctl.onconv_switch_delay_counter >= upload_ctl.n_delays_onconv_switch) ? 0 : upload_ctl.onconv_switch_delay_counter;

            break;
        }
    }


    pthread_mutex_unlock(&conv_updt_guard);


    t1 = mach_absolute_time(); 
    if (n_actual_loops > 0 ) {
        processing_time += subtractTimes(t1, t0);
    }

    int print_interval = 100;
#if 1
    if (ext_verify) 
    {

        for( int c = 0; c < n_channels && !sample_mismatch; c++)
        {
            for (int i = 0; i < input_block_sz && !sample_mismatch; i++)
            {
                float c_val = outputs_v[set][c][i];
                float g_val = outputs[set][c][i];
                if ( !_isnan( c_val) && !_isnan(g_val) && abs(c_val - g_val) > 0.01 )
                {
                    printf("Pipeline mismatch s=%d r=%d rr=%d c=%d i=%d c_v=%f g_v=%f\n", set, (int)n_actual_loops, n_real_rounds, c, i, c_val, g_val);
                    sample_mismatch = 1;
//					exit(0);
                }
            }
        }
        print_interval = 2;
    }
#endif
    if (!sample_mismatch && ( n_actual_loops % print_interval ) == 0 && n_actual_loops > 0 ) 
    {
        printf("Passed set %d round %d\n", set, (int)n_actual_loops);
    }
    else if ( sample_mismatch)
    {
//		exit(0);
    }


    n_actual_loops++;
}
    









static
void Usage(void ){
    printf(">amdrvrbdrv.ex [arguments]\n");
    printf("arguments:\n");
    printf("-b_sz <block size>\n");
    printf("-s_r <sample rate>\n");
    printf("-n_l <stream duration in blocks>\n");
    printf("-n_i <# of instances>\n");
    printf("-i_f <input data> (duration in sec).\n");
    printf("-k_f <kernel data> (duration in sec).\n");
    printf("-alg 0|1|2       0,1 - classic, 2 - head-tail.\n");
    printf("-fft         use FFT transform (classic), otherwise FHT.\n");
//	printf("-fir_g <filter size> - FIR filter size( separate pipeline).\n");
    printf("-v_e <0|1>     - external verification with per sample matching(1).\n");
//	printf("-v_i <0|1|2|3> - internal verification, stream 1st only(1), 2nd only(2), both(3).\n");
    printf("-kub <0|1|2|3> - kernel upload type 0: host ptr 1: client OCL 2: device ptr 3: lib OCL");
    printf("-mt - use multi-threaded test");
}

#define __MAX_VST_BLOCKS__  2048
#define __DEFAULT_STREAM_DIR__ "\\Users\\alyashev\\Music\\"

int main( int argc, char* argv[] )
{
int n_instances = 1;
__int64 num_offline_loops = 10;
__int64 num_conv_updates;
int block_size = 1024;
int sample_rate = 48000;
int per_sample_match = 0;
//int FHT_2streams = __INIT_FLAG_2STREAMS__;
//int heterogen = 0; //__INIT_FLAG_HETEROGEN__;
//int fht = __INIT_FLAG_FHT__;
int fft = 0; //__INIT_FLAG_FFT__
//int fir = 0; // __INIT_FLAG_FIR__
int ext_verify = 0;
int verification = 0; //__INIT_FLAG_VER_TRANSFORM1__; // | __INIT_FLAG_VER_TRANSFORM2__;
const char * input_fl = NULL;
const char * kern_fl = NULL;
char * inp_file = NULL;
char * kern_file = NULL;
int n_input_channels = 1;
int n_sub_channels = 2; // 1 - mono, 2 - stereo
float secs = 2;
float in_secs = 0;
int n_firs = 0;
//int fir_sz[64];
//float * fir_data[64];
int run_flags = 0;//__PROCESSING_FLAG_VERIFY_TRANSFORM__;
int out_file = 0;
int process_dev_buffers = 0;
int kernel_upload_buffers = 3; // 0 - host ptrs, 1 - client OCL buffers, 2 - dev ptrs, 3 - lib OCL buffers
int single_threaded = 1; // 0 - multi, 1 - single
int method = 0;


    for ( int i = 0; i < argc; i++ ) {

        if ( !strcmp(argv[i], "-i_f") && i < argc -1 && argv[i + 1] != 0) {
            if ( isdigit(argv[i+1][0]) ) {
                input_fl = NULL;
                in_secs = atof(argv[++i]);

            } else {
                size_t nm_len= strlen(argv[i + 1]);
                inp_file = (char*)malloc(nm_len + 1);
                assert(inp_file);
                strcpy(inp_file,argv[++i]);
                input_fl = inp_file;
            }
        }
        else
        if ( !strcmp(argv[i], "-k_f") && i < argc -1 && argv[i + 1] != 0) {

            if ( isdigit(argv[i+1][0]) ) {
                kern_fl = NULL;
                secs = atof(argv[++i]);

            } else {
                size_t nm_len= strlen(argv[i + 1]);
                kern_file = (char*)malloc(nm_len + 1);
                assert(kern_file);
                strcpy(kern_file,argv[++i]);
                kern_fl = kern_file;
            }
        }
#if 0
        else
        if ( !strcmp(argv[i], "-fir_g") && i < argc - 1 && argv[i + 1] != 0) {
            if ( isdigit(argv[i+1][0]) ) {
                fir_sz[n_firs++] = atoi(argv[++i]);
            }
            fir = __INIT_FLAG_FIR__;
        }
#endif
        else
        if ( !strcmp(argv[i], "-n_i") && i < argc -1 && argv[i + 1] != 0) {
            n_instances = atoi(argv[++i]);
        }
        else
        if ( !strcmp(argv[i], "-s_r") && i < argc -1 && argv[i + 1] != 0) {
            sample_rate = atoi(argv[++i]);
        }
        else
        if ( !strcmp(argv[i], "-n_l") && i < argc -1 && argv[i + 1] != 0) {
            num_offline_loops = atoi(argv[++i]);
        }
        else
        if ( !strcmp(argv[i], "-v_e") && i < argc -1 && argv[i + 1] != 0) {
            ext_verify = atoi(argv[++i]);
    //		per_sample_match = atoi(argv[++i]);
        }
#if 0
        else
        if ( !strcmp(argv[i], "-v_i") && i < argc -1 && argv[i + 1] != 0) {
            int v = atoi(argv[++i]);
            run_flags = (v == 1) ? __PROCESSING_FLAG_VERIFY_TRANSFORM1__ : (v==2) ? __PROCESSING_FLAG_VERIFY_TRANSFORM2__ : (v==3) ? __PROCESSING_FLAG_VERIFY_TRANSFORM1__ | __PROCESSING_FLAG_VERIFY_TRANSFORM1__ : 0;
        }
#endif
        else
        if ( !strcmp(argv[i], "-b_sz") && i < argc -1 && argv[i + 1] != 0) {
            block_size = atoi(argv[++i]);
        }
        else 
        if (!strcmp(argv[i], "-alg") && i < argc - 1 && argv[i + 1] != 0) {
            method = atoi(argv[++i]);
        }
        else
            if (!strcmp(argv[i], "-m_t") && i < argc - 1 && argv[i + 1] != 0) {
                single_threaded = (atoi(argv[++i]) == 1) ? 0 : 1;
            }
        else
        if ( !strcmp(argv[i], "-fft") ) {
            fft = true;
        }

        else
            if (!strcmp(argv[i], "-alg") && i < argc - 1 && argv[i + 1] != 0) {
                method = atoi(argv[++i]);
            }
        else
        if (!strcmp(argv[i], "-mt")) {
            single_threaded = 0;
        }
        else
        if (!strcmp(argv[i], "-kub") && i < argc - 1 && argv[i + 1] != 0) {
            kernel_upload_buffers = atoi(argv[++i]);
        }
        else
        if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "-help")) {
            Usage();
            exit(0);
        }
        else if ( i > 0 ) {
            printf ("Uknown argument: %s. Bailing out!\n", argv[i]);
            Usage();
            exit(-1);
        }
    }

    input_fl = (input_fl==NULL && in_secs == 0)? "clicks" : input_fl;
    kern_fl = (kern_fl==NULL && secs == 0) ? ((ext_verify || verification ) ? "Nice Drum Room": "ad_10sec_48k") : kern_fl; //"St Nicolaes Church";

    char input_str[1024];
    if ( input_fl ) {
        strcpy(input_str, input_fl);
    } else {
        sprintf(input_str, "%5.1f", in_secs);
    }

    char kernel_str[1024];
    if ( kern_fl ) {
        strcpy(kernel_str, kern_fl);
    } else {
        sprintf(kernel_str, "%5.1f", secs);
    }


    std:: string meth_s;

    meth_s = ((method == graal::ALG_ANY || method == graal::ALG_UNI_HEAD_TAIL) && !fft) ? "uniform head-tail" : "uniform classic";
    printf("AMD Graal library. Transform %s\n", (fft)? "FFT" : "FHT");
    printf("Arguments:\n");
    printf("method %34s\n", meth_s.c_str());
    printf("multi-threaded %26s\n", (single_threaded == 1) ? "no" : "yes");
    printf("block %35d\n", block_size);
    printf("sample rate %29d\n",sample_rate);
    printf("stream duration in blocks %15d\n",num_offline_loops);
    printf("stream duration in sec %18.1f\n",(float)(num_offline_loops * block_size) / (float)sample_rate);
    printf("# of instances %26d\n",n_instances);
    printf("input data %*s\n", 30, input_str);
    printf("kernel data %*s\n", 29, kernel_str);
    printf("external verification %*s\n", 19, (ext_verify == 1) ? "yes" : "no");



int err = 0;
int n_samples = block_size;
//int n_kernel_samples;
int bitPersample = 16;
int n_channels = n_input_channels * n_sub_channels;


size_t kern_ln = 0;
double processing_time = 0.;
double overlaping_processing_time = 0;
double transfer_time = 0;

    kern_ln = (size_t)(secs * sample_rate);

    processing_time = 0.;
    transfer_time = 0;


    int n_sets = 2;
    n_channels = n_instances*n_sub_channels;
    upload_thread upload_ctl;
    memset(&upload_ctl, 0, sizeof(upload_thread));
    upload_ctl.method = method; // graal::ALG_UNI_HEAD_TAIL; // ALG_UNIFORMED;
    upload_ctl.n_sets = n_sets;
    // to make it 0 on the first step;
    upload_ctl.curr_set = upload_ctl.n_sets;
    upload_ctl.n_channels = n_channels;
    upload_ctl.single_threaded = single_threaded;
    upload_ctl.upload_id.resize(n_sets);
    upload_ctl.kernel_id.resize(n_sets);
    upload_ctl.eo_upload.resize(n_sets);
    upload_ctl.kernels_ptrs.resize(n_sets);
    upload_ctl.kernel_len.resize(n_sets);
    upload_ctl.kernel_mems.resize(n_sets);
    upload_ctl.conv_updt_guard.resize(n_sets);
    switch (upload_ctl.method)
    {
    case graal::ALG_UNI_HEAD_TAIL:
        upload_ctl.n_delays_onconv_switch = 2;
        break;
    case graal::ALG_UNIFORMED:
    case graal::ALG_ANY:
    default:
        upload_ctl.n_delays_onconv_switch = 0;
        break;
    }

    std::vector<std::vector<int>> process_upload_id(n_sets);
    std::vector<std::vector<int>> process_kernel_id(n_sets);

    std::vector<std::vector<float*>> inputs(n_sets);
    std::vector<std::vector<float*>> input_accum(n_sets);
    std::vector<std::vector<int>> n_input_accum_blcks(n_sets);
    std::vector<std::vector<float*>> outputs(n_sets);
    std::vector<std::vector<float*>> outputs_v(n_sets);

    

// instantiate grral conv library
    if (fft)
    {
        printf("Using the clFFT based convolution\n");
        upload_ctl.fhtConv = new graal::CGraalConv_clFFT;
    }
    else
    {
        printf("Using the FHT based convolution\n");
        upload_ctl.fhtConv = new graal::CGraalConv;
    }
// initialize the library
    upload_ctl.fhtConv->initializeConv(n_channels, (int)kern_ln, block_size, n_sets
                                                    , upload_ctl.method
                                                    );

//	exit(0);

    int proc_block_sz = upload_ctl.fhtConv->getInputBlockSz();

    // interface type
    upload_ctl.kernel_upload_buffers = kernel_upload_buffers;



    for( int j = 0; j < n_sets; j++)
    {

        upload_ctl.eo_upload[j] = 0;

// initialize guards
        pthread_mutex_init (&upload_ctl.conv_updt_guard[j], NULL);

        upload_ctl.kernel_id[j].resize(n_channels);
        upload_ctl.upload_id[j].resize(n_channels);
        process_upload_id[j].resize(n_channels);
        process_kernel_id[j].resize(n_channels);

        upload_ctl.kernels_ptrs[j].resize(n_channels);
        upload_ctl.kernel_len[j].resize(n_channels);
// host arbitrary pointers
        inputs[j].resize(n_channels);
        outputs[j].resize(n_channels);
// OCL mems
        upload_ctl.kernel_mems[j].resize(n_channels);

        outputs_v[j].resize(n_channels);
        n_input_accum_blcks[j].resize(n_channels);
        input_accum[j].resize(n_channels);

        for(int i = 0; i < n_channels; i++)
        {

            upload_ctl.kernel_id[j][i] = i;
            upload_ctl.upload_id[j][i] = j;
            process_upload_id[j][i] = j;
            process_kernel_id[j][i] = i;

            upload_ctl.kernel_len[j][i] = (int)kern_ln;
// host arbitrary pointers
            if ( !process_dev_buffers ) 
            {
                inputs[j][i] = (float*)malloc(proc_block_sz* sizeof(float));
                outputs[j][i] = (float*)malloc(proc_block_sz* sizeof(float));
            }

            outputs_v[j][i] = (float*)malloc(proc_block_sz* sizeof(float));
            n_input_accum_blcks[j][i] = 1 + (upload_ctl.kernel_len[j][i] + proc_block_sz - 1) / proc_block_sz;
            input_accum[j][i] = (float*)malloc(n_input_accum_blcks[j][i] * proc_block_sz * sizeof(float));
            memset(input_accum[j][i], 0, n_input_accum_blcks[j][i] * proc_block_sz * sizeof(float));
        }

    }

// upload kernels
    if (kernel_upload_buffers == 0 || kernel_upload_buffers == 1 || kernel_upload_buffers == 3)
    {
        for( int j = 0; j < n_sets; j++)
        {
            for (int i = 0; i < upload_ctl.kernels_ptrs[j].size(); i++) 
            {
                upload_ctl.kernels_ptrs[j][i] = malloc(upload_ctl.kernel_len[j][i] * sizeof(float));
            }
        }
    }




// process device buffers
/*
    if ( process_dev_buffers )
    {
        fhtConv.getDevInputPtrs(n_channels, 0, &kernel_id[0], &inputs[0][0]);
    }
*/
    pthread_t t1;
    __int64 n_process_loops = 1;

    if (!upload_ctl.single_threaded)
    {
        num_conv_updates = num_offline_loops;
        upload_ctl.eo_thread = 0;
        pthread_create(&t1, NULL, convUploadRoutine, &upload_ctl);
    }
    else
    {
        upload_ctl.eo_thread = 1;
        num_conv_updates = num_offline_loops / 40;
        num_conv_updates =	(0 < num_conv_updates) ? num_conv_updates : 1;
        n_process_loops = num_offline_loops / num_conv_updates;
    }

    int sample_mismatch = 0;
    __int64 r_count = 0;
#if 1
    if (upload_ctl.single_threaded && !ext_verify)
    {
            convUploadRoutine(&upload_ctl);
    }
#endif

    for( __int64 u_count = 0; u_count < num_conv_updates + 1 && !sample_mismatch; u_count++)
    {
#if 1
        if (upload_ctl.single_threaded && ext_verify)
        {
            convUploadRoutine(&upload_ctl);
        }
#endif
        int i = 0;
        while (true )
        {
                if (upload_ctl.curr_set < upload_ctl.n_sets && upload_ctl.eo_upload[upload_ctl.curr_set])
                {
//					pthread_mutex_lock(&upload_ctl.conv_updt_guard[j]);
                    for(int k = 0; k < upload_ctl.n_channels;k++)
                    {
                        process_upload_id[upload_ctl.curr_set][k] = upload_ctl.upload_id[upload_ctl.curr_set][k];
                    }

                    processingLoop(
                        upload_ctl,
                        r_count,
                        processing_time,
                        ext_verify,
                        n_channels,
                        block_size,
                        proc_block_sz,
                        process_dev_buffers,
                        process_upload_id,
                        process_kernel_id,
                        inputs,
                        outputs,
                        n_input_accum_blcks,
                        input_accum,
                        outputs_v
                    );
//					pthread_mutex_unlock(&upload_ctl.conv_updt_guard[j]);

                }
                
                if (!upload_ctl.single_threaded)
                {
                    Sleep(0);
                }
                if (upload_ctl.single_threaded)
                {
                    i++;
                    if (i >= n_process_loops)
                    {
                        break;
                    }
                }
                else if (r_count >= num_offline_loops)
                {
                    break;
                }
        }
    }

    if (!upload_ctl.single_threaded)
    {

        upload_ctl.eo_thread = 1;
        pthread_join(t1, NULL);
    }



    if (kernel_upload_buffers == 0 || kernel_upload_buffers == 1 || kernel_upload_buffers == 3)
    {
        for( int j = 0; j < n_sets; j++)
        {
            for(int i = 0; i < upload_ctl.kernels_ptrs[j].size(); i++)
            {
                free(upload_ctl.kernels_ptrs[j][i]);
            }
        }
    }

    for( int j = 0; j < n_sets; j++)
    {
        pthread_mutex_destroy (&upload_ctl.conv_updt_guard[j]);

        for(int i = 0; i < inputs[j].size(); i++)
        {
            if (!process_dev_buffers)
            {
                free(inputs[j][i]);
                free(outputs[j][i]);
            }
            free(input_accum[j][i]);
            free(outputs_v[j][i]);
        }
    }

    delete upload_ctl.fhtConv;

    __int64 upload_count = (upload_ctl.upload_count - 1 <= 0) ? 1 : (upload_ctl.upload_count - 1);
    printf( "Channels: %d upload:%6.2fms processing:%6.2fms\n",
        n_channels,
        (upload_ctl.uploading_time/upload_count),
        (processing_time/(double) r_count)
        );



    return(0);
}

#endif