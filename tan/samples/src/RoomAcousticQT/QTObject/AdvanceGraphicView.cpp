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

#include "AdvanceGraphicView.h"
#include <qdrag.h>
#include <qmimedata.h>

AdvanceGraphicView::AdvanceGraphicView(QWidget* parent) : QGraphicsView(parent)
{

}

AdvanceGraphicView::~AdvanceGraphicView()
{
}

void AdvanceGraphicView::wheelEvent(QWheelEvent* event)
{
	if (m_eZoomMode == eZoomMode_Wheel)
	{
		setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		double scaleFactor = 1.15;
		if (event->delta() > 0)
		{
			scale(scaleFactor, scaleFactor);
		}
		else
		{
			scale(1 / scaleFactor, 1 / scaleFactor);
		}
	}
	QGraphicsView::wheelEvent(event);
}

void AdvanceGraphicView::mousePressEvent(QMouseEvent* event)
{
	if (m_eZoomMode == eZoomMode_MiddleButton_Drag && event->button() == Qt::MidButton)
	{
		m_bMiddleButtonPressed = true;
	}
	QGraphicsView::mousePressEvent(event);
}

void AdvanceGraphicView::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_eZoomMode == eZoomMode_MiddleButton_Drag && event->button() == Qt::MidButton)
	{
		m_bMiddleButtonPressed = false;
	}
	QGraphicsView::mouseReleaseEvent(event);
}

void AdvanceGraphicView::mouseMoveEvent(QMouseEvent* event)
{
	if (m_eZoomMode == eZoomMode_MiddleButton_Drag && m_bMiddleButtonPressed == true)
	{
		setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		int deltaY = event->globalY() - m_iMousePositionY;
		double scaleFactor = 1.01;
		if (deltaY > 0)
		{
			scale(scaleFactor, scaleFactor);
		}
		else
		{
			scale(1 / scaleFactor, 1 / scaleFactor);
		}
	}
	m_iMousePositionY = event->globalY();
	QGraphicsView::mouseMoveEvent(event);
}
