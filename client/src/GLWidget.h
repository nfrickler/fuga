#ifndef _GLWIDGET_H
#define _GLWIDGET_H

#include <QtOpenGL/QGLWidget>
#include "stdio.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QTimer>
#include <QMessageBox>

class QTimer;
struct CvCapture;
struct _IplImage;

class GLWidget : public QGLWidget {

	Q_OBJECT // must include this if you use Qt signals/slots

public:
	GLWidget(QWidget *parent = NULL);

protected:
	void initializeGL();
	void paintGL();
     void resizeGL(int w, int h);

  private:
         QTimer *m_timer;
         CvCapture *m_capture;
         _IplImage *m_image;


   private slots:
         void captureFrame();

};

#endif  /* _GLWIDGET_H */