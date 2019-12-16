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

#pragma once
#include <QGraphicsView>
#include <QtWidgets/QWidget>
#include <QWheelEvent>
#include <QtWidgets/QWidget>
/*This is another version of the graphic view that enable user to 
use mous to pan and zoom the graphic view*/

enum eZoomMode
{
	eZoomMode_Wheel,
	eZoomMode_MiddleButton_Drag
};

class AdvanceGraphicView :
	public QGraphicsView
{
public:

	AdvanceGraphicView(QWidget *parent = 0);
	~AdvanceGraphicView();
	void setZoomMode(eZoomMode mode)
	{
		m_eZoomMode = mode;
	}
protected:
	void wheelEvent(QWheelEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	eZoomMode m_eZoomMode = eZoomMode_Wheel;
	bool m_bMiddleButtonPressed = false;
	int m_iMousePositionY = 0;
};

