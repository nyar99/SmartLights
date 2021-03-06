// UARTTestMain.c
// Runs on LM4F120/TM4C123
// Used to test the UART.c driver
// Daniel Valvano
// Jan 3, 2020

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2020

 Copyright 2020 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1
#include <stdint.h>
#include <stdbool.h>
#include "../../inc/PLL.h"
#include "esp8266V.h"
//#include "esp8266.h"
#include "UART1.h"
#include "UARTV.h"
#include "../../inc/tm4c123gh6pm.h"


//---------------------OutCRLF---------------------
// Output a CR,LF to UART to go to a new line
// Input: none
// Output: none
void OutCRLF(void){
  UART_OutChar(CR);
  UART_OutChar(LF);
}


// -----------UART1_Handler-----------
// called on one receiver data input followed by timeout
// or     on going from 1 to 2 data input characters
void UART1_Handler(void){
	if(UART1_RIS_R&UART_RIS_TXRIS){       // hardware TX FIFO <= 2 items
    UART1_ICR_R = UART_ICR_TXIC; 
	}
  if(UART1_RIS_R & UART_RIS_RXRIS){   // rx fifo >= 1/8 full
    UART1_ICR_R = UART_ICR_RXIC;      // acknowledge interrupt
		UART_OutChar(UART1_DR_R);
  }
  if(UART1_RIS_R & UART_RIS_RTRIS){   // receiver timed out
    UART1_ICR_R = UART_ICR_RTIC;      // acknowledge receiver time
  }
}

//arduino tm4c uart debug
/*int main(void){
	PLL_Init(Bus80MHz);
	UART_Init();
	UART1_Init();
	UART_OutString("Start\r\n");
	while(1){
		UART1_OutChar('Z');
		UART_OutChar(UART1_InChar());
	}
}*/

/*int main(void){
	PLL_Init(Bus80MHz);
	UART_Init();
	ESP8266_Init();
	//ESP8266_Reset();
	//DelayMs(5000);
	UART_OutString("Start\r\n");
	ESP8266_OutString("AT+RST\r\n");
	DelayMs(5000);
	ESP8266_OutString("AT\r\n");
	DelayMs(1000);
	//ESP8266_OutString("AT+RST\r\n");
	//DelayMs(1000);
	ESP8266_OutString("AT+CWMODE=1\r\n");
	DelayMs(1000);
	ESP8266_OutString("AT+CWJAP=\"SM-G930PF1C\",\"5128048607\"\r\n");
	//DelayMs(10000);
	ESP8266_OutString("AT+CIPSTART=\"TCP\",\"192.168.43.83\",8080\r\n");
	ESP8266_OutString("AT+CIPSTATUS\r\n");
	DelayMs(1000);
	//ESP8266_OutString("AT+PING=\"192.168.43.33\"\r\n");
	//DelayMs(5000);
	ESP8266_OutString("AT+CWLIF\r\n");
	DelayMs(5000);
	ESP8266_OutString("AT+CIPMUX=0\r\n");
	DelayMs(5000);
	ESP8266_OutString("AT+CIPSTATUS\r\n");
	DelayMs(1000);
	while(1){
		ESP8266_OutString("AT+CIPSTATUS\r\n");
		DelayMs(1000);
		
		//UART_OutString(".");	
		//UART_OutChar(UART5_InCharNonBlocking());
	}
}*/

void Delay1s(uint32_t n){uint32_t volatile time;
  while(n){
		UART_OutString(".");
    time = 72724000*2/91;  // 1msec, tuned at 80 MHz
    while(time){
      time--;
    }
    n--;
  }
}

int main(void){
	PLL_Init(Bus80MHz);
	Output_Init();
	ESP8266_BasicInit(115200);
	ESP8266SendCommand("AT\r\n");
	if(ESP8266_Reset()==0){ 
    UART_OutString("Reset failure, could not reset\n\r"); while(1){};
  }
	if(ESP8266_SetWifiMode(1)==0){ 
    UART_OutString("SetWifiMode, could not set mode\n\r"); while(1){};
  }
	if(ESP8266_JoinAccessPoint("AvalonLexus","VijayawadaVizag")==0){ 
    UART_OutString("JoinAccessPoint error, could not join AP\n\r"); while(1){};
  } else{
		UART_OutString("Succesfully connected to AP\r\n");
		DelayMs(8000);
		ESP8266SendCommand("AT+CIFSR\r\n");
		DelayMs(1000);
		if(ESP8266_SetConnectionMux(0)==0){ // single socket
			UART_OutString("SetConnectionMux error, could not set connection mux\n\r"); while(1){};
		} 
		if(ESP8266_SetDataTransmissionMode(0)==0){ 
    UART_OutString("SetDataTransmissionMode, could not make connection\n\r"); while(1){};
		}
		if(ESP8266_GetStatus()==0){
			UART_OutString("Connection status failed\n\r");
		}
		DelayMs(1000);
		ESP8266SendCommand("AT+CIPSTART=\"TCP\",\"http:\\\\192.168.1.133\",8080\r\n");
		//UART_OutString("ESP8266 IP address: ");
		/*if(ESP8266_MakeTCPConnection("127.0.0.1")==0){ 
			UART_OutString("Could not make TCP connection\n\r"); while(1){};
		} else{
			char* getRequest = "GET /song_info HTTP/1.1\nHost: 127.0.0.1:8080\nConnection: close\n\r\n";
			if(ESP8266_SendTCP(getRequest)==0){ 
				UART_OutString("Send tcp failed\n\r"); while(1){};
			}
		}*/
		//ESP8266SendCommand("AT+CIPSTART=\"TCP\",\"192.168.43.83\",8080\r\n");
		//DelayMs(5000);
		//ESP8266SendCommand("AT+CIPSEND=80\r\n");

	}

	while(1){}
}

