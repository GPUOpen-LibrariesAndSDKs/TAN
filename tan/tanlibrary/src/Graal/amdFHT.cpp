
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

#include "GraalConv.hpp"
#include "amdFHT.h"
#define PI (__FLOAT__) 3.141592653589793238462643
#define TWOPI (__FLOAT__) PI*2.0
#define PI2 (__FLOAT__) PI/2.0
#define PI4 (__FLOAT__) PI/4.0

static
void FHT16(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT16prll(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT32(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT64(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT128(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT128prll(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT256(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT512(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT512prll(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT1024(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT2048(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);
static
void FHT4096(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr);


static
void FHT16RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
static
void FHTMAD16RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);


static
void FHT32RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
static
void FHTMAD32RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);

static
void FHT64RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
static
void FHTMAD64RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);


static
void FHT128RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
static
void FHTMAD128RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);

static
void FHT256RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
static
void FHTMAD256RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);

static
void FHT512RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
static
void FHTMAD512RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);

static
void FHT1024RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
static
void FHTMAD1024RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);
static
void FHT2048RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
static
void FHTMAD2048RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);

static
void FHT4096RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
static
void FHTMAD4096RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);

static short btrvrs16[16] = { 0, 8,  4, 12,
							2, 10, 6, 14,
							1, 9,  5, 13,
							3, 11, 7, 15};

static short btrvrs32[32] = { 0, 16, 8,  24, 4, 20, 12, 28,
							2, 18, 10, 26, 6, 22, 14, 30,
							1, 17, 9,  25, 5, 21, 13, 29,
							3, 19, 11, 27, 7, 23, 15, 31};
static short btrvrs64[64] = {0,32,16,48,8,40,24,56,
							4,36,20,52,12,44,28,60,
							2,34,18,50,10,42,26,58,
							6,38,22,54,14,46,30,62,
							1,33,17,49,9,41,25,57,
							5,37,21,53,13,45,29,61,
							3,35,19,51,11,43,27,59,
							7,39,23,55,15,47,31,63};
static short btrvrs128[128] = {0,64,32,96,16,80,48,112,
								8,72,40,104,24,88,56,120,
								4,68,36,100,20,84,52,116,
								12,76,44,108,28,92,60,124,
								2,66,34,98,18,82,50,114,
								10,74,42,106,26,90,58,122,
								6,70,38,102,22,86,54,118,
								14,78,46,110,30,94,62,126,
								1,65,33,97,17,81,49,113,
								9,73,41,105,25,89,57,121,
								5,69,37,101,21,85,53,117,
								13,77,45,109,29,93,61,125,
								3,67,35,99,19,83,51,115,
								11,75,43,107,27,91,59,123,
								7,71,39,103,23,87,55,119,
								15,79,47,111,31,95,63,127};

static short btrvrs256[256] = {0,128,64,192,32,160,96,224,
	16,144,80,208,48,176,112,240,
	8,136,72,200,40,168,104,232,
	24,152,88,216,56,184,120,248,
	4,132,68,196,36,164,100,228,
	20,148,84,212,52,180,116,244,
	12,140,76,204,44,172,108,236,
	28,156,92,220,60,188,124,252,
	2,130,66,194,34,162,98,226,
	18,146,82,210,50,178,114,242,
	10,138,74,202,42,170,106,234,
	26,154,90,218,58,186,122,250,
	6,134,70,198,38,166,102,230,
	22,150,86,214,54,182,118,246,
	14,142,78,206,46,174,110,238,
	30,158,94,222,62,190,126,254,
	1,129,65,193,33,161,97,225,
	17,145,81,209,49,177,113,241,
	9,137,73,201,41,169,105,233,
	25,153,89,217,57,185,121,249,
	5,133,69,197,37,165,101,229,
	21,149,85,213,53,181,117,245,
	13,141,77,205,45,173,109,237,
	29,157,93,221,61,189,125,253,
	3,131,67,195,35,163,99,227,
	19,147,83,211,51,179,115,243,
	11,139,75,203,43,171,107,235,
	27,155,91,219,59,187,123,251,
	7,135,71,199,39,167,103,231,
	23,151,87,215,55,183,119,247,
	15,143,79,207,47,175,111,239,
	31,159,95,223,63,191,127,255};

static short btrvrs512[512] = {0,256,128,384,64,320,192,448,
	32,288,160,416,96,352,224,480,
	16,272,144,400,80,336,208,464,
	48,304,176,432,112,368,240,496,
	8,264,136,392,72,328,200,456,
	40,296,168,424,104,360,232,488,
	24,280,152,408,88,344,216,472,
	56,312,184,440,120,376,248,504,
	4,260,132,388,68,324,196,452,
	36,292,164,420,100,356,228,484,
	20,276,148,404,84,340,212,468,
	52,308,180,436,116,372,244,500,
	12,268,140,396,76,332,204,460,
	44,300,172,428,108,364,236,492,
	28,284,156,412,92,348,220,476,
	60,316,188,444,124,380,252,508,
	2,258,130,386,66,322,194,450,
	34,290,162,418,98,354,226,482,
	18,274,146,402,82,338,210,466,
	50,306,178,434,114,370,242,498,
	10,266,138,394,74,330,202,458,
	42,298,170,426,106,362,234,490,
	26,282,154,410,90,346,218,474,
	58,314,186,442,122,378,250,506,
	6,262,134,390,70,326,198,454,
	38,294,166,422,102,358,230,486,
	22,278,150,406,86,342,214,470,
	54,310,182,438,118,374,246,502,
	14,270,142,398,78,334,206,462,
	46,302,174,430,110,366,238,494,
	30,286,158,414,94,350,222,478,
	62,318,190,446,126,382,254,510,
	1,257,129,385,65,321,193,449,
	33,289,161,417,97,353,225,481,
	17,273,145,401,81,337,209,465,
	49,305,177,433,113,369,241,497,
	9,265,137,393,73,329,201,457,
	41,297,169,425,105,361,233,489,
	25,281,153,409,89,345,217,473,
	57,313,185,441,121,377,249,505,
	5,261,133,389,69,325,197,453,
	37,293,165,421,101,357,229,485,
	21,277,149,405,85,341,213,469,
	53,309,181,437,117,373,245,501,
	13,269,141,397,77,333,205,461,
	45,301,173,429,109,365,237,493,
	29,285,157,413,93,349,221,477,
	61,317,189,445,125,381,253,509,
	3,259,131,387,67,323,195,451,
	35,291,163,419,99,355,227,483,
	19,275,147,403,83,339,211,467,
	51,307,179,435,115,371,243,499,
	11,267,139,395,75,331,203,459,
	43,299,171,427,107,363,235,491,
	27,283,155,411,91,347,219,475,
	59,315,187,443,123,379,251,507,
	7,263,135,391,71,327,199,455,
	39,295,167,423,103,359,231,487,
	23,279,151,407,87,343,215,471,
	55,311,183,439,119,375,247,503,
	15,271,143,399,79,335,207,463,
	47,303,175,431,111,367,239,495,
	31,287,159,415,95,351,223,479,
	63,319,191,447,127,383,255,511};

static short * revers_db[10] = {
	btrvrs16,
	btrvrs32,
	btrvrs64,
	btrvrs128,
	btrvrs256,
	btrvrs512

};

static FHT_FUNC sFHT_routines[16] = {
//	FHT16prll,
	FHT16,
	FHT32,
	FHT64,
//	FHT128prll,
	FHT128,
	FHT256,
	FHT512,
	FHT1024,
	FHT2048,
	FHT4096
};
static void* sFHT_dirinv[16][2] = {

	{FHT16RevIn2, FHTMAD16RevEnd},
	{FHT32RevIn2, FHTMAD32RevEnd},
	{FHT64RevIn2, FHTMAD64RevEnd},
	{FHT128RevIn2, FHTMAD128RevEnd},
	{FHT256RevIn2, FHTMAD256RevEnd},
	{FHT512RevIn2, FHTMAD512RevEnd},
	{FHT1024RevIn2, FHTMAD1024RevEnd},
	{FHT2048RevIn2, FHTMAD2048RevEnd},
	{FHT4096RevIn2, FHTMAD4096RevEnd},

};

static short* SelectReversedTable( int n ) {
	short * ret = 0;
	int f = (n >> 4);
	for ( int i = 0; i < 10; i++, f >>= 1) {
		if ( f & 1 ) {
			ret = (short*)malloc(sizeof(short)*n);
	        memcpy(ret, revers_db[i],sizeof(short)*n);
			break;
		}
	}

	return (ret);
}

// assuming POW(2)
static short * genBitreverseTable( int n ) {

	short * ret = (short*)malloc(sizeof(short)*n);

	int log2n = 0;
	for(int i = 0; i < 32; i++) {
		if ( n & (1 << i ) ) {
			log2n = i;
			break;
		}
	}

	for(int i = 0; i < n; i++ ) {
		uint bitreversed = 0;
		int indx = i;
		for( int j = log2n-1; j >= 0; j--, indx >>= 1){
			bitreversed |= ((indx & 1) << j);
		}
		ret[i] = bitreversed;
		ret[bitreversed] = i;
	}
	return(ret);
}

static FHT_FUNC SelectRoutine( int n ) {

	FHT_FUNC ret = 0;

	int f = (n >> 4);
	for ( int i = 0; i < 10; i++, f >>= 1) {
		if ( f & 1 ) {
	        ret = sFHT_routines[i];
			break;
		}
	}
	return(ret);
}

static void SelectRoutine2(void * dir_inv[2],  int n ) {



	int f = (n >> 4);
	for ( int i = 0; i < 10; i++, f >>= 1) {
		if ( f & 1 ) {
	        dir_inv[0] = sFHT_dirinv[i][0];
			dir_inv[1] = sFHT_dirinv[i][1];
			break;
		}
	}

}

// sin(k/N*pi), cos(k/N*pi), ... (k=0..N/2-1)
// n is POW(2)
int FHTInit(__FLOAT__ **tsincos, short ** bit_reverse, FHT_FUNC * routine, int n) {
	int err = 0;

// alloc and fill sin_cos structure
	*tsincos = (__FLOAT__*)malloc(sizeof(__FLOAT__) *n);
	for(int k = 0; k < n/2; k++ ) {
		(*tsincos)[k*2] = (__FLOAT__)sin((TWOPI*k/n));
		(*tsincos)[k*2 + 1] = (__FLOAT__)cos((TWOPI*k/n));
	}

	*bit_reverse = genBitreverseTable( n );
//    *bit_reverse = SelectReversedTable( n );

	*routine = SelectRoutine(n);
	return(err);
}


int FHTInit2(void *dir_inv[2], __FLOAT__ **tsincos, short ** bit_reverse, int n) {
	int err = 0;

// alloc and fill sin_cos structure
	*tsincos = (__FLOAT__*)malloc(sizeof(__FLOAT__) *n);
	for(int k = 0; k < n/2; k++ ) {
		(*tsincos)[k*2] = (__FLOAT__)sin((TWOPI*k/n));
		(*tsincos)[k*2 + 1] = (__FLOAT__)cos((TWOPI*k/n));
	}

	*bit_reverse = genBitreverseTable( n );
//    *bit_reverse = SelectReversedTable( n );

	SelectRoutine2(dir_inv, n);
	return(err);
}


#define N 16
#define LOG2_N 4

static
void FHT16(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr)
{
    int n=1;
    int angf=N/2;
      
    while(n<N)
    {
        __FLOAT__ *angle=&tsincos[0];
        
        __FLOAT__ *A=&FHTwarr[0];
        __FLOAT__ *B=&FHTwarr[n];
        __FLOAT__ *C=&FHTwarr[n-1];
        __FLOAT__ *D=&FHTwarr[n*2-1];
			int a_shift;
			int b_shift;
			int c_shift;
			int d_shift;
        // ang=0
        for(int k=0;k<angf;k++)
        {

			    a_shift = (int)(A - FHTwarr);
			    b_shift = (int)(B - FHTwarr);
            __FLOAT__ tmp=*B;
            *B=*A-tmp;
            *A=*A+tmp;

            A+=n<<1;
            B+=n<<1;
        }
        A-=(N-1);
        B-=(N-1);

        
        // full butterflies
		int ang_off = 0;
        while(n>2 && ang_off<N/2 - (angf<<1))
        {

			__FLOAT__ *ang = angle;
			ang_off +=  (angf<<1);
         
            __FLOAT__ dsin=ang[ang_off];
            __FLOAT__ dcos=ang[ang_off + 1];
            
            for(int k=0;k<angf;k++)
            {
                __FLOAT__ E=*B*dcos + *D*dsin;
                __FLOAT__ F=*B*dsin - *D*dcos;
			    a_shift = (int)(A - FHTwarr);
			    b_shift = (int)(B - FHTwarr);
			    c_shift = (int)(C - FHTwarr);
			    d_shift = (int)(D - FHTwarr);
                *B=*A-E;
                *A=*A+E;
                *D=*C-F;
                *C=*C+F;

                A+=n<<1;
                B+=n<<1;
                C+=n<<1;
                D+=n<<1;
            }
            A-=(N-1);
            B-=(N-1);
            C-=(N+1);
            D-=(N+1);
        }

        // ang=pi/2
        for(int k=0;n>1 && k<angf;k++)
        {

			d_shift = (int)(D - FHTwarr);
			c_shift = (int)(C - FHTwarr);
            __FLOAT__ tmp=*D;
            *D=*C-tmp;
            *C=*C+tmp;

            C+=n<<1;
            D+=n<<1;
        }

        n<<=1;
        angf>>=1;
    }
}



static
void FHT16prll(__FLOAT__ *tsincos, __FLOAT__ *FHTdata)
{

    int n=1;
    int n2=N/2;
 
      
    for(int log2_n = 0; log2_n < LOG2_N; log2_n++)
    {

		int log2_n2 = LOG2_N - 1 - log2_n;
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		__FLOAT__ * ang = &tsincos[0];
		__FLOAT__ * A;
		__FLOAT__ * B;
		__FLOAT__ * C;
		__FLOAT__ * D;
		__FLOAT__ a;
		__FLOAT__ b;
		__FLOAT__ c;
		__FLOAT__ d;
		__FLOAT__ e;
		__FLOAT__ f;
		int i, j, ang_off;


		for( int k = 0; k < n2 * (n/2 + 1) ; k++ ){

	
			i = ( k / (n /2 + 1));
			j =  k - i * (n/2 + 1);

			int diff = (j==0) ? n : j;
			__FLOAT__ flip_sign = (__FLOAT__)((j==0)? -1 : 1);
			ang_off = j * (n2 <<1);


			A = &FHTdata[n*2*i	+ j];
			B = &FHTdata[n*2*i + n + j];


			C=&FHTdata[n*2*i + n - diff];
			D=&FHTdata[n*2*i + n*2 - diff];
	
         
			__FLOAT__ dsin=ang[ang_off];
			__FLOAT__ dcos=ang[ang_off + 1];

			a = *A;
			b = *B;
			c = *C;
			d = *D;

            e = b*dcos + d*dsin;
            f = b*dsin - d*dcos;

			f *= flip_sign;

            *B = a-e;
            *A = a+e;
            *D = c-f;
            *C = c+f;
			
			
		}


	}


}

static
void FHT16RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
	FHT16(tsincos, FHT_window);
}

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
static
void FHTMAD16RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail) {
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
			FHT16(tsincos, FHT_window2);
}

