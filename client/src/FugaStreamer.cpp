#include "FugaStreamer.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
gboolean on_sink_message (GstBus* bus, GstMessage* message);
FugaStreamer* thisisme;

// constructor
FugaStreamer::FugaStreamer (QHostAddress* in_address, quint16 in_port, quint16 in_width,
							quint16 in_height, string in_path) {
    cout << "FugaStreamer: construct it" << endl;

	// save input
	m_address = in_address;
	m_port = in_port;
	m_width = in_width;
	m_height = in_height;
    m_path = in_path;
	thisisme = this;

	return;
}

// destructor
FugaStreamer::~FugaStreamer () {
	stop();
}

/* start streaming video using the following pipeline

 gst-launch gstrtpbin name=rtpbin \
		 v4l2src ! ffmpegcolorspace ! ffenc_h263 ! rtph263ppay ! rtpbin.send_rtp_sink_0 \
				   rtpbin.send_rtp_src_0 ! udpsink port=5000                            \
				   rtpbin.send_rtcp_src_0 ! udpsink port=5001 sync=false async=false    \
				   udpsrc port=5005 ! rtpbin.recv_rtcp_sink_0                           \
		 autoaudiosrc ! amrnbenc ! rtpamrpay ! rtpbin.send_rtp_sink_1                   \
				   rtpbin.send_rtp_src_1 ! udpsink port=5002                            \
				   rtpbin.send_rtcp_src_1 ! udpsink port=5003 sync=false async=false    \
				   udpsrc port=5007 ! rtpbin.recv_rtcp_sink_1
 */
void FugaStreamer::start () {
    cout << "FugaStreamer: creating pipeline..." << endl;

	// pipeline
	m_pipeline = gst_pipeline_new ("mypipeline");
	GstElement* gstrtpbin = gst_element_factory_make ("gstrtpbin", NULL);
	gst_bin_add_many(GST_BIN (m_pipeline), gstrtpbin, NULL);

	// video
    GstElement* videosrc;
    if (1 || m_path.empty()) {
        cout << "FugaStreamer: Start streaming of webcam" << endl;
        videosrc = gst_element_factory_make("v4l2src", NULL);
        gst_bin_add_many(GST_BIN (m_pipeline), videosrc, NULL);
    } else {
        cout << "FugaStreamer: Start streaming of file " << m_path << endl;
        GstElement* videosrc0 = gst_element_factory_make("filesrc", NULL);
        g_object_set(G_OBJECT(videosrc), "location", m_path.c_str(), NULL);
        videosrc = gst_element_factory_make("decodebin", NULL);
        gst_bin_add_many(GST_BIN (m_pipeline), videosrc0, videosrc, NULL);
        gst_element_link(videosrc0, videosrc);
    }

    GstElement* videoscale = gst_element_factory_make ("videoscale", NULL);
    GstElement* ffmpegcolorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);
	GstElement* ffenc_h263 = gst_element_factory_make ("ffenc_h263", NULL);
	GstElement* rtph263ppay = gst_element_factory_make ("rtph263ppay", NULL);
    gst_bin_add_many(GST_BIN (m_pipeline), videoscale, ffmpegcolorspace, ffenc_h263, rtph263ppay, NULL);
    gst_element_link(videosrc, videoscale);
    gst_element_link(videoscale, ffmpegcolorspace);
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
	GstElement* audiosrc = gst_element_factory_make ("autoaudiosrc", NULL);
	GstElement* amrnbenc = gst_element_factory_make ("amrnbenc", NULL);
	GstElement* rtpamrpay = gst_element_factory_make ("rtpamrpay", NULL);
	gst_bin_add_many(GST_BIN (m_pipeline), audiosrc, amrnbenc, rtpamrpay, NULL);
	gst_element_link(audiosrc, amrnbenc);
	gst_element_link(amrnbenc, rtpamrpay);
	gst_element_link_pads (rtpamrpay, "src", gstrtpbin, "send_rtp_sink_1");

	// audio -> udp
	GstElement* udpsink_a0 = gst_element_factory_make ("udpsink", NULL);
	g_object_set (G_OBJECT(udpsink_a0), "host", m_address->toString().toAscii().data(), "port", 5002, NULL);
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
        cout << "FugaStreamer: Error on setting up pipeline." << endl;
		return;
	}
    cout << "FugaStreamer: pipeline created" << endl;

	// add signalhandler for bus messages
	GstBus* bus = gst_element_get_bus (m_pipeline);
	gst_bus_add_watch (bus, (GstBusFunc) on_sink_message, NULL);
	gst_object_unref (bus);

	// play!
	gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

// stop streaming
void FugaStreamer::stop()  {
    cout << "FugaStreamer: Stop streaming." << endl;

	// stop pipeline
	gst_element_set_state (m_pipeline, GST_STATE_NULL);

	// free
	gst_object_unref (m_pipeline);
}

// get messages/errors from gstreamer
gboolean on_sink_message (GstBus* bus, GstMessage* message) {
    cout << "FugaStreamer: Bus Message ("
			  << GST_MESSAGE_TYPE_NAME(message)
			  << ")..." << endl;

	switch (GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_EOS:
            cout << "FugaStreamer: No more frames..." << endl;
			break;
		case GST_MESSAGE_ERROR:
            cout << "FugaStreamer: Received error..." << endl;
{
            GError *err = NULL;
            gchar *dbg_info = NULL;

            gst_message_parse_error (message, &err, &dbg_info);
            g_printerr ("FugaStreamer: ERROR from element %s: %s\n",
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
