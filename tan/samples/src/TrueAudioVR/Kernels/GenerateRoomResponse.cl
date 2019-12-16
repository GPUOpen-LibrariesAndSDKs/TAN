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

#define Lx		4
#define Ly		4
#define Lz		4

#define SoundSpeed  340.0f
#define Float2Int   67108864.0f


__kernel
    __attribute__((reqd_work_group_size(Lx, Ly, Lz)))
    void GenerateRoomResponse1(    
    volatile __global uint*  response,		   ///< [out]	
	__global float*  hpF,//__constant 
	__global float*  lpF, // can combine the two filters into one buffers
	float srcX,
	float srcY,
	float srcZ,
	float headX,
	float headY,
	float headZ,
	float earVX,
	float earVY,
	float earVZ,
	float earV,	
	float roomWidth,
	float roomLength,
	float roomHeight,
	float dampRight,
	float dampLeft,
	float dampFront,
	float dampBack,
	float dampTop,
	float dampBottom,
	float maxGain,
	float dMin,    
    int inSampRate,
	int responseLength,
	int headFilterLength,
	int numRefX,
	int numRefY,
	int numRefZ	
    )
{
		
	int x = get_global_id(0); // reflections along left/right directions
	int y = get_global_id(1); // reflections along top/bottom directions
	int z = get_global_id(2); // reflections along front/back directions
	
	
	if ( x > ((numRefX) /2 - 1))
		return;
	
	if ( y > ((numRefY) /2 - 1) )
		return;
	
	if ( z > ((numRefZ) /2 - 1) )
		return;
	
	
	// the indices are from -(numRef)/2:-1
	// number of reflections is always even
	// the pairing is like ... (-5, 4), (-4, 3), (-3, 2), (-2, 1) , (-1, 0)
	// when index reaches 0 the kernel should return
	// the index reaches zero when global_id is bigger than (numRefX)/2 - 1
	
	// | -5   | -4   | -3   | -2   | -1   |  0   |  1   |  2   |  3   |  4   |
	// |----X-|-X----|----X-|-X----|----X-|-X----|----X-|-X----|----X-|-X----|
	
	// Number of reflection = 10
	// Global Size = 5, Global ID Range = (0:4)
	// Negative indices (-5:-1) calculated as follow: Global ID - (Number of Reflection)/2
	// Reflection Position = a*RoomSize + b*SrcPosition	
	//     { index       if index is even 
	// a = {                                    
	//     { index + 1   if index is odd
	
	//     { 1           if index is even 
	// b = {                                   
	//     { -1          if index is odd
	
	
	// x axis
	int indexX = x - (numRefX)/2;
	float a = (indexX % 2 == 0) ? indexX : indexX + 1;
	float b = (indexX % 2 == 0) ? 1.0f : -1.0f;
	
	// Negative reflection position/distance 
	float refXPos = a * roomWidth + b * srcX;
	// Reflection position in the positive direction is -refXPos
	
	
	// y axis
	int indexY = y - (numRefY)/2;
	a = (indexY % 2 == 0) ? indexY : indexY + 1;
	b = (indexY % 2 == 0) ? 1.0f : -1.0f;
	float refYPos = a * roomHeight + b * srcY;
	
	
	// z axis
	int indexZ = z - (numRefZ)/2;
	a = (indexZ % 2 == 0) ? indexZ : indexZ + 1;
	b = (indexZ % 2 == 0) ? 1.0f : -1.0f;
	float refZPos = a * roomLength + b * srcZ;
	
	// combination of reflections per work item
	
	// src_Image0 = ( refXPos,  refYPos,  refZPos)
	// src_Image1 = ( refXPos,  refYPos, -refZPos)
	// src_Image2 = ( refXPos, -refYPos,  refZPos)
	// src_Image3 = ( refXPos, -refYPos, -refZPos)
	// src_Image4 = (-refXPos,  refYPos,  refZPos)
	// src_Image5 = (-refXPos,  refYPos, -refZPos)
	// src_Image6 = (-refXPos, -refYPos,  refZPos)
	// src_Image7 = (-refXPos, -refYPos, -refZPos)	
	
	float8 amplitude = native_powr(dampRight, abs(indexX) - 1) * 
	                   native_powr(dampTop,   abs(indexY) - 1) *
					   native_powr(dampFront, abs(indexZ) - 1);
					   
					   
						   
	amplitude.s0123 *= native_powr(dampLeft, abs(indexX));
	amplitude.s4567 *= native_powr(dampLeft, max(abs(indexX) - 2.0f, 0.0f));
	
	amplitude.s0145 *= native_powr(dampBottom, abs(indexY));
	amplitude.s2367 *= native_powr(dampBottom, max(abs(indexY) - 2.0f, 0.0f));
	
	amplitude.s0246 *= native_powr(dampBack, abs(indexZ));
	amplitude.s1357 *= native_powr(dampBack, max(abs(indexZ) - 2.0f, 0.0f));
		
	float8 dx;
	dx.s0123 =  refXPos - headX;
	dx.s4567 = -refXPos - headX;
	
	float8 dy;
	dy.s0145 =  refYPos - headY;
	dy.s2367 = -refYPos - headY;
	
	float8 dz;
	dz.s0246 =  refZPos - headZ;
	dz.s1357 = -refZPos - headZ;
	
	float8 d = native_sqrt(dx*dx + dy*dy + dz*dz);
		
	
	int8 filterIndex = 1 + convert_int8((d/SoundSpeed) * inSampRate);
	
	int4 maxIndex =  max(filterIndex.s0123, filterIndex.s4567);
	maxIndex.xy = max(maxIndex.xy, maxIndex.zw);
	maxIndex.x = max(maxIndex.x, maxIndex.y);		
	
	if ( maxIndex.x < responseLength )
	{
		float8 dr = ( d <= dMin ) ? maxGain : maxGain*dMin / d;		
		int length = responseLength - maxIndex.x;
		int len = headFilterLength > length ? length : headFilterLength;		
		
	    
	    float8 dp = earVX*dx + earVY*dy + earVZ*dz;
	    float8 cosA = dp / (earV*d);
	    float8 hf = (cosA + 1.0f) / 2.0f;	
				
		for ( int i = 0; i < len; i++)
		{
			uint8 out = convert_uint8(1024.0f*amplitude*dr*(lpF[i] + hf * hpF[i]));
			
			atomic_add(&response[ i + filterIndex.s0], out.s0);
			atomic_add(&response[ i + filterIndex.s1], out.s1);
			atomic_add(&response[ i + filterIndex.s2], out.s2);
			atomic_add(&response[ i + filterIndex.s3], out.s3);
			atomic_add(&response[ i + filterIndex.s4], out.s4);
			atomic_add(&response[ i + filterIndex.s5], out.s5);
			atomic_add(&response[ i + filterIndex.s6], out.s6);
			atomic_add(&response[ i + filterIndex.s7], out.s7);
			
		}		
	}
}

