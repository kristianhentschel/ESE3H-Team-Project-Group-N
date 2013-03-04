#include <zb_transport.h>

/*
 * zb_transport_embedded.h
 *
 * Implementation of the serial receive buffer and send methods using interrupts
 * and the STM32F4's USART peripherals.
 *
 * Author: Kristian Hentschel
 * Team Project 3. University of Glasgow. 2013.
 */

#define ZB_USART USART_4

/* initialise USART Peripheral and set up GPIO pins for its use. Enable interrupts. */
void zb_transport_init();

/* disable interrupts for the usart peripheral */
void zb_transport_stop();

/* blocking write, sending character by character */
void zb_send(char *buf, unsigned char len);

/* disable interrupts, take character from buffer, re-enable interrupts */
char zb_getc();

/* delay for one second through SysTick or busy loop. */
void zb_guard_delay();

/* Interrupt handler for USART.
 * 	- Check if interrupt source is RX buffer full
 * 	- read from RX buffer
 * 	- store in software RX buffer
 * 	- clear interrupt flag
 */
void USART1_IRQHandler(void);