#undef N
#define N 32
static
void FHT32(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr)
{
    int n=1;
    int angf=N/2;
      
    while(n<N)
    {
        __FLOAT__ *angle=&tsincos[0];
        
        __FLOAT__ *A=&FHTwarr[0];
        __FLOAT__ *B=&FHTwarr[n];
        __FLOAT__ *C=&FHTwarr[n-1];
        __FLOAT__ *D=&FHTwarr[n*2-1];

        // ang=0
        for(int k=0;k<angf;k++)
        {
            __FLOAT__ tmp=*B;
            *B=*A-tmp;
            *A=*A+tmp;

            A+=n<<1;
            B+=n<<1;
        }
        A-=(N-1);
        B-=(N-1);

        
        // full butterflies
		int ang_off = 0;
        while(n>2 && ang_off<N/2 - (angf<<1))
        {

			__FLOAT__ *ang = angle;
			ang_off +=  (angf<<1);
         
            __FLOAT__ dsin=ang[ang_off];
            __FLOAT__ dcos=ang[ang_off + 1];
            
            for(int k=0;k<angf;k++)
            {
                __FLOAT__ E=*B*dcos + *D*dsin;
                __FLOAT__ F=*B*dsin - *D*dcos;
                *B=*A-E;
                *A=*A+E;
                *D=*C-F;
                *C=*C+F;

                A+=n<<1;
                B+=n<<1;
                C+=n<<1;
                D+=n<<1;
            }
            A-=(N-1);
            B-=(N-1);
            C-=(N+1);
            D-=(N+1);
        }

        // ang=pi/2
        for(int k=0;n>1 && k<angf;k++)
        {
            __FLOAT__ tmp=*D;
            *D=*C-tmp;
            *C=*C+tmp;

            C+=n<<1;
            D+=n<<1;
        }

        n<<=1;
        angf>>=1;
    }

}


