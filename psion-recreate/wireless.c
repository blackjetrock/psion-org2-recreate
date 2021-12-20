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
#include "hardware/irq.h"

#include "psion_recreate.h"

void start_task(char *label);
void remove_string(char *str);
void remove_n(int n);

int ifm_null(int i);
void ifn_ignore(int i);
int ifm_ipd(int i);
int ifm_cipsend(int i);
int ifm_recv(int i);

void ifn_next_is_ok(int i);
void ifn_check_ok(int i);
void ifn_ready(int i);
void ifn_ipd(int i);
void ifn2_ipd(void);
void ifn_ciprecvdata(int i);
void ifn_null(int i);
void ifn_cipsend(int i);
void ifn_recv(int i);
void ifn_closed(int i);
void ifn_busy(int i);
void ifn_connect(int i);

void ufn_index(void);
void ufn_memory_0(void);
void ufn_memory_at(void);
void ufn_memory_write(void);
void ufn_ram(void);
void ufn_eeprom_write(void);
void ufn_eeprom_read(void);

////////////////////////////////////////////////////////////////////////////////

int w_task_index = 0;
int w_task_running = 0;
uint64_t w_task_delay_end = 0;
int w_task_delaying = 0;

char cmd[200];
char delay_text[20];
char output_text[2000];
int output_text_len = 0;

#define INPUT_TEXT_SIZE  2000

char input_text[INPUT_TEXT_SIZE+1];
int input_text_len = 0;

char input_temp[2000];
char input_data[2000];
char uri[200];

int n_scanned, chan, numchars, num_scanned;
int v1,v2;

// Current connection number
int connection = 0;

int tasks_run = 0;

typedef void (*BYTE_CONT_FN)(void);

char byte_buffer[2000];
int byte_buffer_index;
int collecting_bytes = 0;
BYTE_CONT_FN when_done_fn;
int bytes_left_to_collect = 0;

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


#if UART_INTERRUPTS

#define INTERRUPT_TEXT_SIZE 5000

volatile char interrupt_text[INTERRUPT_TEXT_SIZE+1];
volatile  int irq_text_out_ptr = 0;
volatile  int irq_text_in_ptr = 0;

// RX interrupt handler
volatile int irq_count = 0;
volatile int irq_no_char = 0;
volatile int irq_after = 0;
volatile int char_in_loop = 0;

void on_uart_rx()
{
  int np = 0;
  
  irq_count++;
  
  if( !uart_is_readable(UART_ID) )
    {
      irq_no_char++;
    }
  
  while (uart_is_readable(UART_ID))
    {
      uint8_t ch = uart_getc(UART_ID);

      if( ch != '\0' )
	{
	  interrupt_text[irq_text_in_ptr] = ch;
	  np = irq_text_in_ptr;
	  np++;
	  irq_text_in_ptr = (np % INTERRUPT_TEXT_SIZE);
	}
    }

  if( uart_is_readable(UART_ID) )
    {
      irq_after++;
    }

}

#endif


void wireless_init(void)
{
  
  sleep_us(1000000);  
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
  uart_init(UART_ID, BAUD_RATE);
  
  uart_set_hw_flow(UART_ID, false, false);
  
  uart_set_fifo_enabled(UART_ID, false);

#if UART_INTERRUPTS
  // Set up a RX interrupt
  // We need to set up the handler first
  // Select correct interrupt for the UART we are using
  int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
  
  // And set up and enable the interrupt handlers
  irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
  irq_set_enabled(UART_IRQ, true);
  
  // Now enable the UART to send interrupts - RX only
  uart_set_irq_enables(UART_ID, true, false);
  
#endif
    
  //  gpio_put(7, 1);
  //gpio_set_dir(7, GPIO_OUT);
  
  printf("\nOrganiser 2 Main Board\n");
  stdio_flush();

  // turn power off for a while
  latch2_clear_mask(LAT2PIN_MASK_ESP_ON);
  sleep_ms(100);
  latch2_set_mask(LAT2PIN_MASK_ESP_ON);

  input_text[0] = '\0';
  input_text_len = 0;
}

volatile int setup = 0;

