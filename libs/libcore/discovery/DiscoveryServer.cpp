#include "DiscoveryServer.hpp"
#include <QCryptographicHash>

#include <QRegularExpression>
#include <QDebug>



DiscoveryServerSession * DiscoveryServer::request(QSharedPointer<NodeAssociate> part)
{
	quint32 participantAddCount=0;
	quint32 sessionAddCount=0;
	if(nullptr==part){
		qWarning()<<"ERROR: participant was 0";
		return nullptr;
	}
	if(!part->isValidForServer()){
		qWarning()<<"ERROR: participant was invalid: "<<part->toString();
		return nullptr;
	}
	DiscoveryServerSession *ses=nullptr;
	const QStringList &pins=part->pins();
	if(!pins.isEmpty()){
		QString pin;
		for(QString p: pins){
			pin=p;
			//Look up the pin in question
			if(registry.contains(pin)){
				//qDebug()<<"PIN "<<pin<<" matched with a session";
				ses=registry[pin];
				break;
			}
		}
		if(""!=pin){
			if(nullptr==ses){
				//qDebug()<<"Creating new session";
				ses=new DiscoveryServerSession();
				if(!ses->set(part)){
					qWarning()<<"ERROR: Newly created session will be terminated as participant was not welcome";
					delete ses;
					ses=nullptr;
				}
				else{
					//qDebug()<<"Adding session to registry under pin "<<pin;
					registry[pin]=ses;
					sessionAddCount++;
					participantAddCount++;
				}
			}
			else {
				if(!ses->has(part->id())){
					if(!ses->set(part)){
						qWarning()<<"ERROR: participant was not welcome in existing session";
						ses=nullptr;
					}
					else{
						participantAddCount++;
					}

				}
			}
		}
		else{
			qWarning()<<"ERROR: no pin";
		}
	}
	else{
		qWarning()<<"ERROR: participant had no pins";
	}
	if(sessionAddCount>0){
		qDebug()<<"ADDING "<<sessionAddCount<<" sessions to server";
	}
	if(participantAddCount>0){
		qDebug()<<"ADDING "<<participantAddCount<<" participants to server";
	}
	return ses;
}




void DiscoveryServer::prune(quint64 deadline)
{
	quint64 participantPruneCount=0;
	quint64 sessionPruneCount=0;
	for(QMap<QString, DiscoveryServerSession *>::iterator it=registry.begin();it!=registry.end() /* not hoisted */; /* no increment */ ){
		QString key=it.key();
		DiscoveryServerSession *ses=it.value();
		if(nullptr!=ses){
			const quint64 tmp=ses->prune(deadline);
			if(tmp>0){
				participantPruneCount+=tmp;
				if(ses->count()<=0){
					delete ses;
					ses=nullptr;
					registry.erase(it++);
					sessionPruneCount++;
					continue;
				}
			}
		}
		++it;
	}
	if(participantPruneCount>0){
		qDebug()<<"PRUNING "<<participantPruneCount<<" participants from server";
	}
	if(sessionPruneCount>0){
		qDebug()<<"PRUNING "<<sessionPruneCount<<" sessions from server";
	}
}
