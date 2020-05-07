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
#include "QTExportResponse.h"

#include "RegisterBrowser.h"
#include "FileUtility.h"

#include <QFileDialog>
#include <QString>
#include <QTableWidgetItem>
#include <QList>
#include <QMessageBox>
#include <QTimer>

#include <iostream>
#include <cstring>

RoomAcousticQTConfig::RoomAcousticQTConfig(QWidget *parent):
    QMainWindow(parent)
{
	ConfigUi.setupUi(this);

	mTimer = new QTimer(this);

	QObject::connect(
		mTimer,
		&QTimer::timeout,
		[this]()
		{
			if(m_bDemoStarted)
			{
				m_RoomAcousticInstance.UpdateSoundSourcesPositions();

				for(int index(0); index < MAX_SOURCES; ++index)
				{
					updateSoundSourceGraphics(m_iCurrentSelectedSource);
				}
			}
		}
		);
	mTimer->start(250);

	// Initialize source table
	ConfigUi.SourcesTable->setRowCount(MAX_SOURCES);
	ConfigUi.SourcesTable->setColumnCount(1);

	ConfigUi.CB_AutoSpin->setEnabled(false);

	//Initialize devices
	mLockUpdate = true;
		ConfigUi.CB_RoomDevice->addItem("CPU");
		ConfigUi.CB_ConvolutionDevice->addItem("CPU");

		for (int i = 0; i < m_RoomAcousticInstance.mCPUDevicesCount; i++)
		{
			ConfigUi.CB_RoomDevice->addItem(QString::fromUtf8(m_RoomAcousticInstance.mCPUDevicesNames[i].c_str()));
			ConfigUi.CB_ConvolutionDevice->addItem(QString::fromUtf8(m_RoomAcousticInstance.mCPUDevicesNames[i].c_str()));
		}

		for (int i = 0; i < m_RoomAcousticInstance.mGPUDevicesCount; i++)
		{
			ConfigUi.CB_RoomDevice->addItem(QString::fromUtf8(m_RoomAcousticInstance.mGPUDevicesNames[i].c_str()));
			ConfigUi.CB_ConvolutionDevice->addItem(QString::fromUtf8(m_RoomAcousticInstance.mGPUDevicesNames[i].c_str()));
		}
	mLockUpdate = false;

	// Update Graphics
	QGraphicsScene* m_pGraphicsScene = new QGraphicsScene(this);

	m_RoomAcousticGraphic = new RoomAcousticGraphic(m_pGraphicsScene);
	ConfigUi.RoomView->setScene(m_pGraphicsScene);
	ConfigUi.RoomView->setZoomMode(eZoomMode_MiddleButton_Drag);

	//
#ifdef _WIN32
	ConfigUi.PlayerType->addItem(QString::fromUtf8("WASApi"));
#else
	#if !defined(__APPLE__) && !defined(__MACOSX)
		ConfigUi.PlayerType->addItem(QString::fromUtf8("ALSA"));
	#endif
#endif

#ifdef ENABLE_PORTAUDIO
	ConfigUi.PlayerType->addItem(QString::fromUtf8("PortAudio"));
#endif

	table_selection_changed(-1);
}

RoomAcousticQTConfig::~RoomAcousticQTConfig()
{

}

void RoomAcousticQTConfig::Init()
{
	m_RoomAcousticGraphic->clear();
	m_RoomAcousticInstance.loadConfiguration(m_RoomAcousticInstance.mConfigFileName);

	updateAllFields();
	updateRoomGraphic();
	initSoundSourceGraphic();
	initListenerGraphics();

	updateRoomGraphic();
	updateAllSoundSourceGraphics();
	updateListnerGraphics();

	showMaximized();
}

void RoomAcousticQTConfig::storeSelectedSoundSource()
{
	if(mLockUpdate)
	{
		return;
	}

	// save selected item on the graphcis scene

	// Save last selected item on the table widget
	if(m_iCurrentSelectedSource >= 0 && m_iCurrentSelectedSource < MAX_SOURCES)
	{
		m_RoomAcousticInstance.mSoundSourceEnable[m_iCurrentSelectedSource] = ConfigUi.CB_SoundSourceEnable->isChecked();
		if(!m_iCurrentSelectedSource)
		{
			m_RoomAcousticInstance.mSrc1EnableMic = ConfigUi.CB_UseMicroPhone->isChecked();
		}
		m_RoomAcousticInstance.mSrcTrackHead[m_iCurrentSelectedSource] = ConfigUi.CB_TrackHead->isChecked() ? 1 : 0;

		m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerX = ConfigUi.SB_SoundPositionX->value();
		m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerY = ConfigUi.SB_SoundPositionY->value();
		m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerZ = ConfigUi.SB_SoundPositionZ->value();

		if(m_bDemoStarted)
		{
			m_RoomAcousticInstance.updateSoundSourcePosition(m_iCurrentSelectedSource);
		}
	}
}