char *reply1 = "HTTP/1.1 200 OK\n\
Content-Type: text/html\n\
Connection: close\n\n\
<!DOCTYPE HTML>\n\
<html>\n\
Psion Organiser Recreation\
<br>Ticks:%d<br>\
<pre><br><br>\
<tt>%s</tt><br>\
<br>\
<tt>%s</tt><br>\
<tt>%s</tt><br>\
<tt>%s</tt><br>\
<tt>%s</tt><br>\
<tt>%s</tt><br>\
</pre>\
</html> \r\n";

volatile long cxx = 0L;
int ip_i = 0;
volatile int t = 1;

// returns a string version of a floating point number
// located at the RAM address given

char string_float[40];

char *stringify_float(int address)
{
  char ch[10];

  ch[0] = '\0';
  string_float[0] = '\0';
  
  if( ramdata[address+7] & 0x80)
    {
      strcat(string_float, "-");
    }
  else
    {
      strcat(string_float, " ");
    }
  
  for(int i=5; i>=0; i--)
    {
      if( i == 5 )
	{
	  sprintf(ch, "%01X", ramdata[address+i] >> 4);
	  strcat(string_float, ch);
	  strcat(string_float, ".");
	  sprintf(ch, "%01X", ramdata[address+i] & 0x0F);
	  strcat(string_float, ch);
	}
      else
	{
	  sprintf(ch, "%02X", ramdata[address+i]);
	  strcat(string_float, ch);
	}
    }

  strcat(string_float, " E");
  int8_t exp = ramdata[address+6];
  
  if( exp & 0x80 )
    {
      sprintf(ch, "-%02X", -exp);
    }
  else
    {
      sprintf(ch, "%02X", exp);
    }
  strcat(string_float, ch);
  
  return(string_float);
}

////////////////////////////////////////////////////////////////////////////////
//
// We don't want to introduce delays into the flow of the code as it will
// stop the emulator running if it is a delay on core0 and if it is a core1
// delay then it will stop display updates.
//
// So, we run a simple state machine which is a list of commands
// decisions are taken by code which the points to a new set of commands to
// execute
//

// Wireless task

typedef enum _W_TYPE
  {
   WTY_PUTS = 1,
   WTY_DELAY_MS,
   WTY_LABEL,
   WTY_STOP,
   WTY_SENDDATA
  } W_TYPE;

typedef struct _W_TASK
{
  W_TYPE  type;    // What to do
  char *string;    // String parameter
} W_TASK;

// The master table of tasks
W_TASK tasklist[] =
  {
   {WTY_LABEL,    "init"},
   {WTY_PUTS,     "AT+CWMODE=2\r\n"},
   {WTY_DELAY_MS, "2000"},
   {WTY_PUTS,     "AT+CIPMUX=1\r\n"},
   {WTY_DELAY_MS, "2000"},
   {WTY_PUTS,     "AT+CWSAP=\"PsionOrg2\",\"1234567890\",5,3\r\n"},
   {WTY_DELAY_MS, "5000"},
   {WTY_PUTS,     "AT+CIPSERVER=1,80\r\n"},
   {WTY_DELAY_MS, "2000"},

   //   {WTY_PUTS,     "AT+CIPRECVMODE=0\r\n"},
   //{WTY_DELAY_MS, "2000"},
   {WTY_STOP,     ""},                      // All done

   {WTY_LABEL,    "p_index"},
   //   {WTY_FN,       rfn_index},
   {WTY_STOP,     ""},                      // All done

   {WTY_LABEL,     "send"},
   {WTY_DELAY_MS,  "100"},
   {WTY_SENDDATA,            ""},
   {WTY_STOP,                ""},

   {WTY_LABEL,     "close"},
   {WTY_PUTS,                "AT+CIPCLOSE=0\r\n"},
   {WTY_STOP,                ""},                      // All done
  };

#define W_NUM_TASKS (sizeof(tasklist) / sizeof(W_TASK) )

// Input list

typedef void (*IN_FN)(int i);
typedef int (*IN_MFN)(int i);

typedef enum _I_TYPE
  {
   ITY_STRING = 1,
   ITY_FUNC,
  } I_TYPE;

typedef struct _I_TASK
{
  I_TYPE  type;       // How to match  
  char    *str;       // String to match
  IN_MFN  mfn;        // Match function
  IN_FN   fn;         // Call this function when match found  
} I_TASK;

