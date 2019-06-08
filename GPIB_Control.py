#MIT License
#
#Copyright (c) 2019 TheHWcave
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.
#
# Description:
# ============
# GPIB_Control connects to the GPIBtoUSB interface via a serial port. It supports the 
# follwing commandline parameters all of which are optional 
#  
# -p or --port : serial input device, default /dev/ttyUSB0
# -a or --addr : GPIB address, default 3
# -c or --cmd  : GPIB command default = none, which means just polling 
# -i or --ifcmd: sends a command to the GPIB interface itself (not the GPIB bus)
# -r or --read : if specified means read the response after sending the cmd 
# -d or --debug: followed by an integer (default = 0) for debuging purposes 
#-----------------------------

import serial,argparse
from time import sleep,time,localtime,strftime,perf_counter

parser = argparse.ArgumentParser()

parser.add_argument('--port','-p',help='port (default = /dev/ttyUSB0',
					dest='port_dev',action='store',type=str,default='/dev/ttyUSB0')
parser.add_argument('--addr','-a',help='GPIB address (default = 3)',metavar=' 1..30',
					dest='address',action='store',type=int,default=3,choices=range(1,31))
parser.add_argument('--cmd','-c',help='GPIB command (default = '')',
					dest='cmd_msg',action='store',type=str,default='')
parser.add_argument('--ifcmd','-i',help='GPIB interface command (default = '')',
					dest='ifcmd_msg',action='store',type=str,default='')
parser.add_argument('--read','-r',help='read from device ',
					dest='read_resp',action='store_true')
parser.add_argument('--debug','-d',help='debug level 0.. (def=1)',
					dest='debug',action='store',type=int,default=0)
arg = parser.parse_args()


do_read = arg.read_resp 


GPIB2USB = serial.Serial(
			port=arg.port_dev,
			baudrate=115200,
			timeout=1)
sleep(2)

def readdata():
	buf = ''
	n = 0
	while True:
		buf = GPIB2USB.readline(64).decode().strip()
		if len(buf) > 0:
			if buf.startswith('!'): 
				if arg.debug > 0: print('ignored:'+buf)
			else:
				break
		else:
			print('timeout')
	return (buf)


if arg.ifcmd_msg >'':
	pollmsg = arg.ifcmd_msg+'\x0a'
	do_read = True
else:
	if arg.cmd_msg >'': 
		if do_read:
			pollmsg   = 'R'+str(arg.address)+','+arg.cmd_msg+'\x0a'
		else:
			pollmsg   = 'W'+str(arg.address)+','+arg.cmd_msg+'\x0a'
	else:
		pollmsg   = 'H'+str(arg.address)+',\x0a'
		do_read = True

m = pollmsg.encode('ascii')

# change timeout of GPIB-to-USB interface to 1 s to wait in case we 
# poll and a value is not ready yet
GPIB2USB.write('T1000000\x0a'.encode('ascii'))

Done = False
while not Done:
	try:
		GPIB2USB.write(m)
		if do_read: 
			data = readdata()
			print(data)
		Done = True
	except KeyboardInterrupt:
		quit()

