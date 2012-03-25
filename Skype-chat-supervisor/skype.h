#ifndef _skype_H
#define _skype_H

/***************************************************************
 * skype.h
 * foundation by:
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Last Change: 2008-04-30.
 ***************************************************************/
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include "skypeCommand.h"
#include "skypeComm.h"


class skype : public QObject { 
  Q_OBJECT
	private:
	  skypeComm sk;

	  QString appName, currentUser;
	  bool connected, online;
//	  QHash<QString, QByteArray> streams;
//	  QHash<QString, int> activeStream;

	  QStringList contacts, onlineContacts;
	  bool contactsUpToDate; 

	  skypeResponse::contactStatus myStatus;


	  bool waitingForResponse;
	  QString waitForResponseID;
	  QEventLoop localEventLoop;
	  QTimer *pingTimer;
	  int TimeOut;
	  int pingCounter;
	  skypeResponse response;

	protected:
//	  void readIncomingData(QString contact, int stream);
          int waitForResponse ( QString commandID );

	public:
	  skype(QString AppName);
	  bool connectToSkype();
	  bool isConnectedToSkype() { return connected; }
	  bool disconnectFromSkype();
//	  void newStream(QString contact);
//	  bool writeToStream(QByteArray data, QString contactName); //deprecated
//	  bool writeToSock(QString contactName, QByteArray data) { return writeToStream( data, contactName ); };
//	  bool broadCast(QByteArray data);
//	  QByteArray readFromStream(QString contact);

          bool doCommand(QString cmd, bool blocking = false);

	  void updateOnlineContacts();
	  QString myContactName() const { return currentUser; };
	  skypeResponse::contactStatus getMyStatus() { return myStatus; };
	  void callContact( QString contact );

	signals:
	  void skypeError(int errNo, QString Msg);
//	  void dataInStream(QString contactName);
//	  void newStreamCreated(QString contactName);
	  void changedStatus( skypeResponse::contactStatus status );
	  void changedContactStatus( QString contactName, skypeResponse::contactStatus status );
	  void contactOffline( QString contactName );
	  void contactOnline( QString contactName );
	  void skypeOffline();
	  void skypeOnline();

          void recievedChatMessage( skypeResponse &mes );

	  void connectedToSkype();

	  void connectionLost();
	  void disconnected();

	protected slots:
	  void processMessage(const QString &message);
	  void timeOut();
	  void ping();

	  void attachedToSkype();
};



#endif /* _skype_H */