// The master table of tasks
I_TASK input_list[] =
  {
   {ITY_STRING, " ready",                                       ifm_null,     ifn_ready},
   {ITY_STRING, " AT+CWMODE=2",                                 ifm_null,     ifn_next_is_ok},
   {ITY_STRING, " AT+CIPMUX=1",                                 ifm_null,     ifn_next_is_ok},
   {ITY_STRING, " AT+CWSAP=\"PsionOrg2\",\"1234567890\",5,3",   ifm_null,     ifn_next_is_ok},
   {ITY_STRING, " AT+CIPSERVER=1,80",                           ifm_null,     ifn_next_is_ok},
   {ITY_STRING, " AT+CIPCLOSE=%d",                              ifm_null,     ifn_next_is_ok},
   {ITY_STRING, " %d,CONNECT",                                  ifm_null,     ifn_connect},
   {ITY_FUNC,   " +IPD",                                        ifm_ipd,      ifn_ipd},
   {ITY_FUNC,   " AT+CIPSEND=%d,%d OK >",                       ifm_null,     ifn_cipsend},
   {ITY_FUNC,   " Recv %d bytes SEND OK",                       ifm_recv,     ifn_recv},

   {ITY_STRING, " OK",                                          ifm_null,     ifn_check_ok},
   {ITY_STRING, " %d,CLOSED OK",                                ifm_null,     ifn_closed},
   {ITY_STRING, " %d,CLOSED",                                   ifm_null,     ifn_ignore},
   {ITY_STRING, " busy p...",                                   ifm_null,     ifn_busy},
  };
//AT+CWSAP=\"PsionOrg2\",\"1234567890\",5,3OKAT+CIPSERVER=1,80OK0
#define I_NUM_TASKS (sizeof(input_list) / sizeof(I_TASK) )

// URI task table

typedef void (*U_FN)(void);
typedef struct _U_TASK
{
  char    *str;       // URI/memory/20b0
  U_FN    fn;         // Call this function when match found  
} U_TASK;

// The master table of tasks
U_TASK uri_list[] =
  {
   {"GET /eeprom/%d/write/%x/%x",                      ufn_eeprom_write},
   {"GET /eeprom/%d/%x",                               ufn_eeprom_read},
   {"GET /memory/write/%x/%x",                         ufn_memory_write},
   {"GET /memory/%x",                                  ufn_memory_at},
   {"GET /memory",                                     ufn_memory_0},
   {"GET /ram",                                        ufn_ram},
   {"GET /",                                           ufn_index},
  };

#define U_NUM_TASKS (sizeof(uri_list) / sizeof(U_TASK) )

////////////////////////////////////////////////////////////////////////////////
//
// Get data

typedef void (*BYTE_CONT_FN)(void);


void get_n_bytes_then(int n, BYTE_CONT_FN fn)
{
  // Store parameters
  bytes_left_to_collect = n;
  when_done_fn = fn;
  
  // Start of a new collection
  byte_buffer_index = 0;

  collecting_bytes = 1;

  // If there are any bytes in the input_text buffer then we copy them over
  int original_input_text_len = input_text_len;

  while( (bytes_left_to_collect > 0) && (input_text_len > 0) )
    {
      byte_buffer[byte_buffer_index] = input_text[byte_buffer_index];
      byte_buffer_index++;
      input_text_len--;
      bytes_left_to_collect--;
    }

  input_text_len = original_input_text_len;

  // Take the bytes off the input text array (adjusts input_text_len)
  remove_n(byte_buffer_index);

  // Are we done?
  if( bytes_left_to_collect == 0)
    {
      collecting_bytes = 0;

      // Call done function
      (*when_done_fn)();
    }
}

////////////////////////////////////////////////////////////////////////////////
//

