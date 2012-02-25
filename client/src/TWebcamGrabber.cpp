#include "TWebcamGrabber.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
gboolean on_sink_message (GstBus * bus, GstMessage * message, ProgramData * data);
TWebcamGrabber* thisisme;

/* constructor
 */
TWebcamGrabber::TWebcamGrabber (quint16 img_width, quint16 img_height,
								string videopath, bool is_videostreaming) {
	cout << "TWebcamGrabber: constructor..." << endl;

	// save input
	m_width = img_width;
	m_height = img_height;
	m_videopath = videopath;
	m_isvideostreaming = is_videostreaming;
	thisisme = this;

	// wait 2 seconds for initialisation
	sleep(2);

	return;
}

/* destructor
 */
TWebcamGrabber::~TWebcamGrabber () {

}

/* get messages/errors from gstreamer
 */
gboolean on_sink_message (GstBus * bus, GstMessage * message, ProgramData * data) {
	cout << "TWebcamGrabber: Bus Message ("
			  << GST_MESSAGE_TYPE_NAME(message)
			  << ")..." << endl;

	switch (GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_EOS:
			cout << "TWebcamGrabber: No more frames..." << endl;
			break;
		case GST_MESSAGE_ERROR:
			cout << "TWebcamGrabber: Received error..." << endl;
			g_main_loop_quit (data->loop);
			break;
		default:
			break;
	}

	//gst_message_unref(message);
	return TRUE;
}

/* start grabbing
 */
void TWebcamGrabber::start () {
	cout << "TWebcamGrabber: start..." << endl;

	// init
	data = g_new0 (ProgramData, 1);
	data->loop = g_main_loop_new (NULL, FALSE);

	// set up pipeline
	stringstream ss("");
	ss << "video/x-raw-rgb,"
		<< "height=" << m_height << ",width=" << m_width
		<< ",framerate=25/1";
	gchar* video_caps = (gchar*) ss.str().c_str();
	cout << "TWebcamGrabber: Caps: " << ss.str() << endl;
	gchar* string = g_strdup_printf(
		"v4l2src ! ffmpegcolorspace ! videoscale ! %s ! appsink caps=\"%s\" name=testsink"
		,video_caps, video_caps
	);
	data->sink = gst_parse_launch (string, NULL);
	//g_free (string);

	// pipeline ok?
	if (data->sink == NULL) {
		cout << "TWebcamGrabber: Error on setting up pipeline." << endl;
		return;
	}

	// add signalhandler for bus messages
	GstBus* bus = gst_element_get_bus (data->sink);
	gst_bus_add_watch (bus, (GstBusFunc) on_sink_message, data);
	gst_object_unref (bus);

	// set options
	GstElement* testsink = gst_bin_get_by_name (GST_BIN (data->sink), "testsink");
	g_object_set (G_OBJECT (testsink), "emit-signals", FALSE, "sync", TRUE, NULL);
	gst_object_unref (testsink);

	// play!
	gst_element_set_state(data->sink, GST_STATE_PLAYING);
	cout << "TWebcamGrabber: Streaming started." << endl;
}

/* stop grabbing
 */
void TWebcamGrabber::stop()  {
	cout << "TWebcamGrabber: Stop streaming." << endl;

	// stop pipeline
	gst_element_set_state (data->sink, GST_STATE_NULL);

	// free
	gst_object_unref (data->sink);
	g_main_loop_unref (data->loop);
	g_free (data);
}

/* get next frame (slot)
 */
void TWebcamGrabber::getNextFrame() {
	GstElement* testsink = gst_bin_get_by_name (GST_BIN (data->sink), "testsink");

	// pull buffer
	if (testsink == NULL) return;
	GstBuffer* buffer = gst_app_sink_pull_buffer(GST_APP_SINK(testsink));
	if (buffer == NULL) return;

	// get width and height
	gint width = 0;
	gint height = 0;
	GstCaps *gcaps;
	const GstStructure *gstr;

	// get size of frame
	g_object_get(G_OBJECT(testsink), "caps", &gcaps, NULL);
	gst_object_unref(testsink);
	if (gcaps != NULL) {
		gstr = gst_caps_get_structure(gcaps, 0);

		gst_structure_get_int(gstr, "width", &width);
		gst_structure_get_int(gstr, "height", &height);
	}
	cout << "TWebcamGrabber: Frame (width: " << width << " height: " << height << ")" << std::endl;

	// get QImage
	unsigned char* data2 = (unsigned char *) GST_BUFFER_DATA (buffer);
	QImage* img = new QImage(data2, width, height, width * 3, QImage::Format_RGB888);
	gst_buffer_unref(buffer);

	// output
	thisisme->doEmit(img);
}

/* emit new frame
 */
void TWebcamGrabber::doEmit(QImage* myimage) {
	cout << "TWebcamGrabber: emit..." << endl;
	if (myimage == NULL) return;
	emit isNewFrame(myimage);
}
