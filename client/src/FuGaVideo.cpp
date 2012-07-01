#include "FuGaVideo.h"
#include <QSize>
#include <iostream>

using namespace std;
gboolean on_sink_message_listener (GstBus * bus, GstMessage * message, GstElement * sink);
gchar* g_str_copy_substr (const gchar *str, guint index, guint count);
static void pad_added_cb (GstElement * rtpbin, GstPad * new_pad, GstElement* depay);

GstElement* m_rtpamrdepay;
GstElement* m_rtph263pdepay;


/* constructor
 */
FuGaVideo::FuGaVideo(QHostAddress* in_address, quint16 in_port) {

	// init
	m_address = in_address;
	m_port = in_port;

	// init pipeline
	this->setAttribute(Qt::WA_NativeWindow,true);
	init();
}

/* destructor
 */
FuGaVideo::~FuGaVideo() {
	stop();
}

void FuGaVideo::resizeEvent(QResizeEvent *e) {
	cout << "Do resize!" << endl;
	QSize size(4,3);
	size.scale(e->size().width(),e->size().height(),Qt::KeepAspectRatio);
	if (size != this->size()) resize(size);
}

/* setting up the following pipeline:

 gst-launch -v gstrtpbin name=rtpbin                                          \
	 udpsrc caps="application/x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=(string)H263-1998" \
			 port=5000 ! rtpbin.recv_rtp_sink_0                                \
		 rtpbin. ! rtph263pdepay ! ffdec_h263 ! xvimagesink                    \
	  udpsrc port=5001 ! rtpbin.recv_rtcp_sink_0                               \
	  rtpbin.send_rtcp_src_0 ! udpsink port=5005 sync=false async=false        \
	 udpsrc caps="application/x-rtp,media=(string)audio,clock-rate=(int)8000,encoding-name=(string)AMR,encoding-params=(string)1,octet-align=(string)1" \
			 port=5002 ! rtpbin.recv_rtp_sink_1                                \
		 rtpbin. ! rtpamrdepay ! amrnbdec ! alsasink                           \
	  udpsrc port=5003 ! rtpbin.recv_rtcp_sink_1                               \
	  rtpbin.send_rtcp_src_1 ! udpsink port=5007 sync=false async=false
 */
void FuGaVideo::init() {

	// pipeline
	m_pipeline = gst_pipeline_new ("mypipeline");
	GstElement* gstrtpbin = gst_element_factory_make ("gstrtpbin", NULL);
	gst_bin_add_many(GST_BIN (m_pipeline), gstrtpbin, NULL);

	// udp -> video
	GstElement* udpsrc_v0 = gst_element_factory_make ("udpsrc", "udpsrc_v0");
	g_object_set (G_OBJECT(udpsrc_v0), "port", 5000, NULL);
	GstCaps* v_caps = gst_caps_from_string ("application/x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=(string)H263-1998");
	g_object_set (udpsrc_v0, "caps", v_caps, NULL);
	gst_caps_unref (v_caps);

	// video rtcp-handling
	GstElement* udpsrc_v1 = gst_element_factory_make ("udpsrc", "udpsrc_v1");
	g_object_set (G_OBJECT(udpsrc_v1), "port", 5001, NULL);
	GstElement* udpsink_v2 = gst_element_factory_make ("udpsink", "udpsink_v2");
	g_object_set (G_OBJECT(udpsink_v2), "host", "127.0.0.1", "port", 5005, "sync", FALSE, "async", FALSE, NULL);

	gst_bin_add_many(GST_BIN (m_pipeline), udpsrc_v0, udpsrc_v1, udpsink_v2, NULL);

	// video link
	GstPad* srcpad_v = gst_element_get_static_pad (udpsrc_v0, "src");
	GstPad* sinkpad_v = gst_element_get_request_pad (gstrtpbin, "recv_rtp_sink_0");
	gboolean lres_v = gst_pad_link (srcpad_v, sinkpad_v);
	g_assert (lres_v == GST_PAD_LINK_OK);
	gst_object_unref (srcpad_v);
	gst_object_unref (sinkpad_v);
	gst_element_link_pads (udpsrc_v1, "src", gstrtpbin, "recv_rtcp_sink_0");
	gst_element_link_pads (gstrtpbin, "send_rtcp_src_0", udpsink_v2, "sink");

	// video -> output
	m_rtph263pdepay = gst_element_factory_make ("rtph263pdepay", NULL);
	GstElement* ffdec_h263 = gst_element_factory_make ("ffdec_h263", NULL);
	m_xvimagesink = gst_element_factory_make ("xvimagesink", NULL);
	gst_bin_add_many(GST_BIN (m_pipeline), m_rtph263pdepay, ffdec_h263, m_xvimagesink, NULL);
	gst_element_link(m_rtph263pdepay, ffdec_h263);
	gst_element_link(ffdec_h263, m_xvimagesink);

	// udp -> audio
	GstElement* udpsrc_a0 = gst_element_factory_make ("udpsrc", "udpsrc_a0");
	g_object_set (G_OBJECT(udpsrc_a0), "port", 5002, NULL);
	GstCaps* a_caps = gst_caps_from_string ("application/x-rtp,media=(string)audio,clock-rate=(int)8000,encoding-name=(string)AMR,encoding-params=(string)1,octet-align=(string)1");
	g_object_set (udpsrc_a0, "caps", a_caps, NULL);
	gst_caps_unref (a_caps);

	// audio rtcp-handling
	GstElement* udpsrc_a1 = gst_element_factory_make ("udpsrc", "udpsrc_a1");
	g_object_set (G_OBJECT(udpsrc_a1), "port", 5003, NULL);
	GstElement* udpsink_a2 = gst_element_factory_make ("udpsink", "udpsink_a2");
	g_object_set (G_OBJECT(udpsink_a2), "host", "127.0.0.1", "port", 5007, "sync", FALSE, "async", FALSE, NULL);

	gst_bin_add_many(GST_BIN (m_pipeline), udpsrc_a0, udpsrc_a1, udpsink_a2, NULL);

	// audio link
	GstPad* srcpad_a = gst_element_get_static_pad (udpsrc_a0, "src");
	GstPad* sinkpad_a = gst_element_get_request_pad (gstrtpbin, "recv_rtp_sink_1");
	gboolean lres_a = gst_pad_link (srcpad_a, sinkpad_a);
	g_assert (lres_a == GST_PAD_LINK_OK);
	gst_object_unref (srcpad_a);
	gst_object_unref (sinkpad_a);
	gst_element_link_pads (udpsrc_a1, "src", gstrtpbin, "recv_rtcp_sink_1");
	gst_element_link_pads (gstrtpbin, "send_rtcp_src_1", udpsink_a2, "sink");

	// audio -> output
	m_rtpamrdepay = gst_element_factory_make ("rtpamrdepay", NULL);
	GstElement* amrnbdec = gst_element_factory_make ("amrnbdec", NULL);
	GstElement* alsasink = gst_element_factory_make ("alsasink", NULL);
	gst_bin_add_many(GST_BIN (m_pipeline), m_rtpamrdepay, amrnbdec, alsasink, NULL);
	gst_element_link(m_rtpamrdepay, amrnbdec);
	gst_element_link(amrnbdec, alsasink);

	// dynamically add rtp-stream
	g_signal_connect (gstrtpbin, "pad-added", G_CALLBACK (pad_added_cb), NULL);

	// pipeline ok?
	if (m_pipeline == NULL) {
		cout << "TStreamListener: Error on setting up pipeline." << endl;
		return;
	}

	// add signalhandler for bus messages
	GstBus* bus = gst_element_get_bus (m_pipeline);
	gst_bus_add_watch (bus, (GstBusFunc) on_sink_message_listener, m_pipeline);
	gst_object_unref (bus);
}

