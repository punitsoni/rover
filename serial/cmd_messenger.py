import threading
import re

class CmdMessenger:
    '''
    Interface to the Arduino CmdMessenger library. This uses serial port and
    provides threaded API for sending and receiving simple text commands to
    and from arduino
    '''
    def __init__(self, port):
        self._port = port
        self._callbacks = {}
        self._running = False
        self._waitcmd_id = None
        self._waitcmd_args = None
        self._evt_waitcmd = threading.Event()
        self._thread = threading.Thread(target=self.run, args=())
        #self._thread.setDaemon(True)

    def attach(self, cmd_id, func):
        self._callbacks[cmd_id] = func

    def detach(self, cmd_id):
        del self._callbacks[cmd_id]

    def start(self):
        self._port.flushInput()
        self._running = True
        self._thread.start()

    def stop(self):
        self._running = False
        self._thread.join()

    def wait_for_cmd(self, cmd_id, timeout=None):
        self._waitcmd_id = int(cmd_id)
        self._evt_waitcmd.clear()

        if self._evt_waitcmd.wait(timeout) == False:
            return None

        return self._waitcmd_args

    def on_command(self, cmd, args):
        #print('cmd={0} args={1}'.format(cmd, args))
        if cmd in self._callbacks:
            self._callbacks[cmd](args)
        # signal thread waiting for this command
        if self._waitcmd_id == cmd:
            self._waitcmd_id = None
            self._waitcmd_args = args
            self._evt_waitcmd.set()

    def send_cmd(self, cmd, args=[]):
        cmdstr = str(int(cmd));
        for a in args:
            cmdstr += ','+ str(a)
        cmdstr += ';'
        print('tx>', cmdstr)
        self._port.write(cmdstr.encode('ascii'))

    def run(self):
        while self._running == True:
            while self._port.inWaiting() > 0:
                line = self._port.readline().decode('ascii')
                print('rx> %s' % (line), end='')
                m = re.match(r'([0-9]*),?(.*);', line, 0);
                if m is not None:
                    cmdstr = m.groups()[0]
                    argstr = m.groups()[1]
                    args = argstr.split(',')
                    self.on_command(int(cmdstr), args)
