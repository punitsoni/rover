#!/usr/bin/env python

import os
import subprocess
import time
import sys

class Streamer:
  ''' Class for managing camera streaming over network using gstreamer
      and UDP '''
  def __init__(self, params=None):
    self.width = 640
    self.height = 480
    self.bitrate = 500000
    self.timeout = 0
    self.host_ip = "192.168.1.141"

    # Todo: add more paramters
    if params != None:
      if params["host_ip"] != None:
        self.host_ip = params["host_ip"]

    self.log_file = open("streamer.log", "w")

    self.raspivid_cmd = ["raspivid", "-o", "-", "-w", str(self.width), "-h",
                    str(self.height), "-b", str(self.bitrate), "-t",
                    str(self.timeout)];

    gst_cmd = "gst-launch-1.0 -vvv \
                fdsrc \
              ! h264parse \
              ! rtph264pay config-interval=10 pt=96 \
              ! udpsink host=%s port=9000" % (self.host_ip)
    self.gst_cmd = gst_cmd.split()

  def start(self):
    self.p_vid = subprocess.Popen(self.raspivid_cmd,
                  stdout=subprocess.PIPE, stderr=self.log_file)
    self.p_gst = subprocess.Popen(self.gst_cmd, stdout=self.log_file,
                  stdin=self.p_vid.stdout, stderr=self.log_file)

  def stop(self):
    self.p_gst.terminate()
    self.p_vid.terminate()
    self.p_gst.wait()
    self.p_vid.wait()
    self.log_file.close()

if __name__ == "__main__":
  if len(sys.argv) < 2:
    #host = "punits-mac"
    host = "192.168.1.122"
  else:
    host = str(sys.argv[1])

  params = {"host_ip":host}
  s = Streamer(params=params)
  s.start()
  print "Streaming started for host %s" % (host)
  raw_input("Press Enter to Stop.")
  s.stop()
  print "Finished."
