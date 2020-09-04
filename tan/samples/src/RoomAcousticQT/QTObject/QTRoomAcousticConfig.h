#ifndef ROOMACCOUSTICNEW_H
#define ROOMACCOUSTICNEW_H
#define MAX_DEVICES 10
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QtWidgets>

#include "ui_RoomAcousticConfig.h"
#include "../RoomAcousticQT.h"
#include "QTRoomAcousticGraphic.h"

class QTimer;

class RoomAcousticQTConfig : public QMainWindow
{
	Q_OBJECT
private slots:
	void on_actionLoad_Config_File_triggered();
	void on_actionSave_Config_File_triggered();
	void on_actionAbout_triggered();
	void on_actionASIO_triggered();
	void on_actionExport_Response_triggered();
	void on_AddSoundSourceButton_clicked();
	void on_RemoveSoundSourceButton_clicked();
	void on_SourcesTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
	void on_CB_SoundSourceEnable_stateChanged(int state);
	void on_CB_UseMicroPhone_stateChanged(int state);
	void on_CB_TrackHead_stateChanged(int state);
	void on_CB_AutoSpin_stateChanged(int state);
	void on_SB_ConvolutionLength_valueChanged(int value);
	void on_SB_BufferSize_valueChanged(int value);
	void on_SB_RoomWidth_valueChanged(double value);
	void on_SB_RoomLength_valueChanged(double value);
	void on_SB_RoomHeight_valueChanged(double value);

	void on_SB_RoomDampLeft_valueChanged(double value);
	void on_SB_RoomDampRight_valueChanged(double value);
	void on_SB_RoomDampTop_valueChanged(double value);
	void on_SB_RoomDampBottom_valueChanged(double value);
	void on_SB_RoomDampFront_valueChanged(double value);
	void on_SB_RoomDampBack_valueChanged(double value);
	void on_SB_SoundPositionX_valueChanged(double value);
	void on_SB_SoundPositionY_valueChanged(double value);
	void on_SB_SoundPositionZ_valueChanged(double value);

	void on_SB_HeadPitch_valueChanged(double value);
	void on_SB_HeadYaw_valueChanged(double value);
	void on_SB_HeadRoll_valueChanged(double value);
	void on_SB_HeadPositionX_valueChanged(double value);
	void on_SB_HeadPositionY_valueChanged(double value);
	void on_SB_HeadPositionZ_valueChanged(double value);

	void on_CB_RoomDevice_currentIndexChanged(int index);
	void on_CB_ConvolutionDevice_currentIndexChanged(int index);
	void on_CB_ConvMethod_currentIndexChanged(int index);
	void on_RB_DEF4Room_clicked();
	void on_RB_DEF4Conv_clicked();
	void on_RB_MPr4Room_clicked();
	void on_RB_MPr4Conv_clicked();
	void on_RB_RTQ4Conv_clicked();
	void on_RB_RTQ4Room_clicked();

	void on_PB_RunDemo_clicked();
	void on_ASIO_device0();
	void on_ASIO_device1();
	void on_ASIO_device2();
	void on_ASIO_device3();
	void on_ASIO_device4();
	void on_ASIO_device5();
	void on_ASIO_device6();
	void on_ASIO_device7();
	void on_ASIO_device8();
	void on_ASIO_device9();

	void table_selection_changed(int index);
	void update_sound_position(int index, float x, float y, float z);
	void update_sound_position_top_view(int index, float x, float y);
	void update_listener_postion(float x, float y, float z);
	void update_listener_position_top_view(float x, float y);
	void update_listener_orientation(float pitch, float yaw, float roll);
	void update_listener_orientation_top_view(float yaw);
	void update_instance_sound_sources();
	void update_convMethod(bool gpu);

public:
	RoomAcousticQTConfig(QWidget *parent = 0);
	~RoomAcousticQTConfig();
	void Init();

private:
	void initSoundSourceGraphic();								// Initialize the soundsource graphcis(icon)
	void initListenerGraphics();								// Initialize the listener graphcis(icon)
	/************************************************************************************/
	/*								GUI Functions			 							*/
	/************************************************************************************/
	/*				Function below update the value in instance to gui					*/
	void clear();
	void updateAllFields();
	void updateSelectedSoundSource();							// Trasfer the focus to the current selected item
	void updateSoundsourceNames();
	void updateRoomFields();
	void updateConvolutionFields();
	void updateListenerFields();								// Update the listener fields
	void updateReverbFields();									// Use the reverb function in instance and update reverb field
	void setEnableHeadPositionFields(bool enable);				// set enable for head position fields
	/************************************************************************************/
	/*							Graphics Update Function								*/
	/************************************************************************************/
	void updateRoomGraphic();									// Update the room graphics
	void updateSoundSourceGraphics(int index);
	void updateAllSoundSourceGraphics();						// update the sound source graphics
	void updateListnerGraphics();								// update the listener graphcis
	void addSoundsourceGraphics(int index);						// add sound source to graphics
	void addListenerGraphics();									// add Listener into graphcis
	void removeSoundsourceGraphics(int index);					// remove sound source from graphics
	/************************************************************************************/
	/*							Params Store Functions                                  */
	/************************************************************************************/
	void storeSelectedSoundSource();							// Save the last selected item's info
	void storeTrackedHeadSource();
	void storeAllFieldsToInstance();							// Porting all configuration from UI to instance
	void storeListenerPosition();
	void storeRoomFields();						// Porting room definition from UI to instance
	void storeConvolutionFields();
	/************************************************************************************/
	/*									Debug											*/
	/************************************************************************************/
	void printConfiguration();									// Printing configuration in to debug screen
	std::string getDriverVersion();
	std::string getTANVersion();
	/************************************************************************************/
	/*							Animation Related										*/
	/************************************************************************************/
	void setHeadSpinTimeInterval(float interval);
	void startHeadSpinAnimation();
	void stopHeadSpinAnimation();

	void OnTimer();

	Ui::RoomAcousticConfig ConfigUi;							// The main configuration ConfigUi
	RoomAcousticQT m_RoomAcousticInstance;						// The main room acoustic instance

	RoomAcousticGraphic* m_RoomAcousticGraphic;
	int m_iCurrentSelectedSource = -1;
	bool m_bDemoStarted = false;								// Flag that identifies the demo starting state
	QTimeLine* m_pHeadAnimationTimeline = nullptr;

	QTimer *mTimer = nullptr;
	bool mLockUpdate = false;

	void on_ASIO_device(int id);
};

#endif // ROOMACCOUSTICNEW_H