#include "FugaStreamer.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
gboolean on_sink_message (GstBus* bus, GstMessage* message);
static void streamer_pad_added_cb (GstElement* rtpbin, GstPad* new_pad, FugaStreamer* Streamer);

// constructor
FugaStreamer::FugaStreamer (QHostAddress* in_ip, quint16 in_firstport, string in_path) {

	// save input
    m_ip = in_ip;
    m_firstport = in_firstport;
    m_path = in_path;
    m_pipeline = NULL;

	return;
}

// destructor
FugaStreamer::~FugaStreamer () {
	stop();
}

/* start streaming video using a changed form of the following pipeline

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

    // pipeline
    m_pipeline = gst_pipeline_new("mypipeline");
    g_assert(m_pipeline);

    // get gstrtpbin
    GstElement* gstrtpbin = gst_element_factory_make("gstrtpbin", NULL);
    g_assert(gstrtpbin);
    gst_bin_add_many(GST_BIN (m_pipeline), gstrtpbin, NULL);

    // ###################### sourcebin ############################
    // ################# source -> video/audio #####################

    // create source
    GstElement* webcamsrc;
    GstElement* audiosrc;
    if (m_path.empty()) {
        cout << "FugaStreamer: Start streaming of webcam" << endl;

        // capture webcam
        webcamsrc = gst_element_factory_make("v4l2src", "webcamsrc");
        g_assert(webcamsrc);
        gst_bin_add_many(GST_BIN (m_pipeline), webcamsrc, NULL);

        // capture microphone
        audiosrc = gst_element_factory_make("autoaudiosrc", "audiosrc");
        g_assert(audiosrc);
        gst_bin_add_many(GST_BIN (m_pipeline), audiosrc, NULL);

    } else {
        cout << "FugaStreamer: Start streaming of file " << m_path << endl;

        // capture from file
        GstElement* decodebin = gst_element_factory_make("decodebin", "decodebin");
        g_assert(decodebin);
        GstElement* filesrc = gst_element_factory_make("filesrc", "filesrc");
        g_assert(filesrc);
        g_object_set(G_OBJECT(filesrc), "location", m_path.data(), NULL);
        gst_bin_add_many(GST_BIN(m_pipeline), filesrc, decodebin, NULL);
        gst_element_link(filesrc, decodebin);

        // link src-pads of decodebin dynamically
        g_signal_connect(decodebin, "pad-added", G_CALLBACK(streamer_pad_added_cb), this);
    }

    // ###################### sendaudiobin ########################
    // ###################### audio => rtp ########################

    m_audiohandler = gst_element_factory_make ("audioconvert", NULL);
    GstElement* audiorate = gst_element_factory_make ("audiorate", NULL);
    GstElement* audioresample = gst_element_factory_make ("audioresample", NULL);
    GstElement* amrnbenc = gst_element_factory_make ("amrnbenc", NULL);
    GstElement* rtpamrpay = gst_element_factory_make ("rtpamrpay", NULL);
    g_assert(m_audiohandler && audioresample && amrnbenc && rtpamrpay);
    gst_bin_add_many(GST_BIN (m_pipeline), m_audiohandler, audioresample, audiorate, amrnbenc, rtpamrpay, NULL);
    gst_element_link(m_audiohandler, audiorate);
    gst_element_link(audiorate, audioresample);
    gst_element_link(audioresample, amrnbenc);
    gst_element_link(amrnbenc, rtpamrpay);
    gst_element_link_pads(rtpamrpay, "src", gstrtpbin, "send_rtp_sink_1");

    // ###################### sendvideobin ########################
    // ###################### video => rtp ########################

    m_videohandler = gst_element_factory_make ("videoscale", NULL);
    g_assert(m_videohandler);
    GstElement* ffmpegcolorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);
    GstElement* ffenc_h263 = gst_element_factory_make ("ffenc_h263", NULL);
    GstElement* rtph263ppay = gst_element_factory_make ("rtph263ppay", NULL);
    g_assert(ffmpegcolorspace && ffenc_h263 && rtph263ppay);
    gst_bin_add_many(GST_BIN (m_pipeline), m_videohandler, ffmpegcolorspace, ffenc_h263, rtph263ppay, NULL);
    gst_element_link(m_videohandler, ffmpegcolorspace);
    gst_element_link(ffmpegcolorspace, ffenc_h263);
    gst_element_link(ffenc_h263, rtph263ppay);
    gst_element_link_pads(rtph263ppay, "src", gstrtpbin, "send_rtp_sink_0");

    // ###################### gstrtpbin ##############################
    // ################## rtp/rtcp handling ##########################

    // sink video rtp
    GstElement* udpsink_v0 = gst_element_factory_make("udpsink", NULL);
    g_object_set(G_OBJECT(udpsink_v0), "host", "127.0.0.1", "port", m_firstport, NULL);

    // sink video rtcp
    GstElement* udpsink_v1 = gst_element_factory_make("udpsink", NULL);
    g_object_set(G_OBJECT(udpsink_v1), "host", "127.0.0.1", "port", (m_firstport+1), "sync", FALSE, "async", FALSE, NULL);

    // source video rtcp
    GstElement* udpsrc_v2 = gst_element_factory_make("udpsrc", NULL);
    g_object_set(G_OBJECT(udpsrc_v2), "port", (m_firstport+4), NULL);

    // link video rtp/rtcp
    gst_bin_add_many(GST_BIN (m_pipeline), udpsink_v0, udpsink_v1, udpsrc_v2, NULL);
    gst_element_link_pads(gstrtpbin, "send_rtp_src_0", udpsink_v0, "sink");
    gst_element_link_pads(gstrtpbin, "send_rtcp_src_0", udpsink_v1, "sink");
    gst_element_link_pads(udpsrc_v2, "src", gstrtpbin, "recv_rtcp_sink_0");

    // sink audio rtp
    GstElement* udpsink_a0 = gst_element_factory_make ("udpsink", NULL);
    g_object_set (G_OBJECT(udpsink_a0),
                  "host", m_ip->toString().toAscii().data(),
                  "port", (m_firstport+2), NULL);

    // sink audio rtcp
    GstElement* udpsink_a1 = gst_element_factory_make ("udpsink", NULL);
    g_object_set (G_OBJECT(udpsink_a1),
                  "host", m_ip->toString().toAscii().data(),
                  "port", (m_firstport+3),
                  "sync", FALSE,
                  "async", FALSE, NULL);

    // source audio rtcp
    GstElement* udpsrc_a2 = gst_element_factory_make ("udpsrc", NULL);
    g_object_set (G_OBJECT(udpsrc_a2), "port", (m_firstport+5), NULL);

    // link audio rtp/rtcp
    gst_bin_add_many(GST_BIN (m_pipeline), udpsink_a0, udpsink_a1, udpsrc_a2, NULL);
    gst_element_link_pads(gstrtpbin, "send_rtp_src_1", udpsink_a0, "sink");
    gst_element_link_pads(gstrtpbin, "send_rtcp_src_1", udpsink_a1, "sink");
    gst_element_link_pads(udpsrc_a2, "src", gstrtpbin, "recv_rtcp_sink_1");

    // ###################### link it #############################

    if (m_path.empty()) {

        // video + videoscale
        gst_element_link(webcamsrc, m_videohandler);

        // audio + amrnbenc
        gst_element_link(audiosrc, m_audiohandler);

    }

    // ###################### pipeline ready ######################

	// pipeline ok?
	if (m_pipeline == NULL) {
        cout << "FugaStreamer: Error on setting up pipeline." << endl;
		return;
	}

	// add signalhandler for bus messages
	GstBus* bus = gst_element_get_bus (m_pipeline);
	gst_bus_add_watch (bus, (GstBusFunc) on_sink_message, NULL);
	gst_object_unref (bus);

	// play!
    cout << "FugaStreamer: Start streaming on "
         << m_ip->toString().toAscii().data() << " : "
         << m_firstport << endl;
	gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

// stop streaming
void FugaStreamer::stop()  {
    cout << "FugaStreamer: Stop streaming." << endl;
    if (m_pipeline == NULL) return;
cout << "FugaStreamer: stopped0" << endl;
	// stop pipeline
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    cout << "FugaStreamer: stopped1" << endl;
	// free
    gst_object_unref(m_pipeline);
    m_pipeline = NULL;
}

GstElement* FugaStreamer::getVideohandler () {
    return m_videohandler;
}

GstElement* FugaStreamer::getAudiohandler () {
    return m_audiohandler;
}

// get messages/errors from gstreamer
gboolean on_sink_message (GstBus* bus, GstMessage* message) {
    //cout << "FugaStreamer: Bus Message ("
    //          << GST_MESSAGE_TYPE_NAME(message)
    //          << ")..." << endl;

	switch (GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_EOS:
            cout << "FugaStreamer: No more frames..." << endl;
			break;
        case GST_MESSAGE_STREAM_STATUS:
            //cout << "FugaStreamer: Status changed." << endl;
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

// add pads dynamically
static void streamer_pad_added_cb (GstElement* bin, GstPad* new_pad, FugaStreamer* Streamer) {

    // get type of pad (audio or video)
    GstCaps* caps = gst_pad_get_caps(new_pad);
    g_assert(caps);
    GstStructure* str = gst_caps_get_structure(caps, 0);
    g_assert(str);
    const gchar* c = gst_structure_get_name(str);
    g_print("FugaStreamer: New decodebin-pad with caps-type: %s\n", c);

    // get pad to connect to
    GstElement* depay = (g_strrstr(c, "video")) ? Streamer->getVideohandler() : Streamer->getAudiohandler();
    GstPad* sinkpad = gst_element_get_static_pad(depay, "sink");
    g_assert (sinkpad);

    // link pads
    GstPadLinkReturn lres = gst_pad_link (new_pad, sinkpad);
    g_assert (lres == GST_PAD_LINK_OK);
    gst_object_unref (sinkpad);
}