static
void FHT32RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
	FHT32(tsincos, FHT_window);
}

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
static
void FHTMAD32RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail) {
			FHT_window2[bitrvrs[0]] = FHT_window[0] * FHT_IR[0] *2 + ((tail == 0) ? 0 : tail[0]);
			for( int k = 1; k < N/2; k++ ){
				__FLOAT__ x_k = FHT_window[k];
				__FLOAT__ x_N_k = FHT_window[N-k]; 
				__FLOAT__ y_k = FHT_IR[k];
				__FLOAT__ y_N_k = FHT_IR[N-k];
// write with reverse

				FHT_window2[bitrvrs[k]] = (x_k * (y_k + y_N_k) + x_N_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[k]);
				FHT_window2[bitrvrs[N-k]] = (x_N_k * (y_k + y_N_k) - x_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[N-k]);
			}
			FHT_window2[bitrvrs[N/2]] = FHT_window[(N/2)] * FHT_IR[(N/2)] * 2 + ((tail == 0) ? 0 : tail[(N/2)]);

/////////////
			FHT32(tsincos, FHT_window2);
}


#undef N
#define N 64
static
void FHT64(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr)
{
    int n=1;
    int angf=N/2;
      
    while(n<N)
    {
        __FLOAT__ *angle=&tsincos[0];
        
        __FLOAT__ *A=&FHTwarr[0];
        __FLOAT__ *B=&FHTwarr[n];
        __FLOAT__ *C=&FHTwarr[n-1];
        __FLOAT__ *D=&FHTwarr[n*2-1];

        // ang=0
        for(int k=0;k<angf;k++)
        {
            __FLOAT__ tmp=*B;
            *B=*A-tmp;
            *A=*A+tmp;

            A+=n<<1;
            B+=n<<1;
        }
        A-=(N-1);
        B-=(N-1);

        
        // full butterflies
		int ang_off = 0;
        while(n>2 && ang_off<N/2 - (angf<<1))
        {

			__FLOAT__ *ang = angle;
			ang_off +=  (angf<<1);
         
            __FLOAT__ dsin=ang[ang_off];
            __FLOAT__ dcos=ang[ang_off + 1];
            
            for(int k=0;k<angf;k++)
            {
                __FLOAT__ E=*B*dcos + *D*dsin;
                __FLOAT__ F=*B*dsin - *D*dcos;
                *B=*A-E;
                *A=*A+E;
                *D=*C-F;
                *C=*C+F;

                A+=n<<1;
                B+=n<<1;
                C+=n<<1;
                D+=n<<1;
            }
            A-=(N-1);
            B-=(N-1);
            C-=(N+1);
            D-=(N+1);
        }

        // ang=pi/2
        for(int k=0;n>1 && k<angf;k++)
        {
            __FLOAT__ tmp=*D;
            *D=*C-tmp;
            *C=*C+tmp;

            C+=n<<1;
            D+=n<<1;
        }

        n<<=1;
        angf>>=1;
    }

}

