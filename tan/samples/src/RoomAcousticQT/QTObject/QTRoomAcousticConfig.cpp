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

	ConfigUi.CL_RoomGPU->setEnabled(false);
	ConfigUi.CL_ConvolutionGPU->setEnabled(false);
	ConfigUi.CL_RoomCPU->setEnabled(false);
	ConfigUi.CL_ConvolutionCPU->setEnabled(false);

	ConfigUi.CL_RoomCPU->setChecked(true);
	ConfigUi.CL_ConvolutionCPU->setChecked(true);

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
					//updateSoundSourceGraphics(m_iCurrentSelectedSource);
				}
			}
		}
		);
	mTimer->start(250);

	// Initialize source table
	ConfigUi.SourcesTable->setRowCount(MAX_SOURCES);
	ConfigUi.SourcesTable->setColumnCount(1);

	//
	connect(
		ConfigUi.CB_UseMicroPhone,
		&QCheckBox::stateChanged,
		[this](int state)
		{
			m_RoomAcousticInstance.mSrc1EnableMic = Qt::Checked == state;
		}
		);

	// Initialize device
	for (int i = 0; i < m_RoomAcousticInstance.m_iDeviceCount; i++)
	{
		ConfigUi.CB_UseGPU4Room->addItem(QString::fromUtf8(m_RoomAcousticInstance.m_cpDeviceName[i]));
		ConfigUi.CB_UseGPU4Conv->addItem(QString::fromUtf8(m_RoomAcousticInstance.m_cpDeviceName[i]));
	}

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

	show();
}

void RoomAcousticQTConfig::storeSelectedSoundSource()
{
	if(mLockUpdate)
	{
		return;
	}

	// save selected item on the graphcis scene

	// Save last selected item on the table widget
	if (m_iCurrentSelectedSource >= 0 && m_iCurrentSelectedSource >= 0)
	{
		// Saving the configuration for the last clicked item
		m_RoomAcousticInstance.mSoundSourceEnable[m_iCurrentSelectedSource] = ConfigUi.CB_SoundSourceEnable->isChecked();
		if(!m_iCurrentSelectedSource)
		{
			m_RoomAcousticInstance.mSrc1EnableMic = ConfigUi.CB_UseMicroPhone->isChecked();
		}
		m_RoomAcousticInstance.m_bSrcTrackHead[m_iCurrentSelectedSource] = ConfigUi.CB_TrackHead->isChecked() ? 1 : 0;

		m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerX = ConfigUi.SB_SoundPositionX->value();
		m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerY = ConfigUi.SB_SoundPositionY->value();
		m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerZ = ConfigUi.SB_SoundPositionZ->value();

		if(m_bDemoStarted)
		{
			m_RoomAcousticInstance.updateSoundSourcePosition(m_iCurrentSelectedSource);
		}
	}
}

//Ask about this:
void RoomAcousticQTConfig::updateSelectedSoundSource()
{
	mLockUpdate = true;

	if (m_iCurrentSelectedSource >= 0 && m_iCurrentSelectedSource < m_RoomAcousticInstance.m_iNumOfWavFile)
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
		setEnableSoundsourceFields(true);
		
		ConfigUi.RemoveSoundSourceButton->setEnabled(true);

		ConfigUi.CB_SoundSourceEnable->setChecked(m_RoomAcousticInstance.mSoundSourceEnable[m_iCurrentSelectedSource]);
		ConfigUi.CB_SoundSourceEnable->setEnabled(true);

		ConfigUi.CB_TrackHead->setChecked(m_RoomAcousticInstance.m_bSrcTrackHead[m_iCurrentSelectedSource]);
		ConfigUi.CB_TrackHead->setEnabled(true);

		update_sound_position(m_iCurrentSelectedSource,
			m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerX,
			m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerY,
			m_RoomAcousticInstance.m_SoundSources[m_iCurrentSelectedSource].speakerZ);

		ConfigUi.SB_SoundPositionX->setEnabled(true);
		ConfigUi.SB_SoundPositionY->setEnabled(true);
		ConfigUi.SB_SoundPositionZ->setEnabled(true);

		ConfigUi.RemoveSoundSourceButton->setEnabled(true);
		
		ConfigUi.SoundConfigurationGroup->setEnabled(true);
	}
	else
	{
		// If the slot does not have any sound source, disable the parameter settings.
		ConfigUi.SoundConfigurationGroup->setEnabled(false);

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
		ConfigUi.RemoveSoundSourceButton->setEnabled(false);
	}
	
	mLockUpdate = false;
}