// Routes a URI to a function
void process_uri(char *uri)
{

  for(int i=0; i<U_NUM_TASKS; i++)
    {

      if( match(uri, uri_list[i].str) )
	{

	  (*uri_list[i].fn)();
	  break;
	}
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// remove the given string from the input text

char error_str[100];

void remove_string(char *str)
{
  if( strncmp(input_text, str, strlen(str)) == 0 )
    {
      // Copy rest of string to temporary buffer then back to input text buffer
      // This could be faster but we want it to work for now
#if 0      
      strcpy(input_temp, &(input_text[strlen(str)]));
      strcpy(input_text, input_temp);
#else
  if( strlen(str) > input_text_len)
  {
    //    DEBUG_STOP;
  }
      memcpy(input_temp, &(input_text[strlen(str)]), input_text_len - strlen(str)+1);
      memcpy(input_text, input_temp, input_text_len - strlen(str)+1);
      input_text_len -= strlen(str);
#endif
    }
  else
    {
      strcpy(error_str, str);
    }
}

// Remove n characters from the input stream
void remove_n(int n)
{
#if 0  
  strcpy(input_temp, &(input_text[n]));
  strcpy(input_text, input_temp);
#else
  if( n > input_text_len)
  {
    //    DEBUG_STOP;
  }

  memcpy(input_temp, &(input_text[n]), input_text_len - n+1);
  memcpy(input_text, input_temp, input_text_len - n+1);
  input_text_len -= n;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
// Matching functions that test for commands

int ifm_ipd(int i)
{

  if(match(input_text, " +IPD,%d,%d:") )
    {
      //DEBUG_STOP;
      return(1);
    }
  else
    {
      return(0);
    }
}

int ifm_cipsend(int i)
{
  if( match(input_text, input_list[i].str) )
    {
      //      DEBUG_STOP;
      return(1);
    }
  else
    {
      return(0);
    }
}

int ifm_recv(int i)
{
  if( match(input_text, " Recv %d bytes SEND OK") )
    {
      return(1);
    }
  else
    {
      return(0);
    }
}

int ifm_null(int i)
{
  return(1);
}

////////////////////////////////////////////////////////////////////////////////
//
// Input functions
//
// These handle the matching strings in the input buffer
//

// Do nothing
void ifn_null(int i)
{
  // remove string we tested for
  remove_n(match_num_scanned);
  //remove_string(input_list[i].str);
}

// Set when we want to check that the next string is OK
int next_str_ok = 0;

void ifn_next_is_ok(int i)
{
  // remove string we tested for
  //remove_string(input_list[i].str);
  remove_n(match_num_scanned);
  next_str_ok = 1;
}

void ifn_check_ok(int i)
{
  if( next_str_ok )
    {
      // All ok

      // remove string we tested for
      remove_n(match_num_scanned);
      //remove_string(input_list[i].str);
      next_str_ok = 0;
    }
  else
    {
      // Do nothing, the input strings will stop here.
    }
}

void ifn_ready(int i)
{
  setup = 1;
  
#if WIFI_TEST
  printxy_str(0,1, "\nSetting up...");
#else
  write_display_extra(0, 'S');
#endif
  start_task("init");

  // Remove the string
  //remove_string("ready");
  remove_n(match_num_scanned);
}



// Processes the +IPD string
void ifn_ipd(int i)
{
  write_display_extra(2, 'P');

  //DEBUG_STOP;
  
  // We need to get the IPD textoff the input stream
  // +IPD,0,370:
  
  if( match(input_text, " +IPD,%d,%d:") )
    {
      // remove the command
      remove_n(match_num_scanned);
      
      // Get the data
      // numchars holds the number of characters of data we can get
      // with a CIPREVDATA command. The URI should be in the input stream next
      // though, so we get that.

      // We now need to wait for numchars bytes to come in as that is the data for the IPD
      get_n_bytes_then(match_int_arg[1], ifn2_ipd);
    }
}

// We continue here after all data collected

void ifn2_ipd(void)
{
  //DEBUG_STOP;
  // We have the URI, process it and find out which page to return
  // based on it
  process_uri(byte_buffer);
}

// Processes the AT+CIPSEND string

void ifn_cipsend(int i)
{
  write_display_extra(1, 'C');

  if( match(input_text, input_list[i].str))
    {
      // remove the command
      remove_n(match_num_scanned);

      // We now have to send the data and then close the connection
      start_task("send");

      // The reply to the send will trigger the close
      
    }
}

void ifn_recv(int i)
{
  write_display_extra(1, 'r');

  if( match(input_text, input_list[i].str))
    {
      // remove the command
      remove_n(match_num_scanned);

      // We now have to send the data and then close the connection
      start_task("close");

      // The reply to the send will trigger the close
    }
}

void ifn_connect(int i)
{
  write_display_extra(1, 'n');

  if( match(input_text, input_list[i].str))
    {
      // remove the command
      remove_n(match_num_scanned);

      // Store the connection number
      connection = match_int_arg[0];
    }
}

void ifn_ignore(int i)
{
  write_display_extra(1, 'i');

  if( match(input_text, input_list[i].str))
    {
      // remove the command
      remove_n(match_num_scanned);

      // All done
    }
}

void ifn_closed(int i)
{
  write_display_extra(1, 'c');

  if( match(input_text, " 0,CLOSED OK"))
    {
      // remove the command
      remove_n(match_num_scanned);

      // All done
    }
}

void ifn_busy(int i)
{
  write_display_extra(1, 'c');

  if( match(input_text, " busy p..."))
    {
      // remove the command
      remove_n(match_num_scanned);

      // All done
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Starts off a reply. The WTY_SENDDATA task will send the actual
// data. Reply must be in output_text

void send_reply(void)
{
  output_text_len = strlen(output_text);
  sprintf(cmd, "AT+CIPSEND=0,%d\r\n", output_text_len);
  uart_puts(UART_ID, cmd);
}

////////////////////////////////////////////////////////////////////////////////
//
// URI processing

// Index page, we send back the main page with data added

void ufn_index(void)
{
  char mems[10*(8+1+1+2+2+10)];
  char t[40];
  
  // Get calculator memory 0
  mems[0] = '\0';
  for(int m=0; m<10; m++)
    {
      sprintf(t, "<br>M%d:", m);
      strcat(mems, t);
#if 0	      	      
      for(int i=0; i<8; i++)
	{
	  sprintf(t, "%02X", ramdata[0x20ff+m*8+i]);
	  strcat(mems, t);
	}
#else
      strcat(mems, stringify_float(0x20ff+m*8));
    }
#endif
  sprintf(output_text, reply1, cxx, mems, display_line[0], display_line[1], display_line[2], display_line[3], input_text);

  // Send reply back
  // We send the command and have to wait for the OK and '>' prompt
  output_text_len = strlen(output_text);
  sprintf(cmd, "AT+CIPSEND=%d,%d\r\n", connection, output_text_len);
  uart_puts(UART_ID, cmd);
  
#if WIFI_TEST	  
  printxy_str(0,2, cmd);
#endif

}

char *reply2 = "HTTP/1.1 200 OK\n\
Content-Type: text/html\n\
Connection: close\n\n\
<!DOCTYPE HTML>\n\
<html>\n\
Psion Organiser Recreation\
<br>Ticks:%d<br>\
<br>Memory<br>\
<pre><br><br>\
<tt>%s</tt><br>\
<br>\
</pre>\
</html> \r\n";


void ufn_memory_addr(int addr)
{
#define MEM_LEN  128
#define MEM_LINE 16
  
  char mems[(MEM_LEN)*3+(MEM_LEN*6)+20];
  char t[40];
  char ascii[MEM_LINE+1];
  
  // Get calculator memory 0
  mems[0] = '\0';
  ascii[0] = '\0';
  
  for(int m=addr; m<addr+MEM_LEN; m++)
    {
      if( (m % MEM_LINE) == 0 )
	{
	  sprintf(t, "  %s<br>%04X:", ascii, m);
	  strcat(mems, t);
	  ascii[0] = '\0';
	}
      
      sprintf(t, "%02X ", ramdata[m]);
      strcat(mems, t);
      if( isprint(ramdata[m]) )
	{
	  t[0] = ramdata[m];
	  t[1] = '\0';
	  strcat(ascii, t);
	}
      else
	{
	  strcat(ascii, ".");
	}
    }

  strcat(mems, "  ");
  strcat(mems, ascii);
  sprintf(output_text, reply2, cxx, mems);

  
  // Send reply back
  
  // We send the command and have to wait for the OK and '>' prompt
  output_text_len = strlen(output_text);
  sprintf(cmd, "AT+CIPSEND=0,%d\r\n", output_text_len);
  uart_puts(UART_ID, cmd);
  
#if WIFI_TEST	  
  printxy_str(0,2, cmd);
#endif


}

void ufn_memory_0(void)
{
  ufn_memory_addr(0x1000);
}

void ufn_memory_at(void)
{
  ufn_memory_addr(match_int_arg[0]);
}

void ufn_ram(void)
{

}

char *reply_mem_write = "HTTP/1.1 200 OK\n\
Content-Type: text/html\n\
Connection: close\n\n\
<!DOCTYPE HTML>\n\
<html>\n\
Psion Organiser Recreation\
<br>Ticks:%d<br>\
<br>%s: Wrote %02X to %04X<br>\
</html> \r\n";

void ufn_memory_write(void)
{
  // Write the byte to the RAM

  ramdata[match_int_arg[0]] = match_int_arg[1];

  // Give a reply
  sprintf(output_text, reply_mem_write, cxx, "Memory", match_int_arg[1], match_int_arg[0]);

  send_reply();
}



void ufn_eeprom_write(void)
{
  BYTE data[1];
  int slave_addr;
  int start;
  
  data[0] = match_int_arg[2];
  if(match_int_arg[0]==0)
    {
      slave_addr = EEPROM_0_ADDR_RD;
    }
  else
    {
      slave_addr = EEPROM_1_ADDR_RD;
    }
  
  start = match_int_arg[1];
  
  // Write the byte to the RAM
  write_eeprom(slave_addr, start, 1, data);

  // Give a reply
  sprintf(output_text, reply_mem_write, cxx, "EEPROM", match_int_arg[2], match_int_arg[1]);

  send_reply();
}

void ufn_eeprom_read(void)
{
#define MEM_LEN  128
#define MEM_LINE 16
  
  char mems[(MEM_LEN)*3+(MEM_LEN*6)+20];
  char t[40];
  char ascii[MEM_LINE+1];
  BYTE data[MEM_LINE];
  int start;
  int slave_addr;

  slave_addr = (match_int_arg[0]==0)?EEPROM_0_ADDR_RD:EEPROM_1_ADDR_RD;
  start = match_int_arg[1];

  // Get calculator memory 0
  mems[0] = '\0';
  ascii[0] = '\0';

  read_eeprom(slave_addr, start, MEM_LINE, data);
  
  for(int m=start; m<start+MEM_LEN; m++)
    {
      if( (m % MEM_LINE) == 0 )
	{
	  sprintf(t, "  %s<br>%04X:", ascii, m);
	  strcat(mems, t);
	  ascii[0] = '\0';
	  read_eeprom(slave_addr, m, MEM_LINE, data);
	}
      
      sprintf(t, "%02X ", data[m % MEM_LINE]);
      strcat(mems, t);
      if( isprint(data[m % MEM_LINE]) )
	{
	  t[0] = data[m];
	  t[1] = '\0';
	  strcat(ascii, t);
	}
      else
	{
	  strcat(ascii, ".");
	}
    }

  strcat(mems, "  ");
  strcat(mems, ascii);
  sprintf(output_text, reply2, cxx, mems);

  
  // Send reply back
  
  // We send the command and have to wait for the OK and '>' prompt
  output_text_len = strlen(output_text);
  sprintf(cmd, "AT+CIPSEND=0,%d\r\n", output_text_len);
  uart_puts(UART_ID, cmd);

}

////////////////////////////////////////////////////////////////////////////////


void start_task(char *label)
{
  for(int i=0; i<W_NUM_TASKS; i++)
    {
      if( (tasklist[i].type == WTY_LABEL) && (strcmp(tasklist[i].string, label)==0) )
	{
	  // Found task label, so set it up to run
	  w_task_index = i;
	  w_task_running = 1;

	  // All done
	  tasks_run++;
	  return;
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Main task loop for communicatinmg with the WROOM
//

void wireless_taskloop(void)
{
  long parameter = 0;
  int c;
  int process_text = 0;
  
  cxx++;
	
  if( (cxx % 100) == 0 )
    {
      if( t )
	{
	  write_display_extra(3, 'W');
	}
      else
	{
	  write_display_extra(3, ' ');
	}
      t = !t;
    }


#if UART_INTERRUPTS

  // Any more characters?
  // If so, collect them
  if( uart_is_readable(UART_ID) )
    {
      on_uart_rx();
      char_in_loop++;
    }
  
  // Has anything come in?
  // If so, add it to the input buffer

  while( irq_text_out_ptr != irq_text_in_ptr )  
    {
      char ch[2] = " ";
      ch[0] = interrupt_text[irq_text_out_ptr++];
      irq_text_out_ptr = (irq_text_out_ptr % INTERRUPT_TEXT_SIZE);
      
#else
    
  while( uart_is_readable (UART_ID) )
    {
      char ch[2] = " ";
      ch[0] = uart_getc(UART_ID);
#endif
      
      //#if WIFI_TEST      
      //      printxy(1,19, ch[0]);
      //#endif

      // If we are collecting data then store it in buffer and don't process it
      if( collecting_bytes,0 )
	{
	  byte_buffer[byte_buffer_index++] = ch[0];
	  bytes_left_to_collect--;

	  if( bytes_left_to_collect == 0)
	    {
	      collecting_bytes = 0;
	      
	      // Call done function
	      (*when_done_fn)();
	    }
	  return;
	}
      else
	{
	  
	  // Add to input buffer if it is a character that we don't want to ignore
	  // Only process the input text if a full line has been received, i.e.
	  // a '\n' has been seen
	  
	  switch(ch[0])
	    {
	    case '\0':
	      break;
	      
	    case '\r':
	      //break;
	      
	    case '\n':
	      process_text = 1;
	      //break;
	      
	    default: 
	      strcat(input_text, ch);
	      input_text_len++;
	      break;
	    }
	}
    }
  
  // We collect bytes for buffers here on the output of the input_text
  // array so the ordering of characters is correct. We have to count
  // '\r' and '\n' (and '\0' if they exist) characters
  
  if( collecting_bytes )
    {
      char ch;
      
      while( (bytes_left_to_collect > 0) && (input_text_len > 0) )
	{
	  ch = input_text[0];
	  remove_n(1);
	  
	  byte_buffer[byte_buffer_index++] = ch;
	  bytes_left_to_collect--;
	}
      
      if( bytes_left_to_collect == 0)
	{
	  collecting_bytes = 0;
	  
	  // Call done function
	  (*when_done_fn)();
	}
      
      // 
      return;
    }
  
  // Check the table of input string to see if anything matched
  if( process_text,1 )
    {
      // Drop leading whitespace

      while( isspace(input_text[0]) )
	{
	  remove_n(1);
	}
      
      for(int i=0; i<I_NUM_TASKS; i++)
	{
	  switch(input_list[i].type )
	    {
	    case ITY_STRING:
	      if( match(input_text, input_list[i].str) )
		{
		  // Match, execute the function
		  (*input_list[i].fn)(i);
		}
	      break;
	      
	    case ITY_FUNC:
	      if( (*(input_list[i].mfn))(i) )
		{
		  // Match, execute the function
		  (*input_list[i].fn)(i);
		}
	      break;
	    }
	}
    }
  
  // Run one task from the task table
  if( w_task_running )
    {
      if( w_task_delaying )
	{
	  u_int64_t now = time_us_64();
	  
	  // Waiting for a delay to finish
	  if( now >= w_task_delay_end )
	    {
	      // Delay over
	      w_task_delaying = 0;

	      // We want to go round and process the opcode that we have
	      // delayed on, it isn't delay as the index was incremented
	      // after the delay was set up
	      return;
	    }
	  else
	    {
	      // Delay not over
	      return;
	    }
	}
      else
	{
	  //DEBUG_STOP;
	  switch(tasklist[w_task_index].type)
	    {
	    case WTY_DELAY_MS:
	      sscanf(tasklist[w_task_index].string, "%ld", &parameter);
	      w_task_delaying = 1;
	      w_task_delay_end = time_us_64() + parameter*1000;
	      sprintf(delay_text, "%ld", parameter);
	  
#if WIFI_TEST
	      printxy_str(2,2,delay_text);
#endif
	      break;
	      
	    case WTY_PUTS:
	      sprintf(cmd,  tasklist[w_task_index].string);
#if WIFI_TEST
	      printxy_str(0,3, cmd);
#endif
	      uart_puts(UART_ID, cmd);
	      break;

	    case WTY_SENDDATA:
	      uart_puts(UART_ID, output_text);
	      break;
	      
	    case WTY_STOP:
	      w_task_running = 0;
	      break;
	    }
	}
      
      // Move to next iondex, if we aren't pointing to a stop
      if( tasklist[w_task_index].type != WTY_STOP )
	{
	  w_task_index++;
#if WIFI_TEST
	  printxy_hex(1,1,w_task_index);
#endif
	}
    }
}
