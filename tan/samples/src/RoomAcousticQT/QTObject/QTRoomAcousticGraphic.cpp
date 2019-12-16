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

#include "QTRoomAcousticGraphic.h"
#include "string"

void RoomAcousticListenerGraphics::rotateListenerYaw(qreal yaw)
{
	this->setRotation(yaw * 360);
	emit top_view_orientation_changed(yaw * 360);
}

RoomAcousticListenerGraphics::RoomAcousticListenerGraphics(QGraphicsItem* parent) : QGraphicsItemGroup(parent)
{
	// Loading icon
	m_icon = new QGraphicsPixmapItem(QPixmap(":/images/Resources/listener.png").scaled(60, 60));
	m_icon->moveBy(m_icon->boundingRect().width()/2, m_icon->boundingRect().height()/2);
	m_icon->setRotation(180.0f);
	m_label = new QGraphicsTextItem("Listener");
	m_label->moveBy(-m_icon->boundingRect().width()/2, m_icon->boundingRect().height()/2);
	this->addToGroup(m_icon);
	this->addToGroup(m_label);
	this->setFlag(QGraphicsItem::ItemIsMovable);
	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_label->setTransformOriginPoint(0, -(m_label->boundingRect().height() / 2));
	//int x = this->transformOriginPoint().x() + (this->m_icon->boundingRect().width() / 2);
	//int y = this->transformOriginPoint().y() + (this->m_icon->boundingRect().height() / 2);
	//this->setTransformOriginPoint(x,y);
	//initAnimation();
}

RoomAcousticListenerGraphics::~RoomAcousticListenerGraphics()
{
}

void RoomAcousticListenerGraphics::update_listener_position(float x, float y, float z)
{
	this->setPos(-x*ROOMSCALE, z*ROOMSCALE);
}


void RoomAcousticListenerGraphics::update_listener_orientation(float pitch, float yaw, float roll)
{
	//float dA = yaw - this->rotation();
	this->setRotation(yaw);
	//m_label->setRotation(-yaw);
}


void RoomAcousticListenerGraphics::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	/*The implementation for now allows dragging a listener out side of a room box,
	but if the listener is located outside of the room box, then he will not here anything*/
	float x = event->scenePos().x();
	float y = event->scenePos().y();
#ifdef _DEBUG
	qInfo("Listener postion: %f, %f", x, y);
#endif
	emit top_view_position_changed(-(this->x()) / ROOMSCALE, (this->y()) / ROOMSCALE);
	QGraphicsItemGroup::mouseMoveEvent(event);
}

void RoomAcousticListenerGraphics::wheelEvent(QGraphicsSceneWheelEvent* event)
{
	int delta = event->delta();
	m_iRotation += delta / 120;
	rotateListenerYaw(m_iRotation / 360.0f);
	QGraphicsItemGroup::wheelEvent(event);
}

void RoomAcousticListenerGraphics::initAnimation()
{
	m_pSpinTimeLine = new QTimeLine(5000,this);
	m_pSpinTimeLine->setLoopCount(0);
	m_pSpinTimeLine->setCurveShape(QTimeLine::LinearCurve);
	QObject::connect(m_pSpinTimeLine, &QTimeLine::valueChanged, this, &RoomAcousticListenerGraphics::rotateListenerYaw);
	float k = m_pSpinTimeLine->valueForTime(100);
	m_pSpinTimeLine->start();
}

RoomAcousticGraphic::RoomAcousticGraphic(QGraphicsScene* scene) : m_pScene(scene), m_pRoom(nullptr)
{
	// Initialize the sound source ptr
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		m_pSoundSource[i] = nullptr;
	}
	m_pListener = nullptr;
}

RoomAcousticGraphic::~RoomAcousticGraphic()
{
}

void RoomAcousticGraphic::clear()
{
	m_pScene->clear();
	m_pRoom = NULL;
	for (int i = 0; i < MAX_SOURCES; i++)
	{
		m_pSoundSource[i] = nullptr;
	}
	m_pListener = nullptr;
}

void RoomAcousticGraphic::update_Room_definition(float width, float height, float length)
{
	if (m_pRoom == NULL)
	{
		// First time initiate room graphics, need to add room
		m_pRoom = new RoomAcousticRoomGraphics(width,height,length);
		m_pScene->addItem(m_pRoom);
		//m_pRoom->setFla
	}
	m_pRoom->updateDimensions(width, height, length);
	m_pScene->update();
}

