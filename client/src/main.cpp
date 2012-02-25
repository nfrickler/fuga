#include "Supervisor.h"
#include <QtGui/QApplication>
#include <iostream>
#include <X11/Xlib.h>
#include <QFont>

#include <gst/gst.h>

using namespace std;

int main(int argc, char *argv[]) {
	//XInitThreads();

	cout << endl;
	cout << " ############ FuGa ##################" << endl;
	cout << " # Escape virtuality!               #" << endl;
	cout << " # Version alpha 1.0                #" << endl;
	cout << " # ################################ #" << endl;
	cout << endl;

	/*
	 * init gstreamer
	 */
	const gchar *nano_str;
	guint major, minor, micro, nano;

	// init and get version
	gst_init (&argc, &argv);
	gst_version (&major, &minor, &micro, &nano);

	// print version
	if (nano == 1)
	  nano_str = "(CVS)";
	else if (nano == 2)
	  nano_str = "(Prerelease)";
	else
	  nano_str = "";
	printf ("This program is linked against GStreamer %d.%d.%d %s\n",
			major, minor, micro, nano_str);

	/*
	 * start qt-application
	 */

	// start application
	QApplication app(argc, argv);
	app.setFont(QFont("Arial"));

	// create main object
	new Supervisor();

	// run application
	int appreturn = app.exec();

	/*
	 * finish qt-application
	 */

	cout << endl;
	cout << " # You have quitted FuGa...         #" << endl;
	cout << endl;
	return appreturn;
}
