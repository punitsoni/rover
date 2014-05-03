###Video streaming from Rapberry Pi

####Server
```bash
raspivid -n -w 320 -h 240 -b 250000 -fps 20 -t 0 -o - | \
  gst-launch-1.0 -v fdsrc ! h264parse ! rtph264pay config-interval=10 pt=96 ! \
  udpsink host=192.168.1.141 port=9000
```
####Clinet
```bash
gst-launch-1.0 -v udpsrc port=9000 caps='application/x-rtp, \
  media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' ! \
  rtph264depay ! avdec_h264 ! videoconvert ! autovideosink sync=false
```