void RoomAcousticQTConfig::updateSelectedSoundSource()
{
	mLockUpdate = true;

	if(m_iCurrentSelectedSource >= 0 && m_iCurrentSelectedSource < m_RoomAcousticInstance.m_iNumOfWavFile)
	{
		if(!m_iCurrentSelectedSource)
		{
			ConfigUi.CB_UseMicroPhone->setChecked(m_RoomAcousticInstance.mSrc1EnableMic);
			ConfigUi.CB_UseMicroPhone->setEnabled(true);
		}
		else
		{
			ConfigUi.CB_UseMicroPhone->setEnabled(false);
			ConfigUi.CB_UseMicroPhone->setChecked(false);
		}

		ConfigUi.CB_SoundSourceEnable->setChecked(m_RoomAcousticInstance.mSoundSourceEnable[m_iCurrentSelectedSource]);
		ConfigUi.CB_SoundSourceEnable->setEnabled(true);

		ConfigUi.CB_TrackHead->setChecked(m_RoomAcousticInstance.mSrcTrackHead[m_iCurrentSelectedSource]);
		ConfigUi.CB_TrackHead->setEnabled(true);

		update_sound_position(
			m_iCurrentSelectedSource,
			m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerX,
			m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerY,
			m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerZ
			);
		ConfigUi.SB_SoundPositionX->setEnabled(true);
		ConfigUi.SB_SoundPositionY->setEnabled(true);
		ConfigUi.SB_SoundPositionZ->setEnabled(true);

		ConfigUi.RemoveSoundSourceButton->setEnabled(true);

		ConfigUi.SoundConfigurationGroup->setEnabled(true);
		ConfigUi.SoundSourcePositionGroup->setEnabled(
			!m_RoomAcousticInstance.mSrcTrackHead[m_iCurrentSelectedSource]
			);
	}
	else
	{
		ConfigUi.SoundConfigurationGroup->setEnabled(false);
		ConfigUi.SoundSourcePositionGroup->setEnabled(false);

		ConfigUi.CB_SoundSourceEnable->setEnabled(false);
		ConfigUi.CB_SoundSourceEnable->setChecked(false);
		ConfigUi.CB_UseMicroPhone->setEnabled(false);
		ConfigUi.CB_UseMicroPhone->setChecked(false);
		ConfigUi.CB_TrackHead->setEnabled(false);
		ConfigUi.CB_TrackHead->setChecked(false);

		ConfigUi.SB_SoundPositionX->setEnabled(false);
		ConfigUi.SB_SoundPositionX->setValue(0);
		ConfigUi.SB_SoundPositionY->setEnabled(false);
		ConfigUi.SB_SoundPositionY->setValue(0);
		ConfigUi.SB_SoundPositionZ->setEnabled(false);
		ConfigUi.SB_SoundPositionZ->setValue(0);
	}

	//todo: move to separated function
	ConfigUi.RemoveSoundSourceButton->setEnabled(m_iCurrentSelectedSource >= 0 && m_iCurrentSelectedSource < m_RoomAcousticInstance.m_iNumOfWavFile);
	ConfigUi.AddSoundSourceButton->setEnabled(m_RoomAcousticInstance.m_iNumOfWavFile < MAX_SOURCES);

	mLockUpdate = false;
}

void RoomAcousticQTConfig::updateAllFields()
{
	updateSoundsourceNames();
	updateSelectedSoundSource();
	updateRoomFields();
	updateListenerFields();
	updateConvolutionFields();

	ConfigUi.PlayerType->setCurrentText(m_RoomAcousticInstance.mPlayerName.c_str());
}

void RoomAcousticQTConfig::updateSoundsourceNames()
{
	if(mLockUpdate)
	{
		return;
	}

	// Update Fields in list view

	for (unsigned int i = 0; i < MAX_SOURCES; i++)
	{
		QTableWidgetItem* item = ConfigUi.SourcesTable->item(i, 0);
		if (item == nullptr)
		{
			item = new QTableWidgetItem();
			ConfigUi.SourcesTable->setItem(i, 0, item);
		}

		std::string display_name = "";

		if(m_RoomAcousticInstance.mWavFileNames[i].length())
		{
			// Check the file existance before assign it into the cell
			display_name = m_RoomAcousticInstance.mWavFileNames[i];

			if (!checkFileExist(m_RoomAcousticInstance.mWavFileNames[i]))
			{
				display_name += " <Invalid>";
			}

			if(!m_RoomAcousticInstance.mSoundSourceEnable[i])
			{
				display_name += " <Disabled>";
			}

			if(!i && m_RoomAcousticInstance.mSrc1EnableMic)
			{
				display_name += " <Mic>";
			}

			if(m_RoomAcousticInstance.mSrcTrackHead[i])
			{
				display_name += " <Trac>";
			}
		}

		item->setText(QString::fromStdString(display_name));
	}
}

void RoomAcousticQTConfig::updateRoomFields()
{
	mLockUpdate = true;

	ConfigUi.SB_RoomWidth->setValue(m_RoomAcousticInstance.m_RoomDefinition.width);
	ConfigUi.SB_RoomHeight->setValue(m_RoomAcousticInstance.m_RoomDefinition.height);
	ConfigUi.SB_RoomLength->setValue(m_RoomAcousticInstance.m_RoomDefinition.length);

	// Update the damping factor to UI
	ConfigUi.SB_RoomDampFront->setValue(DAMPTODB(m_RoomAcousticInstance.m_RoomDefinition.mFront.damp));
	ConfigUi.SB_RoomDampBack->setValue(DAMPTODB(m_RoomAcousticInstance.m_RoomDefinition.mBack.damp));
	ConfigUi.SB_RoomDampLeft->setValue(DAMPTODB(m_RoomAcousticInstance.m_RoomDefinition.mLeft.damp));
	ConfigUi.SB_RoomDampRight->setValue(DAMPTODB(m_RoomAcousticInstance.m_RoomDefinition.mRight.damp));
	ConfigUi.SB_RoomDampTop->setValue(DAMPTODB(m_RoomAcousticInstance.m_RoomDefinition.mTop.damp));
	ConfigUi.SB_RoomDampBottom->setValue(DAMPTODB(m_RoomAcousticInstance.m_RoomDefinition.mBottom.damp));

	ConfigUi.CB_RoomDevice->setCurrentIndex(
		!m_RoomAcousticInstance.mRoomOverCL
		    ? 0
			:
			(
				!m_RoomAcousticInstance.mRoomOverGPU
				    ? m_RoomAcousticInstance.mRoomDeviceIndex + 1
					: m_RoomAcousticInstance.mRoomDeviceIndex + 1 + m_RoomAcousticInstance.mCPUDevicesCount
			)
		);

	ConfigUi.RB_DEF4Room->setChecked(0 == m_RoomAcousticInstance.mRoomPriority);
	ConfigUi.RB_MPr4Room->setChecked(1 == m_RoomAcousticInstance.mRoomPriority);
	ConfigUi.RB_RTQ4Room->setChecked(2 == m_RoomAcousticInstance.mRoomPriority);

	ConfigUi.SB_RoomCU->setValue(m_RoomAcousticInstance.mRoomCUCount);
	ConfigUi.SB_RoomCU->setEnabled(
#ifdef RTQ_ENABLED
		m_RoomAcousticInstance.mRoomOverCL && (m_RoomAcousticInstance.mRoomPriority > 0)
#else
		false
#endif
		);

	ConfigUi.CULabelRoom->setEnabled(ConfigUi.SB_RoomCU->isEnabled());
	ConfigUi.RoomQueueGroup->setEnabled(
#ifdef RTQ_ENABLED
		m_RoomAcousticInstance.mRoomOverCL && m_RoomAcousticInstance.mRoomOverGPU
#else
		false
#endif
		);

	mLockUpdate = false;
}