static
void FHT64RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
	FHT64(tsincos, FHT_window);
}

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
static
void FHTMAD64RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail) {
			FHT_window2[bitrvrs[0]] = FHT_window[0] * FHT_IR[0] *2 + ((tail == 0) ? 0 : tail[0]);
			for( int k = 1; k < N/2; k++ ){
				__FLOAT__ x_k = FHT_window[k];
				__FLOAT__ x_N_k = FHT_window[N-k]; 
				__FLOAT__ y_k = FHT_IR[k];
				__FLOAT__ y_N_k = FHT_IR[N-k];
// write with reverse

				FHT_window2[bitrvrs[k]] = (x_k * (y_k + y_N_k) + x_N_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[k]);
				FHT_window2[bitrvrs[N-k]] = (x_N_k * (y_k + y_N_k) - x_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[N-k]);
			}
			FHT_window2[bitrvrs[N/2]] = FHT_window[(N/2)] * FHT_IR[(N/2)] * 2 + ((tail == 0) ? 0 : tail[(N/2)]);

///////////////
			FHT64(tsincos, FHT_window2);
}



#undef N
#undef LOG2_N
#define N 128
#define LOG2_N 7

static
void FHT128(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr)
{
    int n=1;
    int angf=N/2;

      
    while(n < N)
    {
        __FLOAT__ *angle=&tsincos[0];
        
        __FLOAT__ *A=&FHTwarr[0];
        __FLOAT__ *B=&FHTwarr[n];
        __FLOAT__ *C=&FHTwarr[n-1];
        __FLOAT__ *D=&FHTwarr[n*2-1];

        // ang=0
        for(int k=0;k<angf;k++)
        {
            __FLOAT__ tmp=*B;
            *B=*A-tmp;
            *A=*A+tmp;

            A+=n<<1;
            B+=n<<1;
        }
        A-=(N-1);
        B-=(N-1);

        
        // full butterflies
		int ang_off = 0;
        while(n>2 && ang_off<N/2 - (angf<<1))
        {

			__FLOAT__ *ang = angle;
			ang_off +=  (angf<<1);
         
            __FLOAT__ dsin=ang[ang_off];
            __FLOAT__ dcos=ang[ang_off + 1];
            
            for(int k=0;k<angf;k++)
            {
                __FLOAT__ E=*B*dcos + *D*dsin;
                __FLOAT__ F=*B*dsin - *D*dcos;
                *B=*A-E;
                *A=*A+E;
                *D=*C-F;
                *C=*C+F;

                A+=n<<1;
                B+=n<<1;
                C+=n<<1;
                D+=n<<1;
            }
            A-=(N-1);
            B-=(N-1);
            C-=(N+1);
            D-=(N+1);
        }

        // ang=pi/2
        for(int k=0;n>1 && k<angf;k++)
        {
            __FLOAT__ tmp=*D;
            *D=*C-tmp;
            *C=*C+tmp;

            C+=n<<1;
            D+=n<<1;
        }

        n<<=1;
        angf>>=1;
    }

}

