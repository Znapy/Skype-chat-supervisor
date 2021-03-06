/***************************************************************
 * skypeX11.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2008-05-14.
 * @Last Change: 2008-05-14.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/
#include <QtGui/QApplication>
#ifdef Q_WS_WIN
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtCore/QTime>

#include "skypeComm.h"
#include "application.h"

enum {
  SKYPE_ATTACH_SUCCESS=0,
  SKYPE_TRY_NOW=0x8001,
  SKYPE_REFUSED=2,
  SKYPE_PENDING_AUTHORIZATION=1,
  SKYPE_TRY_AGAIN=3
};

UINT skypeComm::attachMSG = 0;
UINT skypeComm::discoverMSG = 0;
QWidget *skypeComm::mainWin = NULL;
WId skypeComm::main_window = 0;

skypeComm::skypeComm() { 
  if ( attachMSG == 0 || discoverMSG == 0 ) { 
    attachMSG=RegisterWindowMessage(L"SkypeControlAPIAttach");
    discoverMSG=RegisterWindowMessage(L"SkypeControlAPIDiscover");
  }
  if ( mainWin == NULL ) {
      mainWin = new QWidget();
      main_window = mainWin->winId();
  }

  connect( qApp, SIGNAL( winMessage( MSG *) ), this, SLOT( processWINMessage( MSG *) ) );
  msgTimer = new QTimer(this);
  skype_win=0;
  connected = false;
  refused = false;
  tryLater = false;
  waitingForConnect=false;
}

void skypeComm::sendMsgToSkype(const QString &message) {
  COPYDATASTRUCT copyData;
  QByteArray tmp;
  //qDebug()<<"SENDING MESSAGE:"<<message;

  if ( refused || tryLater ) return;
  if ( ! connected ) attachToSkype();
  if ( ! connected ) return;
  
  tmp.append(message);


  copyData.dwData=0;
  copyData.lpData=tmp.data();
  copyData.cbData=tmp.size()+1;

  SendMessage( skype_win, WM_COPYDATA, (WPARAM) main_window, (LPARAM) &copyData );
  qDebug()<<"MESSAGE SENT:"<<message<<" to"<<skype_win;

}

void skypeComm::detachFromSkype() {
  qDebug() << "Detaching from skype window "<<skype_win;
  connected = false;
  msgTimer->stop();
  disconnect( msgTimer, 0, 0, 0 );
}

bool skypeComm::attachToSkype(int TimeOut) {
  if ( connected ) return true;
  if ( refused || tryLater ) return false;
  SendMessage( HWND_BROADCAST, discoverMSG, (WPARAM) main_window, 0 );
  connect( msgTimer, SIGNAL( timeout() ), this, SLOT( processMsgs() ) );
  msgTimer->start(100);
  if ( TimeOut > 0 ) {
    QTimer::singleShot(TimeOut, this, SLOT(timeOut()));
    waitingForConnect = true;
    localEventLoop.exec();
    waitingForConnect = false;
    if ( connected ) emit attachedToSkype();
  };
  return connected;
}

void skypeComm::timeOut() {
  if ( waitingForConnect ) localEventLoop.exit(1);
}

void skypeComm::processMsgs() { 
  while ( inMessageQueue.size() > 0 ) {
    emit newMsgFromSkype( inMessageQueue.takeFirst() );
  }
}

void skypeComm::processWINMessage( MSG *msg ) {
  QByteArray tmp;
  char *data=NULL;
  COPYDATASTRUCT *copyData;
  application::eventHandled=true;
  application::eventResult=1;
  switch ( msg->message ) { 
	  case WM_COPYDATA:
		  if ( skype_win != (WId) msg->wParam ) {
		    qDebug() << "Message not from skype";
		    return;
		  }
		  copyData = (COPYDATASTRUCT *)msg->lParam;
		  data = new char[ copyData->cbData ];
		  data = qstrncpy( data, (char *) copyData->lpData, copyData->cbData );
		  tmp.append(data);
		  Q_ASSERT( data != NULL );
		  delete data;
		  qDebug() << "WM_COPYDATA:" << tmp;
		  inMessageQueue.append(tmp);
		  //emit newMsgFromSkype( tmp );
		  return;
	  default:
	    if ( msg->message == attachMSG ) {
	      qDebug() << "Attach status";
		  switch ( msg->lParam ) {
			  case SKYPE_ATTACH_SUCCESS:
				  connected=true;
				  tryLater=false;
				  skype_win = (WId) msg->wParam;
				  qDebug() << "Attached to "<<skype_win;
				  if ( waitingForConnect ) localEventLoop.quit();
				  else emit attachedToSkype();
				  return;
			  case SKYPE_TRY_NOW:
				  qDebug() << "Try to attach now";
				  tryLater=false;
				  attachToSkype();
				  return;
			  case SKYPE_REFUSED:
				  qDebug() << "Refused";
				  refused=true;
				  return;
			  case SKYPE_PENDING_AUTHORIZATION:
				  qDebug() << "Pending authorization";
				  return;
			  case SKYPE_TRY_AGAIN:
				  qDebug() << "Try Again";
				  tryLater=true;
				  if ( waitingForConnect ) localEventLoop.quit();
				  return;
			  default: 
				  qDebug() <<"WEIRD STATUS:"<<msg->lParam;
				  return;
		  }
		  
	    }
  }
  if ( skype_win == (WId) msg->wParam && connected ) { 
    qDebug() << "<b style='color:red'>!! Weird message !!</b>:" << msg->message;
  } else {
    application::eventHandled=false;
  }
}



//#include "skypeComm.moc"
#endif /* Q_WS_WIN */