void RoomAcousticQTConfig::updateConvolutionFields()
{
	mLockUpdate = true;

	ConfigUi.SB_ConvolutionLength->setValue(m_RoomAcousticInstance.m_iConvolutionLength);
	ConfigUi.SB_BufferSize->setValue(m_RoomAcousticInstance.m_iBufferSize);
	ConfigUi.LB_ConvolutionTime->setText(QString::fromStdString(std::to_string(m_RoomAcousticInstance.getConvolutionTime())));
	ConfigUi.LB_BufferTime->setText(QString::fromStdString(std::to_string(m_RoomAcousticInstance.getBufferTime())));

	//convolution method
	{
		if
		(
			m_RoomAcousticInstance.mConvolutionOverCL
			&&
			m_RoomAcousticInstance.mConvolutionOverGPU
		)
		{
			update_convMethod(true);
			//todo: set index manually, enum could be changed in future!
		}
		else
		{
			update_convMethod(false);
//			ConfigUi.CB_ConvMethod->setCurrentIndex(0);
		}
		//ConfigUi.CB_ConvMethod->setCurrentIndex(m_RoomAcousticInstance.m_eConvolutionMethod);
		ConfigUi.CB_ConvMethod->setCurrentIndex(0);
	}

	ConfigUi.CB_ConvolutionDevice->setCurrentIndex(
		!m_RoomAcousticInstance.mConvolutionOverCL
		    ? 0
			:
			(
				!m_RoomAcousticInstance.mConvolutionOverGPU
				    ? m_RoomAcousticInstance.mConvolutionDeviceIndex + 1
					: m_RoomAcousticInstance.mConvolutionDeviceIndex + 1 + m_RoomAcousticInstance.mCPUDevicesCount
			)
		);

	ConfigUi.RB_DEF4Conv->setChecked(0 == m_RoomAcousticInstance.mConvolutionPriority);
	ConfigUi.RB_MPr4Conv->setChecked(1 == m_RoomAcousticInstance.mConvolutionPriority);
	ConfigUi.RB_RTQ4Conv->setChecked(2 == m_RoomAcousticInstance.mConvolutionPriority);

	ConfigUi.SB_ConvCU->setValue(m_RoomAcousticInstance.mConvolutionCUCount);
	ConfigUi.SB_ConvCU->setEnabled(
#ifdef RTQ_ENABLED
		m_RoomAcousticInstance.mConvolutionOverCL && (m_RoomAcousticInstance.mConvolutionPriority > 0)
#else
		false
#endif
		);

	ConfigUi.CULabelConvolution->setEnabled(ConfigUi.SB_ConvCU->isEnabled());
	ConfigUi.ConvQueueGroup->setEnabled(
#ifdef RTQ_ENABLED
		m_RoomAcousticInstance.mConvolutionOverCL && m_RoomAcousticInstance.mConvolutionOverGPU
#else
		false
#endif
		);

	mLockUpdate = false;
}

void RoomAcousticQTConfig::updateListenerFields()
{
	mLockUpdate = true;

	ConfigUi.SB_HeadPositionX->setValue(m_RoomAcousticInstance.m_Listener.headX);
	ConfigUi.SB_HeadPositionY->setValue(m_RoomAcousticInstance.m_Listener.headY);
	ConfigUi.SB_HeadPositionZ->setValue(m_RoomAcousticInstance.m_Listener.headZ);

	ConfigUi.SB_HeadYaw->setValue(m_RoomAcousticInstance.m_Listener.yaw);
	ConfigUi.SB_HeadPitch->setValue(m_RoomAcousticInstance.m_Listener.pitch);
	ConfigUi.SB_HeadRoll->setValue(m_RoomAcousticInstance.m_Listener.roll);

	ConfigUi.SB_EarSpacing->setValue(m_RoomAcousticInstance.m_Listener.earSpacing);

	mLockUpdate = false;
}

void RoomAcousticQTConfig::updateReverbFields()
{
	int nReflection60(0);
	float reverbtime60 = m_RoomAcousticInstance.getReverbTime(60, &nReflection60);

	int nReflection120(0);
	float reverbtime120 = m_RoomAcousticInstance.getReverbTime(120, &nReflection120);

	ConfigUi.LB_T60Reflection->setText(QString::fromStdString(std::to_string(nReflection60)));
	ConfigUi.LB_T120Reflection->setText(QString::fromStdString(std::to_string(nReflection120)));
	ConfigUi.LB_T60ResponseTime->setText(QString::fromStdString(std::to_string(reverbtime60)));
	ConfigUi.LB_T120ResponseTime->setText(QString::fromStdString(std::to_string(reverbtime120)));
}

void RoomAcousticQTConfig::setEnableHeadPositionFields(bool enable)
{
	ConfigUi.HeadPositionGroup->setEnabled(enable);
}

void RoomAcousticQTConfig::storeAllFieldsToInstance()
{
	if(mLockUpdate)
	{
		return;
	}

	storeSelectedSoundSource();
	storeListenerPosition();
	storeRoomFields();
	storeConvolutionFields();

	m_RoomAcousticInstance.mPlayerName = ConfigUi.PlayerType->currentText().toStdString();
}

void RoomAcousticQTConfig::storeListenerPosition()
{
	if(mLockUpdate)
	{
		return;
	}

	// Since wave file names are handled, we only need to port in the other stuff
	// Porting Listener configuration
	m_RoomAcousticInstance.m_Listener.headX = ConfigUi.SB_HeadPositionX->value();
	m_RoomAcousticInstance.m_Listener.headY = ConfigUi.SB_HeadPositionY->value();
	m_RoomAcousticInstance.m_Listener.headZ = ConfigUi.SB_HeadPositionZ->value();
	m_RoomAcousticInstance.m_Listener.pitch = ConfigUi.SB_HeadPitch->value();
	m_RoomAcousticInstance.m_Listener.yaw = ConfigUi.SB_HeadYaw->value();
	m_RoomAcousticInstance.m_Listener.roll = ConfigUi.SB_HeadRoll->value();
	m_RoomAcousticInstance.m_Listener.earSpacing = ConfigUi.SB_EarSpacing->value();

	m_RoomAcousticInstance.m_iHeadAutoSpin = ConfigUi.CB_AutoSpin->isChecked() ?  1 : 0;

	if(m_bDemoStarted)
	{
		m_RoomAcousticInstance.updateListenerPosition();
	}
}

