
/* GStreamer
 *
 * Copyright (C) 2015 Samsung Electronics. All rights reserved.
 *   Author: Thiago Santos <thiagoss@osg.samsung.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
/* demo for showing v4l2src renegotiating */

#include <gst/gst.h>

static GMainLoop *loop;
static GstElement *pipeline;
static GstElement *src, *capsfilter;

static const gchar *caps_array[] = {
  "video/x-raw, format=NV12, width=(int)3840, height=(int)2160, colorimetry=bt709",
  "video/x-raw, format=NV12, width=(int)1920, height=(int)1080, colorimetry=bt709",
};

static gint caps_index = 0;

static gboolean
bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
  switch (message->type) {
    case GST_MESSAGE_EOS:
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ERROR:{
      GError *gerror;
      gchar *debug;

      gst_message_parse_error (message, &gerror, &debug);
      gst_object_default_error (GST_MESSAGE_SRC (message), gerror, debug);
      g_error_free (gerror);
      g_free (debug);
      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}

static gboolean
change_caps (gpointer data)
{
  GstCaps *caps;

  if (caps_index >= G_N_ELEMENTS (caps_array)) {
    gst_element_send_event (pipeline, gst_event_new_eos ());
    return FALSE;
  }

  g_print ("Setting caps '%s'\n", caps_array[caps_index]);

  caps = gst_caps_from_string (caps_array[caps_index++]);
  g_object_set (capsfilter, "caps", caps, NULL);
  gst_caps_unref (caps);

  return TRUE;
}

gint
main (gint argc, gchar ** argv)
{
  GstBus *bus;

  /* init gstreamer */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  pipeline =
      gst_parse_launch
      ("videotestsrc name=src ! capsfilter name=cf ! omxh264enc ! fakesink",
      NULL);
  src = gst_bin_get_by_name (GST_BIN (pipeline), "src");
  capsfilter = gst_bin_get_by_name (GST_BIN (pipeline), "cf");
  change_caps (NULL);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_callback, NULL);
  gst_object_unref (bus);

  /* play and wait */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  g_timeout_add_seconds (3, change_caps, NULL);

  /* mainloop and wait for eos */
  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  /* stop and cleanup */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (src);
  gst_object_unref (capsfilter);
  gst_object_unref (GST_OBJECT (pipeline));
  return 0;
}
