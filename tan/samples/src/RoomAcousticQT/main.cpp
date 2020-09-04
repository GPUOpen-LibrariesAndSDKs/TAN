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

#include "QTRoomAcousticConfig.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);

	QApplication application(argc, argv);
	Q_INIT_RESOURCE(roomaccousticnew);

	application.setWindowIcon(QIcon(":/images/Resources/RoomAcousticsNew.png"));

	RoomAcousticQTConfig configWindow;
	configWindow.Init();

#ifdef USE_ASIO
	long err = 0;
	if ((err = ASIOExit()) != ASE_OK)
		printf("ASIOExit error %d", err);
#endif
	return application.exec();
}
