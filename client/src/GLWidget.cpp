#include <QtGui/QMouseEvent>
#include "GLWidget.h"
#include "stdio.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

GLWidget::GLWidget(QWidget *parent):QGLWidget(parent)
{

	//initializeGL();

//	setMouseTracking(true);

	//Initialize variable members
	m_timer = new QTimer();
	m_image = 0;
	m_capture = cvCaptureFromCAM(0);
//	if(!m_capture)
//		QMessageBox::critical(this,"Error","Error initializing capture from WebCam");

	//Get an initial frame from the webcam
//	m_image = cvQueryFrame(m_capture);

	//Connect the timer signal with the capture action
//	connect(m_timer, SIGNAL(timeout()), this, SLOT(captureFrame()));        

	//Start the timer scheduled for firing every 33ms (30fps)
//	m_timer->start(33);
	
}


void GLWidget::captureFrame() {
//	m_image = cvQueryFrame(m_capture);

}

void GLWidget::initializeGL() {

	return;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_BLEND);
	glEnable(GL_POLYGON_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT);

}

void GLWidget::resizeGL(int w, int h) {
/*
     glViewport(0, 0, w, h);
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     gluOrtho2D(0, w, 0, h); // set origin to bottom left corner
     glMatrixMode(GL_MODELVIEW);
     glLoadIdentity();
*/
}

void GLWidget::paintGL() {

	/*
     glClear(GL_COLOR_BUFFER_BIT);
     glColor3f(1,0,0);
     glBegin(GL_POLYGON);
     glVertex2f(0,0);
     glVertex2f(100,500);
     glVertex2f(500,100);
     glEnd();
*/
//	initializeGL();

	//Set the raster position
	/*
	The position seems to be the inverse because the rendering is
	affected by the glPixelZoom call.
	*/ 
//	glRasterPos2i(this->width()/2,-this->height()/2);

	//Inver the image (the data coming from OpenCV is inverted)
//	glPixelZoom(-1.0f,-1.0f);

	//Draw image from OpenCV capture
//	glDrawPixels(m_image->width, m_image->height, GL_RGB, GL_UNSIGNED_BYTE,m_image->imageData); 
}


