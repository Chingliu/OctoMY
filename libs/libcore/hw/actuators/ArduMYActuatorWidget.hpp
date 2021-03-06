#ifndef ARDUMYACTUATORWIDGET_HPP
#define ARDUMYACTUATORWIDGET_HPP



#include <QWidget>

class ArduMYActuator;
namespace Ui
{
class ActuatorWidget;
}

class Settings;

const int INITIAL_SERVO_TRIM=150;

class ArduMYActuatorWidget: public QWidget
{
	Q_OBJECT
private:
	Ui::ActuatorWidget *ui;
	static quint32 s_sid;
	quint32 mID;
	int mLowTrim;
	int mHighTrim;
	quint32 mTabIndex;
	ArduMYActuator *mActuator;

public:
	explicit ArduMYActuatorWidget(QWidget *parent = 0);
	virtual ~ArduMYActuatorWidget();

private:
	void reconfigureTrim();
	void updateTabsVisibility();
	void reset();

public:
	void configure(ArduMYActuator *);
	void disableServo();
signals:
	void servoMoved(quint32 id, qreal val);
	void servoKilled(quint32 id);

private slots:
	void onServoMoved();
	void onServoKilled();
private slots:
	void on_pushButtonCenter_clicked();

	void on_pushButtonConfigure_toggled(bool checked);
	void on_spinBoxLowTrim_valueChanged(int arg1);
	void on_spinBoxHighTrim_valueChanged(int arg1);
	void on_tabWidget_currentChanged(int index);
	void on_comboBoxActuatorType_currentIndexChanged(int index);
	void on_pushButtonResetGearRatio_clicked();
	void on_pushButtonName_clicked();
};

#endif // ARDUMYACTUATORWIDGET_HPP