static
void FHT128prll(__FLOAT__ *tsincos, __FLOAT__ *FHTdata)
{

    int n=1;
    int n2=N/2;
 
      
    for(int log2_n = 0; log2_n < LOG2_N; log2_n++)
    {

		int log2_n2 = LOG2_N - 1 - log2_n;
		n = (1 << log2_n);
		n2 = (1 << log2_n2);

		__FLOAT__ * ang = &tsincos[0];
		__FLOAT__ * A;
		__FLOAT__ * B;
		__FLOAT__ * C;
		__FLOAT__ * D;
		__FLOAT__ a;
		__FLOAT__ b;
		__FLOAT__ c;
		__FLOAT__ d;
		__FLOAT__ e;
		__FLOAT__ f;
		int i, j, ang_off;


		for( int k = 0; k < n2 * (n/2 + 1) ; k++ ){

	
			i = ( k / (n /2 + 1));
			j =  k - i * (n/2 + 1);

			int diff = (j==0) ? n : j;
			__FLOAT__ flip_sign = (__FLOAT__)((j==0)? -1 : 1);
			ang_off = j * (n2 <<1);


			A = &FHTdata[n*2*i	+ j];
			B = &FHTdata[n*2*i + n + j];


			C=&FHTdata[n*2*i + n - diff];
			D=&FHTdata[n*2*i + n*2 - diff];
	
         
			__FLOAT__ dsin=ang[ang_off];
			__FLOAT__ dcos=ang[ang_off + 1];

			a = *A;
			b = *B;
			c = *C;
			d = *D;

            e = b*dcos + d*dsin;
            f = b*dsin - d*dcos;

			f *= flip_sign;

            *B = a-e;
            *A = a+e;
            *D = c-f;
            *C = c+f;
			
			
		}


	}


}

static
void FHT128RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
	FHT128(tsincos, FHT_window);
}

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
static
void FHTMAD128RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail) {
			FHT_window2[bitrvrs[0]] = FHT_window[0] * FHT_IR[0] *2 + ((tail == 0) ? 0 : tail[0]);
			for( int k = 1; k < N/2; k++ ){
				__FLOAT__ x_k = FHT_window[k];
				__FLOAT__ x_N_k = FHT_window[N-k]; 
				__FLOAT__ y_k = FHT_IR[k];
				__FLOAT__ y_N_k = FHT_IR[N-k];
// write with reverse

				FHT_window2[bitrvrs[k]] = (x_k * (y_k + y_N_k) + x_N_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[k]);
				FHT_window2[bitrvrs[N-k]] = (x_N_k * (y_k + y_N_k) - x_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[N-k]);
			}
			FHT_window2[bitrvrs[N/2]] = FHT_window[(N/2)] * FHT_IR[(N/2)] * 2 + ((tail == 0) ? 0 : tail[(N/2)]);

////////////////
			FHT128(tsincos, FHT_window2);
}

#undef N
#define N 256
static
void FHT256(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr)
{
    int n=1;
    int angf=N/2;
      
    while(n<N)
    {
        __FLOAT__ *angle=&tsincos[0];
        
        __FLOAT__ *A=&FHTwarr[0];
        __FLOAT__ *B=&FHTwarr[n];
        __FLOAT__ *C=&FHTwarr[n-1];
        __FLOAT__ *D=&FHTwarr[n*2-1];

        // ang=0
        for(int k=0;k<angf;k++)
        {
            __FLOAT__ tmp=*B;
            *B=*A-tmp;
            *A=*A+tmp;

            A+=n<<1;
            B+=n<<1;
        }
        A-=(N-1);
        B-=(N-1);

        
        // full butterflies
		int ang_off = 0;
        while(n>2 && ang_off<N/2 - (angf<<1))
        {

			__FLOAT__ *ang = angle;
			ang_off +=  (angf<<1);
         
            __FLOAT__ dsin=ang[ang_off];
            __FLOAT__ dcos=ang[ang_off + 1];
            
            for(int k=0;k<angf;k++)
            {
                __FLOAT__ E=*B*dcos + *D*dsin;
                __FLOAT__ F=*B*dsin - *D*dcos;
                *B=*A-E;
                *A=*A+E;
                *D=*C-F;
                *C=*C+F;

                A+=n<<1;
                B+=n<<1;
                C+=n<<1;
                D+=n<<1;
            }
            A-=(N-1);
            B-=(N-1);
            C-=(N+1);
            D-=(N+1);
        }

        // ang=pi/2
        for(int k=0;n>1 && k<angf;k++)
        {
            __FLOAT__ tmp=*D;
            *D=*C-tmp;
            *C=*C+tmp;

            C+=n<<1;
            D+=n<<1;
        }

        n<<=1;
        angf>>=1;
    }

}

static
void FHT256RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
	FHT256(tsincos, FHT_window);
}

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
static
void FHTMAD256RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail) {
			FHT_window2[bitrvrs[0]] = FHT_window[0] * FHT_IR[0] *2 + ((tail == 0) ? 0 : tail[0]);
			for( int k = 1; k < N/2; k++ ){
				__FLOAT__ x_k = FHT_window[k];
				__FLOAT__ x_N_k = FHT_window[N-k]; 
				__FLOAT__ y_k = FHT_IR[k];
				__FLOAT__ y_N_k = FHT_IR[N-k];
// write with reverse

				FHT_window2[bitrvrs[k]] = (x_k * (y_k + y_N_k) + x_N_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[k]);
				FHT_window2[bitrvrs[N-k]] = (x_N_k * (y_k + y_N_k) - x_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[N-k]);
			}
			FHT_window2[bitrvrs[N/2]] = FHT_window[(N/2)] * FHT_IR[(N/2)] * 2 + ((tail == 0) ? 0 : tail[(N/2)]);

/////////////////
			FHT256(tsincos, FHT_window2);
}