void RoomAcousticQTConfig::storeRoomFields()
{
	if(mLockUpdate)
	{
		return;
	}

	m_RoomAcousticInstance.m_RoomDefinition.width = ConfigUi.SB_RoomWidth->value();
	m_RoomAcousticInstance.m_RoomDefinition.height = ConfigUi.SB_RoomHeight->value();
	m_RoomAcousticInstance.m_RoomDefinition.length = ConfigUi.SB_RoomLength->value();

	m_RoomAcousticInstance.m_RoomDefinition.mLeft.damp = DBTODAMP(ConfigUi.SB_RoomDampLeft->value());
	m_RoomAcousticInstance.m_RoomDefinition.mRight.damp = DBTODAMP(ConfigUi.SB_RoomDampRight->value());
	m_RoomAcousticInstance.m_RoomDefinition.mTop.damp = DBTODAMP(ConfigUi.SB_RoomDampTop->value());
	m_RoomAcousticInstance.m_RoomDefinition.mBottom.damp = DBTODAMP(ConfigUi.SB_RoomDampBottom->value());
	m_RoomAcousticInstance.m_RoomDefinition.mFront.damp = DBTODAMP(ConfigUi.SB_RoomDampFront->value());
	m_RoomAcousticInstance.m_RoomDefinition.mBack.damp = DBTODAMP(ConfigUi.SB_RoomDampBack->value());

	auto currentIndex(ConfigUi.CB_RoomDevice->currentIndex());

	m_RoomAcousticInstance.mRoomOverCL = currentIndex > 0;
	if(m_RoomAcousticInstance.mRoomOverCL)
	{
		--currentIndex; //exclude first software item

		if(currentIndex < m_RoomAcousticInstance.mCPUDevicesCount)
		{
			m_RoomAcousticInstance.mRoomOverGPU = false;
			m_RoomAcousticInstance.mRoomDeviceIndex = currentIndex;
		}
		else
		{
			currentIndex -= m_RoomAcousticInstance.mCPUDevicesCount;

			m_RoomAcousticInstance.mRoomOverGPU = true;
			m_RoomAcousticInstance.mRoomDeviceIndex = currentIndex;
		}
	}
	else
	{
		m_RoomAcousticInstance.mRoomOverGPU = false;
		m_RoomAcousticInstance.mRoomDeviceIndex = 0;
	}

#ifdef RTQ_ENABLED
	m_RoomAcousticInstance.mRoomPriority = ConfigUi.RB_RTQ4Room->isChecked()
	    ? 2
		: ( ConfigUi.RB_MPr4Room->isChecked() ? 1 : 0);
	m_RoomAcousticInstance.mRoomCUCount = ConfigUi.SB_RoomCU->value();
#else
    m_RoomAcousticInstance.mRoomPriority = 0;
	m_RoomAcousticInstance.mRoomCUCount = 0;
#endif

	updateReverbFields();

	if(m_bDemoStarted)
	{
		m_RoomAcousticInstance.updateRoomDamping();
	}
}

void RoomAcousticQTConfig::storeConvolutionFields()
{
	m_RoomAcousticInstance.m_iConvolutionLength = ConfigUi.SB_ConvolutionLength->value();

	auto currentIndex(ConfigUi.CB_ConvolutionDevice->currentIndex());

	m_RoomAcousticInstance.mConvolutionOverCL = currentIndex > 0;

	bool gpuReset = false;

	if(m_RoomAcousticInstance.mConvolutionOverCL)
	{
		--currentIndex; //exclude first software item

		if(currentIndex < m_RoomAcousticInstance.mCPUDevicesCount)
		{
			if(m_RoomAcousticInstance.mConvolutionOverGPU)
			{
				gpuReset = true;
			}

			m_RoomAcousticInstance.mConvolutionOverGPU = false;
			m_RoomAcousticInstance.mConvolutionDeviceIndex = currentIndex;
		}
		else
		{
			currentIndex -= m_RoomAcousticInstance.mCPUDevicesCount;

			m_RoomAcousticInstance.mConvolutionOverGPU = true;
			m_RoomAcousticInstance.mConvolutionDeviceIndex = currentIndex;
		}
	}
	else
	{
		m_RoomAcousticInstance.mConvolutionOverGPU = false;
		m_RoomAcousticInstance.mConvolutionDeviceIndex = 0;
	}

	if(gpuReset)
	{
		m_RoomAcousticInstance.m_eConvolutionMethod = TAN_CONVOLUTION_METHOD_FFT_OVERLAP_ADD;
	}
	else
	{
		m_RoomAcousticInstance.m_eConvolutionMethod = m_RoomAcousticInstance.getConvMethodFlag(ConfigUi.CB_ConvMethod->currentText().toStdString());
	}

#ifdef RTQ_ENABLED
	m_RoomAcousticInstance.mConvolutionPriority = ConfigUi.RB_RTQ4Conv->isChecked()
	    ? 2
		: ( ConfigUi.RB_MPr4Conv->isChecked() ? 1 : 0);
	m_RoomAcousticInstance.mConvolutionCUCount = ConfigUi.SB_ConvCU->value();
#else
    m_RoomAcousticInstance.mConvolutionPriority = 0;
	m_RoomAcousticInstance.mConvolutionCUCount = 0;
#endif
}

