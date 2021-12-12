////////////////////////////////////////////////////////////////////////////////
//
//
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
//#include "hardware/i2c.h"
//#include "hardware/pio.h"
//#include "hardware/clocks.h"
#include "hardware/uart.h"

#include "psion_recreate.h"



////////////////////////////////////////////////////////////////////////////////
#define UART_ID uart1 //uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 8 //0
#define UART_RX_PIN 9 //1


void wireless_init(void)
{
  
  sleep_us(1000000);  
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  uart_init(UART_ID, BAUD_RATE);
  
  uart_set_hw_flow(UART_ID, false, false);
  //uart_set_fifo_enabled(UART_ID, false);
  
  //  gpio_put(7, 1);
  //gpio_set_dir(7, GPIO_OUT);
  
  printf("\nOrganiser 2 Main Board\n");
  stdio_flush();

  // turn power off for a while
  latch2_clear_mask(LAT2PIN_MASK_ESP_ON);
  sleep_ms(100);
  latch2_set_mask(LAT2PIN_MASK_ESP_ON);

}

volatile int setup = 0;

char *reply1 = "HTTP/1.1 200 OK\n\
Content-Type: text/html\n\
Connection: close\n\n\
<!DOCTYPE HTML>\n\
<html>\n\
Psion Organiser Recreation\
<br>Ticks:%d<br>\
<pre><br><br><br>\
<tt>%s</tt><br>\
<tt>%s</tt><br>\
<tt>%s</tt><br>\
<tt>%s</tt><br>\
</pre>\
</html> ";

volatile long cxx = 0L;
int ip_i = 0;
volatile int t = 1;

void wireless_loop(void)
{
  char cmd[200];
  char output_text[2000];
  char input_text[2000];

  input_text[0] = '\0';

#if WIFI_TEST  
  printxy_str(0,0,"Wifi Test Mode");
#endif
    
  int c;

  cxx++;
	
  if( (cxx % 100000) == 0 )
    {
      if( t )
	{
	  printxy_str(20,3, "W");
	}
      else
	{
	  printxy_str(20,3, " ");
	}
      t = !t;
    }
#if 0	
  c = getchar_timeout_us(0);
	
  if( c != PICO_ERROR_TIMEOUT )
    {
      //printf("%c", c);
      uart_putc(UART_ID, c);
      if( c == 13 )
	{
	  uart_putc(UART_ID, 10);
	}
    }
#endif
  while( uart_is_readable (UART_ID) )
    {
      char ch[2] = " ";
      ch[0] = uart_getc(UART_ID);

      //#if WIFI_TEST      
      printxy(1,19, ch[0]);
      //#endif
      // Add to input buffer
      strcat(input_text, ch);

      if( ch[0] == 10 )
	{
	  input_text[0] = '\0';
	}

      if( ch[0] == 13 )
	{
	  input_text[0] = '\0';
	}
	  
      if( strncmp(input_text, "ready", 5) == 0 )
	//if( !setup && (cxx > 2000000) )
	{
	  setup = 1;
#if WIFI_TEST
	  printxy_str(0,1, "\nSetting up...");
#else
	  printxy(20,0, 'S');
#endif
	      
	  sleep_us(5000000);
	  sprintf(cmd, "AT+CWMODE=2\r\n");
	  uart_puts(UART_ID, cmd);

	  sleep_us(5000000);
	  sprintf(cmd, "AT+CIPMUX=1\r\n");
	  uart_puts(UART_ID, cmd);

	  sleep_us(1000000);
	  sprintf(cmd, "AT+CIPSERVER=1,80\r\n");
	  uart_puts(UART_ID, cmd);
	      
	  input_text[0] = '\0';
	}
		  
	  
      if( strncmp(input_text, "+IPD", 4) == 0 )
	{
	  printxy(20,1, 'P');

	  sprintf(output_text, reply1, cxx, display_line[0], display_line[1], display_line[2], display_line[3]);
	      
	  // Wait for a second and then send reply
	  sleep_us(1000000);
	      
	  sprintf(cmd, "AT+CIPSEND=0,%d\r\n", strlen(output_text));
	  uart_puts(UART_ID, cmd);
#if WIFI_TEST	  
	  printxy_str(0,2, cmd);
#endif 
	  sleep_us(1000000);

	  uart_puts(UART_ID, output_text);
	  printf("%s\n", output_text);
	      
	  sleep_us(1000000);

	  uart_puts(UART_ID, "AT+CIPCLOSE=0\r\n");

	  input_text[0] = '\0';
	}
    }
}
