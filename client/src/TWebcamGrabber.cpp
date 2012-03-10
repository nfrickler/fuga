#include "TWebcamGrabber.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
gboolean on_sink_message (GstBus * bus, GstMessage * message);
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

	return;
}

/* destructor
 */
TWebcamGrabber::~TWebcamGrabber () {

}

/* get messages/errors from gstreamer
 */
gboolean on_sink_message (GstBus * bus, GstMessage * message) {
	cout << "TWebcamGrabber: Bus Message ("
			  << GST_MESSAGE_TYPE_NAME(message)
			  << ")..." << endl;

	switch (GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_EOS:
			cout << "TWebcamGrabber: No more frames..." << endl;
			break;
		case GST_MESSAGE_ERROR:
			cout << "TWebcamGrabber: Received error..." << endl;
			break;
		default:
			break;
	}

	return TRUE;
}

/* start grabbing
 */
void TWebcamGrabber::start () {
	cout << "TWebcamGrabber: start..." << endl;

	/*
	gst-launch gstrtpbin name=rtpbin \
			v4l2src ! ffmpegcolorspace ! ffenc_h263 ! rtph263ppay ! rtpbin.send_rtp_sink_0 \
					  rtpbin.send_rtp_src_0 ! udpsink port=5000                            \
					  rtpbin.send_rtcp_src_0 ! udpsink port=5001 sync=false async=false    \
					  udpsrc port=5005 ! rtpbin.recv_rtcp_sink_0                           \
			audiotestsrc ! amrnbenc ! rtpamrpay ! rtpbin.send_rtp_sink_1                   \
					  rtpbin.send_rtp_src_1 ! udpsink port=5002                            \
					  rtpbin.send_rtcp_src_1 ! udpsink port=5003 sync=false async=false    \
					  udpsrc port=5007 ! rtpbin.recv_rtcp_sink_1
	*/

	// pipeline
	m_pipeline = gst_pipeline_new ("mypipeline");
	GstElement* gstrtpbin = gst_element_factory_make ("gstrtpbin", NULL);
	gst_bin_add_many(GST_BIN (m_pipeline), gstrtpbin, NULL);

	// video
	GstElement* v4l2src = gst_element_factory_make ("v4l2src", NULL);
	GstElement* ffmpegcolorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);
	GstElement* ffenc_h263 = gst_element_factory_make ("ffenc_h263", NULL);
	GstElement* rtph263ppay = gst_element_factory_make ("rtph263ppay", NULL);
	gst_bin_add_many(GST_BIN (m_pipeline), v4l2src, ffmpegcolorspace, ffenc_h263, rtph263ppay, NULL);
	gst_element_link(v4l2src, ffmpegcolorspace);
	gst_element_link(ffmpegcolorspace, ffenc_h263);
	gst_element_link(ffenc_h263, rtph263ppay);
	gst_element_link_pads (rtph263ppay, "src", gstrtpbin, "send_rtp_sink_0");

	// video -> udp
	GstElement* udpsink_v0 = gst_element_factory_make ("udpsink", NULL);
	g_object_set (G_OBJECT(udpsink_v0), "host", "127.0.0.1", "port", 5000, NULL);
	GstElement* udpsink_v1 = gst_element_factory_make ("udpsink", NULL);
	g_object_set (G_OBJECT(udpsink_v1), "host", "127.0.0.1", "port", 5001, "sync", FALSE, "async", FALSE, NULL);
	GstElement* udpsrc_v2 = gst_element_factory_make ("udpsrc", NULL);
	g_object_set (G_OBJECT(udpsrc_v2), "port", 5005, NULL);

	gst_bin_add_many(GST_BIN (m_pipeline), udpsink_v0, udpsink_v1, udpsrc_v2, NULL);

	gst_element_link_pads (gstrtpbin, "send_rtp_src_0", udpsink_v0, "sink");
	gst_element_link_pads (gstrtpbin, "send_rtcp_src_0", udpsink_v1, "sink");
	gst_element_link_pads (udpsrc_v2, "src", gstrtpbin, "recv_rtcp_sink_0");

	// audio
	GstElement* audiotestsrc = gst_element_factory_make ("audiotestsrc", NULL);
	GstElement* amrnbenc = gst_element_factory_make ("amrnbenc", NULL);
	GstElement* rtpamrpay = gst_element_factory_make ("rtpamrpay", NULL);
	gst_bin_add_many(GST_BIN (m_pipeline), audiotestsrc, amrnbenc, rtpamrpay, NULL);
	gst_element_link(audiotestsrc, amrnbenc);
	gst_element_link(amrnbenc, rtpamrpay);
	gst_element_link_pads (rtpamrpay, "src", gstrtpbin, "send_rtp_sink_1");

	// audio -> udp
	GstElement* udpsink_a0 = gst_element_factory_make ("udpsink", NULL);
	g_object_set (G_OBJECT(udpsink_a0), "host", "127.0.0.1", "port", 5002, NULL);
	GstElement* udpsink_a1 = gst_element_factory_make ("udpsink", NULL);
	g_object_set (G_OBJECT(udpsink_a1), "host", "127.0.0.1", "port", 5003, "sync", FALSE, "async", FALSE, NULL);
	GstElement* udpsrc_a2 = gst_element_factory_make ("udpsrc", NULL);
	g_object_set (G_OBJECT(udpsrc_a2), "port", 5007, NULL);

	gst_bin_add_many(GST_BIN (m_pipeline), udpsink_a0, udpsink_a1, udpsrc_a2, NULL);

	gst_element_link_pads (gstrtpbin, "send_rtp_src_1", udpsink_a0, "sink");
	gst_element_link_pads (gstrtpbin, "send_rtcp_src_1", udpsink_a1, "sink");
	gst_element_link_pads (udpsrc_a2, "src", gstrtpbin, "recv_rtcp_sink_1");

	// pipeline ok?
	if (m_pipeline == NULL) {
		cout << "TWebcamGrabber: Error on setting up pipeline." << endl;
		return;
	}
	cout << "TWebcamGrabber: pipeline created" << endl;

	// add signalhandler for bus messages
	GstBus* bus = gst_element_get_bus (m_pipeline);
	gst_bus_add_watch (bus, (GstBusFunc) on_sink_message, NULL);
	gst_object_unref (bus);

	// play!
	gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

/* stop grabbing
 */
void TWebcamGrabber::stop()  {
	cout << "TWebcamGrabber: Stop streaming." << endl;

	// stop pipeline
	gst_element_set_state (m_pipeline, GST_STATE_NULL);

	// free
	gst_object_unref (m_pipeline);
}
