#!/usr/bin/python
import sys
import serial
import time

from cmdmessenger import CmdMessenger
from serial.tools import list_ports

DEV = '/dev/tty.wchusbserialfd12410'

CMD_ACK = 0
CMD_INFO = 1
CMD_ERROR = 2
CMD_SET_SPEED = 3

port = serial.Serial(DEV, 9600)
print('connected to', port.name)

msgr = CmdMessenger(port)

print('waiting for arduino...')
#msgr.send_cmd(CMD_ACK)
#msgr.wait_for_ack(CMD_ACK)
print("arduino ready")


def on_error(received_command, *args, **kwargs):
    print('Error: ', args[0][0])

def on_info(received_command, *args, **kwargs):
    print('Info: ', args[0][0])

def on_unknown_cmd(received_command, *args, **kwargs):
    print('unknown cmd')

msgr.attach(func=on_unknown_cmd)
msgr.attach(func=on_error, msgid=CMD_ERROR)
msgr.attach(func=on_info, msgid=CMD_INFO)

while True:
    msgr.feed_in_data()
