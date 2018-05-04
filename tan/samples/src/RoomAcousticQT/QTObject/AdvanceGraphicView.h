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