void RoomAcousticGraphic::update_sound_source_position(int index, float x, float y, float z)
{
	// If sound source does not exist, create one
	if (m_pSoundSource[index] == nullptr)
	{
		add_sound_source(index);
	}
	// Only update the top view
	m_pSoundSource[index]->update_sound_source_position(x,y,z);
	m_pScene->update();
}

void RoomAcousticGraphic::update_Listener_position(float x, float y, float z, float pitch, float yaw, float roll)
{
	// If listner does not exist, create a new one
	if (m_pListener == nullptr)
	{
		m_pListener = new RoomAcousticListenerGraphics();
		m_pScene->addItem(m_pListener);
	}
	m_pListener->update_listener_orientation(pitch,yaw,roll);
	m_pListener->update_listener_position(x, y, z);
	m_pScene->update();
}

void RoomAcousticGraphic::add_sound_source(int index)
{
	if (index < MAX_SOURCES)
	{
		// create a new sound graphic and add it to the scene
		m_pSoundSource[index] = new RoomAcousticSoundSourceGraphics(index);
		m_pScene->addItem(m_pSoundSource[index]);
		// update the scene
		m_pScene->update();
	}
}

void RoomAcousticGraphic::remove_sound_source(int index)
{
	this->m_pScene->removeItem(m_pSoundSource[index]);
	delete m_pSoundSource[index];
	m_pSoundSource[index] = NULL;
}

void RoomAcousticSoundSourceGraphics::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	/*The implementation for now allows dragging a sound source out side of a room box,
	but the sound out side of the room box will never be played.*/
	float x = event->scenePos().x();
	float y = event->scenePos().y();
#ifdef _DEBUG
	qInfo("Source %d postion: %f, %f", this->m_iIndex, x, y);
#endif
	if (!m_pTrackHead)
		emit top_view_position_changed(m_iIndex, -(this->x()) / ROOMSCALE, (this->y()) / ROOMSCALE);
	
	QGraphicsItemGroup::mouseMoveEvent(event);
}

void RoomAcousticSoundSourceGraphics::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	emit current_selection_changed(this->m_iIndex);
	QGraphicsItemGroup::mousePressEvent(event);
}

RoomAcousticRoomGraphics::RoomAcousticRoomGraphics(qreal width, qreal height, qreal length, QGraphicsItem* parent)
{
	m_drawpen.setWidth(4);
	m_pTopview = new QGraphicsRectItem(0, 0, -width, length);
}

void RoomAcousticRoomGraphics::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(m_drawpen);
	painter->drawRect(m_pTopview->rect());
	QGraphicsItemGroup::paint(painter, option, widget);
}

void RoomAcousticRoomGraphics::updateDimensions(qreal width, qreal height, qreal length)
{
	m_pTopview->setRect(0, 0, -width * ROOMSCALE, length * ROOMSCALE);
}

RoomAcousticSoundSourceGraphics::RoomAcousticSoundSourceGraphics(int index,QGraphicsItem* parent) : QGraphicsItemGroup(parent)
{
	//QGraphicsItem::setAcceptDrops(true);
	QPixmap a(":/RoomAcousticsNew.ico");
	m_icon = new QGraphicsPixmapItem(QPixmap(":/images/Resources/speaker.png").scaled(50, 50));
	m_icon->moveBy(-25, -25);
	// since the index start from 0, we need to add one to match the displate on screen
	QGraphicsTextItem* m_label = new QGraphicsTextItem(QString::fromStdString(std::to_string(index + 1)));
	m_label->moveBy(0, 25);
	this->addToGroup(m_icon);
	this->addToGroup(m_label);
	this->setFlag(QGraphicsItem::ItemIsMovable);
	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	m_iIndex = index;
}

RoomAcousticSoundSourceGraphics::~RoomAcousticSoundSourceGraphics()
{
}

void RoomAcousticSoundSourceGraphics::update_sound_source_position(float x, float y, float z)
{
	// Apply Transformation
	this->setPos(-x*ROOMSCALE, z*ROOMSCALE);
}

void RoomAcousticSoundSourceGraphics::setTrackHead(bool _b)
{
	m_pTrackHead = _b;
	this->setFlag(QGraphicsItem::ItemIsMovable, !_b);
}
