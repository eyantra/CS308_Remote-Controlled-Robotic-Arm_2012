/********************************************************************************
 Platform: SPARK V
 Experiment: 10B_Serial_Comminucation_ZigBee_wireless
 Written by: Vinod Desai, NEX Robotics Pvt. Ltd.
 Edited By: Sachitanand Malewar, NEX Robotics Pvt. Ltd.
 Last Modification: 22nd September 2010
 AVR Studio Version 4.17, Build 666

 Concepts covered: serial communication using zigbee wireless module

 This experiment demonstrates Robot control over serial port via ZigBee wireless 
 comunication module 

 There are two components to the motion control:
 1. Direction control using pins PORTB0 to 	PORTB3
 2. Velocity control by PWM on pins PD4 and PD5.

 In this experiment for the simplicity PD4 and PD5 are kept at logic 1.
 
 Pins for PWM are kept at logic 1.
  
 Connection Details:   L-1---->PB0;		L-2---->PB1;
   					   R-1---->PB2;		R-2---->PB3;
   					   PD4 (OC1B) ----> Logic 1; 	PD5 (OC1A) ----> Logic 1; 

 Serial Communication: PORTD 0 --> RXD UART receive for RS232 serial communication
					   PORTD 1 --> TXD UART transmit for RS232 serial communication

 Make sure that J5 us set towards the back side of the robot 
 i.e. XBee wireless module is connected to the serial port of the microcontroller.
 For more details on the jumper settings refer to the hardware manual.

 Use baud rate as 9600bps.
 
 To control robot use number pad of the keyboard which is located on the right hand
 side of the keyboard. Make sure that NUM lock is on.

 For more detail on hardware connection and theory refer the hardware manual.

 Note: 
 
 1. Make sure that in the configuration options following settings are 
 	done for proper operation of the code

 	Microcontroller: atmega16
 	Frequency: 7372800
 	Optimization: -O0 (For more information read section: Selecting proper optimization
	              options below figure 4.22 in the hardware manual)
 *********************************************************************************/

 /********************************************************************************

   Copyright (c) 2010, NEX Robotics Pvt. Ltd.                       -*- c -*-
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   * Source code can be used for academic purpose. 
	 For commercial use permission form the author needs to be taken.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. 

  Software released under Creative Commence cc by-nc-sa licence.
  For legal information refer to: 
  http://creativecommons.org/licenses/by-nc-sa/3.0/legalcode


********************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void motion_pin_config (void)
{
 DDRA = DDRA & 0xFE; 
 DDRB = DDRB | 0x0F;   //set direction of the PORTB3 to PORTB0 pins as output
 PORTB = PORTB & 0xF0; // set initial value of the PORTB3 to PORTB0 pins to logic 0
 DDRD = DDRD | 0x30;   //Setting PD4 and PD5 pins as output for PWM generation
 PORTD = PORTD | 0x30; //PD4 and PD5 pins are for velocity control using PWM
}

void buzzer_pin_config (void)
{
 DDRC = DDRC | 0x08;		//Setting PORTC 3 as outpt
 PORTC = PORTC & 0xF7;		//Setting PORTC 3 logic low to turnoff buzzer
}

//Function to initialize ports
void port_init()
{
 motion_pin_config();
 buzzer_pin_config();
}

//UART0 initialisation
// desired baud rate: 9600
// actual: baud rate:9600 (0.0%)
// char size: 8 bit
// parity: Disabled
void uart0_init(void)
{
 UCSRB = 0x00; //disable while setting baud rate
 UCSRA = 0x00;
 UCSRC = 0x86;
 UBRRL = 0x2F; //set baud rate lo  //67 is for 16MHz 9600 baudrate
 UBRRH = 0x00; //set baud rate hi
 UCSRB = 0x98; 
}

//Function used for setting motor's direction
void motion_set (unsigned char Direction)
{
 unsigned char PortBRestore = 0;

 Direction &= 0x0F; 			// removing upper nibbel as it is not needed
 PortBRestore = PORTB; 			// reading the PORTB's original status
 PortBRestore &= 0xF0; 			// setting lower direction nibbel to 0
 PortBRestore |= Direction; 	// adding lower nibbel for direction command and restoring the PORTB status
 PORTB = PortBRestore; 			// setting the command to the port
}

void init_devices (void)
{
 cli();         //Clears the global interrupts
 port_init();
 uart0_init();
 sei();         //Enables the global interrupts
}
unsigned char ADC_Conversion(unsigned char Ch)
{
 unsigned char a;
 Ch = Ch & 0x07;  			
 ADMUX= 0x20| Ch;	   		
 ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
 while((ADCSRA&0x10)==0);	    //Wait for ADC conversion to complete
 a=ADCH;
 ADCSRA = ADCSRA|0x10;          //clear ADIF (ADC Interrupt Flag) by writing 1 to it
 return a;
}

//OUR PROJECT SPECIFIC CODE TO READ POTENTIOMETER VALUES
SIGNAL(SIG_UART_RECV) 
{
 unsigned char receive_data=0;   // used to save Receiving data
//when a data is received we will send the 6 ADC values with a delay of 10ms between each of them.
 receive_data = UDR;
		  unsigned char s[10];
		  sprintf(s,"%d",ADC_Conversion(1));
		  _delay_ms(10);
		  UDR=ADC_Conversion(0);
		  _delay_ms(10);
		  UDR=ADC_Conversion(1);
		  _delay_ms(10);
		  UDR=ADC_Conversion(2);
		  _delay_ms(10);
		  UDR=ADC_Conversion(3);
		  _delay_ms(10);
		  UDR=ADC_Conversion(4);
		  _delay_ms(10);
		  UDR=ADC_Conversion(5);
		  _delay_ms(10);

		 // UDR=s[1];
		 // _delay_ms(10);
		 // UDR=s[2];
		 // _delay_ms(10);
		 // UDR=s[3];
		 // _delay_ms(10);
		   
}


void adc_pin_config (void) 
{
DDRA = 0x00; //set PORTA direction as input 
 PORTA = 0x00; //set PORTA pins floating 
} 

void adc_init() 
{ 
           ADCSRA = 0x00; 
           ADMUX = 0x20;              //Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000 
           ACSR = 0x80; 
           ADCSRA = 0x86;             //ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0 
} 

void init_devices1 (void) 
{ 
 cli(); //Clears the global interrupts 
 port_init(); 
 adc_init(); 
 uart0_init();
 sei(); //Enables the global interrupts 
} 

//Main Function
int main()
{
	init_devices1();
	while(1);
}

