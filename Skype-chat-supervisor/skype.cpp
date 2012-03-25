/***************************************************************
 * skype.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2008-04-30.
 * @Last Change: 2008-04-30.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/
#include <QtGui/QApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>


#include "skype.h"
#include "skypeCommand.h"



skype::skype(QString AppName): appName(AppName) {
  connected=false;
  TimeOut=10000;
  pingTimer = new QTimer(this);
  contactsUpToDate=false;
  myStatus=skypeResponse::offline;
  currentUser="Myself";
  waitingForResponse=false;
  online=false;
  connect( &sk, SIGNAL( attachedToSkype() ), this, SLOT( attachedToSkype() ) );
}


void skype::updateOnlineContacts() { 
  if ( connected ) { 
    doCommand( skypeCommand::GET_CONTACT_LIST(), true );
    QString contact;
    onlineContacts.clear();
    foreach( contact, contacts ) { 
      doCommand( skypeCommand::GET_CONTACT_STATUS( contact ), false );
    }
  }
}
   


void skype::timeOut() {
  if ( waitingForResponse ) { 
    qDebug() << "Timeout while waiting for event #"<<waitForResponseID;
    localEventLoop.exit(2);
  }
}

void skype::ping() { 
  sk.sendMsgToSkype( skypeCommand::PING() );
  pingCounter--;
  if ( pingCounter < 0 ) { 
    disconnect( &sk, 0, this, 0 );
    disconnect( pingTimer, 0, 0, 0 );
    pingTimer->stop();
    connected = false;
    qDebug() << "Connection lost;";
    sk.detachFromSkype();
    emit connectionLost();
  }
}

void skype::callContact( QString contactName ) { 
  sk.sendMsgToSkype( skypeCommand::CALL_CONTACT( contactName ) );
}


//void skype::readIncomingData(QString contactName, int streamNum) {
//  sk.sendMsgToSkype( skypeCommand::READ(appName, contactName, streamNum) );
//}

void skype::processMessage(const QString &message) {
//  qDebug() << "SKYPE: <=" << message;

  skypeResponse cmd;

  if ( ! cmd.parse(message) ) {
    emit skypeError( -1, "Error parsing Skype output" );
    if ( waitingForResponse ) localEventLoop.exit(1);
    return;
  }

  if ( cmd.type() == SK_PING ) { 
    pingCounter++;
  }else {
      qDebug() << "bad news";
  }

  if ( cmd.type() == SK_CONTACT_LIST ) {
    contacts = cmd.getContacts();
    contactsUpToDate = true;
  }

  if ( cmd.type() == SK_CURRENT_USER ) { 
    currentUser = cmd.contactName();
  }

  if ( cmd.type() == SK_USER_STATUS ) { 
    myStatus = cmd.getStatus();
    if ( myStatus == skypeResponse::offline ) {
      qDebug() << "skype::processMessage(): user status offline";
      if ( online )  {
	online=false;
	emit skypeOffline();
      }
    } else {
      qDebug() << "skype::processMessage(): user status online";
      if ( ! online ) {
	online = true;
        emit skypeOnline();
      }
    }
    emit changedStatus( cmd.getStatus() );
  }

  if ( cmd.type() == SK_CONTACT_STATUS ) { 
    if ( cmd.getStatus() == skypeResponse::offline ) {
      onlineContacts.removeAll( cmd.contactName() );
      emit contactOffline( cmd.contactName() );
    } else if ( ! onlineContacts.contains( cmd.contactName() ) ) {
      qDebug() << "skype::processMessage(): "<< cmd.contactName() << " online";
      onlineContacts.append( cmd.contactName() );
      emit contactOnline( cmd.contactName() );
    }

    emit changedContactStatus( cmd.contactName(), cmd.getStatus() );
  }


  if ( cmd.type() == SK_CHATMESSAGE ) {
//    if ( cmd.getMStatus() == skypeResponse::SK_CMS_RECEIVED ) {
      emit recievedChatMessage( cmd );
//    }else if(cmd.getChatMessageProperty() = "BODY"){

//    }
  }


  if ( waitingForResponse && cmd.responseID() == waitForResponseID ) {
    qDebug() << "Received event "<<cmd.responseID() <<" we've been waiting for.";
    qDebug() << "Response received:"<<message;
    localEventLoop.exit(0);
    return;
  }

  if ( cmd.type() == SK_UNKNOWN ) { 
      qDebug() << message;
    return;
  }

  if ( cmd.type() == SK_ERROR ) { 
      qDebug() << message;
    emit skypeError( cmd.errorCode(), cmd.errorMsg() );
    return;
  }
  if ( cmd.appName() != appName ) return;

//  if ( cmd.type() == SK_READY_TO_READ ) {
//    readIncomingData( cmd.contactName(), cmd.streamNum() );
//    return;
//  }

//  if ( cmd.type() == SK_DATA ) {
//    if ( streams.contains( cmd.contactName() ) ) {
//      streams[cmd.contactName()].append( cmd.data() );
//    } else { // should not happen (a SK_STREAMS message should always arrive before)
//      qDebug() << "ASSERT: Data arriving before stream Created (" << cmd.contactName() <<":"<< cmd.streamNum() << cmd.data() << ")";
//      streams[cmd.contactName()] = cmd.data();
//      activeStream[cmd.contactName()] = cmd.streamNum();
//    }
//    emit dataInStream( cmd.contactName() );
//    return;
//  }

//  if ( cmd.type() == SK_STREAMS ) {
//    QByteArray data;
//    if (! streams.contains( cmd.contactName() ) ) {
//      streams.insert( cmd.contactName(), data );
//    }
//    activeStream[ cmd.contactName() ] = cmd.streamNum();
//    emit newStreamCreated( cmd.contactName() );
//    return;
//  }
}



int skype::waitForResponse( QString cID ) {
  waitForResponseID = cID;
  QTimer *timer = new QTimer(this);
  timer->setSingleShot( true );
  connect( timer, SIGNAL( timeout() ), this, SLOT( timeOut() ) );
  timer->start( TimeOut );
  /*QTimer::singleShot(TimeOut, this, SLOT(timeOut()));*/
  waitingForResponse = true;
  int result = localEventLoop.exec();
  waitingForResponse = false;
  disconnect( timer, 0, this, 0 );
  delete timer;
  return result;
}