#undef N
#define N 512
static
void FHT512(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr)
{
    int n=1;
    int angf=N/2;
      
    while(n<N)
    {
        __FLOAT__ *angle=&tsincos[0];
        
        __FLOAT__ *A=&FHTwarr[0];
        __FLOAT__ *B=&FHTwarr[n];
        __FLOAT__ *C=&FHTwarr[n-1];
        __FLOAT__ *D=&FHTwarr[n*2-1];

        // ang=0
        for(int k=0;k<angf;k++)
        {
            __FLOAT__ tmp=*B;
            *B=*A-tmp;
            *A=*A+tmp;

            A+=n<<1;
            B+=n<<1;
        }
        A-=(N-1);
        B-=(N-1);

        
        // full butterflies
		int ang_off = 0;
        while(n>2 && ang_off<N/2 - (angf<<1))
        {

			__FLOAT__ *ang = angle;
			ang_off +=  (angf<<1);
         
            __FLOAT__ dsin=ang[ang_off];
            __FLOAT__ dcos=ang[ang_off + 1];
            
            for(int k=0;k<angf;k++)
            {
                __FLOAT__ E=*B*dcos + *D*dsin;
                __FLOAT__ F=*B*dsin - *D*dcos;
                *B=*A-E;
                *A=*A+E;
                *D=*C-F;
                *C=*C+F;

                A+=n<<1;
                B+=n<<1;
                C+=n<<1;
                D+=n<<1;
            }
            A-=(N-1);
            B-=(N-1);
            C-=(N+1);
            D-=(N+1);
        }

        // ang=pi/2
        for(int k=0;n>1 && k<angf;k++)
        {
            __FLOAT__ tmp=*D;
            *D=*C-tmp;
            *C=*C+tmp;

            C+=n<<1;
            D+=n<<1;
        }

        n<<=1;
        angf>>=1;
    }

}

static
void FHT512RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
	FHT512(tsincos, FHT_window);
}

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
static
void FHTMAD512RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail) {
			FHT_window2[bitrvrs[0]] = FHT_window[0] * FHT_IR[0] *2 + ((tail == 0) ? 0 : tail[0]);
			for( int k = 1; k < N/2; k++ ){
				__FLOAT__ x_k = FHT_window[k];
				__FLOAT__ x_N_k = FHT_window[N-k]; 
				__FLOAT__ y_k = FHT_IR[k];
				__FLOAT__ y_N_k = FHT_IR[N-k];
// write with reverse

				FHT_window2[bitrvrs[k]] = (x_k * (y_k + y_N_k) + x_N_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[k]);
				FHT_window2[bitrvrs[N-k]] = (x_N_k * (y_k + y_N_k) - x_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[N-k]);
			}
			FHT_window2[bitrvrs[N/2]] = FHT_window[(N/2)] * FHT_IR[(N/2)] * 2 + ((tail == 0) ? 0 : tail[(N/2)]);

/////////
			FHT512(tsincos, FHT_window2);
}

#undef N
#define N 1024
static
void FHT1024(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr)
{
    int n=1;
    int angf=N/2;
      
    while(n<N)
    {
        __FLOAT__ *angle=&tsincos[0];
        
        __FLOAT__ *A=&FHTwarr[0];
        __FLOAT__ *B=&FHTwarr[n];
        __FLOAT__ *C=&FHTwarr[n-1];
        __FLOAT__ *D=&FHTwarr[n*2-1];

        // ang=0
        for(int k=0;k<angf;k++)
        {
            __FLOAT__ tmp=*B;
            *B=*A-tmp;
            *A=*A+tmp;

            A+=n<<1;
            B+=n<<1;
        }
        A-=(N-1);
        B-=(N-1);

        
        // full butterflies
		int ang_off = 0;
        while(n>2 && ang_off<N/2 - (angf<<1))
        {

			__FLOAT__ *ang = angle;
			ang_off +=  (angf<<1);
         
            __FLOAT__ dsin=ang[ang_off];
            __FLOAT__ dcos=ang[ang_off + 1];
            
            for(int k=0;k<angf;k++)
            {
                __FLOAT__ E=*B*dcos + *D*dsin;
                __FLOAT__ F=*B*dsin - *D*dcos;
                *B=*A-E;
                *A=*A+E;
                *D=*C-F;
                *C=*C+F;

                A+=n<<1;
                B+=n<<1;
                C+=n<<1;
                D+=n<<1;
            }
            A-=(N-1);
            B-=(N-1);
            C-=(N+1);
            D-=(N+1);
        }

        // ang=pi/2
        for(int k=0;n>1 && k<angf;k++)
        {
            __FLOAT__ tmp=*D;
            *D=*C-tmp;
            *C=*C+tmp;

            C+=n<<1;
            D+=n<<1;
        }

        n<<=1;
        angf>>=1;
    }

}

static
void FHT1024RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
	FHT1024(tsincos, FHT_window);
}

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
static
void FHTMAD1024RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail) {
			FHT_window2[bitrvrs[0]] = FHT_window[0] * FHT_IR[0] *2 + ((tail == 0) ? 0 : tail[0]);
			for( int k = 1; k < N/2; k++ ){
				__FLOAT__ x_k = FHT_window[k];
				__FLOAT__ x_N_k = FHT_window[N-k]; 
				__FLOAT__ y_k = FHT_IR[k];
				__FLOAT__ y_N_k = FHT_IR[N-k];
// write with reverse

				FHT_window2[bitrvrs[k]] = (x_k * (y_k + y_N_k) + x_N_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[k]);
				FHT_window2[bitrvrs[N-k]] = (x_N_k * (y_k + y_N_k) - x_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[N-k]);
			}
			FHT_window2[bitrvrs[N/2]] = FHT_window[(N/2)] * FHT_IR[(N/2)] * 2 + ((tail == 0) ? 0 : tail[(N/2)]);

//////////////
			FHT1024(tsincos, FHT_window2);
}


