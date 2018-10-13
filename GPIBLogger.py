#MIT License
#
#Copyright (c) 2018 TheHWcave
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
# GPIBlogger connects to the GPIBtoUSB interface via a serial port. It supports the 
# follwing commandline parameters all of which are optional because the defaults
# happen to match my setup (sorry). 
#  
# -i or --inp  : serial input device, default /dev/ttyUSB0
# -o or --out  : filename in default directory. default: LOG_<date_time>.csv
# -t or --time : time in seconds between polls, default: 1.0
# -d or --delta: if specified, logging occurs only if the value differs from previous by more than delta 
# -a or --addr : GPIB address, default 3
# -c or --cmd  : GPIB command string needed to poll data, default = none
#-----------------------------
import serial,argparse
from time import sleep,time,localtime,strftime,perf_counter

parser = argparse.ArgumentParser()
parser.add_argument('--time','-t',help='interval time in seconds between measurements (def=1.0)',
					dest='int_time',action='store',type=float,default=1.0)
parser.add_argument('--inp','-i',help='input device (default = /dev/ttyUSB0',
					dest='inp_dev',action='store',type=str,default='/dev/ttyUSB0')
parser.add_argument('--out','-o',help='output filename (default=LOG_<timestamp>.csv)',
					dest='out_name',action='store',type=str,default='!')
parser.add_argument('--delta','-d',help='record only when change greater than DELTA_VAL detected',
					dest='delta_val',action='store',type=float,default=0.0)
parser.add_argument('--addr','-a',help='GPIB address (default = 3)',metavar=' 1..30',
					dest='address',action='store',type=int,default=3,choices=range(1,31))
parser.add_argument('--cmd','-c',help='GPIB command needed (default = '')',
					dest='cmd_msg',action='store',type=str,default='')

arg = parser.parse_args()

if arg.out_name=='!':
	out_name = 'LOG_'+strftime('%Y%m%d%H%M%S',localtime())+'.csv'
else:
	out_name = arg.out_name

f = open(out_name,'w')
f.write('Seconds,Value,Unit\n')


GPIB2USB = serial.Serial(
			port=arg.inp_dev,
			baudrate=115200,
			timeout=0)

def readdata():
	buf = ""
	n = 0
	while True:
		c = GPIB2USB.read(1)
		if len(c) > 0:
			if c == b"\x0a":
				break
			else:
				buf = buf + c.decode()
	return (buf)

GPIB2USB.write("I\x0a".encode('iso-8859-1'))
print(readdata())
if arg.cmd_msg >'': 
	pollmsg   = 'R'+str(arg.address)+','+arg.cmd_msg+'\x0a'
else:
	pollmsg   = 'H'+str(arg.address)+',\x0a'
	

start = perf_counter()
old = 9.9999E99

# change timeout of GPIB-to-USB interface to 1 s to wait in case we 
# poll and a value is not ready yet
GPIB2USB.write('T1000000\x0a'.encode('iso-8859-1'))


while True:
	try:
		GPIB2USB.write(pollmsg.encode('iso-8859-1'))
		data = readdata()
		now = perf_counter() - start
		nowstr='{:05.1f}'.format(now)
		if data.startswith('!'):
			print(data)
		else:
			val = data.split('  ')
			try:
				cur = float(val[0])
			except ValueError:
				cur=old
				print('Valueerror: '+val[0])
				if val[0].startswith('+') or val[0].startswith('-'):
					# see if we can extract a value
					valtmp= val[0].spit(' ')
					try:
						cur = float(valtmp[0])
					except:
						print(' no value')
					
			if arg.delta_val > 0.0:
				if abs(old - cur) >= arg.delta_val:
					f.write(nowstr+ ','+val[0]+','+val[1])
					print(nowstr+','+val[0]+','+val[1])
					old = cur
			else:
				f.write(nowstr+ ','+val[0]+','+val[1])
				print(nowstr+','+val[0]+','+val[1])
		sleep(arg.int_time)
		#now = now + arg.int_time
	except KeyboardInterrupt:
		quit()
GPIB2USB.disconnect() # Disconnects the device
f.close()
