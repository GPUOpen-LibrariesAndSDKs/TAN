#include "QTRoomAcousticConfig.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	
	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
	QApplication a(argc, argv);
	Q_INIT_RESOURCE(roomaccousticnew);
	a.setWindowIcon(QIcon(":/images/Resources/RoomAcousticsNew.png"));
	RoomAcousticQT w;
	w.Init();
	return a.exec();
}
