#include "Supervisor.h"
#include <QtGui/QApplication>
#include <iostream>
#include <X11/Xlib.h>
#include <QFont>

using namespace std;

int main(int argc, char *argv[]) {
	XInitThreads();

	cout << endl;
	cout << " ############ Videochat #############" << endl;
	cout << " # Escape virtuality!               #" << endl;
	cout << " # Version alpha 1.0                #" << endl;
	cout << " # ################################ #" << endl;
	cout << endl;

	// start application
	QApplication app(argc, argv);
	app.setFont(QFont("Arial"));

	// create main object
	new Supervisor();

	// run application
	int appreturn = app.exec();

	cout << endl;
	cout << " # You have quitted FuGa...         #" << endl;
	cout << endl;
	return appreturn;
}