#undef N
#define N 2048
static
void FHT2048(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr)
{
    int n=1;
    int angf=N/2;
      
    while(n<N)
    {
        __FLOAT__ *angle=&tsincos[0];
        
        __FLOAT__ *A=&FHTwarr[0];
        __FLOAT__ *B=&FHTwarr[n];
        __FLOAT__ *C=&FHTwarr[n-1];
        __FLOAT__ *D=&FHTwarr[n*2-1];

        // ang=0
        for(int k=0;k<angf;k++)
        {
            __FLOAT__ tmp=*B;
            *B=*A-tmp;
            *A=*A+tmp;

            A+=n<<1;
            B+=n<<1;
        }
        A-=(N-1);
        B-=(N-1);

        
        // full butterflies
		int ang_off = 0;
        while(n>2 && ang_off<N/2 - (angf<<1))
        {

			__FLOAT__ *ang = angle;
			ang_off +=  (angf<<1);
         
            __FLOAT__ dsin=ang[ang_off];
            __FLOAT__ dcos=ang[ang_off + 1];
            
            for(int k=0;k<angf;k++)
            {
                __FLOAT__ E=*B*dcos + *D*dsin;
                __FLOAT__ F=*B*dsin - *D*dcos;
                *B=*A-E;
                *A=*A+E;
                *D=*C-F;
                *C=*C+F;

                A+=n<<1;
                B+=n<<1;
                C+=n<<1;
                D+=n<<1;
            }
            A-=(N-1);
            B-=(N-1);
            C-=(N+1);
            D-=(N+1);
        }

        // ang=pi/2
        for(int k=0;n>1 && k<angf;k++)
        {
            __FLOAT__ tmp=*D;
            *D=*C-tmp;
            *C=*C+tmp;

            C+=n<<1;
            D+=n<<1;
        }

        n<<=1;
        angf>>=1;
    }

}

static
void FHT2048RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
	FHT2048(tsincos, FHT_window);
}

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
static
void FHTMAD2048RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail) {
			FHT_window2[bitrvrs[0]] = FHT_window[0] * FHT_IR[0] *2 + ((tail == 0) ? 0 : tail[0]);
			for( int k = 1; k < N/2; k++ ){
				__FLOAT__ x_k = FHT_window[k];
				__FLOAT__ x_N_k = FHT_window[N-k]; 
				__FLOAT__ y_k = FHT_IR[k];
				__FLOAT__ y_N_k = FHT_IR[N-k];
// write with reverse

				FHT_window2[bitrvrs[k]] = (x_k * (y_k + y_N_k) + x_N_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[k]);
				FHT_window2[bitrvrs[N-k]] = (x_N_k * (y_k + y_N_k) - x_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[N-k]);
			}
			FHT_window2[bitrvrs[N/2]] = FHT_window[(N/2)] * FHT_IR[(N/2)] * 2 + ((tail == 0) ? 0 : tail[(N/2)]);

/////////////////
			FHT2048(tsincos, FHT_window2);
}


#undef N
#define N 4096
static
void FHT4096(__FLOAT__ *tsincos, __FLOAT__ *FHTwarr)
{
    int n=1;
    int angf=N/2;
      
    while(n<N)
    {
        __FLOAT__ *angle=&tsincos[0];
        
        __FLOAT__ *A=&FHTwarr[0];
        __FLOAT__ *B=&FHTwarr[n];
        __FLOAT__ *C=&FHTwarr[n-1];
        __FLOAT__ *D=&FHTwarr[n*2-1];

        // ang=0
        for(int k=0;k<angf;k++)
        {
            __FLOAT__ tmp=*B;
            *B=*A-tmp;
            *A=*A+tmp;

            A+=n<<1;
            B+=n<<1;
        }
        A-=(N-1);
        B-=(N-1);

        
        // full butterflies
		int ang_off = 0;
        while(n>2 && ang_off<N/2 - (angf<<1))
        {

			__FLOAT__ *ang = angle;
			ang_off +=  (angf<<1);
         
            __FLOAT__ dsin=ang[ang_off];
            __FLOAT__ dcos=ang[ang_off + 1];
            
            for(int k=0;k<angf;k++)
            {
                __FLOAT__ E=*B*dcos + *D*dsin;
                __FLOAT__ F=*B*dsin - *D*dcos;
                *B=*A-E;
                *A=*A+E;
                *D=*C-F;
                *C=*C+F;

                A+=n<<1;
                B+=n<<1;
                C+=n<<1;
                D+=n<<1;
            }
            A-=(N-1);
            B-=(N-1);
            C-=(N+1);
            D-=(N+1);
        }

        // ang=pi/2
        for(int k=0;n>1 && k<angf;k++)
        {
            __FLOAT__ tmp=*D;
            *D=*C-tmp;
            *C=*C+tmp;

            C+=n<<1;
            D+=n<<1;
        }

        n<<=1;
        angf>>=1;
    }

}


static
void FHT4096RevIn2(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs) {
	for(int j = 0; j < (N/2); j++) {
		FHT_window[bitrvrs[j]] = new_data[j];
		FHT_window[bitrvrs[(N/2) + j]] = prev_data[j];
	}
	FHT4096(tsincos, FHT_window);
}

