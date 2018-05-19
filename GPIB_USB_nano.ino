/***********************************************************************/
/*                                                                     */
/*  FILE        :GPIB_USB_nano.c                                       */
/*  DATE        :2013-10-28                                            */
/*  DESCRIPTION :Main Program                                          */
/*  CPU TYPE    :ATMega328-20AU                                        */
/*                                                                     */
/*  This is the main file for the GPIB-USB converter project  V1.0     */
/*  (C) 2010 by R. Schuster                                            */
/*  2011-06-25 RudolfReuter, added comand B, D, E, L, U                */
/*  2015-12-23 v1.2, compiler warning fixed, CRLF                      */
/*  2016-01-16 v1.3, ATN_D1 Out direction added to SET_TALK            */
/*  2016-08-20 v1.4, T1 settle time 10 -> 2 us, 115200 baud, timeout errors
/*  2018-05-18 v1.5  TheHWcave: added H command, greeting at startup   */
/*                   and "!" in front of all msgs to help distinguish  */
/*                   them from msgs coming via GPIB                    */ 
/***********************************************************************/

#include <avr/io.h>

#include "GPIB_nano.h"
#include "stdio.h"
#include "stdlib.h"

// interrupt functions
//#pragma interrupt UART1_receive_int;

//external functions
void Init_Gpib_interface(void);
unsigned int send_gpib_command(unsigned int addr, unsigned char *sendstring);
unsigned int send_gpib_query(unsigned int addr, unsigned char *sendstring);
unsigned int send_gpib_byte(unsigned int mode, unsigned int data);
// v. 1.1 2011-06-24 RR
unsigned int send_gpib_command_byte(unsigned int addr, unsigned char data);
unsigned int send_gpib_listen(unsigned int addr);
unsigned int send_gpib_unlisten(void);

// global Variables
char	ID_String[] = "!GPIB/USB converter V1.5\r\n";
char  CRLF[] = "\r\n";  // ver. 1.2
char	buff[20];
char	input_buffer[256]; // do NOT change size, because of pointers!
char	cmd_buffer[64];
char	cmd;
char	cmd_pointer, wr_pointer, rd_pointer, Timer_1ms_over;
char    trmEOI;            // terminate on EOI only
char    res_data;          // restore file data byte
unsigned int gpib_comand;

unsigned int	deviceaddr, weight, komma, n;

unsigned long gpib_timeout = 200000; // gpib- timeout default = 200ms

union {
  struct {
    unsigned char data;
    unsigned char flags;
  } byte;
  unsigned int word;
} U1input;


// function prototypes
//void com_print(char *daten);           // send string to UART1
//*********************************   send string to UART1
void com_print(char *daten) {
  while ( *daten) {
    Serial.write(*daten++);
  }
}

//auxiliary function: return position of char c in string buf (0 if not present)
unsigned int instr(char *buf, unsigned int c) {
  unsigned int n = 0;

  while (buf[n]) 	{
    if (buf[n++] == c) return n;
  }
  return 0;
}


//*************************************   UART1 functions*************************
void setup() {
  Serial.begin(115200);

  Init_Gpib_interface();	       // init gpib port
  com_print(ID_String);

}


