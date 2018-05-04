#include "QTReverbMixerMainWindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "QTSaveResponse.h"
#include <cmath>

#define SHOW_MESSAGE_IF_AMF_FAIL(AMFErr, Message) if(AMFErr == AMF_FAIL){m_UIMainWindow.statusbar->showMessage(Message);}

void ReverbMixer::on_actionLoad_Source_triggered()
{
	m_sSourceFilePath = QFileDialog::getOpenFileName(this, tr("Open Source"), ".", tr("WAV Files (*.wav, *.WAV)"));
	if (m_sSourceFilePath != "")
	{
		m_UIMainWindow.LE_InputSourceDisplay->setText(m_sSourceFilePath);
		m_UIMainWindow.statusbar->showMessage(m_sSourceFilePath + " Loaded");
		int SampleRate = 0;
		int BitPerSample = 0;
		int NumOfChannel = 0;
		long TotalNumberOfSample = 0;
		char* filepathCtr;
		std::string filepathSTDString = m_sSourceFilePath.toStdString();
		filepathCtr = new char[filepathSTDString.size() + 1];
		strcpy(filepathCtr, filepathSTDString.c_str());
		m_ReverbProcessor.getWAVFileInfo(filepathCtr, &SampleRate, &BitPerSample, &NumOfChannel, &TotalNumberOfSample);
		m_UIMainWindow.LE_SourceSampleRate->setText(QString::number(SampleRate));
		m_UIMainWindow.LE_BitsPerSample->setText(QString::number(BitPerSample));
		m_UIMainWindow.LE_NumOfChannel->setText(QString::number(NumOfChannel));
		delete[] filepathCtr;
	}
}

void ReverbMixer::on_actionLoad_Response_triggered()
{
	m_sResponseFilePath = QFileDialog::getOpenFileName(this, tr("Open Source"), ".", tr("WAV Files (*.wav, *.WAV)"));
	if (m_sResponseFilePath != "")
	{
		m_UIMainWindow.LE_ResponseDisplay->setText(m_sResponseFilePath);
		m_UIMainWindow.statusbar->showMessage(m_sResponseFilePath + " Loaded");


	}
}

