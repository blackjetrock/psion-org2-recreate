////////////////////////////////////////////////////////////////////////////////
//
// Serial CLI Handling
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/bootrom.h"

#include "psion_recreate.h"
#include "emulator.h"
#include "wireless.h"

int keypress = 0;
int parameter = 0;
unsigned int address = 0;

void serial_help(void);
void prompt(void);

void cli_boot_mass(void)
{
  reset_usb_boot(0,0);
}

void cli_set_address(void)
{
  address = parameter;
}

void cli_zero_parameter(void)
{
  parameter = 0;
}

void cli_dump_memory(void)
{
  // Display memory from address
  printf("\n");
  
  for(int z = parameter; z<parameter+256; z++)
    {
      int byte = 0;
      
      if( (z % 8) == 0)
	{
	  printf("\n%03X: ", z);
	}

      if( z >= ROM_START )
	{
	  printf("%02X ", ROMDATA(z-ROM_START));
	}
      else
	{
	  printf("%02X ", RAMDATA(z));
	}
    }
  
  printf("\n");

}

void cli_info(void)
{
  printf("\nBank RAM Offset:%08X", ram_bank_off);
  printf("\nBank ROM Offset:%08X", rom_bank_off);

  printf("\nProcessor State");
  printf("\n===============");

  printf("\nPC:%04X", REG_PC);
  printf("\n%04X      A:%02X B:%02X X:%04X", REG_A, REG_B, REG_X);

}


// Another digit pressed, update the parameter variable
void cli_digit(void)
{
  unsigned int  n = 0;

  printf("\nK:%02X", keypress);
  printf("\nn:%02X", n);
  if( keypress > '9' )
    {
      n = keypress - 'a' + 10;
    }
  else
    {
      n = keypress - '0';
    }

  printf("\nn:%02X", n);
  parameter <<= 4;
  parameter |= n;
}

void cli_trace_dump_from(void)
{
  for(int i=0; i<addr_trace_i; i++)
    {
      printf("\n%04X:%04X      A:%02X B:%02X X:%04X SP:%04X FLAGS:%02X (%s)",	     
	     i,
	     addr_trace_from[i],
	     addr_trace_from_a[i],
	     addr_trace_from_b[i],
	     addr_trace_from_x[i],
	     addr_trace_from_sp[i],
	     addr_trace_from_flags[i],
	     decode_flag_value(addr_trace_from_flags[i])
	     );
    }
  
  printf("\n");
}

void cli_trace_dump_to(void)
{
  int j = addr_trace_to_i;
  
  for(int i=0; i<NUM_ADDR_TRACE; i++)
    {
      printf("\n%04X:%04X      A:%02X B:%02X X:%04X SP:%04X FLAGS:%02X (%s)",
	     i,
	     addr_trace_to[j],
	     addr_trace_to_a[j],
	     addr_trace_to_b[j],
	     addr_trace_to_x[j],
	     addr_trace_to_sp[j],
	     addr_trace_to_flags[j],
	     decode_flag_value(addr_trace_to_flags[j])
	     );
      j = ((j + 1) % NUM_ADDR_TRACE);
    }
  
  printf("\n");
}

////////////////////////////////////////////////////////////////////////////////

// Serial loop command structure

typedef void (*SERIAL_FPTR)(void);

typedef struct
{
  char key;
  char *desc;
  SERIAL_FPTR fn;
} SERIAL_COMMAND;

SERIAL_COMMAND serial_cmds[] =
  {
   {
    '?',
    "Serial command help",
    serial_help,
   },
   {
    '!',
    "Boot to mass storage",
    cli_boot_mass,
   },
   {
    '0',
    "*Digit",
    cli_digit,
   },
   {
    '1',
    "*Digit",
    cli_digit,
   },
   {
    '2',
    "*Digit",
    cli_digit,
   },
   {
    '3',
    "*Digit",
    cli_digit,
   },
   {
    '4',
    "*Digit",
    cli_digit,
   },
   {
    '5',
    "*Digit",
    cli_digit,
   },
   {
    '6',
    "*Digit",
    cli_digit,
   },
   {
    '7',
    "*Digit",
    cli_digit,
   },
   {
    '8',
    "*Digit",
    cli_digit,
   },
   {
    '9',
    "*Digit",
    cli_digit,
   },
   {
    'a',
    "*Digit",
    cli_digit,
   },
   {
    'b',
    "*Digit",
    cli_digit,
   },
   {
    'c',
    "*Digit",
    cli_digit,
   },
   {
    'd',
    "*Digit",
    cli_digit,
   },
   {
    'e',
    "*Digit",
    cli_digit,
   },
   {
    'f',
    "*Digit",
    cli_digit,
   },
   {
    'M',
    "Dump Memory",
    cli_dump_memory,
   },
   {
    'z',
    "Zero Parameter",
    cli_zero_parameter,
   },
   {
    'i',
    "Information",
    cli_info,
   },
   {
    'T',
    "Trace To Dump",
    cli_trace_dump_to,
   },
   {
    'F',
    "Trace From Dump",
    cli_trace_dump_from,
   },
  };

int pcount = 0;
int periodic_read = 0;

void serial_loop()
{
  int  key;

  if( ((key = getchar_timeout_us(1000)) != PICO_ERROR_TIMEOUT))
    {
      for(int i=0; i<sizeof(serial_cmds)/sizeof(SERIAL_COMMAND);i++)
	{
	  if( serial_cmds[i].key == key )
	    {

	      keypress = key;
	      (*serial_cmds[i].fn)();
	      prompt();
	      break;
	    }
	}
    }
}

void serial_help(void)
{
  printf("\n");
  
  for(int i=0; i<sizeof(serial_cmds)/sizeof(SERIAL_COMMAND);i++)
    {
      if( *(serial_cmds[i].desc) != '*' )
	{
	  printf("\n%c:   %s", serial_cmds[i].key, serial_cmds[i].desc);
	}
    }
  
  printf("\nEnter 0-9,A-F for hex parameter");  
  printf("\n");
}


void prompt(void)
{
  printf("\nP:%08X >", parameter);
}
