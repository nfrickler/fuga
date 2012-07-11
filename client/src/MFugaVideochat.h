#ifndef __MFUGAVIDEOCHAT_H__
#define __MFUGAVIDEOCHAT_H__

#include "FugaWindow.h"
#include "FugaVideo.h"
#include "FugaStreamer.h"
#include "Fuga.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>

class FugaWindow;
class Fuga;
class FugaVideo;
class FugaChat;
class FugaStreamer;
class FugaContact;

class MFugaVideochat : public QObject {
	Q_OBJECT

	public:
        MFugaVideochat();
        MFugaVideochat(FugaWindow* in_Window, Fuga* in_Fuga);
        ~MFugaVideochat();

	protected:
        FugaWindow* m_main_window;
        Fuga* m_Fuga;
        QTimer* m_timer;
        std::string m_partner;

		void askForPerson();
		void showVideo();

		// set connection details
		QLineEdit* m_name_input;

		// chat
        FugaChat* m_Chat;

    public slots:
		void setConnectionData();
        void slot_showVideo(std::string);
        void slot_videoFailed();

};

#endif // __MFUGAVIDEOCHAT_H__
