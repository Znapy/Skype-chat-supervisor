#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef WS_X11
 #include <unistd.h>
#elif WS_WIN
 #include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect( ui->pushButton_Close, SIGNAL(clicked()), this, SLOT(close()) );

    mesInQueue = new QStringList;

    mySkype = new skype( "Skype-chat-supervisor" );
    connect( mySkype, SIGNAL( connectedToSkype() ), this, SLOT( skypeConnected() ) );
    emit status( "Connecting to Skype...", 0 );
    mySkype->connectToSkype();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::skypeConnected() {
  connect( mySkype, SIGNAL( connectionLost() ), this, SLOT( skypeConnectionLost() ) );
  connect( mySkype, SIGNAL( recievedChatMessage(skypeResponse&) ), this, SLOT( chatMessageRecieved(skypeResponse&) ) );
//  connect( mySkype, SIGNAL( dataInStream( QString ) ), this, SLOT( skypeData( QString ) ) );
//  connect( mySkype, SIGNAL( newStreamCreated( QString ) ), this, SLOT( newPeer( QString ) ) );
//  connect( mySkype, SIGNAL( contactOffline( QString ) ), this, SLOT( contactOffline( QString ) ) );
//  connect( mySkype, SIGNAL( contactOnline( QString ) ), this, SLOT( contactOnline( QString ) ) );
//  connect( mySkype, SIGNAL( skypeOffline() ), this, SLOT( skypeOffline() ) );
//  connect( mySkype, SIGNAL( skypeOnline() ), this, SLOT( skypeOnline() ) );
  emit status( "Connected to Skype", 0);
//  mySkype->updateOnlineContacts();
}

void MainWindow::skypeConnectionLost() {
//  qDebug() << "Lost Connection to Skype.";
  emit status("Lost connection to Skype", 0);
}

void MainWindow::chatMessageRecieved(skypeResponse &mes) {
    if (mes.getMStatus() == skypeResponse::SK_CMS_RECEIVED ){
        mySkype->doCommand( skypeCommand::GET_CHATMESSAGE_PROPERTY(mes.getChatMessageId(), "BODY") );
    }else if(mes.getChatMessageProperty() == "BODY"){
        if (mes.getChatMessageContent() == "!ping"){
            mesInQueue->append( mes.getChatMessageId() );
            mySkype->doCommand( skypeCommand::GET_CHATMESSAGE_PROPERTY(mes.getChatMessageId(), "CHATNAME") );
        }
    }else if( mes.getChatMessageProperty() == "CHATNAME" && mesInQueue->contains(mes.getChatMessageId()) ){
        mySkype->doCommand( skypeCommand::SEND_CHATMESSAGE(mes.getChatMessageContent(), "pong") );
        mesInQueue->removeOne(mes.getChatMessageId());
    }
}