void RoomAcousticQTConfig::updateAllFields()
{
	updateSoundsourceNames();
	updateRoomDefinitionFields();
	updateListenerFields();
	updateConvolutionFields();
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

		if (m_RoomAcousticInstance.mWavFileNames[i].length())
		{
			// Check the file existance before assign it into the cell
			display_name = m_RoomAcousticInstance.mWavFileNames[i];

			if (!checkFileExist(display_name))
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
			if(m_RoomAcousticInstance.m_bSrcTrackHead[i])
			{
				display_name += " <Trac>";
			}
			if (i >= (unsigned int)m_RoomAcousticInstance.m_iNumOfWavFile)
			{
				display_name += " <Disabled>";
				m_RoomAcousticInstance.mSoundSourceEnable[i] = false;
			}
			else
			{
				m_RoomAcousticInstance.mSoundSourceEnable[i] = true;
			}
			item->setText(QString::fromStdString(display_name));

		}
	}
	
	mLockUpdate = false;
}

void RoomAcousticQTConfig::updateRoomDefinitionFields()
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

	m_RoomAcousticInstance.m_iuseGPU4Room == 0 ? ConfigUi.CB_UseGPU4Room->setCurrentIndex(0) : ConfigUi.CB_UseGPU4Room->setCurrentIndex(1);

#ifdef RTQ_ENABLED
	m_RoomAcousticInstance.m_iuseMPr4Room == 0 ? ConfigUi.RB_MPr4Room->setChecked(false) : ConfigUi.RB_MPr4Room->setChecked(true);
	m_RoomAcousticInstance.m_iuseRTQ4Room == 0 ? ConfigUi.RB_RTQ4Room->setChecked(false) : ConfigUi.RB_MPr4Room->setChecked(true);
	ConfigUi.SB_RoomCU->setValue(m_RoomAcousticInstance.m_iRoomCUCount);
#endif // RTQ_ENABLED

	mLockUpdate = false;
}