void ReverbMixer::on_actionSave_Output_triggered()
{
	char* inputFilecstr;
	std::string InputFilePathStr = m_sSourceFilePath.toStdString();
	if (InputFilePathStr == "")
	{
		m_UIMainWindow.statusbar->showMessage("No input file specified, please load an input file.");
		return;
	}
	QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save As"), ".", tr("WAV Files (*.wav *.WAV"));
	if(saveFileName!="")
	{
		m_UIMainWindow.statusbar->showMessage("Output: " + saveFileName + " saved");
		inputFilecstr = new char[InputFilePathStr.size() + 1];
		strcpy(inputFilecstr, InputFilePathStr.c_str());

		char* outputFilecstr;
		std::string OutputFilePathStr = saveFileName.toStdString();
		outputFilecstr = new char[OutputFilePathStr.size() + 1];
		strcpy(outputFilecstr, OutputFilePathStr.c_str());
		adjustEqualizerFilter();
		AMF_RESULT AMFErr = m_ReverbProcessor.playerProcessToWAV(inputFilecstr, outputFilecstr);
		delete[]inputFilecstr;
		delete[]outputFilecstr;
	}
}

void ReverbMixer::on_actionSave_Response_triggered()
{
	QD_SaveEQResponseWindow newWindow(&m_ReverbProcessor);
	newWindow.updateResponse(m_pEqualizerLevels);
	newWindow.exec();
}

void ReverbMixer::on_actionPlay_triggered()
{	
	char* filepathCtr;
	std::string filepathSTDString;

	if (m_iResponsefilterIndex != -1)
	{
		m_ReverbProcessor.deleteFilter(m_iResponsefilterIndex);
		m_iResponsefilterIndex = -1;
	}
	if(m_UIMainWindow.LE_ResponseDisplay->text()!="")
	{
		filepathSTDString = m_UIMainWindow.LE_ResponseDisplay->text().toStdString();
		filepathCtr = new char[filepathSTDString.size() + 1];
		strcpy(filepathCtr, filepathSTDString.c_str());
		AMF_RESULT AMFErr;
		m_iResponsefilterIndex = m_ReverbProcessor.addFilterTDFromWAV(filepathCtr, &AMFErr);
		delete[]filepathCtr;

	}

	if (m_sSourceFilePath != "")
	{
		adjustEqualizerFilter();
		
		filepathSTDString = m_sSourceFilePath.toStdString();
		filepathCtr = new char[filepathSTDString.size() + 1];
		strcpy(filepathCtr, filepathSTDString.c_str());
		m_ReverbProcessor.playerPlay(filepathCtr);
		delete[]filepathCtr;
	}
}

void ReverbMixer::on_actionStop_triggered()
{
	m_ReverbProcessor.playerStop(); 
}

void ReverbMixer::on_actionRecorderStart_triggered()
{
	QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save As"), ".", tr("WAV Files (*.wav *.WAV"));
	if (saveFileName != "")
	{
		std::string filepathSTDString = saveFileName.toStdString();
		char* filepathCtr;
		filepathCtr = new char[filepathSTDString.size() + 1];
		strcpy(filepathCtr, filepathSTDString.c_str());
		AMF_RESULT AMFErr = m_ReverbProcessor.recorderStart(filepathCtr);
		if (AMFErr != AMF_OK)
		{
			m_UIMainWindow.statusbar->showMessage("Failed to initialize audio device");
		}
		updateProcessorStatus();
	}
}

void ReverbMixer::on_actionRecorderStop_triggered()
{
	m_ReverbProcessor.recorderStop();
	updateProcessorStatus();
}

void ReverbMixer::on_PB_EQApply_clicked()
{
	adjustEqualizerFilter();
}

void ReverbMixer::on_PB_EQReset_clicked()
{
	resetEqualizerFilter();
}

void ReverbMixer::on_PB_0DB_clicked()
{
	setAllEqualizerLevelUIs(0);
}

void ReverbMixer::on_PB_N20DB_clicked()
{
	setAllEqualizerLevelUIs(-20);
}

void ReverbMixer::on_PB_P20DB_clicked()
{
	setAllEqualizerLevelUIs(20);
}
void ReverbMixer::on_SL_B1_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::on_SL_B2_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::on_SL_B3_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::on_SL_B4_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::on_SL_B5_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::on_SL_B6_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::on_SL_B7_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::on_SL_B8_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::on_SL_B9_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::on_SL_B10_valueChanged(int value)
{
	adjustEqualizerFilter();
}
void ReverbMixer::adjustProcessorSettings()
{
	AMF_RESULT AMFErr;
	eOperationMode mode = eOperationMode::eUnset;
	int deviceID = 0;
	if (m_UIMainWindow.CB_Devices->currentIndex() == 0)
	{
		mode = eCPU;
	}
	else
	{
		mode = eGPU;
		deviceID = m_UIMainWindow.CB_Devices->currentIndex() - 1;
	}
	size_t numOfChannel = m_UIMainWindow.LE_NumOfChannel->text().toInt() > 2 ? m_UIMainWindow.LE_NumOfChannel->text().toInt() : 2;

	SHOW_MESSAGE_IF_AMF_FAIL(
		m_ReverbProcessor.init(mode, deviceID, m_UIMainWindow.SB_FilterLength->value(), m_UIMainWindow.SB_BufferLength->value(), numOfChannel),
		"Failed to init ReverbProcessor\n");
}

void ReverbMixer::adjustEqualizerFilter()
{
	// Porting UI changes
	m_pEqualizerLevels[0] = m_UIMainWindow.SL_B1->value();
	m_pEqualizerLevels[1] = m_UIMainWindow.SL_B2->value();
	m_pEqualizerLevels[2] = m_UIMainWindow.SL_B3->value();
	m_pEqualizerLevels[3] = m_UIMainWindow.SL_B4->value();
	m_pEqualizerLevels[4] = m_UIMainWindow.SL_B5->value();
	m_pEqualizerLevels[5] = m_UIMainWindow.SL_B6->value();
	m_pEqualizerLevels[6] = m_UIMainWindow.SL_B7->value();
	m_pEqualizerLevels[7] = m_UIMainWindow.SL_B8->value();
	m_pEqualizerLevels[8] = m_UIMainWindow.SL_B9->value();
	m_pEqualizerLevels[9] = m_UIMainWindow.SL_B10->value();
	adjustProcessorSettings();
	if (m_sSourceFilePath != "")
	{
		// Recreate the equalizer filter;
		size_t requireFilterLengthInComplex = m_UIMainWindow.SB_FilterLength->value();
		size_t numOfChannel = m_UIMainWindow.LE_NumOfChannel->text().toInt() > 2 ? m_UIMainWindow.LE_NumOfChannel->text().toInt() : 2;
		adjustEqualizerFilterSize(numOfChannel, requireFilterLengthInComplex);
		size_t sampleRate = m_UIMainWindow.LE_SourceSampleRate->text().toInt();
		size_t EQFilterLengthInComplexLog2 = base2toLog2_32Bit(requireFilterLengthInComplex);



		SHOW_MESSAGE_IF_AMF_FAIL(m_ReverbProcessor.fill10BandEQFilterFD(m_pEqualizerLevels, sampleRate, m_pEqualizerFilter, EQFilterLengthInComplexLog2, numOfChannel),
			"Fail to generate EQ Filte\n");
		if (m_iEqualizerFilterIndex == -1 || !m_ReverbProcessor.isFilterValid(m_iEqualizerFilterIndex))
		{
			//Filter for equalizer has not been initialized yet
			AMF_RESULT AMFErr = AMF_OK;
			m_iEqualizerFilterIndex = m_ReverbProcessor.addFilterFD(m_pEqualizerFilter, &AMFErr);
			SHOW_MESSAGE_IF_AMF_FAIL(AMFErr, "Failed to add equalizer filter");
			SHOW_MESSAGE_IF_AMF_FAIL(m_ReverbProcessor.processFilter(), "Failed to update Filter");
		}
		else
		{
			// Tell the processor to reprocess the filter
			SHOW_MESSAGE_IF_AMF_FAIL(m_ReverbProcessor.processFilter(), "Failed to update Filter");
		}
	}
}

void ReverbMixer::resetEqualizerFilter()
{
	adjustProcessorSettings();
	setAllEqualizerLevelUIs(0);
	adjustEqualizerFilter();
}

void ReverbMixer::updateDeviceName()
{
	int maxdevice = 10;
	char** deviceName = new char*[maxdevice];
	for (int i = 0; i < maxdevice; i++)
	{
		deviceName[i] = new char[MAX_PATH];
	}
	int device = m_ReverbProcessor.getDeviceNames(&deviceName, maxdevice);
	for (int i = 0; i < device; i++)
	{
		m_UIMainWindow.CB_Devices->addItem(deviceName[i]);
	}
}

void ReverbMixer::updateProcessorStatus()
{
	QPalette player_palette = m_UIMainWindow.LB_PlayerStatus->palette();
	QPalette recorder_pallette = m_UIMainWindow.LB_RecorderStatus->palette();
	if (m_ReverbProcessor.isPlayerPlaying())
	{
		player_palette.setColor(m_UIMainWindow.LB_PlayerStatus->foregroundRole(), Qt::red);
		m_UIMainWindow.LB_PlayerStatus->setPalette(player_palette);
		m_UIMainWindow.LB_PlayerStatus->setText("Running...");
	}
	else
	{
		player_palette.setColor(m_UIMainWindow.LB_RecorderStatus->foregroundRole(), Qt::blue);
		m_UIMainWindow.LB_PlayerStatus->setPalette(player_palette);
		m_UIMainWindow.LB_PlayerStatus->setText("Idling...");
	}
	if (m_ReverbProcessor.isRecorderRunning())
	{
		player_palette.setColor(m_UIMainWindow.LB_RecorderStatus->foregroundRole(), Qt::red);
		m_UIMainWindow.LB_RecorderStatus->setPalette(player_palette);
		m_UIMainWindow.LB_RecorderStatus->setText("Running...");
	}
	else
	{
		player_palette.setColor(m_UIMainWindow.LB_RecorderStatus->foregroundRole(), Qt::blue);
		m_UIMainWindow.LB_RecorderStatus->setPalette(player_palette);
		m_UIMainWindow.LB_RecorderStatus->setText("Idling...");
	}
}

void ReverbMixer::setAllEqualizerLevelUIs(int levelIndB)
{
	// Spin box
	m_UIMainWindow.SB_B1V->setValue(levelIndB);
	m_UIMainWindow.SB_B2V->setValue(levelIndB);
	m_UIMainWindow.SB_B3V->setValue(levelIndB);
	m_UIMainWindow.SB_B4V->setValue(levelIndB);
	m_UIMainWindow.SB_B5V->setValue(levelIndB);
	m_UIMainWindow.SB_B6V->setValue(levelIndB);
	m_UIMainWindow.SB_B7V->setValue(levelIndB);
	m_UIMainWindow.SB_B8V->setValue(levelIndB);
	m_UIMainWindow.SB_B9V->setValue(levelIndB);
	m_UIMainWindow.SB_B10V->setValue(levelIndB);
	// Slider
	m_UIMainWindow.SL_B1->setValue(levelIndB);
	m_UIMainWindow.SL_B2->setValue(levelIndB);
	m_UIMainWindow.SL_B3->setValue(levelIndB);
	m_UIMainWindow.SL_B4->setValue(levelIndB);
	m_UIMainWindow.SL_B5->setValue(levelIndB);
	m_UIMainWindow.SL_B6->setValue(levelIndB);
	m_UIMainWindow.SL_B7->setValue(levelIndB);
	m_UIMainWindow.SL_B8->setValue(levelIndB);
	m_UIMainWindow.SL_B9->setValue(levelIndB);
	m_UIMainWindow.SL_B10->setValue(levelIndB);
}

ReverbMixer::ReverbMixer(QWidget* parent)
{
	m_UIMainWindow.setupUi(this);
}

ReverbMixer::~ReverbMixer()
{
	for (size_t i = 0; i < m_iNumOfChannel; i++)
	{
		delete[]m_pEqualizerFilter[i];
	}
	delete[]m_pEqualizerFilter;
}

void ReverbMixer::Init()
{
	updateDeviceName();
	adjustEqualizerFilter();
	this->show();
}

float ReverbMixer::db2Mag(float db)
{
	return pow(10.0f, db / 10.0f);
}

unsigned int ReverbMixer::base2toLog2_32Bit(unsigned int base2)
{
	unsigned int v =base2;
	unsigned int c = 32;
	if (v) c--;
	if (v & 0x0000FFFF) c -= 16;
	if (v & 0x00FF00FF) c -= 8;
	if (v & 0x0F0F0F0F) c -= 4;
	if (v & 0x33333333) c -= 2;
	if (v & 0x55555555) c -= 1;
	return c;
}

void ReverbMixer::adjustEqualizerFilterSize(size_t numOfChannel, size_t sizePerChannelInComplex)
{
	
	if (sizePerChannelInComplex > m_iEQFilterLengthInComplex || numOfChannel > m_iNumOfChannel)
	{
		if (m_pEqualizerFilter!=nullptr)
		{
			for (size_t i = 0; i < m_iNumOfChannel; i++)
			{
				delete[]m_pEqualizerFilter[i];
			}
			delete[]m_pEqualizerFilter;
			m_pEqualizerFilter = nullptr;
		}
		m_pEqualizerFilter = new float*[numOfChannel];
		for (size_t i = 0; i < numOfChannel; i++)
		{
			m_pEqualizerFilter[i] = new float[sizePerChannelInComplex * 2];
			RtlSecureZeroMemory(m_pEqualizerFilter[i], sizePerChannelInComplex * 2 * sizeof(float));
		}
	}
	m_iNumOfChannel = numOfChannel;
	m_iEQFilterLengthInComplex = sizePerChannelInComplex;
}
