#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <QtGui/QApplication>


class application : public QApplication {
  Q_OBJECT
	public:
                application( int &argc, char **argv): QApplication( argc, argv, true ) {};

#ifdef Q_WS_X11
		virtual bool x11EventFilter(XEvent *event);
#endif

#ifdef Q_WS_WIN
		static bool eventHandled;
		static long eventResult;

		virtual bool winEventFilter ( MSG * msg, long * result );
	signals:
		void winMessage( MSG *msg );
#endif

	public slots:
		void closeApp();
};

#endif /* _APPLICATION_H */
