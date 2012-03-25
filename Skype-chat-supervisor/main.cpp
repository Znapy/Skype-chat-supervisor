#include <QtGui/QApplication>
#include "mainwindow.h"

#ifdef Q_WS_X11

#include <X11/Xlib.h>
#include "xmessages.h"

#endif

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include "application.h"

#ifdef Q_WS_X11
// Handle X11 API messages
bool application::x11EventFilter(XEvent *event) {
        switch(event->type)
        {
                case ClientMessage:
                        XMessages::processXMessages(event);
                        return true;
                        break;
        }
        return false;
}
#endif

#ifdef Q_WS_WIN
bool application::eventHandled=false;
long application::eventResult=0;

bool application::winEventFilter( MSG *msg, long *result) {
    eventHandled=false;
    emit winMessage( msg );
    if ( eventHandled )
      *result = eventResult;
    return eventHandled;
}
#endif


int main(int argc, char *argv[])
{
    application a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

void application::closeApp() {
  quit();
}