__kernel
    __attribute__((reqd_work_group_size(Lx, Ly, Lz)))
    void GenerateRoomResponse(
    volatile __global int*  response,		   ///< [out]	
	__global float*  hpF,//__constant 
	__global float*  lpF, // can combine the two filters into one buffers
	float srcX,
	float srcY,
	float srcZ,
	float headX,
	float headY,
	float headZ,
	float earVX,
	float earVY,
	float earVZ,
	float earV,	
	float roomWidth,
	float roomLength,
	float roomHeight,
	float dampRight,
	float dampLeft,
	float dampFront,
	float dampBack,
	float dampTop,
	float dampBottom,
	float maxGain,
	float dMin,    
    int inSampRate,
	int responseLength,
	int hrtfResponseLength,
	int headFilterLength,
	int numRefX,
	int numRefY,
	int numRefZ	
    )
{
		
	int x = get_global_id(0); // reflections along left/right directions
	int y = get_global_id(1); // reflections along top/bottom directions
	int z = get_global_id(2); // reflections along front/back directions
	
	
	if ( x > numRefX )
		return;
	
	if ( y > numRefY )
		return;
	
	if ( z > numRefZ )
		return;
	
		
	// x axis
	int indexX = x - (numRefX)/2;
	float a = (indexX % 2 == 0) ? indexX : indexX + 1;
	float b = (indexX % 2 == 0) ? 1.0f : -1.0f;
	float refXPos = a * roomWidth + b * srcX;
		
	// y axis
	int indexY = y - (numRefY)/2;
	a = (indexY % 2 == 0) ? indexY : indexY + 1;
	b = (indexY % 2 == 0) ? 1.0f : -1.0f;
	float refYPos = a * roomHeight + b * srcY;
		
	// z axis
	int indexZ = z - (numRefZ)/2;
	a = (indexZ % 2 == 0) ? indexZ : indexZ + 1;
	b = (indexZ % 2 == 0) ? 1.0f : -1.0f;
	float refZPos = a * roomLength + b * srcZ;
	
	float attenuationXLeft   = (indexX > 0 ) ? indexX/2 : abs(indexX - 1)/2;
	float attenuationYBottom = (indexY > 0 ) ? indexY/2 : abs(indexY - 1)/2;
	float attenuationZBack   = (indexZ > 0 ) ? indexZ/2 : abs(indexZ - 1)/2;
	
	float attenuationXRight   = (indexX > 0) ? (indexX + 1)/2 : abs(indexX)/2;
	float attenuationYTop     = (indexY > 0) ? (indexY + 1)/2 : abs(indexY)/2;
	float attenuationZFront   = (indexZ > 0) ? (indexZ + 1)/2 : abs(indexZ)/2;
	
	
	float amplitude = native_powr(dampRight,  attenuationXRight)  * 
	                  native_powr(dampLeft,   attenuationXLeft)   * 
	                  native_powr(dampTop,    attenuationYTop)    *
					  native_powr(dampBottom, attenuationYBottom) *
					  native_powr(dampFront,  attenuationZFront)  *
					  native_powr(dampBack,   attenuationZBack);
	
	float dx  =  refXPos - headX;	
	
	float dy = refYPos - headY;	
	
	float dz =  refZPos - headZ;	
	
	float d = native_sqrt(dx*dx + dy*dy + dz*dz);
		
	
	int filterIndex = 1 + (int)((d/SoundSpeed) * inSampRate);
	
	
	if ( filterIndex < responseLength )
	{
		float dr = ( d <= dMin ) ? maxGain : maxGain*dMin / d;		
		int length = responseLength - filterIndex;
		int len = headFilterLength > length ? length : headFilterLength;	
			    
	    float dp = earVX*dx + earVY*dy + earVZ*dz;
	    float cosA = dp / (earV*d);
	    float hf = (cosA + 1.0f) / 2.0f;	
		
		if ( filterIndex < hrtfResponseLength)
		{	
			for ( int i = 0; i < len; i++)
			{
				int out = convert_int(Float2Int*amplitude*dr*(lpF[i] + hf * hpF[i]));
				atomic_add(&response[ i + filterIndex], out);			
			}
		}
		else
		{
			int out = convert_int(Float2Int*amplitude*dr);
			atomic_add(&response[ filterIndex], out);		
		}
	}
}