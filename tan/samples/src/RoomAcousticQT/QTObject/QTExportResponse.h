#pragma once
#include <QtWidgets/QDialog>
#include "ui_ExportResponse.h"
#include "../RoomAcoustic.h"
class QTExportResponse : public QDialog
{
	Q_OBJECT
public slots:
	void browseOutputFile();
	void generateResponse();
public:
	QTExportResponse(QWidget* parent, const Qt::WindowFlags& f);
	void Init(RoomAcoustic*	m_pRoomAcoustic);
private:
	void updateCurrentRoomConfig(bool in_bChecked);
	void updateCurrentListenerConfig(bool in_bChecked);
	void updateCurrentSourceConfig();
	void updateCustomRoomConfig(bool in_bChecked);
	void updateCustomListenerConfig(bool in_bChecked);
	void updateCustomSourceConfig();
	void connectSignals();
	Ui::ExportResponse m_UIExportResponse;
	RoomAcoustic*	m_pRoomAcoustic = nullptr;
};