void RoomAcousticQTConfig::updateConvolutionFields()
{
	mLockUpdate = true;

	ConfigUi.SB_ConvolutionLength->setValue(m_RoomAcousticInstance.m_iConvolutionLength);
	ConfigUi.SB_BufferSize->setValue(m_RoomAcousticInstance.m_iBufferSize);
	ConfigUi.LB_ConvolutionTime->setText(QString::fromStdString(std::to_string(m_RoomAcousticInstance.getConvolutionTime())));
	ConfigUi.LB_BufferTime->setText(QString::fromStdString(std::to_string(m_RoomAcousticInstance.getBufferTime())));
	m_RoomAcousticInstance.m_iuseGPU4Conv == 0 ? ConfigUi.CB_UseGPU4Conv->setCurrentIndex(0) : ConfigUi.CB_UseGPU4Conv->setCurrentIndex(1);
#ifdef RTQ_ENABLED
	m_RoomAcousticInstance.m_iuseMPr4Conv == 0 ? ConfigUi.RB_MPr4Conv->setChecked(false) : ConfigUi.RB_MPr4Conv->setChecked(true);
#endif // RTQ_ENABLED

	if (m_RoomAcousticInstance.m_iuseGPU4Conv != 0
		|| m_RoomAcousticInstance.m_iuseMPr4Conv != 0
#ifdef RTQ_ENABLED
		|| m_RoomAcousticInstance.m_iuseRTQ4Conv != 0
#endif // RTQ_ENABLED

		)
	{
		update_convMethod_GPU();
		ConfigUi.CB_ConvMethod->setCurrentIndex(m_RoomAcousticInstance.m_eConvolutionMethod);
	}
	else
	{
		update_convMethod_CPU();
	}
	ConfigUi.CB_ConvMethod->setCurrentIndex(m_RoomAcousticInstance.m_eConvolutionMethod);
#ifdef RTQ_ENABLED
	m_RoomAcousticInstance.m_iuseRTQ4Conv == 0 ? ConfigUi.RB_RTQ4Conv->setChecked(false) : ConfigUi.RB_MPr4Conv->setChecked(true);
	ConfigUi.SB_ConvCU->setValue(m_RoomAcousticInstance.m_iConvolutionCUCount);
#endif // RTQ_ENABLED

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

void RoomAcousticQTConfig::setEnableSoundsourceFields(bool enable)
{
	ConfigUi.SoundConfigurationGroup->setEnabled(enable);
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
	storeRoomDefinitionToInstance();
	
	ConfigUi.CB_AutoSpin->isChecked() ? m_RoomAcousticInstance.m_iHeadAutoSpin = 1 : m_RoomAcousticInstance.m_iHeadAutoSpin = 0;
	ConfigUi.CB_TrackHead->isChecked() ? m_RoomAcousticInstance.m_isrc1TrackHeadPos = 1 : m_RoomAcousticInstance.m_isrc1TrackHeadPos = 0;

	m_RoomAcousticInstance.m_iuseGPU4Room = !ConfigUi.CB_UseGPU4Room->currentIndex() 
	    ? 0 
		: ConfigUi.CB_UseGPU4Room->currentIndex();
	m_RoomAcousticInstance.m_iuseGPU4Conv = !ConfigUi.CB_UseGPU4Conv->currentIndex() 
	    ? 0 
		: ConfigUi.CB_UseGPU4Conv->currentIndex();

#ifdef RTQ_ENABLED
	ConfigUi.RB_MPr4Conv->isChecked() ? m_RoomAcousticInstance.m_iuseMPr4Conv = 1 : m_RoomAcousticInstance.m_iuseMPr4Conv = 0;
	ConfigUi.RB_MPr4Room->isChecked() ? m_RoomAcousticInstance.m_iuseMPr4Room = 1 : m_RoomAcousticInstance.m_iuseMPr4Room = 0;
	ConfigUi.RB_RTQ4Conv->isChecked() ? m_RoomAcousticInstance.m_iuseRTQ4Conv = 1 : m_RoomAcousticInstance.m_iuseRTQ4Conv = 0;
	m_RoomAcousticInstance.m_iConvolutionCUCount = ConfigUi.SB_ConvCU->value();
	ConfigUi.RB_RTQ4Room->isChecked() ? m_RoomAcousticInstance.m_iuseRTQ4Room = 1 : m_RoomAcousticInstance.m_iuseRTQ4Room = 0;
	m_RoomAcousticInstance.m_iRoomCUCount = ConfigUi.SB_RoomCU->value();
#endif // RTQ_ENABLED

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

void RoomAcousticQTConfig::storeRoomDefinitionToInstance()
{
	if(mLockUpdate)
	{
		return;
	}

	m_RoomAcousticInstance.m_RoomDefinition.width = ConfigUi.SB_RoomLength->value();
	m_RoomAcousticInstance.m_RoomDefinition.height = ConfigUi.SB_RoomHeight->value();
	m_RoomAcousticInstance.m_RoomDefinition.length = ConfigUi.SB_RoomLength->value();

	m_RoomAcousticInstance.m_RoomDefinition.mLeft.damp = DBTODAMP(ConfigUi.SB_RoomDampLeft->value());
	m_RoomAcousticInstance.m_RoomDefinition.mRight.damp = DBTODAMP(ConfigUi.SB_RoomDampRight->value());
	m_RoomAcousticInstance.m_RoomDefinition.mTop.damp = DBTODAMP(ConfigUi.SB_RoomDampTop->value());
	m_RoomAcousticInstance.m_RoomDefinition.mBottom.damp = DBTODAMP(ConfigUi.SB_RoomDampBottom->value());
	m_RoomAcousticInstance.m_RoomDefinition.mFront.damp = DBTODAMP(ConfigUi.SB_RoomDampFront->value());
	m_RoomAcousticInstance.m_RoomDefinition.mBack.damp = DBTODAMP(ConfigUi.SB_RoomDampBack->value());

	//todo: move to separated helper function
	m_RoomAcousticInstance.mCLRoomOverGPU = ConfigUi.CL_RoomGPU->isChecked();
	m_RoomAcousticInstance.mCLConvolutionOverGPU = ConfigUi.CL_ConvolutionGPU->isChecked();

	updateReverbFields();

	if(m_bDemoStarted)
	{
		m_RoomAcousticInstance.updateRoomDamping();
	}
}

void RoomAcousticQTConfig::printConfiguration()
{
	// Printing Source Name
	qInfo("Number of wav files: %d", m_RoomAcousticInstance.m_iNumOfWavFile);
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		qInfo(
			"Source %d: Name: %s, Position: (%f,%f,%f)",
			i,
			m_RoomAcousticInstance.mWavFileNames[i].c_str(),
			m_RoomAcousticInstance.m_SoundSources[i].speakerX, m_RoomAcousticInstance.m_SoundSources[i].speakerY,
			m_RoomAcousticInstance.m_SoundSources[i].speakerZ);
	}

	// Print Listener configuration
	qInfo("Head Position: %f,%f,%f. Pitch: %f, Yaw: %f, Roll: %f",
		this->m_RoomAcousticInstance.m_Listener.headX,this->m_RoomAcousticInstance.m_Listener.headY,
		this->m_RoomAcousticInstance.m_Listener.headZ,this->m_RoomAcousticInstance.m_Listener.pitch,
		this->m_RoomAcousticInstance.m_Listener.yaw, this->m_RoomAcousticInstance.m_Listener.roll);
	qInfo("Head Configuration: Auto Spin: %d, Track Head: %d, Ear Spacing: %f", this->m_RoomAcousticInstance.m_iHeadAutoSpin,
		this->m_RoomAcousticInstance.m_isrc1TrackHeadPos, this->m_RoomAcousticInstance.m_Listener.earSpacing);

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
	qInfo("Convolution Running On: %d, Normal queue: %d, Medium Queue: %d, RTQ: %d, CUS: %d", this->m_RoomAcousticInstance.m_iuseGPU4Conv,
		this->m_RoomAcousticInstance.m_iuseGPU4Conv, this->m_RoomAcousticInstance.m_iuseMPr4Conv, this->m_RoomAcousticInstance.m_iuseRTQ4Conv,
		this->m_RoomAcousticInstance.m_iConvolutionCUCount);
	qInfo("Room Running On: %d, Normal queue: %d, Medium Queue: %d, RTQ: %d, CUS: %d", this->m_RoomAcousticInstance.m_iuseGPU4Room,
		this->m_RoomAcousticInstance.m_iuseGPU4Room, this->m_RoomAcousticInstance.m_iuseMPr4Room, this->m_RoomAcousticInstance.m_iuseRTQ4Room,
		this->m_RoomAcousticInstance.m_iRoomCUCount);
#endif // RTQ_ENABLED


}

//todo: ask Geoffrey how to retrive version information under linux
//may be lspci?
std::string RoomAcousticQTConfig::getDriverVersion()
{
#ifdef _WIN32
	WindowsRegister newreg(HKEY_LOCAL_MACHINE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}");
	WindowsRegister* subkeys = newreg.getSubKeys();
	for (unsigned int i = 0; i < newreg.getNumOfSubKeys(); i++)
	{
		subkeys[i].printSubKeyInfo();
		for (unsigned int j = 0; j < subkeys[i].getNumOfSubKeys(); i++)
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
		if (m_RoomAcousticInstance.m_bSrcTrackHead[i])
		{
			update_sound_position(i, m_RoomAcousticInstance.m_Listener.headX, m_RoomAcousticInstance.m_Listener.headY, m_RoomAcousticInstance.m_Listener.headZ);
		}
	}
}

/*QT Slots: Triggered when loading configuration file action clicked*/
void RoomAcousticQTConfig::on_actionLoad_Config_File_triggered()
{
	std::string fileName = QFileDialog::getOpenFileName(
		this,
		tr("Open Configuration File"),
		m_RoomAcousticInstance.mTANDLLPath.c_str(),
		tr("Configuration File (*.xml)")
		).toStdString();
	if (fileName[0] != '\0')
	{
		m_RoomAcousticGraphic->clear();
		char* filenamecpy = new char[MAX_PATH];
		std::strncpy(filenamecpy, fileName.c_str(), MAX_PATH);
		m_RoomAcousticInstance.loadConfiguration(filenamecpy);
		updateAllFields();
		updateRoomGraphic();
		initSoundSourceGraphic();
		initListenerGraphics();
		delete[]filenamecpy;
	}
}
/*QT Slots: Triggered when saving configuration file action clicked*/
void RoomAcousticQTConfig::on_actionSave_Config_File_triggered()
{
	std::string fileName = QFileDialog::getSaveFileName(
		this,
		tr("Save Configuration File"),
		m_RoomAcousticInstance.mTANDLLPath.c_str(),
		tr("Configuration File (*.xml)")
		).toStdString();
	char* filenamecpy = new char[MAX_PATH];
	std::strncpy(filenamecpy, fileName.c_str(), MAX_PATH);
	storeAllFieldsToInstance();
	m_RoomAcousticInstance.saveConfiguraiton(filenamecpy);
	delete[]filenamecpy;
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
}

void RoomAcousticQTConfig::on_SourcesTable_cellClicked(int row, int col)
{
	table_selection_changed(row);
}


void RoomAcousticQTConfig::on_CB_TrackHead_stateChanged(int stage)
{
	if(mLockUpdate)
	{
		return;
	}

	storeSelectedSoundSource();
	updateSoundsourceNames();

	if (!ConfigUi.CB_TrackHead->isChecked())
	{
		ConfigUi.SoundSourcePositionGroup->setEnabled(true);
		if (m_iCurrentSelectedSource != -1)
		{
			m_RoomAcousticInstance.m_bSrcTrackHead[m_iCurrentSelectedSource] = false;
			m_RoomAcousticGraphic->m_pSoundSource[m_iCurrentSelectedSource]->setTrackHead(false);
		}

	}
	else
	{
		ConfigUi.SoundSourcePositionGroup->setEnabled(false);
		if (m_iCurrentSelectedSource != -1)
		{
			m_RoomAcousticInstance.m_bSrcTrackHead[m_iCurrentSelectedSource] = true;
			m_RoomAcousticGraphic->m_pSoundSource[m_iCurrentSelectedSource]->setTrackHead(true);
		}
	}
}

void RoomAcousticQTConfig::on_CB_SoundSourceEnable_stateChanged(int stage)
{
	if(mLockUpdate)
	{
		return;
	}

	if (m_iCurrentSelectedSource >= 0)
	{
		if (!stage)
		{
			// Current sound source is disabled
			std::string new_name = m_RoomAcousticInstance.mWavFileNames[m_iCurrentSelectedSource];
			new_name += " <Disabled>";

			m_RoomAcousticInstance.mSoundSourceEnable[m_iCurrentSelectedSource] = false;
			QTableWidgetItem* item = ConfigUi.SourcesTable->item(m_iCurrentSelectedSource, 0);
			item->setText(QString::fromStdString(new_name));
		}
		else
		{
			// Current sound source is enabled
			std::string new_name = m_RoomAcousticInstance.mWavFileNames[m_iCurrentSelectedSource];

			m_RoomAcousticInstance.mSoundSourceEnable[m_iCurrentSelectedSource] = true;
			QTableWidgetItem* item = ConfigUi.SourcesTable->item(m_iCurrentSelectedSource, 0);
			item->setText(QString::fromStdString(new_name));
		}
	}
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
	
	storeRoomDefinitionToInstance();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomLength_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}
	
	storeRoomDefinitionToInstance();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomHeight_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}
	
	storeRoomDefinitionToInstance();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampLeft_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}
	
	storeRoomDefinitionToInstance();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampRight_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}
	
	storeRoomDefinitionToInstance();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampTop_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}
	
	storeRoomDefinitionToInstance();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampBottom_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}
	
	storeRoomDefinitionToInstance();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampFront_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}
	
	storeRoomDefinitionToInstance();
	updateReverbFields();
}

