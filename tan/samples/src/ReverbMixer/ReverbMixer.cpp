// ReverbMixer.cpp : Defines the entry point for the console application.
//
#include "QTReverbMixerMainWindow.h"
#include <QtWidgets/QApplication>
#include "ReverbProcessor.h"

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