/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
X0 == XN etc
below the division by 2 deffered to the final scaling
*/
static
void FHTMAD4096RevEnd(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail) {
			FHT_window2[bitrvrs[0]] = FHT_window[0] * FHT_IR[0] *2 + ((tail == 0) ? 0 : tail[0]);
			for( int k = 1; k < N/2; k++ ){
				__FLOAT__ x_k = FHT_window[k];
				__FLOAT__ x_N_k = FHT_window[N-k]; 
				__FLOAT__ y_k = FHT_IR[k];
				__FLOAT__ y_N_k = FHT_IR[N-k];
// write with reverse

				FHT_window2[bitrvrs[k]] = (x_k * (y_k + y_N_k) + x_N_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[k]);
				FHT_window2[bitrvrs[N-k]] = (x_N_k * (y_k + y_N_k) - x_k * (y_k - y_N_k)) + ((tail == 0) ? 0 : tail[N-k]);
			}
			FHT_window2[bitrvrs[N/2]] = FHT_window[(N/2)] * FHT_IR[(N/2)] * 2 + ((tail == 0) ? 0 : tail[(N/2)]);

///////////////
			FHT4096(tsincos, FHT_window2);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////////////////


void FHTReverseInplace(__FLOAT__ *data, short * bitrvrse, int n ) {
	__FLOAT__ *tmp = (__FLOAT__*)malloc(sizeof(__FLOAT__)*n);
	for(int r = 0; r < n; r++ ) {
		tmp[r] = data[bitrvrse[r]];
	}
	memcpy(data, tmp, sizeof(__FLOAT__) *n);
	free(tmp);
}

void FHT(__FLOAT__ *tsincos, __FLOAT__ *FHTdata, int n)
{

	int f = (n >> 4);
	for ( int i = 0; i < 10; i++, f >>= 1) {
		if ( f & 1 ) {
	        sFHT_routines[i](tsincos, FHTdata);
			break;
		}
	}
}



void FHTWithReverse(__FLOAT__ *tsincos, short * bitrvrse, __FLOAT__ *FHTdata, int n)
{
	FHTReverseInplace(FHTdata, bitrvrse, n );
	FHT(tsincos, FHTdata, n);
}

// 
#define PI_D (double) 3.141592653589793238462643
void FHT_def_D(double *FHTdata, int n) {
	double *transform = (double*)malloc(sizeof(double) * n);

	for ( int i = 0; i < n; i++ ) {
		transform[i] = 0;
		for (int j = 0; j < n; j++ ) {
			transform[i] += FHTdata[j] * (sin((PI_D * 2 * j * i) / n) + cos((PI_D * 2 * j * i) / n));
		}
	}
	memcpy(FHTdata, transform, sizeof(double) * n );
	free(transform);
}

int FHT_verify(const __FLOAT__ * data_in, const __FLOAT__ *data_totest, int n, int inv, int n_totest, __FLOAT__ scale ) {
	int err = -1;
	double *data_vd = (double*)malloc(sizeof(double) * n );
	for (int i = 0; i < n; i++ ) {
		data_vd[i] = data_in[i];
	}
	FHT_def_D(data_vd, n);
	for( int i = 0; i < n_totest; i++ ) {
	    data_vd[i] = (inv)?data_vd[i] * scale/(double)n : data_vd[i];
		if ( abs(data_vd[i] - data_totest[i]) > 0.0001) {
#if _DEBUG_PRINTF
			printf("FHT filter mismatch: i=%d g=%f c=%f\n", i, data_totest[i], data_vd[i]);
#endif
			err = i;
			break;
		}
	}
	return (err);
}

void FHTTest(int n ) {
	FHT_FUNC fht_routine;
	__FLOAT__ * tsincos;
	__FLOAT__ * data;
	__FLOAT__ * data_v;
	__FLOAT__ * data_t;
	short * bit_rvrse = 0;
	data = (__FLOAT__*)malloc(sizeof(__FLOAT__)*n);
	data_v = (__FLOAT__*)malloc(sizeof(__FLOAT__)*n);
	data_t = (__FLOAT__*)malloc(sizeof(__FLOAT__)*n);
	double *data_d = (double*)malloc(sizeof(double) * n );
	double *data_vd = (double*)malloc(sizeof(double) * n );
	FHTInit(&tsincos, &bit_rvrse, &fht_routine, n);
	for (int i = 0; i < n; i++ ) {
#if 1
		if ( i == 0 || i == 1 ) {
			data_v[i] = 1.f;

		} else { 
			data_v[i] = 0;
		}

//		data_v[i] = 1 ;
#else
		data_v[i] = (2.0f * (float) rand() / (float) RAND_MAX - 1.0f) ;
#endif
		data_d[i] = data_v[i];
		data_vd[i] = data_d[i];
	}

	
	FHT_def_D(data_d, n);
	FHT_def_D(data_d, n);
	for( int i = 0; i < n; i++ ) {
		data_d[i] /= (double)n;
		if ( abs(data_d[i] - data_vd[i]) > 0.0000001 ) {
#if _DEBUG_PRINTF
			printf("dp t: %d %f %f\n", i, data_vd[i], data_d[i]);
#endif
		}
	}

	for (int i = 0; i < n; i++ ) {
		data_d[i] = data_v[i];
		data_t[i] = data_v[bit_rvrse[i]] ;
	}
//	FHT(tsincos, data_t, n);
	fht_routine(tsincos, data_t);
	FHT_def_D(data_d, n);
	for( int i = 0; i < n; i++ ) {
	
		if ( abs(data_d[i] - data_t[i]) > 0.00001) {
#if _DEBUG_PRINTF
			printf("dir t: %d %f %f\n", i, data_t[i], data_d[i]);
#endif
		}
	}
	for (int i = 0; i < n; i++ ) {
		data_d[i] = data_t[i];
		data[i] = data_t[bit_rvrse[i]]; ///(__FLOAT__)n ;
	}
//	FHT(tsincos, data, n);
	fht_routine(tsincos, data);

	FHT_def_D(data_d, n);
	for( int i = 0; i < n; i++ ) {
		double l = abs(data_d[i] - (double)data[i]);
	
		if ( l >  0.0001 ) {
#if _DEBUG_PRINTF
			printf("inv t:%d %f %f\n", i, data[i], data_d[i]);
#endif
		}
	}
	for( int i = 0; i < n; i++ ) {
		data[i] /= (__FLOAT__)n;
		data_d[i] /= (double)n; 
		if ( abs(data[i] - data_v[i]) > 0.00001 ) {
#if _DEBUG_PRINTF
			printf("full t:%d %f %f %f\n", i, data[i], data_v[i], data_d[i]);
#endif
		}
	}
	free(tsincos);
	free(data);
	free(data_v);
	free(data_t);
	free(data_d);
	free(data_vd);
}