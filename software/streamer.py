#!/usr/bin/env python

import os
import subprocess
import time
import sys

width = 640
height = 480
bitrate = 250000
timeout = 0

log_file = open("streamer.log", "w")

raspivid_cmd = ["raspivid", "-o", "-", "-w", str(width), "-h", str(height),
                "-b", str(bitrate), "-t", str(timeout)];

gst_cmd = "gst-launch-1.0 -v fdsrc ! h264parse ! \
            rtph264pay config-interval=10 pt=96 ! \
            udpsink host=192.168.1.141 port=9000".split()

p_vid = subprocess.Popen(raspivid_cmd,
              stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=False)

p_gst = subprocess.Popen(gst_cmd, stdout=subprocess.PIPE,
         stdin=p_vid.stdout, shell=False)

print "Streaming started."
raw_input("Press Enter to Stop.")

p_gst.terminate()
p_vid.terminate()

print "Finished."