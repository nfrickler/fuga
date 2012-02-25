#include "MyVideo.h"
#include <iostream>
#include <QGraphicsWidget>
#include <QTime>

using namespace std;

/* constructor
 */
MyVideo::MyVideo(QHostAddress* ip, quint16 port, quint16 width, quint16 height) {

	// init
	m_image = NULL;
	m_width = 64;
	m_height = 48;
	timelag = 0;

	// start StreamListener
	m_thread = new QThread();
	m_thread->start();
	m_ts_listener = new TStreamListener(ip, port, width, height);
	m_ts_listener->moveToThread(m_thread);
	connect(m_ts_listener, SIGNAL(newFrame(QImage*)),
			this, SLOT(updateWidget(QImage*)));
	connect(this, SIGNAL(signal_startListening()),
			m_ts_listener, SLOT(slot_startListening()));

	// start listening
	emit signal_startListening();
}

/* destructor
 */
MyVideo::~MyVideo() {
	m_thread->exit();
	delete m_ts_listener;
	delete m_thread;
}

/* ######################## gl ############################################# */

/* init gl
 */
/*
void MyVideo::initializeGL() {

	glClearColor(0.3, 0.4, 0.7, 0.0); //Hintergrundfarbe: Hier ein leichtes Blau
	//glEnable(GL_DEPTH_TEST);          //Tiefentest aktivieren
	//glEnable(GL_CULL_FACE);           //Backface Culling aktivieren

	glViewport(0,0,this->width(), this->height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-this->width()/2, this->width()/2, this->height()/2, -this->height()/2, -1, 1);

}
*/

/* paint
 */
void MyVideo::paintGL() {
	if (m_image == NULL) return;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(m_image->width(), m_image->height(), GL_RGBA, GL_UNSIGNED_BYTE, m_image->bits());
}

/* resize
 */
void MyVideo::resizeGL(int inwidth, int inheight) {
	m_height = inheight;
	m_width = inwidth;

	//int pos_v = (this->height() - m_height) / 2;

	glViewport (20, 20, m_width, m_height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_width,0,m_height,-1,1);
	glMatrixMode (GL_MODELVIEW);
}

/* ######################## slots ########################################## */

/* slot: receive new image
 */
void MyVideo::updateWidget(QImage* myimage) {

	// skip image?
	if (timelag > 40) {
		delete myimage;
		myimage = NULL;
		timelag-= 20;
		cout << "MyVideo: xxxxxxxxxxxxxxx skip image xxxxxxxxxxxxx" << endl;
		return;
	}

	QTime* mytime = new QTime();
	mytime->start();

	// delete old image
	if (m_image != NULL) {
		delete m_image;
		m_image = NULL;
	}

	// resize image to fit to screen
	QImage* m_tmp_image = new QImage();
	*m_tmp_image = (m_width/myimage->width() > m_height/myimage->height())
		? myimage->scaledToHeight(m_height)
		: myimage->scaledToWidth(m_width);

	// conver to gl-format
	m_image = new QImage();
	*m_image = QGLWidget::convertToGLFormat(m_tmp_image->mirrored(true, false));
	delete myimage;
	delete m_tmp_image;
	m_tmp_image = NULL;
	myimage = NULL;

	updateGL();

	timelag+= mytime->elapsed() - 30;
	if (timelag < 0) timelag = 0;
	cout << "MyVideo: Image drawn in " << mytime->elapsed() << "|" << timelag << "ms" << endl;
}

void MyVideo::doResize(int width, int height) {
	this->resizeGL(width, height);
}


// ###################### TestGraphicsView ##########################

TestGraphicsView::TestGraphicsView()
	: QGraphicsView()
{
}

void TestGraphicsView::resizeEvent(QResizeEvent *event) {
	emit is_resized(event->size().width(), event->size().height());
}

void TestGraphicsView::setupViewport(QWidget *viewport) {
	QGLWidget *glWidget = dynamic_cast<QGLWidget*>(viewport);
	if (glWidget) glWidget->updateGL();
}