void RoomAcousticQTConfig::printConfiguration()
{
	// Printing Source Name
	qInfo("Number of wav files: %d", m_RoomAcousticInstance.m_iNumOfWavFile);
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		qInfo(
			"Source %d: Name: %s, Position: (%f,%f,%f), Track: %d",
			i,
			m_RoomAcousticInstance.mWavFileNames[i].c_str(),
			m_RoomAcousticInstance.m_SoundSources[i].speakerX,
			m_RoomAcousticInstance.m_SoundSources[i].speakerY,
			m_RoomAcousticInstance.m_SoundSources[i].speakerZ,
			m_RoomAcousticInstance.mSrcTrackHead[i]
			);
	}

	// Print Listener configuration
	qInfo("Head Position: %f,%f,%f. Pitch: %f, Yaw: %f, Roll: %f",
		m_RoomAcousticInstance.m_Listener.headX,
		m_RoomAcousticInstance.m_Listener.headY,
		m_RoomAcousticInstance.m_Listener.headZ,

		m_RoomAcousticInstance.m_Listener.pitch,
		m_RoomAcousticInstance.m_Listener.yaw,
		m_RoomAcousticInstance.m_Listener.roll
		);
	qInfo("Head Configuration: Auto Spin: %d, Ear Spacing: %f",
	    m_RoomAcousticInstance.m_iHeadAutoSpin,
		m_RoomAcousticInstance.m_Listener.earSpacing
		);

	// Print Room Infomation
	qInfo("Room Definition: width: %f, Height: %f, Length: %f", m_RoomAcousticInstance.m_RoomDefinition.width,
		m_RoomAcousticInstance.m_RoomDefinition.height, m_RoomAcousticInstance.m_RoomDefinition.length);
	qInfo("Wall Damping factors: Left: %f, Right: %f, Front: %f, Back: %f, Top: %f, Botton: %f",
		this->m_RoomAcousticInstance.m_RoomDefinition.mLeft.damp, this->m_RoomAcousticInstance.m_RoomDefinition.mRight.damp,
		this->m_RoomAcousticInstance.m_RoomDefinition.mFront.damp, this->m_RoomAcousticInstance.m_RoomDefinition.mBack.damp,
		this->m_RoomAcousticInstance.m_RoomDefinition.mTop.damp, this->m_RoomAcousticInstance.m_RoomDefinition.mBottom.damp);

	// Print Convolution Infomation
	qInfo("Convolution length: %d, Buffer sieze: %d", this->m_RoomAcousticInstance.m_iConvolutionLength, this->m_RoomAcousticInstance.m_iBufferSize);
#ifdef RTQ_ENABLED
	qInfo("Convolution using OpenCL: %c, Running On GPU: %c, Normal queue: %c, Medium Queue: %c, RTQ: %c, CUS: %d",
		m_RoomAcousticInstance.mConvolutionOverCL ? 'y' : 'n',
		m_RoomAcousticInstance.mConvolutionOverGPU ? 'y' : 'n',
		m_RoomAcousticInstance.mConvolutionPriority == 0 ? 'y' : 'n',
		m_RoomAcousticInstance.mConvolutionPriority == 1 ? 'y' : 'n',
		m_RoomAcousticInstance.mConvolutionPriority == 2 ? 'y' : 'n',
		m_RoomAcousticInstance.mConvolutionCUCount
		);
	qInfo("Room using OpenCL: %c, Running On GPU: %c, Normal queue: %c, Medium Queue: %c, RTQ: %c, CUS: %d",
		m_RoomAcousticInstance.mRoomOverCL ? 'y' : 'n',
		m_RoomAcousticInstance.mRoomOverGPU ? 'y' : 'n',
		m_RoomAcousticInstance.mRoomPriority == 0 ? 'y' : 'n',
		m_RoomAcousticInstance.mRoomPriority == 1 ? 'y' : 'n',
		m_RoomAcousticInstance.mRoomPriority == 2 ? 'y' : 'n',
		m_RoomAcousticInstance.mRoomCUCount
		);
#endif
}

//todo: retrive version information for linux and mac
//may be lspci?
std::string RoomAcousticQTConfig::getDriverVersion()
{
#ifdef _WIN32
	WindowsRegister newreg(HKEY_LOCAL_MACHINE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}");
	WindowsRegister* subkeys = newreg.getSubKeys();
	for (int i = 0; i < newreg.getNumOfSubKeys(); i++)
	{
		subkeys[i].printSubKeyInfo();
		for (int j = 0; j < subkeys[i].getNumOfSubKeys(); i++)
		{
			if (subkeys[i].hasSubKey("VolatileSettings"))
			{
				std::string version = subkeys[i].getStringValue("ReleaseVersion");
				return version;
			}
		}
	}
#endif

	return "";
}

std::string RoomAcousticQTConfig::getTANVersion()
{
#ifdef _WIN32
	std::string current_dir = getCurrentDirectory();
	std::string dll_dir = joinPaths(current_dir, "tanrt64.dll");
	return getFileVersionString(dll_dir);
#else
	return "Error: not implemented!";
#endif
}

void RoomAcousticQTConfig::setHeadSpinTimeInterval(float interval)
{
	m_pHeadAnimationTimeline->setDuration(interval);
}

void RoomAcousticQTConfig::startHeadSpinAnimation()
{
	m_pHeadAnimationTimeline->start();
}

void RoomAcousticQTConfig::stopHeadSpinAnimation()
{
	m_pHeadAnimationTimeline->stop();
}

void RoomAcousticQTConfig::updateRoomGraphic()
{
	m_RoomAcousticGraphic->update_Room_definition(ConfigUi.SB_RoomWidth->value(), 0, ConfigUi.SB_RoomLength->value());
}

void RoomAcousticQTConfig::updateSoundSourceGraphics(int index)
{
	if (m_RoomAcousticInstance.mWavFileNames[index].length())
	{
		m_RoomAcousticGraphic->update_sound_source_position(
			index,
			m_RoomAcousticInstance.m_SoundSources[index].speakerX,
			m_RoomAcousticInstance.m_SoundSources[index].speakerY,
			m_RoomAcousticInstance.m_SoundSources[index].speakerZ
			);
	}
}

void RoomAcousticQTConfig::initSoundSourceGraphic()
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if (m_RoomAcousticInstance.mWavFileNames[i].length())
		{
			addSoundsourceGraphics(i);
		}
	}
}

void RoomAcousticQTConfig::initListenerGraphics()
{
	addListenerGraphics();
}

void RoomAcousticQTConfig::updateAllSoundSourceGraphics()
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if (m_RoomAcousticInstance.mWavFileNames[i].length())
		{
			m_RoomAcousticGraphic->update_sound_source_position(i,
				m_RoomAcousticInstance.m_SoundSources[i].speakerX,
				m_RoomAcousticInstance.m_SoundSources[i].speakerY,
				m_RoomAcousticInstance.m_SoundSources[i].speakerZ);
		}
	}
}

