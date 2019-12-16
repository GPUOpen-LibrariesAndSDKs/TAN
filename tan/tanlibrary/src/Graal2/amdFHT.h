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

#ifndef __AMD_FHT_H__
#define __AMD_FHT_H__

#include <cmath>


// http://www.elektronika.kvalitne.cz/ATMEL/necoteorie/transformation/AVRFHT/AVRFHT.html
/* DHT convolution
Z[k] =  (X[k] *( Y[k] + Y[N-k]) + X[N-k] * ( Y[k] - Y[N-k] ) ) / 2 
Z[N-k] = (X[N-k] * ( Y[k] + Y[N-k]) - X[k] * ( Y[k] - Y[N-k] ) ) / 2
*/

typedef void (* FHT_FUNC )(__FLOAT__ *tsincos, __FLOAT__ *FHTdata);
typedef void (* FHT_DIRFUNC )(__FLOAT__ *FHT_window, __FLOAT__*new_data, __FLOAT__ *prev_data,__FLOAT__ *tsincos, short *bitrvrs);
typedef void (* FHT_INVFUNC )(__FLOAT__ *FHT_window2, __FLOAT__ *FHT_window, __FLOAT__ * FHT_IR, __FLOAT__ *tsincos, short* bitrvrs, __FLOAT__ * tail);

int FHTInit(__FLOAT__ **tsincos,  short ** bit_reverse, FHT_FUNC * routine, int n);
int FHTInit2(void *dir_inv[2], __FLOAT__ **tsincos, short ** bit_reverse, int n);
void FHTReverseInplace(__FLOAT__ *data, short * bitrvrs, int n );
void FHT(__FLOAT__ *tsincos, __FLOAT__ *FHTdata, int n);
void FHTWithReverse(__FLOAT__ *tsincos, short * bitrvrse, __FLOAT__ *FHTdata, int n);
int FHT_verify(const __FLOAT__ * data_in, const __FLOAT__ *data_totest, int n, int inv, int n_totest, __FLOAT__ scale );

void FHT_def_D(double *FHTdata, int n);
void FHTTest(int n );
#endif