/* start pipeline
 */
void FuGaVideo::start() {

	// set xoverlay
	WId xwinid = this->winId();
	gst_x_overlay_set_window_handle (GST_X_OVERLAY (m_xvimagesink), xwinid);

	// start playing
	GstStateChangeReturn sret = gst_element_set_state (m_pipeline, GST_STATE_PLAYING);
	if (sret == GST_STATE_CHANGE_FAILURE) {
		gst_element_set_state (m_pipeline, GST_STATE_NULL);
		gst_object_unref (m_pipeline);
	}
	cout << "FuGaVideo: start playing!" << endl;
}

/* stop pipeline
 */
void FuGaVideo::stop () {
	gst_element_set_state (m_pipeline, GST_STATE_NULL);
	g_object_unref(m_xvimagesink);
	g_object_unref(m_rtpamrdepay);
	g_object_unref(m_rtph263pdepay);
	g_object_unref(m_pipeline);
}

/* get messages/errors from gstreamer
 */
gboolean on_sink_message_listener (GstBus* bus, GstMessage* message, GstElement* sink) {
	cout << "FuGaVideo: Bus Message ("
			  << GST_MESSAGE_TYPE_NAME(message)
			  << ")..."
			  << endl;

	switch (GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_EOS:
			cout << "TStreamListener: No more frames..." << endl;
			break;
		case GST_MESSAGE_ERROR:
			{
				GError *err = NULL;
				gchar *dbg_info = NULL;

				gst_message_parse_error (message, &err, &dbg_info);
				g_printerr ("FuGaVideo: ERROR from element %s: %s\n",
					GST_OBJECT_NAME (message->src), err->message);
				g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
				g_error_free (err);
				g_free (dbg_info);
			}
			break;
		default:
			break;
	}

	return TRUE;
}

/* add rtp-streams dynamically
  */
static void pad_added_cb (GstElement * rtpbin, GstPad * new_pad, GstElement* depay) {
  g_print ("New rtp-pad: %s\n", GST_PAD_NAME (new_pad));
  gchar* myname = g_str_copy_substr(GST_PAD_NAME (new_pad), 0, 14);

  // get pad to connect to
  depay = (g_strcasecmp(myname, "recv_rtp_src_1") == 0) ? m_rtpamrdepay : m_rtph263pdepay ;
  GstPad* sinkpad = gst_element_get_static_pad (depay, "sink");
  g_assert (sinkpad);

  GstPadLinkReturn lres = gst_pad_link (new_pad, sinkpad);
  g_assert (lres == GST_PAD_LINK_OK);
  gst_object_unref (sinkpad);
}

/* substr
  */
gchar* g_str_copy_substr (const gchar *str, guint index, guint count) {
	gchar *result;
	guint i;

	if (str == NULL || count <= 0) return NULL;

	result = g_new0 (gchar, count + 1);
	for (i = 0; i < count; i++)
		result[i] = str[index + i];
	return result;
}