void RoomAcousticQTConfig::updateListnerGraphics()
{
	if (this->m_RoomAcousticGraphic->m_pListener == nullptr){
		m_RoomAcousticGraphic->update_Listener_position(m_RoomAcousticInstance.m_Listener.headX,
			m_RoomAcousticInstance.m_Listener.headY,
			m_RoomAcousticInstance.m_Listener.headZ,
			m_RoomAcousticInstance.m_Listener.pitch,
			m_RoomAcousticInstance.m_Listener.yaw,
			m_RoomAcousticInstance.m_Listener.roll);
		QObject::connect(m_RoomAcousticGraphic->m_pListener, &RoomAcousticListenerGraphics::top_view_position_changed,
			this, &RoomAcousticQTConfig::update_listener_position_top_view);
		// Setting up animation
		m_pHeadAnimationTimeline = new QTimeLine();
		m_pHeadAnimationTimeline->setLoopCount(0);
		m_pHeadAnimationTimeline->setCurveShape(QTimeLine::LinearCurve);
		QObject::connect(m_pHeadAnimationTimeline, &QTimeLine::valueChanged, m_RoomAcousticGraphic->m_pListener, &RoomAcousticListenerGraphics::rotateListenerYaw);
		QObject::connect(m_RoomAcousticGraphic->m_pListener, &RoomAcousticListenerGraphics::top_view_orientation_changed, this, &RoomAcousticQTConfig::update_listener_orientation_top_view);
		setHeadSpinTimeInterval(5000);
	}
	else
	{
		m_RoomAcousticGraphic->update_Listener_position(m_RoomAcousticInstance.m_Listener.headX,
			m_RoomAcousticInstance.m_Listener.headY,
			m_RoomAcousticInstance.m_Listener.headZ,
			m_RoomAcousticInstance.m_Listener.pitch,
			m_RoomAcousticInstance.m_Listener.yaw,
			m_RoomAcousticInstance.m_Listener.roll);
	}
}

void RoomAcousticQTConfig::addSoundsourceGraphics(int index)
{
	m_RoomAcousticGraphic->update_sound_source_position(
		index,
		m_RoomAcousticInstance.m_SoundSources[index].speakerX,
		m_RoomAcousticInstance.m_SoundSources[index].speakerY,
		m_RoomAcousticInstance.m_SoundSources[index].speakerZ
		);

	QObject::connect(
		m_RoomAcousticGraphic->m_pSoundSource[index],
		&RoomAcousticSoundSourceGraphics::top_view_position_changed,
		this,
		&RoomAcousticQTConfig::update_sound_position_top_view
		);
	QObject::connect(
		m_RoomAcousticGraphic->m_pSoundSource[index],
		&RoomAcousticSoundSourceGraphics::current_selection_changed,
		this,
		&RoomAcousticQTConfig::table_selection_changed
		);
}

void RoomAcousticQTConfig::addListenerGraphics()
{
	m_RoomAcousticGraphic->update_Listener_position(m_RoomAcousticInstance.m_Listener.headX,
		m_RoomAcousticInstance.m_Listener.headY,
		m_RoomAcousticInstance.m_Listener.headZ,
		m_RoomAcousticInstance.m_Listener.pitch,
		m_RoomAcousticInstance.m_Listener.yaw,
		m_RoomAcousticInstance.m_Listener.roll);
	QObject::connect(m_RoomAcousticGraphic->m_pListener, &RoomAcousticListenerGraphics::top_view_position_changed,
		this, &RoomAcousticQTConfig::update_listener_position_top_view);
}

void RoomAcousticQTConfig::removeSoundsourceGraphics(int index)
{
	this->m_RoomAcousticGraphic->remove_sound_source(index);
}

void RoomAcousticQTConfig::storeTrackedHeadSource()
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		if (m_RoomAcousticInstance.mSrcTrackHead[i])
		{
			update_sound_position(i, m_RoomAcousticInstance.m_Listener.headX, m_RoomAcousticInstance.m_Listener.headY, m_RoomAcousticInstance.m_Listener.headZ);
		}
	}
}

/*QT Slots: Triggered when loading configuration file action clicked*/
void RoomAcousticQTConfig::on_actionLoad_Config_File_triggered()
{
	QString fileName;

	{
		fileName = QFileDialog::getOpenFileName(
			this,
			tr("Open Configuration File"),
			m_RoomAcousticInstance.mTANDLLPath.c_str(),
			tr("Configuration File (*.xml)")
			);
	}

	if(fileName.length())
	{
		m_RoomAcousticGraphic->clear();
		m_RoomAcousticInstance.loadConfiguration(fileName.toStdString());

		m_iCurrentSelectedSource = m_RoomAcousticInstance.m_iNumOfWavFile
			? 0
			: -1;

		updateAllFields();

		initSoundSourceGraphic();
		initListenerGraphics();

		updateRoomGraphic();
		updateAllSoundSourceGraphics();
		updateListnerGraphics();

		if(m_RoomAcousticInstance.m_iNumOfWavFile)
		{
			mLockUpdate = true;
			QTableWidgetItem* selected_item = ConfigUi.SourcesTable->item(m_iCurrentSelectedSource, 0);
			ConfigUi.SourcesTable->setCurrentItem(selected_item);
			mLockUpdate = false;
		}
	}
}
/*QT Slots: Triggered when saving configuration file action clicked*/
void RoomAcousticQTConfig::on_actionSave_Config_File_triggered()
{
	QString fileName;

	{
		fileName = QFileDialog::getSaveFileName(
			this,
			tr("Save Configuration File"),
			m_RoomAcousticInstance.mTANDLLPath.c_str(),
			tr("Configuration File (*.xml)")
			);

	}

	if(fileName.length())
	{
		storeAllFieldsToInstance();
	    m_RoomAcousticInstance.saveConfiguraiton(fileName.toStdString());
	}
}

void RoomAcousticQTConfig::on_actionAbout_triggered()
{
	std::string driverversion = "Driver Version: " + getDriverVersion() + '\n';
	std::string tandllversion = "True Audio Next Version: " + getTANVersion() + '\n';
	QMessageBox info("About:", QString::fromStdString(driverversion+tandllversion),
		QMessageBox::Information,
		QMessageBox::Ok | QMessageBox::Default,
		QMessageBox::NoButton,
		QMessageBox::NoButton);
	info.exec();
}

void RoomAcousticQTConfig::on_actionExport_Response_triggered()
{
	QTExportResponse newWindow(nullptr, Qt::Dialog);
	newWindow.Init(&m_RoomAcousticInstance);
	newWindow.exec();
}

