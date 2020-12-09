// UART.c
// Runs on LM4F120, TM4C123
// Simple device driver for the UART.
// Daniel Valvano
// Jan 3, 2020

/* This example accompanies the books
  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2020

"Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
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
#include <stdio.h>
#include "UART1.h"
#include "../inc/tm4c123gh6pm.h"


#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable

void UART1_Init(void){
 volatile int delay;
  SYSCTL_RCGCUART_R |= 0x02; // Enable UART1
  while((SYSCTL_PRUART_R&0x02)==0){};
  SYSCTL_RCGCGPIO_R |= 0x02; // Enable PORT B clock gating
  while((SYSCTL_PRGPIO_R&0x02)==0){}; 
  GPIO_PORTB_AFSEL_R |= 0x03;
  GPIO_PORTB_DIR_R |= 0x20; // PB5 output to reset
  GPIO_PORTB_PCTL_R =(GPIO_PORTB_PCTL_R&0xFF0FFF00)|0x00000011;
  GPIO_PORTB_DEN_R   |= 0x23; //23 
  GPIO_PORTB_DATA_R |= 0x20; // reset high
  UART1_CTL_R &= ~UART_CTL_UARTEN;                  // Clear UART1 enable bit during config 
  UART1_IBRD_R = 43;       // IBRD = int(80,000,000 / (16 * 115,200)) = int(43.403)
  UART1_FBRD_R = 26;       // FBRD = round(0.4028 * 64 ) = 26

  UART1_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);  // 8 bit word length, 1 stop, no parity, FIFOs enabled
  UART1_IFLS_R &= ~0x3F;                            // Clear TX and RX interrupt FIFO level fields
  UART1_IFLS_R += UART_IFLS_RX1_8 ;                 // RX FIFO interrupt threshold >= 1/8th full
  UART1_IM_R  |= UART_IM_RXIM | UART_IM_RTIM;       // Enable interupt on RX and RX transmission end
  UART1_CTL_R |= UART_CTL_UARTEN;                   // Set UART1 enable bit    
}

//------------UART1_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART1_OutString(char *pt){
  while(*pt){
    UART1_OutChar(*pt);
    pt++;
  }
}

void UART1_OutChar(char data){
  while((UART1_FR_R&UART_FR_TXFF) != 0);
  UART1_DR_R = data;
}



void UART1_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    UART_OutUDec(n/10);
    n = n%10;
  }
  UART_OutChar(n+'0'); /* n is between 0 and 9 */
}
