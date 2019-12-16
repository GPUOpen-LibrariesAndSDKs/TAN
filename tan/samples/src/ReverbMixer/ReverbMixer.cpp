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

// ReverbMixer.cpp : Defines the entry point for the console application.
//
#include "QTReverbMixerMainWindow.h"
#include <QtWidgets/QApplication>
#include "ReverbProcessor.h"

#include "FileUtility.h"

//#define PROCESSOR_TEST

int main(int argc, char* argv[])
{
#ifdef PROCESSOR_TEST
	ReverbProcessor processor;
	// GPU Mode with Device ID 0
	processor.init(eGPU, 0, 8192, 4096, 2);
	processor.playerPlay("C:/lomo/zli2_TAN_Mark3/nda/samples/src/RoomAcousticQT/WAV/quirksx48S.wav");
#else
	QApplication a(argc, argv);
	ReverbMixer w;
	w.Init();
	return a.exec();
#endif // PROCESSOR_TEST
}

