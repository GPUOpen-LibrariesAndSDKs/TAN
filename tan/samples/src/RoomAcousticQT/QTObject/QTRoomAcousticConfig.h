#ifndef ROOMACCOUSTICNEW_H
#define ROOMACCOUSTICNEW_H
#define MAX_DEVICES 10
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QtWidgets>

#ifdef RTQ_ENABLED
#include "ui_RoomAcousticConfig.h"
#else
#include "ui_RoomAcousticConfig_NoRTQ.h"
#endif // RTQ_ENABLED
#include "..\RoomAcoustic.h"
#include "QTRoomAcousticGraphic.h"

class RoomAcousticQT : public QMainWindow
{
	Q_OBJECT
private slots:
	void on_actionLoad_Config_File_triggered();
	void on_actionSave_Config_File_triggered();
	void on_actionAbout_triggered();
	void on_actionExport_Response_triggered();
	void on_AddSoundSourceButton_clicked();
	void on_RemoveSoundSourceButton_clicked();
	void on_SourcesTable_cellClicked(int row, int col);
	void on_CB_TrackHead_stateChanged(int stage);
	void on_CB_SoundSourceEnable_stateChanged(int stage);
	void on_CB_AutoSpin_stateChanged(int stage);
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
	
	void on_CB_UseGPU4Room_currentIndexChanged(int index);
	void on_CB_UseGPU4Conv_currentIndexChanged(int index);
	void on_CB_ConvMethod_currentIndexChanged(int index);
	void on_RB_DEF4Room_clicked();
	

	void on_RB_DEF4Conv_clicked();
#ifdef RTQ_ENABLED
	void on_RB_MPr4Room_clicked();
	void on_RB_MPr4Conv_clicked();
	void on_RB_RTQ4Conv_clicked();
	void on_RB_RTQ4Room_clicked();
#endif // RTQ_ENABLED
	void on_PB_RunDemo_clicked();

	void table_selection_changed(int index);
	void update_sound_position(int index, float x, float y, float z);
	void update_sound_position_top_view(int index, float x, float y);
	void update_listener_postion(float x, float y, float z);
	void update_listener_position_top_view(float x, float y);
	void update_listener_orientation(float pitch, float yaw, float roll);
	void update_listener_orientation_top_view(float yaw);
	void update_instance_sound_sources();
	void update_convMethod_CPU();
	void update_convMethod_GPU();
public:
	RoomAcousticQT(QWidget *parent = 0);
	~RoomAcousticQT();
	void Init();
private:

	void initSoundSourceGraphic();								// Initialize the soundsource graphcis(icon)
	void initListenerGraphics();								// Initialize the listener graphcis(icon)
	void saveLastSelectedSoundSource();							// Save the last selected item's info
	void highlightSelectedSoundSource(QTableWidgetItem* item);	// Trasfer the focus to the current selected item
	/************************************************************************************/
	/*								GUI Functions			 							*/
	/************************************************************************************/
	/*				Function below update the value in instance to gui					*/			
	void updateAllFields();	
	void updateSoundsourceNames();
	void updateRoomDefinitionFields();
	void updateConvolutionFields();
	void updateListenerFields();								// Update the listener fields
	void updateReverbFields();									// Use the reverb function in instance and update reverb field
	void setEnableSoundsourceFields(bool enable);				// set enable for sound source fields
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
	/*							Instance Update Function								*/
	/************************************************************************************/
	void updateTrackedHeadSource();
	void updateAllFieldsToInstance();							// Porting all configuration from UI to instance
	void updateRoomDefinitionToInstance();						// Porting room definition from UI to instance
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
	
	Ui::RoomAcousticConfig ConfigUi;							// The main configuration ConfigUi
	RoomAcoustic m_RoomAcousticInstance;						// The main room acoustic instance

	RoomAcousticGraphic* m_RoomAcousticGraphic;
	int m_iLastClickedRow = -1;									// Record the last selected sound source id, Initially
	int m_iLastClickedCol = -1;									// Would be zero
	int m_iCurrentSelectedSource = -1;
	bool m_bDemoStarted = false;								// Flag that identifies the demo starting state
	QTimeLine* m_pHeadAnimationTimeline;
};



#endif // ROOMACCOUSTICNEW_H
