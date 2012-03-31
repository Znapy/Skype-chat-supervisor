#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

#include "skype.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    skype *mySkype;
    QStringList *mesInQueue;

public slots:
    //void quit();

private slots:
    void skypeConnected();
    void skypeConnectionLost();
    void chatMessageRecieved(skypeResponse &mes);

signals:
    void status( const QString & statusMessage, int );

};

#endif // MAINWINDOW_H
