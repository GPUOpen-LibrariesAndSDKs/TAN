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

#include "QTExportResponse.h"
#include "wav.h"

#include <QFileDialog>

void QTExportResponse::browseOutputFile()
{
	QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save As"), ".", tr("WAV Files (*.wav *.WAV"));
	if(saveFileName!="")
	{
		m_UIExportResponse.LE_OutputFilePath->setText(saveFileName);
	}
}

void QTExportResponse::generateResponse()
{
	RoomDefinition targetRoomdef;
	MonoSource targetSource;
	StereoListener targetListener;

	targetListener.headX = m_UIExportResponse.SB_ListenerPositionX->value();
	targetListener.headY = m_UIExportResponse.SB_ListenerPositionY->value();
	targetListener.headZ = m_UIExportResponse.SB_ListenerPositionZ->value();
	targetListener.yaw = m_UIExportResponse.SB_ListenerOrientationYaw->value();
	targetListener.pitch = m_UIExportResponse.SB_ListenerOrientationPitch->value();
	targetListener.roll = m_UIExportResponse.SB_ListenerOrientationRoll->value();
	targetListener.earSpacing = m_UIExportResponse.SB_ListenerEarSpacing->value();

	targetSource.speakerX = m_UIExportResponse.SB_SourcePositionX->value();
	targetSource.speakerY = m_UIExportResponse.SB_SourcePositionY->value();
	targetSource.speakerZ = m_UIExportResponse.SB_SourcePositionZ->value();

	targetRoomdef.width = m_UIExportResponse.SB_RoomWidth->value();
	targetRoomdef.height = m_UIExportResponse.SB_RoomHeight->value();
	targetRoomdef.length = m_UIExportResponse.SB_RoomLength->value();
	targetRoomdef.mBack.damp = m_UIExportResponse.SB_RoomDampBack->value();
	targetRoomdef.mFront.damp = m_UIExportResponse.SB_RoomDampFront->value();
	targetRoomdef.mLeft.damp = m_UIExportResponse.SB_RoomDampLeft->value();
	targetRoomdef.mRight.damp = m_UIExportResponse.SB_RoomDampRight->value();
	targetRoomdef.mTop.damp = m_UIExportResponse.SB_RoomDampTop->value();
	targetRoomdef.mBottom.damp = m_UIExportResponse.SB_RoomDampBottom->value();

	float** deinterleaved_IR_32bit = new float*[2];
	for (size_t i = 0; i < 2; i++)
	{
		deinterleaved_IR_32bit[i] = new float[m_UIExportResponse.SB_ResponseLength->value()];
	}
	m_pRoomAcoustic->getAMDTrueAudioVR()->generateRoomResponse(targetRoomdef, targetSource, targetListener, m_UIExportResponse.SB_SampleRate->currentText().toInt(), m_UIExportResponse.SB_ResponseLength->value(), deinterleaved_IR_32bit[0], deinterleaved_IR_32bit[1], 0, 50);


	if (m_UIExportResponse.SB_SampleSize->currentText().toInt() == 16)
	{
		size_t interleave_counter = 0;
		float* interleaved_IR_32bit = new float[m_UIExportResponse.SB_ResponseLength->value() * 2];
		for (size_t i = 0; i < m_UIExportResponse.SB_ResponseLength->value(); i++)
		{
			for (size_t j = 0; j < 2; j++)
			{
				interleaved_IR_32bit[interleave_counter++] = deinterleaved_IR_32bit[j][i];
			}
		}
		short* interleaved_IR_16bit = new short[m_UIExportResponse.SB_ResponseLength->value() * 2];
		m_pRoomAcoustic->getTANConverter()->Convert(interleaved_IR_32bit, 1, m_UIExportResponse.SB_ResponseLength->value(), interleaved_IR_16bit, 1, 1.0f);
		WriteWaveFileS(m_UIExportResponse.LE_OutputFilePath->text().toStdString().c_str(), m_UIExportResponse.SB_SampleRate->currentText().toInt(), 2, m_UIExportResponse.SB_SampleSize->currentText().toInt(),
			m_UIExportResponse.SB_ResponseLength->value(), interleaved_IR_16bit);
		delete[]interleaved_IR_32bit;
		delete[]interleaved_IR_16bit;

	}
	if(m_UIExportResponse.SB_SampleSize->currentText().toInt() == 32)
	{
		WriteWaveFileF(m_UIExportResponse.LE_OutputFilePath->text().toStdString().c_str(), m_UIExportResponse.SB_SampleRate->currentText().toInt(), 2, m_UIExportResponse.SB_SampleSize->currentText().toInt(),
			m_UIExportResponse.SB_ResponseLength->value(), deinterleaved_IR_32bit);
	}

	for (size_t i = 0; i < 2; i++)
	{
		delete[]deinterleaved_IR_32bit[i];
	}
	delete[]deinterleaved_IR_32bit;
}


void QTExportResponse::updateCurrentRoomConfig(bool in_bChecked)
{
	if(in_bChecked)
	{
		m_UIExportResponse.SB_RoomHeight->setValue(m_pRoomAcoustic->m_RoomDefinition.height);
		m_UIExportResponse.SB_RoomWidth->setValue(m_pRoomAcoustic->m_RoomDefinition.width);
		m_UIExportResponse.SB_RoomLength->setValue(m_pRoomAcoustic->m_RoomDefinition.length);

		m_UIExportResponse.SB_RoomDampLeft->setValue(m_pRoomAcoustic->m_RoomDefinition.mLeft.damp);
		m_UIExportResponse.SB_RoomDampRight->setValue(m_pRoomAcoustic->m_RoomDefinition.mRight.damp);
		m_UIExportResponse.SB_RoomDampTop->setValue(m_pRoomAcoustic->m_RoomDefinition.mTop.damp);
		m_UIExportResponse.SB_RoomDampBottom->setValue(m_pRoomAcoustic->m_RoomDefinition.mBottom.damp);
		m_UIExportResponse.SB_RoomDampBack->setValue(m_pRoomAcoustic->m_RoomDefinition.mBack.damp);
		m_UIExportResponse.SB_RoomDampFront->setValue(m_pRoomAcoustic->m_RoomDefinition.mFront.damp);
	}
}

void QTExportResponse::updateCurrentListenerConfig(bool in_bChecked)
{
	if(in_bChecked)
	{
		m_UIExportResponse.SB_ListenerPositionX->setValue(m_pRoomAcoustic->m_Listener.headX);
		m_UIExportResponse.SB_ListenerPositionY->setValue(m_pRoomAcoustic->m_Listener.headY);
		m_UIExportResponse.SB_ListenerPositionZ->setValue(m_pRoomAcoustic->m_Listener.headZ);

		m_UIExportResponse.SB_ListenerOrientationPitch->setValue(m_pRoomAcoustic->m_Listener.pitch);
		m_UIExportResponse.SB_ListenerOrientationRoll->setValue(m_pRoomAcoustic->m_Listener.roll);
		m_UIExportResponse.SB_ListenerOrientationYaw->setValue(m_pRoomAcoustic->m_Listener.yaw);
		m_UIExportResponse.SB_ListenerEarSpacing->setValue(m_pRoomAcoustic->m_Listener.earSpacing);
	}
}

void QTExportResponse::updateCurrentSourceConfig()
{
	double targetX = 0.0f;
	double targetY = 0.0f;
	double targetZ = 0.0f;
	if(m_UIExportResponse.CB_SelectSource->currentText() != "Custom")
	{
		int soundSourceIndex = m_pRoomAcoustic->findSoundSource(m_UIExportResponse.CB_SelectSource->currentText().toStdString().c_str());
		if(soundSourceIndex!=-1)
		{
			targetX = m_pRoomAcoustic->m_SoundSources[soundSourceIndex].speakerX;
			targetY = m_pRoomAcoustic->m_SoundSources[soundSourceIndex].speakerY;
			targetZ = m_pRoomAcoustic->m_SoundSources[soundSourceIndex].speakerZ;
		}
	}
	m_UIExportResponse.SB_SourcePositionX->setValue(targetX);
	m_UIExportResponse.SB_SourcePositionY->setValue(targetY);
	m_UIExportResponse.SB_SourcePositionZ->setValue(targetZ);
}

void QTExportResponse::updateCustomRoomConfig(bool in_bChecked)
{
	if(in_bChecked)
	{
		m_UIExportResponse.SB_RoomHeight->setValue(10.0f);
		m_UIExportResponse.SB_RoomWidth->setValue(10.0f);
		m_UIExportResponse.SB_RoomLength->setValue(10.0f);

		m_UIExportResponse.SB_RoomDampLeft->setValue(1.0f);
		m_UIExportResponse.SB_RoomDampRight->setValue(1.0f);
		m_UIExportResponse.SB_RoomDampTop->setValue(1.0f);
		m_UIExportResponse.SB_RoomDampBottom->setValue(1.0f);
		m_UIExportResponse.SB_RoomDampBack->setValue(1.0f);
		m_UIExportResponse.SB_RoomDampFront->setValue(1.0f);
	}

}

void QTExportResponse::updateCustomSourceConfig()
{
	m_UIExportResponse.SB_SourcePositionX->setValue(0.0f);
	m_UIExportResponse.SB_SourcePositionY->setValue(0.0f);
	m_UIExportResponse.SB_SourcePositionZ->setValue(0.0f);
}

void QTExportResponse::updateCustomListenerConfig(bool ch)
{
	m_UIExportResponse.SB_ListenerPositionX->setValue(0.0f);
	m_UIExportResponse.SB_ListenerPositionY->setValue(0.0f);
	m_UIExportResponse.SB_ListenerPositionZ->setValue(0.0f);

	m_UIExportResponse.SB_ListenerOrientationPitch->setValue(0.0f);
	m_UIExportResponse.SB_ListenerOrientationRoll->setValue(0.0f);
	m_UIExportResponse.SB_ListenerOrientationYaw->setValue(0.0f);
	m_UIExportResponse.SB_ListenerEarSpacing->setValue(0.16f);
}

void QTExportResponse::Init(RoomAcousticQT * in_pRoomAcoustic)
{
	m_pRoomAcoustic = in_pRoomAcoustic;
	m_UIExportResponse.setupUi(this);
}

void QTExportResponse::connectSignals()
{
	connect(m_UIExportResponse.PB_BrowseFileName, &QPushButton::clicked, this, &QTExportResponse::browseOutputFile);
	connect(m_UIExportResponse.buttonBox, &QDialogButtonBox::accepted, this, &QTExportResponse::generateResponse);
	connect(m_UIExportResponse.RB_UseCurrentListener, &QAbstractButton::toggled, this, &QTExportResponse::updateCurrentListenerConfig);
	connect(m_UIExportResponse.RB_UseCustomListener, &QAbstractButton::toggled, this, &QTExportResponse::updateCustomListenerConfig);
	connect(m_UIExportResponse.RB_UseCurrentRoom, &QAbstractButton::toggled, this, &QTExportResponse::updateCurrentRoomConfig);
	connect(m_UIExportResponse.RB_UseCustomRoom, &QAbstractButton::toggled, this, &QTExportResponse::updateCustomRoomConfig);
}

QTExportResponse::QTExportResponse(QWidget* parent, const Qt::WindowFlags& f): QDialog(parent, f)
{
	m_UIExportResponse.setupUi(this);
	connectSignals();
}
