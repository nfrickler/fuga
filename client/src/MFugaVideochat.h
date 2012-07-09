#ifndef __MFUGAVIDEOCHAT_H__
#define __MFUGAVIDEOCHAT_H__

#include "FuGaModule.h"
#include "FugaWindow.h"
#include "FugaVideo.h"
#include "FugaStreamer.h"
#include "Chatbox.h"
#include "Fuga.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>

class FugaWindow;
class Fuga;
class FugaVideo;
class FugaModule;
class Chatbox;
class FugaStreamer;
class FugaContact;

class MFugaVideochat : public FuGaModule {
	Q_OBJECT

	public:
        MFugaVideochat();
        MFugaVideochat(FugaWindow* in_Window, Fuga* in_Fuga);

	protected:
        FugaWindow* m_main_window;
        Fuga* m_Fuga;
        QTimer* m_timer;
        FugaContact* m_Contact;

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
        void slot_showVideo();
		void slot_videoFailed();

};

#endif // __MFUGAVIDEOCHAT_H__
