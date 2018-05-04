#include "QTSaveResponse.h"
#include "QFileDialog"
#define NUM_OF_SAMPLE_ITERATION 5
#define SR_SAMPLE_RATE_44K 44100
#define SR_SAMPLE_RATE_48K 48000
#define SR_BITS_PER_SAMPLE_32 32
#define SR_BITS_PER_SAMPLE_16 16

void QD_SaveEQResponseWindow::getSavePath()
{
	QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save As"), ".", tr("WAV Files (*.wav *.WAV"));
	if(saveFileName!="")
	{
		m_UISaveResponse.LE_OutputResponseName->setText(saveFileName);
	}
}

void QD_SaveEQResponseWindow::writeResponseToWAV()
{
	int NumberOfChannel = m_UISaveResponse.SB_NumOfChannel->value();
	int NumberOfSample = m_UISaveResponse.CB_NumOfSample->currentText().toInt();
	int SampleRate = m_UISaveResponse.CB_SampleRate->currentText().toInt();
	int BitsPerSample = m_UISaveResponse.CB_BitsPerSample->currentText().toInt();
	int log2Level = 0;
	while (NumberOfSample >>= 1) log2Level++;
	float** outputBuffer = nullptr;
	m_rReverbProcessor->generate10BandEQFilterTD(m_pEQResponse, SampleRate, &outputBuffer, log2Level, NumberOfChannel);
	m_rReverbProcessor->writeToWAV(outputBuffer, NumberOfChannel, SampleRate, BitsPerSample, 1 << log2Level, m_UISaveResponse.LE_OutputResponseName->text().toStdString().c_str());
	for (size_t i = 0; i < NumberOfSample; i++)
	{
		delete[]outputBuffer[i];
	}
	delete[]outputBuffer;
}

void QD_SaveEQResponseWindow::updateResponse(float in_pResponseLevel[10])
{
	if(in_pResponseLevel!=nullptr)
	{
		for (size_t i = 0; i < 10; i++)
		{
			m_pEQResponse[i] = in_pResponseLevel[i];
		}
	}
}


QD_SaveEQResponseWindow::QD_SaveEQResponseWindow(ReverbProcessor* in_rReverbProcessor)
{
	m_rReverbProcessor = in_rReverbProcessor;
	m_UISaveResponse.setupUi(this);
	initializeConfigs();
	connectSignals();
}

QD_SaveEQResponseWindow::~QD_SaveEQResponseWindow()
{
	m_rReverbProcessor = nullptr;
}

void QD_SaveEQResponseWindow::initializeConfigs()
{
	// Adding choices for number of samples
	int minNumOfSamples = 256;
	for (size_t i = 0; i < NUM_OF_SAMPLE_ITERATION; i++)
	{
		m_UISaveResponse.CB_NumOfSample->addItem(QString::fromStdString(std::to_string(minNumOfSamples << i)));
	}
	// Adding choice for Sample Rate
	m_UISaveResponse.CB_SampleRate->addItem(QString::fromStdString(std::to_string(SR_SAMPLE_RATE_44K)));
	m_UISaveResponse.CB_SampleRate->addItem(QString::fromStdString(std::to_string(SR_SAMPLE_RATE_48K)));
	// Adding choive for Bits Per Sample
	m_UISaveResponse.CB_BitsPerSample->addItem(QString::fromStdString(std::to_string(SR_BITS_PER_SAMPLE_32)));
	m_UISaveResponse.CB_BitsPerSample->addItem(QString::fromStdString(std::to_string(SR_BITS_PER_SAMPLE_16)));
}

void QD_SaveEQResponseWindow::connectSignals()
{
	connect(m_UISaveResponse.PB_BrowsResponse, &QPushButton::clicked, this, &QD_SaveEQResponseWindow::getSavePath);
	connect(m_UISaveResponse.buttonBox, &QDialogButtonBox::accepted, this, &QD_SaveEQResponseWindow::writeResponseToWAV);
}