void RoomAcousticQTConfig::on_RemoveSoundSourceButton_clicked()
{
	QList<QTableWidgetItem*> selected_sources = ConfigUi.SourcesTable->selectedItems();

	int firstSelectedIndex = -1;

	for (int i = 0; i < selected_sources.size(); i++)
	{
		QTableWidgetItem* selected_source = selected_sources[i];

		int row = selected_source->row();

		if(firstSelectedIndex == -1)
		{
			firstSelectedIndex = row;
		}

		m_RoomAcousticInstance.removeSoundSource(row);
		removeSoundsourceGraphics(row);
	}

	m_iCurrentSelectedSource = firstSelectedIndex >= 0 && firstSelectedIndex < m_RoomAcousticInstance.m_iNumOfWavFile
	  	? firstSelectedIndex
		: -1;

	updateSoundsourceNames();
	updateSelectedSoundSource();
}

void RoomAcousticQTConfig::on_SourcesTable_currentCellChanged(int row, int currentColumn, int previousRow, int previousColumn)
{
	if(mLockUpdate)
	{
		return;
	}

	table_selection_changed(row);
}

void RoomAcousticQTConfig::on_CB_SoundSourceEnable_stateChanged(int state)
{
	if(mLockUpdate)
	{
		return;
	}

	storeSelectedSoundSource();
	updateSoundsourceNames();
}

void RoomAcousticQTConfig::on_CB_UseMicroPhone_stateChanged(int state)
{
	if(mLockUpdate)
	{
		return;
	}

	storeSelectedSoundSource();
	updateSoundsourceNames();
}

void RoomAcousticQTConfig::on_CB_TrackHead_stateChanged(int state)
{
	if(mLockUpdate)
	{
		return;
	}

	storeSelectedSoundSource();
	updateSoundsourceNames();
	updateSelectedSoundSource();

	m_RoomAcousticGraphic->m_pSoundSource[m_iCurrentSelectedSource]->setTrackHead(state ? true : false);
}

void RoomAcousticQTConfig::on_CB_AutoSpin_stateChanged(int stage)
{
	if(mLockUpdate)
	{
		return;
	}

	if(ConfigUi.CB_AutoSpin->isChecked())
	{
		startHeadSpinAnimation();
	}
	else
	{
		stopHeadSpinAnimation();
	}
}

void RoomAcousticQTConfig::on_SB_ConvolutionLength_valueChanged(int value)
{
	if(mLockUpdate)
	{
		return;
	}

	m_RoomAcousticInstance.m_iConvolutionLength = value;
	ConfigUi.LB_ConvolutionTime->setText(QString::fromStdString(std::to_string(m_RoomAcousticInstance.getConvolutionTime())));
}

void RoomAcousticQTConfig::on_SB_BufferSize_valueChanged(int value)
{
	if(mLockUpdate)
	{
		return;
	}

	m_RoomAcousticInstance.m_iBufferSize = value;
	ConfigUi.LB_BufferTime->setText(QString::fromStdString(std::to_string(m_RoomAcousticInstance.getBufferTime())));
}


void RoomAcousticQTConfig::on_SB_RoomWidth_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomLength_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomHeight_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampLeft_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampRight_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampTop_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampBottom_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampFront_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampBack_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_SoundPositionX_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeSelectedSoundSource();
	updateSoundSourceGraphics(m_iCurrentSelectedSource);
}

void RoomAcousticQTConfig::on_SB_SoundPositionY_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeSelectedSoundSource();
	updateSoundSourceGraphics(m_iCurrentSelectedSource);
}

void RoomAcousticQTConfig::on_SB_SoundPositionZ_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeSelectedSoundSource();
	updateSoundSourceGraphics(m_iCurrentSelectedSource);
}

void RoomAcousticQTConfig::on_SB_HeadPitch_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeListenerPosition();
	updateListnerGraphics();
}

void RoomAcousticQTConfig::on_SB_HeadYaw_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeListenerPosition();
	updateListnerGraphics();
}

void RoomAcousticQTConfig::on_SB_HeadRoll_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeListenerPosition();
	updateListnerGraphics();
}

void RoomAcousticQTConfig::on_SB_HeadPositionX_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeListenerPosition();
	updateListnerGraphics();
}

void RoomAcousticQTConfig::on_SB_HeadPositionY_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeListenerPosition();
	updateListnerGraphics();
}

void RoomAcousticQTConfig::on_SB_HeadPositionZ_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}

	storeListenerPosition();
	updateListnerGraphics();
}

void RoomAcousticQTConfig::on_CB_RoomDevice_currentIndexChanged(int index)
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateRoomFields();
}

void RoomAcousticQTConfig::on_CB_ConvolutionDevice_currentIndexChanged(int index)
{
	if(mLockUpdate)
	{
		return;
	}

	storeConvolutionFields();
	updateConvolutionFields();
}

void RoomAcousticQTConfig::on_CB_ConvMethod_currentIndexChanged(int index)
{
	if(mLockUpdate)
	{
		return;
	}

	storeConvolutionFields();
	//updateConvolutionFields();
}

void RoomAcousticQTConfig::on_RB_DEF4Room_clicked()
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateRoomFields();
}

void RoomAcousticQTConfig::on_RB_MPr4Room_clicked()
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateRoomFields();
}

void RoomAcousticQTConfig::on_RB_RTQ4Room_clicked()
{
	if(mLockUpdate)
	{
		return;
	}

	storeRoomFields();
	updateRoomFields();
}

void RoomAcousticQTConfig::on_RB_DEF4Conv_clicked()
{
	if(mLockUpdate)
	{
		return;
	}

	storeConvolutionFields();
	updateConvolutionFields();
}

void RoomAcousticQTConfig::on_RB_MPr4Conv_clicked()
{
	if(mLockUpdate)
	{
		return;
	}

	storeConvolutionFields();
	updateConvolutionFields();
}

void RoomAcousticQTConfig::on_RB_RTQ4Conv_clicked()
{
	if(mLockUpdate)
	{
		return;
	}

	storeConvolutionFields();
	updateConvolutionFields();
}

