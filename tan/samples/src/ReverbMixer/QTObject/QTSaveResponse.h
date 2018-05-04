#pragma once

#include <QtWidgets/QtWidgets>
#include "ReverbProcessor.h"
#include "ui_QD_SaveResponse.h"

class QD_SaveEQResponseWindow : public QDialog
{
	Q_OBJECT
private slots:
public slots :
	void getSavePath();
	void writeResponseToWAV();
	void updateResponse(float in_pResponseLevel[10]);
public:
	QD_SaveEQResponseWindow(ReverbProcessor* in_rReverbProcessor);
	virtual ~QD_SaveEQResponseWindow();
private:
	void initializeConfigs();
	void connectSignals();
	ReverbProcessor*		m_rReverbProcessor = nullptr;
	Ui::QD_SaveResponse		m_UISaveResponse;
	float					m_pEQResponse[10];
};