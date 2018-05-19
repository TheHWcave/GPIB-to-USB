//This file includes all GPIB/USB project- specific declarations

// general constants

#define LINEFEED  10
#define CARRIAGE_RETURN 13

//GPIB commands
#define TALK 0x40
#define LISTEN 0x20
#define UNTALK 0x5F
#define UNLISTEN 0x3F
#define DEVICE_CLEAR 0x14

#define DEVICE_NOT_PRESENT_ERROR 1
#define GPIB_TIMEOUT_ERROR_TX 2
#define GPIB_TIMEOUT_ERROR_RX1 3
#define GPIB_TIMEOUT_ERROR_RX2 4 

#define CONTROLLERADDR 0

#define COMMAND 1
#define DATA 2

//GPIB - Port assignment, PC6 and PC7 are not digital, use PB2 and PB3

/** Define all I/O Bits to make it better portable */
/** Handshake lines: DAV, NRFD and NDAC */
#define DAV (PIND & (1<<7))
#define DAV_1 PORTD |= 1<<7;
#define DAV_0 PORTD &= ~(1<<7)
#define DAV_D1 DDRD |= 1<<7
#define DAV_D0 DDRD &= ~(1<<7)
#define NRFD (PIND & (1<<6))
#define NRFD_1 PORTD |= 1<<6;
#define NRFD_0 PORTD &= ~(1<<6)
#define NRFD_D1 DDRD |= 1<<6
#define NRFD_D0 DDRD &= ~(1<<6)
#define NDAC (PIND & (1<<5))
#define NDAC_1 PORTD |= 1<<5;
#define NDAC_0 PORTD &= ~(1<<5)
#define NDAC_D1 DDRD |= 1<<5
#define NDAC_D0 DDRD &= ~(1<<5)

/** management lines: ATN, SRQ, IFC, EOI and REN  */
#define ATN (PIND & (1<<2))
#define ATN_1 PORTD |= 1<<2;
#define ATN_0 PORTD &= ~(1<<2)
#define ATN_D1 DDRD |= 1<<2
#define ATN_D0 DDRD &= ~(1<<2)
#define SRQ (PIND & (1<<3))
#define SRQ_1 PORTD |= 1<<3;
#define SRQ_0 PORTD &= ~(1<<3)
#define SRQ_D1 DDRD |= 1<<3
#define SRQ_D0 DDRD &= ~(1<<3)
#define IFC (PIND & (1<<4))
#define IFC_1 PORTD |= 1<<4;
#define IFC_0 PORTD &= ~(1<<4)
#define IFC_D1 DDRD |= 1<<4
#define IFC_D0 DDRD &= ~(1<<4)
#define EOI (PINB & (1<<0))
#define EOI_1 PORTB |= 1<<0;
#define EOI_0 PORTB &= ~(1<<0)
#define EOI_D1 DDRB |= 1<<0
#define EOI_D0 DDRB &= ~(1<<0)
#define REN (PINB & (1<<1))
#define REN_1 PORTB |= 1<<1;
#define REN_0 PORTB &= ~(1<<1)
#define REN_D1 DDRB |= 1<<1
#define REN_D0 DDRB &= ~(1<<1)

#define SET_TALK_DIR IFC_D1; REN_D1; ATN_D1; DAV_D1; EOI_D1; NRFD_D0; NDAC_D0;
/* IFC,REN,ATN,DAV,EOI= Output, NRFD,NDAC= Input */

#define SET_LISTEN_DIR IFC_D1; REN_D1; ATN_D1; NRFD_D1; NDAC_D1; DAV_D0; EOI_D0;
/* IFC,REN,ATN,NRFD,NDAC= Output, DAV,EOI= Input */

#define DATA_OUT DDRC = 0x3f; DDRB |= 0x0c
#define DATA_IN DDRC = 0; DDRB &= ~0x0c

#define PORTIB PORTC // GPIB Data output port PC0..5
#define PINIB PINC   // GPIB Data Input Port  PC0..5
// Bit 7
#define D8_1 PORTB |= 1<<3
#define D8_0 PORTB &= ~(1<<3)
#define D8 (PINB & (1<<3))
// Bit 6
#define D7_1 PORTB |= 1<<2
#define D7_0 PORTB &= ~(1<<2)
#define D7 (PINB & (1<<2))