void loop() {
  unsigned int error;

  while (Serial.available() > 0) {

    cmd = Serial.read();
    //sprintf(buff, "%c", cmd);    // 2011-06-16 RR for debug
    //com_print(buff);
    if (cmd == LINEFEED)           //LF= received String completed
    {
      cmd_buffer[cmd_pointer] = '\0';
      switch (cmd_buffer[0])
      {
        case 'B': //Send Command Byte to specified device (no answer) 2011-06-19 RR
          {
            komma = instr(&cmd_buffer[0], ',');
            deviceaddr = 0;

            if ((komma > 2) && (komma < 5))
            {
              weight = 1;
              for (n = komma - 2; n == 1; n--) // retrieve deviceaddr
              {
                deviceaddr = deviceaddr + ((cmd_buffer[n] - 0x30) * weight);
                weight = weight * 10;
              }
              gpib_comand = atoi(cmd_buffer + komma);
              error = send_gpib_command_byte(deviceaddr, gpib_comand);
              if (error)
              {
                sprintf(buff, "!sndError %d\r\n", error);
                com_print(buff);
              }
            }
            break;
          }
        case 'C':  //Reset  and Init GPIB Bus
          {
            Init_Gpib_interface();
            break;
          }
        case 'D':  //Send GPIB Data byte
          {
            while (Serial.available() == 0); //wait for one byte received on UART1
            res_data = Serial.read();
            error = send_gpib_byte(DATA, res_data);
            // in order to have a handshake to control speed
            Serial.write(res_data);
            if (error) {
              sprintf(buff, "!wrtError %d\r\n", error);
              com_print(buff);
            }
          }
        case 'E': //Send string to specified device and read back answer 2011-06-19 RR
          {
            komma = instr(&cmd_buffer[0], ',');
            deviceaddr = 0;
            trmEOI = 1; 			       // terminate on EOI only, no LineFeed

            if ((komma > 2) && (komma < 5))
            {
              weight = 1;
              for (n = komma - 2; n == 1; n--) // retrieve deviceaddr
              {
                deviceaddr = deviceaddr + ((cmd_buffer[n] - 0x30) * weight);
                weight = weight * 10;
              }
              error = send_gpib_query(deviceaddr, (unsigned char*)&cmd_buffer[komma]);
              if (error)
              {
                sprintf(buff, "!sndError %d\r\n", error);
                com_print(buff);
              }
            }
            break;
          }
        case 'G': //Send GPIB Command
          {
            gpib_comand = atoi(cmd_buffer + 1);
            error = send_gpib_byte(COMMAND, gpib_comand);
            if (error)
            {
              sprintf(buff, "!sndError %d\r\n", error);
              com_print(buff);
            }
            break;
          }
        case 'H': // just read whatever
          {
            komma = instr(&cmd_buffer[0], ',');
            deviceaddr = 0;

            if ((komma > 2) && (komma < 5))
            {
              weight = 1;
              for (n = komma - 2; n == 1; n--) // retrieve deviceaddr
              {
                deviceaddr = deviceaddr + ((cmd_buffer[n] - 0x30) * weight);
                weight = weight * 10;
              }
              error = do_gpib_read(deviceaddr);
              if (error)
              {
                sprintf(buff, "!rdError %d\r\n", error);
                com_print(buff);
              }
            }
            break;
          }
        case 'I':  //Send ID String
          {
            com_print(ID_String);
            break;
          }
        case 'L': //Address Listener (no answer) 2011-06-19 RR
          {
            komma = instr(&cmd_buffer[0], ',');
            deviceaddr = 0;

            if ((komma > 2) && (komma < 5))
            {
              weight = 1;
              for (n = komma - 2; n == 1; n--) // retrieve deviceaddr
              {
                deviceaddr = deviceaddr + ((cmd_buffer[n] - 0x30) * weight);
                weight = weight * 10;
              }
              error = send_gpib_listen(deviceaddr);
              if (error)
              {
                sprintf(buff, "!sndError %d\r\n", error);
                com_print(buff);
              }
            }
            break;
          }
        case 'R': //Send string to specified device and read back answer
          {
            komma = instr(&cmd_buffer[0], ',');
            deviceaddr = 0;

            if ((komma > 2) && (komma < 5))
            {
              weight = 1;
              for (n = komma - 2; n == 1; n--) // retrieve deviceaddr
              {
                deviceaddr = deviceaddr + ((cmd_buffer[n] - 0x30) * weight);
                weight = weight * 10;
              }
              error = send_gpib_query(deviceaddr, (unsigned char*)&cmd_buffer[komma]);
              if (error)
              {
                sprintf(buff, "!sndError %d\r\n", error);
                com_print(buff);
              }
            }
            break;
          }
        case 'S':  //Check SRQ line (if device needs to be serviced)
          {
            if (SRQ) error = 0; else error = 1;
            sprintf(buff, "!SRQ %d\r\n", error);
            com_print(buff);
            break;
          }
        case 'T':  //Set GPIB Timeout in ms
          {
            gpib_timeout = atol(cmd_buffer + 1);
            break;
          }
        case 'U':  //Output LF+EOI, UNL, UNT
          {
            send_gpib_unlisten();
            break;
          }
        case 'W': //Send string to specified device (no answer)
          {
            komma = instr(&cmd_buffer[0], ',');
            deviceaddr = 0;

            if ((komma > 2) && (komma < 5))
            {
              weight = 1;
              for (n = komma - 2; n == 1; n--) // retrieve deviceaddr
              {
                deviceaddr = deviceaddr + ((cmd_buffer[n] - 0x30) * weight);
                weight = weight * 10;
              }
              error = send_gpib_command(deviceaddr, (unsigned char*)&cmd_buffer[komma]);
              if (error)
              {
                sprintf(buff, "!sndError %d\r\n", error);
                com_print(buff);
              }
            }
            break;
          }
        default: break;
      }
      cmd_pointer = 0;
    }
    else
    {
      if (cmd != CARRIAGE_RETURN) cmd_buffer[cmd_pointer++] = cmd;
    }
  } // while ()
} // loop ()
