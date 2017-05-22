#!/usr/bin/python
import sys
import serial
import time
import threading
import re
from enum import IntEnum, auto
from cmd_messenger import CmdMessenger

DEV = '/dev/tty.wchusbserialfd12410'

class Cmd(IntEnum):
    ACK = 0
    MSG = auto()
    ERROR = auto()
    PING = auto()
    SET_SPEED = auto()

port = serial.Serial(DEV, 115200)
print('connected to', port.name)
msgr = CmdMessenger(port)


def on_msg(args):
    print('MSG:', args[0])

def wait_for_device():
    while True:
        args = msgr.wait_for_cmd(Cmd.MSG)
        if (args[0] == '__ready_for_business__'):
            break

def ping_test():
    N = 400
    t0 = time.clock()
    for i in range(0,N):
        msgr.send_cmd(Cmd.PING)
        msgr.wait_for_cmd(Cmd.ACK)
    t1 = time.clock()
    print('ping_test duration = {0}s, avg_ping_time = {1}s, freq = {2}Hz'
            .format(t1-t0, (t1-t0)/N, N/(t1-t0)))

def main():
    try:
        msgr.attach(Cmd.MSG, on_msg)
        msgr.start()
        print('waiting for device')
        wait_for_device()
        print('arduino ready')

        ping_test()

    except KeyboardInterrupt:
        print('\nDONE.')
        #sys.exit(0)
    finally:
        msgr.stop()
        port.close()


if __name__ == '__main__':
    main()
