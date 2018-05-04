#include <QtWidgets/QtWidgets>
#include <samples/src/common/maxlimits.h>
#include <QGraphicsItemGroup>
#define ROOMSCALE 100

class RoomAcousticRoomGraphics : public QGraphicsItemGroup
{
public:
	RoomAcousticRoomGraphics(qreal width, qreal height, qreal length, QGraphicsItem *parent = Q_NULLPTR);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
	void updateDimensions(qreal width, qreal height, qreal length);
private:
	QPen m_drawpen;
	QGraphicsRectItem* m_pTopview;
};


class RoomAcousticSoundSourceGraphics : public QObject, public QGraphicsItemGroup
{
	Q_OBJECT
signals:
	void top_view_position_changed(int index, float x, float y);
	void current_selection_changed(int index);
public:
	RoomAcousticSoundSourceGraphics(int index, QGraphicsItem* parent = 0);
	virtual ~RoomAcousticSoundSourceGraphics();
	void update_sound_source_position(float x, float y, float z);
	void setTrackHead(bool _b);
protected:
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
private:
	QGraphicsRectItem* m_pRoom;
	QGraphicsPixmapItem* m_icon = NULL;
	bool m_pTrackHead = false;
	int m_iIndex;
};

class RoomAcousticListenerGraphics : public QObject, public QGraphicsItemGroup
{
	Q_OBJECT
signals :
	void top_view_position_changed(float x, float y);
	void top_view_orientation_changed(float yaw);
public slots:
	void rotateListenerYaw(qreal yaw);

public:
	RoomAcousticListenerGraphics(QGraphicsItem* parent = 0);
	virtual ~RoomAcousticListenerGraphics();
	void update_listener_position(float x, float y, float z);
	void update_listener_orientation(float pitch, float yaw, float roll);
protected:
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	void wheelEvent(QGraphicsSceneWheelEvent *event) override;
private:
	void initAnimation();
	QGraphicsPixmapItem* m_icon = NULL;
	QGraphicsTextItem* m_label = NULL;
	QTimeLine* m_pSpinTimeLine;
	int m_iRotation = 0;
};


class RoomAcousticGraphic
{
public:
	RoomAcousticGraphic(QGraphicsScene* scene);
	virtual ~RoomAcousticGraphic();
	void clear();
	void update_Room_definition(float width, float height, float length);
	void update_sound_source_position(int index, float x, float y, float z);
	void update_Listener_position(float x, float y, float z, float pitch, float yaw, float roll);
	void add_sound_source(int index);
	void remove_sound_source(int index);
	RoomAcousticSoundSourceGraphics* m_pSoundSource[MAX_SOURCES];
	RoomAcousticListenerGraphics* m_pListener = nullptr;
	QGraphicsScene* m_pScene;
private:
	RoomAcousticRoomGraphics* m_pRoom;
};