bool skype::doCommand(QString cmd, bool blocking) {
  /*qDebug() << "Doing command:" << cmd;*/
  QString cID = skypeCommand::prependID( cmd );
  QString ID = skypeCommand::getID( cID );
  sk.sendMsgToSkype( cID );
  if ( blocking && waitingForResponse ) { 
    qDebug() << "********************************************************************";
    qDebug() << "WARNING:: can't wait for response to "<< cID <<" already waiting for:" <<waitForResponseID;
    qDebug() << "********************************************************************";
    return true;
  }
  
  if ( blocking ) {
    qDebug() << "Waiting for response to message "<<ID;
    int result = waitForResponse( ID );
    qDebug() << "Result of waiting" << result;
    if ( result == 0 ) {
       if ( response.type() != SK_ERROR ) return true;
    }
    return false;
  } else return true;
}

void skype::attachedToSkype() {
 qDebug() << "skype::attachedToSkype";
 disconnect( &sk, SIGNAL( newMsgFromSkype(const QString) ), this, SLOT( processMessage(const QString) ) );
 connect( &sk, SIGNAL( newMsgFromSkype(const QString) ), this, SLOT( processMessage(const QString) ) );
 if ( ! doCommand( skypeCommand::CONNECT_TO_SKYPE(appName) ), true ) {
   qDebug() << "Unauthorized ??? (Or skype responding too slowly)";
 }
 if ( ! doCommand( skypeCommand::PROTOCOL(5) ), true ) {
   qDebug() << "Wrong protocol number ??? (Or skype responding too slowly)";
 }
 if ( ! doCommand( skypeCommand::CREATE(appName) ), true ) {
   qDebug() << "Unable to create application object (Or skype responding too slowly) ";
 }
 pingCounter=15;
 connect( pingTimer, SIGNAL( timeout() ), this, SLOT( ping() ) );
 pingTimer->start(2000);
 connected = true;
 emit connectedToSkype();
};

bool skype::connectToSkype() { 
 disconnect( &sk, SIGNAL( attachedToSkype() ), this, SLOT( attachedToSkype() ) );
 connect( &sk, SIGNAL( attachedToSkype() ), this, SLOT( attachedToSkype() ) );
 qDebug() << "Connecting to skype ...";
 if ( connected ) {
   qDebug() << "Already connected";
   return true;
 }
 if ( ! sk.attachToSkype() ) {
   qDebug() << "Unable to establish communication channel";
   return false;
 } else if ( ! connected ) attachedToSkype();
 return connected;
};


bool skype::disconnectFromSkype() {

#ifdef Q_WS_WIN
#undef DELETE
#endif

  if ( ! connected ) return true;
  if ( ! doCommand( skypeCommand::DELETE(appName) ), false ) return false;
  disconnect( &sk, 0, this, 0 );
  pingTimer->stop();
  disconnect( pingTimer, 0, 0, 0 );
  connected = false;
  emit disconnected();
  return true;
}


//void skype::newStream(QString contact) {
//  doCommand( skypeCommand::CONNECT( appName, contact ), false );
//}

//bool skype::writeToStream(QByteArray data, QString contactName ) {
//  if ( connected ) {
//  if ( ! activeStream.contains( contactName ) )  return false; // We are not connected to contactName

//  doCommand( skypeCommand::WRITE( appName, contactName, activeStream[contactName],data ), false );
//  return true;
//  } else return false;
//}

//bool skype::broadCast(QByteArray data) {
//  if ( connected ) {
//    QString contactName;
//    QList<QString> contacts = activeStream.keys();
//    foreach( contactName, contacts ) {
//      doCommand( skypeCommand::WRITE( appName, contactName, activeStream[contactName], data), false );
//    }
//    return true;
//  } else return false;
//}



//QByteArray skype::readFromStream(QString contactName) {
//  QByteArray ret;
//  ret.clear();
//  if ( streams.contains( contactName ) ) {
////   qDebug() << "DEBUG: streams["<<contactName<<"]="<< streams[contactName];
//   ret.append( streams[contactName] );
//   streams[contactName].clear();
//  }
//  return ret;
//}


//#include "skype.moc"
