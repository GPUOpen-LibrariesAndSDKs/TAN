#pragma once

#include <QtWidgets/QtWidgets>
#include "ui_ReverbMixer.h"
#include "ReverbProcessor.h"
class ReverbMixer : public QMainWindow
{
	Q_OBJECT
private slots:
	void on_actionLoad_Source_triggered();
	void on_actionLoad_Response_triggered();
	void on_actionSave_Output_triggered();
	void on_actionSave_Response_triggered();
	void on_actionPlay_triggered();
	void on_actionStop_triggered();
	void on_actionRecorderStart_triggered();
	void on_actionRecorderStop_triggered();
	void on_PB_EQApply_clicked();
	void on_PB_EQReset_clicked();
	void on_PB_0DB_clicked();
	void on_PB_N20DB_clicked();
	void on_PB_P20DB_clicked();

	void on_SL_B1_valueChanged(int value);
	void on_SL_B2_valueChanged(int value);
	void on_SL_B3_valueChanged(int value);
	void on_SL_B4_valueChanged(int value);
	void on_SL_B5_valueChanged(int value);
	void on_SL_B6_valueChanged(int value);
	void on_SL_B7_valueChanged(int value);
	void on_SL_B8_valueChanged(int value);
	void on_SL_B9_valueChanged(int value);
	void on_SL_B10_valueChanged(int value);

public slots:
	void adjustProcessorSettings();
	void adjustEqualizerFilter();
	void resetEqualizerFilter();
	void updateDeviceName();
	void updateProcessorStatus();
	void setAllEqualizerLevelUIs(int levelIndB);
public:
	ReverbMixer(QWidget* parent = 0);
	~ReverbMixer();
	void Init();
private:
	float db2Mag(float db);
	unsigned int base2toLog2_32Bit(unsigned int base2);
	void adjustEqualizerFilterSize(size_t numOfChannel, size_t sizePerChannelInFloat);
	Ui::ReverbMixerMainWindow						m_UIMainWindow;
	QString											m_sSourceFilePath;
	QString											m_sResponseFilePath;
	size_t											m_iEQFilterLengthInComplex = 0;
	size_t											m_iNumOfChannel = 0;
	float**											m_pEqualizerFilter = nullptr;
	int												m_iEqualizerFilterIndex = -1;
	float											m_pEqualizerLevels[10];
	int												m_iResponsefilterIndex = -1;
	bool											m_bPlayerPlaying = false;
	ReverbProcessor									m_ReverbProcessor;
};