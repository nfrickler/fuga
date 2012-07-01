#include "Supervisor.h"
#include <QApplication>
#include <QFont>
#include <iostream>

#include <gst/gst.h>

using namespace std;

int main(int argc, char *argv[]) {

	cout << endl;
	cout << " ############ FuGa ##################" << endl;
	cout << " # Escape virtuality!               #" << endl;
	cout << " # Version alpha 1.0                #" << endl;
	cout << " # ################################ #" << endl;
	cout << endl;

	// init gstreamer
	const gchar *nano_str;
	guint major, minor, micro, nano;
	if (!g_thread_supported ()) g_thread_init (NULL);
	gst_init (&argc, &argv);

	// print version of gstreamer
	gst_version (&major, &minor, &micro, &nano);
	if (nano == 1)		nano_str = "(CVS)";
	else if (nano == 2) nano_str = "(Prerelease)";
	else				nano_str = "";
	printf ("This program is linked against GStreamer %d.%d.%d %s\n",
			major, minor, micro, nano_str);

	// start application
	QApplication app(argc, argv);
	app.setFont(QFont("Arial"));

	// create Supervisor object
	new Supervisor();

	// run application
	app.exec();

	// finish qt-application
	cout << endl;
	cout << " # You have quitted FuGa...         #" << endl;
	cout << endl;
}