void RoomAcousticQTConfig::on_SB_RoomDampBack_valueChanged(double value)
{
	if(mLockUpdate)
	{
		return;
	}
	
	storeRoomDefinitionToInstance();
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

void RoomAcousticQTConfig::on_CB_UseGPU4Room_currentIndexChanged(int index)
{
	if(mLockUpdate)
	{
		return;
	}

	if(index == 0)
	{
		// Running in CPU mode
		ConfigUi.RB_DEF4Room->setChecked(true);
		// disable queue selection
		ConfigUi.RoomQueueGroup->setEnabled(false);

#ifdef RTQ_ENABLED
		// disable CU selection
		ConfigUi.SB_RoomCU->setEnabled(false);
#endif // RTQ_ENABLED

		ConfigUi.CL_RoomGPU->setEnabled(false);
		ConfigUi.CL_RoomCPU->setEnabled(false);
		
		ConfigUi.CL_RoomCPU->setChecked(true);
	}
	else
	{
		ConfigUi.CL_RoomGPU->setEnabled(true);
		ConfigUi.CL_RoomCPU->setEnabled(true);
		
		ConfigUi.CL_RoomGPU->setChecked(true);

		// Running in GPU mode
		// enable queue selection
		ConfigUi.RoomQueueGroup->setEnabled(true);

#ifdef RTQ_ENABLED
		// disable CU selection
		ConfigUi.SB_RoomCU->setEnabled(false);
#endif
		// set default queue
		ConfigUi.RB_DEF4Room->setChecked(true);
	}

	this->m_RoomAcousticInstance.m_iRoomDeviceID = index;
}

void RoomAcousticQTConfig::on_CB_UseGPU4Conv_currentIndexChanged(int index)
{
	if(mLockUpdate)
	{
		return;
	}

	if(index == 0)
	{
		ConfigUi.CL_ConvolutionGPU->setEnabled(false);
		ConfigUi.CL_ConvolutionCPU->setEnabled(false);
		
		ConfigUi.CL_ConvolutionCPU->setChecked(true);

		// Running in CPU mode
		ConfigUi.RB_DEF4Conv->setChecked(true);
		// disable queue selection
		ConfigUi.ConvQueueGroup->setEnabled(false);

#ifdef RTQ_ENABLED
		// disable CU selection
		ConfigUi.SB_ConvCU->setEnabled(false);
#endif
		update_convMethod_CPU();
	}
	else
	{
		ConfigUi.CL_ConvolutionGPU->setEnabled(true);
		ConfigUi.CL_ConvolutionCPU->setEnabled(true);
		
		ConfigUi.CL_ConvolutionGPU->setChecked(true);

		// Running in GPU mode
		// disable queue selection
		ConfigUi.ConvQueueGroup->setEnabled(true);
#ifdef RTQ_ENABLED
		// disable CU selection
		ConfigUi.SB_ConvCU->setEnabled(false);
#endif // RTQ_ENABLED

		// set default queue
		ConfigUi.RB_DEF4Conv->setChecked(true);
		update_convMethod_GPU();
	}
	this->m_RoomAcousticInstance.m_iConvolutionDeviceID = index;
}

void RoomAcousticQTConfig::on_CB_ConvMethod_currentIndexChanged(int index)
{

	QString text = ConfigUi.CB_ConvMethod->itemText(index);
	std::string stdText;
	stdText = text.toStdString();
		
	this->m_RoomAcousticInstance.m_eConvolutionMethod = this->m_RoomAcousticInstance.getConvMethodFlag(stdText);
	return;

}

void RoomAcousticQTConfig::on_RB_DEF4Room_clicked()
{
#ifdef RTQ_ENABLED
	// Default queue selected, disable CU
	ConfigUi.SB_RoomCU->setEnabled(false);
#endif
}

#ifdef RTQ_ENABLED
void RoomAcousticQTConfig::on_RB_MPr4Room_clicked()
{
	if (ConfigUi.RB_MPr4Room->isChecked())
	{
		if (this->ConfigUi.CB_UseGPU4Conv->currentIndex() == this->ConfigUi.CB_UseGPU4Conv->currentIndex())
		{
			// Room select for MPR, need to set convolution's runnin queue
			if (ConfigUi.RB_MPr4Conv->isChecked())
			{
				ConfigUi.RB_MPr4Conv->setChecked(false);
			}


			if (ConfigUi.RB_RTQ4Conv->isChecked())
			{
				ConfigUi.RB_RTQ4Conv->setChecked(false);
			}
			// Since RTQ is not selected for room, disble CU
			ConfigUi.SB_RoomCU->setEnabled(false);
			// Since rtq is not selected, might as well disable cu
			ConfigUi.SB_ConvCU->setEnabled(false);

			// Set conlution queue to be default queuem (if running on GPU)
			if (ConfigUi.CB_UseGPU4Conv->currentIndex() >= 1)
				ConfigUi.RB_DEF4Conv->setChecked(true);
			else
				ConfigUi.RB_DEF4Conv->setChecked(false);

		}
	}
}
#endif // RTQ_ENABLED
#ifdef RTQ_ENABLED
void RoomAcousticQTConfig::on_RB_RTQ4Room_clicked()
{
	if (ConfigUi.RB_RTQ4Room->isChecked())
	{
		if (this->ConfigUi.CB_UseGPU4Conv->currentIndex() == this->ConfigUi.CB_UseGPU4Conv->currentIndex())
		{
			// Room select for MPR, need to set convolution's runnin queue
			if (ConfigUi.RB_MPr4Conv->isChecked())
			{
				ConfigUi.RB_MPr4Conv->setChecked(false);
			}
			if (ConfigUi.RB_RTQ4Conv->isChecked())
			{
				ConfigUi.RB_RTQ4Conv->setChecked(false);
			}
			// Since RTQ is selected for room, enable CU
			ConfigUi.SB_RoomCU->setEnabled(true);
			// Set conlution queue to be default queuem (if running on GPU)
			if (ConfigUi.CB_UseGPU4Conv->currentIndex() >= 1)
				ConfigUi.RB_DEF4Conv->setChecked(true);
			else
				ConfigUi.RB_DEF4Conv->setChecked(false);
			// Since rtq is not selected, might as well disable cu
			ConfigUi.SB_ConvCU->setEnabled(false);
		}
	}
}

#endif // RTQ_ENABLED

void RoomAcousticQTConfig::on_RB_DEF4Conv_clicked()
{
	// Default queue selected, disable CU
#ifdef RTQ_ENABLED
	ConfigUi.SB_ConvCU->setDisabled(true);
#endif

}
#ifdef RTQ_ENABLED
void RoomAcousticQTConfig::on_RB_MPr4Conv_clicked()
{
	if (ConfigUi.RB_MPr4Conv->isChecked())
	{
		// MPR selected for convolution queue, need to set room's runnin queue
		if (ConfigUi.RB_MPr4Room->isChecked())
		{
			ConfigUi.RB_MPr4Room->setChecked(false);
		}

		if (ConfigUi.RB_RTQ4Room->isChecked())
		{
			ConfigUi.RB_RTQ4Room->setChecked(false);
		}
		// Since RTQ is not selected for room, disble CU
		ConfigUi.SB_ConvCU->setEnabled(false);
		ConfigUi.SB_RoomCU->setEnabled(false);


		// Set Room queue to be default queue (if running on GPU)
		if (ConfigUi.CB_UseGPU4Room->currentIndex() >= 1)
			ConfigUi.RB_DEF4Room->setChecked(true);
		else
			ConfigUi.RB_DEF4Room->setChecked(false);
		// Since rtq is not selected, might as well disable cu

	}
}
#endif // RTQ_ENABLED
#ifdef RTQ_ENABLED
void RoomAcousticQTConfig::on_RB_RTQ4Conv_clicked()
{
	if (ConfigUi.RB_RTQ4Conv->isChecked())
	{
		// Room select for MPR, need to set convolution's runnin queue
		if (ConfigUi.RB_MPr4Room->isChecked())
		{
			ConfigUi.RB_MPr4Room->setChecked(false);
		}
		if (ConfigUi.RB_RTQ4Room->isChecked())
		{
			ConfigUi.RB_RTQ4Room->setChecked(false);
		}
		// Since RTQ is selected for room, enable CU
		ConfigUi.SB_ConvCU->setEnabled(true);
		// Set conlution queue to be default queuem (if running on GPU)
		if (ConfigUi.CB_UseGPU4Room->currentIndex() >= 1)
			ConfigUi.RB_DEF4Room->setChecked(true);
		else
			ConfigUi.RB_DEF4Room->setChecked(false);
		// Since rtq is not selected, might as well disable cu
		ConfigUi.SB_RoomCU->setEnabled(false);
	}
}
#endif

void RoomAcousticQTConfig::on_PB_RunDemo_clicked()
{
	if(!m_bDemoStarted)
	{
		storeAllFieldsToInstance();

#ifdef _DEBUG
		printConfiguration();
#endif

		m_bDemoStarted = 0 == m_RoomAcousticInstance.start();

		if(m_bDemoStarted)
		{
			ConfigUi.PB_RunDemo->setText("Stop");
		}
		else
		{
			QMessageBox::critical(
				this,
				"Error",
				"Could not start playing, please see output log!"
				);
		}
	}
	else
	{
		m_RoomAcousticInstance.stop();
		m_bDemoStarted = false;

		ConfigUi.PB_RunDemo->setText("Run");
	}
}

void RoomAcousticQTConfig::table_selection_changed(int index)
{
	storeSelectedSoundSource();

	if(index >= 0 && index <  m_RoomAcousticInstance.m_iNumOfWavFile)
	{
		m_iCurrentSelectedSource = index;
	}

	QTableWidgetItem* selected_item = ConfigUi.SourcesTable->item(index, 0);
	ConfigUi.SourcesTable->setCurrentItem(selected_item);


	updateSelectedSoundSource();
}

void RoomAcousticQTConfig::update_sound_position(int index, float x, float y, float z)
{
	m_RoomAcousticInstance.m_SoundSources[index].speakerY = y;
	m_RoomAcousticInstance.m_SoundSources[index].speakerX = x;
	m_RoomAcousticInstance.m_SoundSources[index].speakerZ = z;
	updateSoundSourceGraphics(index);
	if (index == m_iCurrentSelectedSource)
	{
//		// if the sound source is inside the room, then allow user to enable it
//		if (m_RoomAcousticInstance.isInsideRoom(x, y, z))
//		{
//			this->ConfigUi.CB_SoundSourceEnable->setCheckable(true);
//			this->ConfigUi.CB_SoundSourceEnable->setEnabled(true);
//		}
//		// Else, the sound source is automatically disable, and you cannot enable it
//		// unleass you move the sound source inside the room
//		else
//		{
//			this->ConfigUi.CB_SoundSourceEnable->setCheckable(false);
//			this->ConfigUi.CB_SoundSourceEnable->setEnabled(false);
//		}
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

void RoomAcousticQTConfig::update_convMethod_CPU()
{
	std::string* methods = nullptr;
	int num = 0;
	m_RoomAcousticInstance.getCPUConvMethod(&methods, &num);
	ConfigUi.CB_ConvMethod->clear();
	for (int i = 0; i < num; i++)
	{
		ConfigUi.CB_ConvMethod->addItem(QString::fromStdString(methods[i]));
	}
	delete[]methods;
}

void RoomAcousticQTConfig::update_convMethod_GPU()
{
	std::string* methods = nullptr;
	int num = 0;
	m_RoomAcousticInstance.getGPUConvMethod(&methods, &num);
	ConfigUi.CB_ConvMethod->clear();
	for (int i = 0; i < num; i++)
	{
		ConfigUi.CB_ConvMethod->addItem(QString::fromStdString(methods[i]));
	}
	delete[]methods;
}

void RoomAcousticQTConfig::on_AddSoundSourceButton_clicked()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(
		this,
		tr("Open Source File"),
		m_RoomAcousticInstance.mTANDLLPath.c_str(),
		tr("WAV File (*.wav)")
		);

	/*QList<QTableWidgetItem*> selected_sources = ConfigUi.SourcesTable->selectedItems();

	// Try to add sound source without selecting empty slots
	if (selected_sources.isEmpty())*/
	{
		for (int i = 0; i < fileNames.size(); i++)
		{
			int index = m_RoomAcousticInstance.addSoundSource(fileNames[i].toStdString());
			addSoundsourceGraphics(index);
		}
	}
	/*// Try to replace a sound source
	else
	{
		for (int i = 0; i < fileNames.size(); i++)
		{
			QTableWidgetItem* item = selected_sources.first();
			m_RoomAcousticInstance.replaceSoundSource(fileNames[i].toStdString(), item->row());
			addSoundsourceGraphics(item->row());

			selected_sources.pop_front();
			if (selected_sources.size() <= 0) break;
		}
	}*/

	updateSoundsourceNames();
	updateAllSoundSourceGraphics();
}