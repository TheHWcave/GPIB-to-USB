#include <avr/io.h> 
#include "GPIB_nano.h"

//external variables
extern unsigned long gpib_timeout;
extern char    trmEOI;      // terminate on EOI only, 2011-06-19 RR

//external functions
void sendTxd1(char data);              
void com_print(char *daten);           

//auxiliary functions
//delay microseconds
static void delayus (unsigned long micros)
{
	unsigned long i;
	for (i = 0; i < micros; i++)
	{
		asm("nop");
	}
}

//Set port pins, establish controller in charge, reset connected devices
void Init_Gpib_interface(void) {
        pinMode(A6, OUTPUT);      // sets the analog pin as output
        pinMode(A7, OUTPUT);      // sets the analog pin as output
	DATA_OUT;
	SET_TALK_DIR;
	EOI_1;
	DAV_1;
	ATN_1;
	IFC_0;
	REN_0;
	delayus(10000);
	IFC_1;
}

//Send one byte to gpib bus: MODE= COMMAND:  With ATN asserted, MODE= DATA: release ATN
unsigned int send_gpib_byte(unsigned int mode, unsigned int data)
{
	unsigned long count;

	DATA_OUT;
	SET_TALK_DIR;
	if (mode == COMMAND)
	{
		 ATN_0;
	}
	count = gpib_timeout;
	while( !NRFD && count)
			{count--;
			 delayus(1);}
	if( count == 0) return DEVICE_NOT_PRESENT_ERROR;

	PORTIB = ~data;
        if (data & 0x80) D8_0; else D8_1;
        if (data & 0x40) D7_0; else D7_1;
	delayus(2);  // T1 data settle time
	DAV_0;
	count = gpib_timeout;
	while( !NDAC && count)
			{count--;
			 delayus(1);}
	if( count == 0) return GPIB_TIMEOUT_ERROR_TX;
	DAV_1;
	ATN_1;
	return 0;
}

//send a string to selected device, assert EOI with last byte
unsigned int send_gpib_string(unsigned char *s)
{
	
	while (*s)
	{
		send_gpib_byte(DATA,*s++);
	}
	EOI_0;
	send_gpib_byte(DATA, LINEFEED);
	EOI_1;
	return 0;
}

//read a string from selected device until LF received and/or EOI activated
unsigned int read_gpib_string(void)
{
	unsigned long count;
	unsigned int ch;
	
	DATA_IN;		
	SET_LISTEN_DIR;
	
	do
	{
		NDAC_0;
		NRFD_1;
		count = gpib_timeout;
		while ( DAV && count)
			{count--;
			 delayus(1);}
		if( count == 0 ) return GPIB_TIMEOUT_ERROR_RX1;
		NRFD_0;
		ch = (~PINIB) & 0x3f;
                if (D8 == 0) ch |= 0x80;
                if (D7 == 0) ch |= 0x40;
		Serial.write(ch);
		count = gpib_timeout;
		NDAC_1;
		while ( !DAV && count)
			{count--;
			 delayus(1);}
		if( count == 0 ) return GPIB_TIMEOUT_ERROR_RX2;
		// In case of command "E" terminate on EOI only, 2011-06-19 RR
	} while ( ( (ch != LINEFEED) || trmEOI) && EOI);
	NDAC-0;
	NRFD_0;
	trmEOI = 0;
	if (ch != LINEFEED)
		com_print(CRLF);
	return 0;
}

//address a device as listener, send a query string, then address this device as talker and read answer
unsigned int send_gpib_query(unsigned int addr,unsigned char *sendstring)
{
	unsigned int error;

	error = send_gpib_byte(COMMAND,UNLISTEN);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,UNTALK);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,LISTEN | addr);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_string(&sendstring[0]);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,UNLISTEN);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,UNTALK);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,TALK | addr);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = read_gpib_string();
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	// append UNL and UNT 2011-06-19 RR
	error = send_gpib_byte(COMMAND,UNLISTEN);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,UNTALK);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	return 0;
}

//address a device as talker and read whatever it has
unsigned int do_gpib_read(unsigned int addr)
{
  unsigned int error;

  error = send_gpib_byte(COMMAND,TALK | addr);
  if (error) 
  {
    Init_Gpib_interface();
    return error;
  }
  error = read_gpib_string();
  if (error) 
  {
    Init_Gpib_interface();
    return error;
  }
  // append UNL and UNT 2011-06-19 RR
  error = send_gpib_byte(COMMAND,UNLISTEN);
  if (error) 
  {
    Init_Gpib_interface();
    return error;
  }
  error = send_gpib_byte(COMMAND,UNTALK);
  if (error) 
  {
    Init_Gpib_interface();
    return error;
  }
  return 0;
}


//address a device as listener, then send a string without waiting for answer
unsigned int send_gpib_command(unsigned int addr,unsigned char *sendstring)
{
	unsigned int error;

	error = send_gpib_byte(COMMAND,UNLISTEN);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,UNTALK);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,LISTEN | addr);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_string(&sendstring[0]);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	return 0;
}

//address a device as listener, then send a byte without waiting for answer
// 2011-06-19 RR
unsigned int send_gpib_command_byte(unsigned int addr,unsigned char data)
{
	unsigned int error;

	error = send_gpib_byte(COMMAND,UNLISTEN);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,UNTALK);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,LISTEN | addr);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND, data);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	return 0;
}

//address a device as listener
// 2011-06-19 RR
unsigned int send_gpib_listen(unsigned int addr)
{
	unsigned int error;

	error = send_gpib_byte(COMMAND,UNLISTEN);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,UNTALK);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,LISTEN | addr);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	return 0;
}

//send LF+EOI, UNL, UNT
// 2011-06-19 RR
unsigned int send_gpib_unlisten(void)
{
	unsigned int error;
	EOI_0;
	send_gpib_byte(DATA, LINEFEED);
	EOI_1;
	error = send_gpib_byte(COMMAND,UNLISTEN);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	error = send_gpib_byte(COMMAND,UNTALK);
	if (error) 
	{
		Init_Gpib_interface();
		return error;
	}
	return 0;
}
