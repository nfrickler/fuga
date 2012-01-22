#ifndef __MYWINDOW_H__
#define __MYWINDOW_H__

#include "Supervisor.h"
#include <QMainWindow>

class Supervisor;

class MyWindow : public QMainWindow {
	Q_OBJECT

	public:
		MyWindow (Supervisor* mysupervisor);

	protected:
		Supervisor* m_supervisor;
		void drawMenu();

};

#endif // __MYWINDOW_H__
