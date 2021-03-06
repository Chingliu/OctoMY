#include "HexyTool.hpp"
#include "ui_HexyTool.h"
#include "../libutil/utility/Standard.hpp"
#include "hw/actuators/ArduMYActuatorWidget.hpp"
#include "agent/AgentConstants.hpp"
#include <QSpacerItem>

HexyTool::HexyTool(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::HexyTool)
	, serial(new HexySerial(this))
	, settings(nullptr)
{
	ui->setupUi(this);
	ui->tryToggleConnect->configure("Connect","Connecting","Connected", AgentConstants::AGENT_CONNECT_BUTTON_COLOR);
	if(!connect(serial,SIGNAL(settingsChanged()), this, SLOT(onHexySettingsChanged()))) {
		qWarning()<<"ERROR: could not connect";
	}

	if(!connect(serial,SIGNAL(connectionChanged()), this, SLOT(onHexyConenctionChanged()))) {
		qWarning()<<"ERROR: could not connect";
	}
	if(!connect(ui->tryToggleConnect, SIGNAL(stateChanged(const TryToggleState, const TryToggleState)), this, SLOT(onConnectChanged(const TryToggleState, const TryToggleState)))) {
		qWarning()<<"ERROR: could not connect";
	}

	if(!connect(ui->widgetLimbIK, SIGNAL(IKUpadted()), this, SLOT(onLimbIKUpdated()))) {
		qWarning()<<"ERROR: could not connect";
	}


	QSpacerItem *vs = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	ui->verticalLayoutServos->addItem(vs);
	ui->scrollAreaServos->setEnabled(false);
	ui->widgetGait->setGait(gait);
	killAll();
	gaitTimer.setTimerType(Qt::PreciseTimer);
	gaitTimer.setInterval(40);
	if(!connect(&gaitTimer,SIGNAL(timeout()),this,SLOT(onUpdateGaitTimer()))) {
		qWarning()<<"ERROR: Could not connect";
	}
	gait.setDirection(0.5,0.5);
}

/*
 OLD STUFF FROM HUBWINDOW
	hexyTimer.setSingleShot(true);
	hexyTimer.setTimerType(Qt::PreciseTimer);

		if(!connect(&hexyTimer,SIGNAL(timeout()),this,SLOT(onHexyTimer()))){
			qWarning()<<"ERROR: Could not connect";
		}

void HubWindow::onHexyTimer(){
	if(0==hexy){
		return;
	}
	if(0!=m_gait){
		m_gait->update();
	}
	qreal pos[HexySerial::SERVO_COUNT]={0.0f};
	const quint64 now=QDateTime::currentMSecsSinceEpoch();
	const quint64 ival=now- lastTime;
	if(ival>0){
		lastTime=now;
		angle+=(((qreal)ival)/1000.0);
		angle-=floor(angle);
		for(quint32 i=0;i<HexySerial::SERVO_COUNT;++i){
			pos[i]=sin(angle*M_PI*2.0+(qreal)i*3.0);
			if(0==i){
				//qDebug()<<"SERVO "<<i<<": "<<pos[i];
			}
		}
		hexy->move(pos,0b00000000000000000000000000000011);
	}
	hexyTimer.start();
}

*/
void HexyTool::onUpdateGaitTimer()
{
	//qDebug()<<"----------------------UPDATE";
	gait.update();
	const qreal PI2=M_PI*-2;
	const int map[]= {7,6,5,11,10,9,15,14,13,16,17,18,20,21,22,24,25,26};
	int i=0;
	quint32 flags=0;
	for(int leg=0; leg<6; ++leg) {
		const quint32 coxID=map[i++];
		const quint32 femID=map[i++];
		const quint32 tibID=map[i++];
		flags |= (1<<coxID)|(1<<femID)|(1<<tibID);
		qreal cox=0,fem=0,tib=0;
		gait.getLimbVars(leg,cox,fem,tib);
		pos[coxID]=tib/PI2;
		pos[femID]=fem/PI2;
		pos[tibID]=cox/PI2;
	}
	serial->move(pos,flags);
}

HexyTool::~HexyTool()
{
	delete ui;
}


void HexyTool::onConnectChanged(const TryToggleState last, const TryToggleState current)
{
	ui->scrollAreaServos->setEnabled(ON==current);
	switch(current) {
	case(OFF): {
		killAll();
		if(0!=serial) {
			serial->closeSerialPort();
		}
	}
	break;
	case(TRYING): {
		setEnabled(false);
		serial->configure();
	}
	break;
	case(ON): {
		killAll();
	}
	break;
	}

}

void HexyTool::onLimbIKUpdated()
{
	const int map[]= {7,6,5,11,10,9,15,14,13,16,17,18,20,21,22,24,25,26};
	quint32 i=0;
	const quint32 coxID=map[i++];
	const quint32 femID=map[i++];
	const quint32 tibID=map[i++];
	quint32 flags = (1<<coxID)|(1<<femID)|(1<<tibID);
	qreal cox=0,fem=0,tib=0;
	ui->widgetLimbIK->getLimbVars(cox,fem,tib);
	pos[coxID]=tib;
	pos[femID]=fem;
	pos[tibID]=cox;
	serial->move(pos,flags);
}


void HexyTool::onHexySettingsChanged()
{
	const bool e=serial->isConnected();
	ui->tryToggleConnect->setState(e?ON:OFF);
	setEnabled(true);
}



void HexyTool::onHexyConenctionChanged()
{
	const bool e=serial->isConnected();
	ui->tryToggleConnect->setState(e?ON:OFF);
	setEnabled(true);
}





void HexyTool::onServoMoved(quint32 id, qreal val)
{
	if(id>= HexySerial::SERVO_COUNT) {
		return;
	}
	pos[id]=(val*2.0)-1.0;
	const quint32 bit=1<<id;
	serial->move(pos,bit);
}

void HexyTool::onServoKilled(quint32 id)
{
	if( id>= HexySerial::SERVO_COUNT) {
		return;
	}
	const quint32 bit=1<<id;
	qDebug()<<"KILL "<<id << " (for )=bit "<<bit<<")";
	serial->kill(bit);
}


void HexyTool::configure(Settings *s)
{
	settings=s;
	int i=0;
	auto sis=ui->scrollAreaWidgetContents->findChildren<ArduMYActuatorWidget *>();
	for(ArduMYActuatorWidget *si:sis) {
//		si->configure(settings,i++);
	}
}

void HexyTool::killAll()
{
	//qDebug()<<"KILL ALL";
	if(serial->isConnected()) {
		serial->kill();
		QList<ArduMYActuatorWidget *> si=ui->scrollAreaWidgetContents->findChildren<ArduMYActuatorWidget *>();
		for(QList<ArduMYActuatorWidget*>::iterator it=si.begin(),e=si.end(); it!=e; ++it) {
			ArduMYActuatorWidget *s=*it;
			if(0!=s) {
				QSignalBlocker sb(s);
				s->disableServo();
			}
		}
	}
}

void HexyTool::on_pushButtonDisableAll_clicked()
{
	killAll();
}

void HexyTool::on_pushButtonGait_toggled(bool checked)
{
	if(checked) {
		gaitTimer.start();
	} else {
		gaitTimer.stop();
	}
}

void HexyTool::on_dialFeedrate_valueChanged(int value)
{
	qreal ma=ui->dialFeedrate->maximum();
	qreal mi=ui->dialFeedrate->minimum();
	qreal va=ui->dialFeedrate->value();
	qreal range=(ma-mi)/2;
	qreal feedrate=((va-mi)/range);
	gait.setFeedrate(feedrate);
}
