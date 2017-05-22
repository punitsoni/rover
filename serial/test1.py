#!/usr/bin/python
import sys
import serial
import time
import threading
import re;

from cmdmessenger import CmdMessenger
from serial.tools import list_ports

DEV = '/dev/tty.wchusbserialfd12410'

CMD_ACK = 0
CMD_INFO = 1
CMD_ERROR = 2
CMD_SET_SPEED = 3

port = serial.Serial(DEV, 9600)
print('connected to', port.name)

#print('waiting for arduino...')
#msgr.send_cmd(CMD_ACK)
#msgr.wait_for_ack(CMD_ACK)
#print("arduino ready"

def on_command(cmd, args):
    print('cmd={0} args={1}'.format(cmd, args))
    request_ack()

def on_data(data):
    print('rx> %s' % (data), end='')
    m = re.match(r'([0-9]*),?(.*);', data, 0);
    if m is not None:
        cmdstr = m.groups()[0]
        argstr = m.groups()[1]
        args = argstr.split(',')
        on_command(int(cmdstr), args)

def send_cmd(cmd, args = []):
    cmdstr = str(cmd);
    for a in args:
        cmdstr += ','+ str(a)
    cmdstr += ';'
    print('tx>', cmdstr)
    port.write(cmdstr.encode('utf-8'))

def request_ack():
    send_cmd(CMD_ACK)

linebuffer = ''

def read_serial():
    while (port.inWaiting() > 0):
        c = port.read().decode('ascii')

        #print(c)
        global linebuffer
        # ignore CR
        if c != '\r':
            linebuffer += c

        if c == '\n':
            on_data(linebuffer)
            linebuffer = ''

while True:
    read_serial()