void RoomAcousticQTConfig::on_PB_RunDemo_clicked()
{
	if(!m_bDemoStarted)
	{
		storeAllFieldsToInstance();

#ifdef _DEBUG
		printConfiguration();
#endif

		m_bDemoStarted = m_RoomAcousticInstance.start();

		if(m_bDemoStarted)
		{
			ConfigUi.PB_RunDemo->setText("Stop");
			ConfigUi.ConvolutionSettings->setEnabled(false);
			ConfigUi.RoomOptions->setEnabled(false);
			ConfigUi.AddSoundSourceButton->setEnabled(false);
			ConfigUi.RemoveSoundSourceButton->setEnabled(false);
		}
		else
		{
			QMessageBox::critical(
				this,
				"Error",
				m_RoomAcousticInstance.getLastError().length()
				    ? m_RoomAcousticInstance.getLastError().c_str()
					: "Could not start playing, please see output log!"
				);

			if(m_RoomAcousticInstance.getLastError().length())
			{
				std::cerr << m_RoomAcousticInstance.getLastError() << std::endl;
			}
		}
	}
	else
	{
		m_RoomAcousticInstance.stop();
		m_bDemoStarted = false;

		ConfigUi.PB_RunDemo->setText("Run");
		ConfigUi.ConvolutionSettings->setEnabled(true);
		ConfigUi.RoomOptions->setEnabled(true);
		//todo: move to separated function
		ConfigUi.AddSoundSourceButton->setEnabled(m_RoomAcousticInstance.m_iNumOfWavFile < MAX_SOURCES);
		ConfigUi.RemoveSoundSourceButton->setEnabled(m_iCurrentSelectedSource >= 0 && m_iCurrentSelectedSource < m_RoomAcousticInstance.m_iNumOfWavFile);
	}
}

void RoomAcousticQTConfig::table_selection_changed(int index)
{
	if(mLockUpdate)
	{
		return;
	}

	//save previously selected params
	//storeSelectedSoundSource();

	if(index >= 0 && index <  m_RoomAcousticInstance.m_iNumOfWavFile)
	{
		m_iCurrentSelectedSource = index;
	}
	else
	{
		m_iCurrentSelectedSource = -1;
	}


	updateSelectedSoundSource();
}

void RoomAcousticQTConfig::update_sound_position(int index, float x, float y, float z)
{
	m_RoomAcousticInstance.m_SoundSources[index].speakerY = y;
	m_RoomAcousticInstance.m_SoundSources[index].speakerX = x;
	m_RoomAcousticInstance.m_SoundSources[index].speakerZ = z;

	updateSoundSourceGraphics(index);

	if(index == m_iCurrentSelectedSource)
	{
		ConfigUi.SB_SoundPositionX->setValue(m_RoomAcousticInstance.m_SoundSources[index].speakerX);
		ConfigUi.SB_SoundPositionY->setValue(m_RoomAcousticInstance.m_SoundSources[index].speakerY);
		ConfigUi.SB_SoundPositionZ->setValue(m_RoomAcousticInstance.m_SoundSources[index].speakerZ);
	}

//	if (m_bDemoStarted && m_RoomAcousticInstance.m_iSoundSourceEnable[index])
//	{
//		this->m_RoomAcousticInstance.updateSoundSourcePosition(index);
//	}
}

void RoomAcousticQTConfig::update_sound_position_top_view(int index, float x, float y)
{
	if (index >= MAX_SOURCES) return;
	update_sound_position(index, x,m_RoomAcousticInstance.m_SoundSources[index].speakerY, y);
}

void RoomAcousticQTConfig::update_instance_sound_sources()
{
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		RoomAcousticSoundSourceGraphics* source = m_RoomAcousticGraphic->m_pSoundSource[i];
		if (source != NULL)
		{
			update_sound_position(i, source->pos().x() / ROOMSCALE, source->pos().y() / ROOMSCALE, m_RoomAcousticInstance.m_SoundSources[i].speakerZ);
		}
	}
}

void RoomAcousticQTConfig::update_listener_position_top_view(float x, float y)
{
	update_listener_postion(x, m_RoomAcousticInstance.m_Listener.headY, y);
}

void RoomAcousticQTConfig::update_listener_postion(float x, float y, float z)
{
	mLockUpdate = true;
		ConfigUi.SB_HeadPositionX->setValue(x);
		ConfigUi.SB_HeadPositionY->setValue(y);
		ConfigUi.SB_HeadPositionZ->setValue(z);
	mLockUpdate = false;

	storeListenerPosition();
	updateListnerGraphics();
}

void RoomAcousticQTConfig::update_listener_orientation_top_view(float yaw)
{
	update_listener_orientation(m_RoomAcousticInstance.m_Listener.pitch, yaw, m_RoomAcousticInstance.m_Listener.roll);
}

void RoomAcousticQTConfig::update_listener_orientation(float pitch, float yaw, float roll)
{
	mLockUpdate = true;
		ConfigUi.SB_HeadPitch->setValue(pitch);
		ConfigUi.SB_HeadRoll->setValue(roll);
		ConfigUi.SB_HeadYaw->setValue(yaw);
	mLockUpdate = false;

	storeListenerPosition();
	updateListnerGraphics();
}

void RoomAcousticQTConfig::update_convMethod(bool gpu)
{
	auto methods = gpu ? m_RoomAcousticInstance.getGPUConvMethod() : m_RoomAcousticInstance.getCPUConvMethod();
	ConfigUi.CB_ConvMethod->clear();
	for (int i = 0; i < methods.size(); i++)
	{
		ConfigUi.CB_ConvMethod->addItem(QString::fromStdString(methods[i]));
	}
}

void RoomAcousticQTConfig::on_AddSoundSourceButton_clicked()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(
		this,
		tr("Open Source File"),
		m_RoomAcousticInstance.mTANDLLPath.c_str(),
		tr("WAV File (*.wav)")
		);

	int firstIndex(-1);

	for(int fileNameIndex = 0; fileNameIndex < fileNames.size(); ++fileNameIndex)
	{
		int insertedIndex = m_RoomAcousticInstance.addSoundSource(fileNames[fileNameIndex].toStdString());

		if(insertedIndex >= 0)
		{
			addSoundsourceGraphics(insertedIndex);

			if(-1 == firstIndex)
			{
				firstIndex = insertedIndex;
			}
		}
	}

	updateSoundsourceNames();
	updateAllSoundSourceGraphics();

	if(-1 != firstIndex)
	{
		table_selection_changed(firstIndex);
	}
}
