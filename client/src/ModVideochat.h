#ifndef __MODVIDEOCHAT_H__
#define __MODVIDEOCHAT_H__

#include "FuGaModule.h"
#include "FugaWindow.h"
#include "FuGaVideo.h"
#include "FuGaStreamer.h"
#include "Chatbox.h"
#include "Fuga.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>

class FugaWindow;
class Fuga;
class FuGaVideo;
class FuGaModule;
class Chatbox;
class FugaStreamer;

class ModVideochat : public FuGaModule {
	Q_OBJECT

	public:
        ModVideochat(FugaWindow* in_Window, Fuga* in_Fuga);

	protected:
        FugaWindow* m_main_window;
        Fuga* m_Fuga;
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
