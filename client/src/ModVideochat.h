#ifndef __MODVIDEOCHAT_H__
#define __MODVIDEOCHAT_H__

#include "FuGaModule.h"
#include "MyWindow.h"
#include "FuGaVideo.h"
#include "Chatbox.h"
#include "Supervisor.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>

class MyWindow;
class Supervisor;
class FuGaVideo;
class FuGaModule;
class Chatbox;

class ModVideochat : public FuGaModule {
	Q_OBJECT

	public:
		ModVideochat(MyWindow* mywindow, Supervisor* mysupervisor);

	protected:
		MyWindow* m_main_window;
		Supervisor* m_supervisor;
		QTimer* m_timer;

		void askForPerson();
		void showVideo();

		// current connection
		std::string m_name;
		bool m_islistening;

		// set connection details
		QLineEdit* m_name_input;

		// chat
		Chatbox* m_chatbox;

	public slots:
		void newError(char *msg);
		void setConnectionData();
		void slot_showVideo(std::string name);
		void slot_videoFailed();

};

#endif // __MODVIDEOCHAT_H__
