////////////////////////////////////////////////////////////////////////////////
//
// The Menu
//
// This is the menu that lives outside the emulation and allows 'meta' tasks
// to be performed.
//
//
////////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "psion_recreate.h"

#include "menu.h"
#include "emulator.h"

////////////////////////////////////////////////////////////////////////////////
//
// Scan the keyboard for keys

int scan_drive = 0x01;
int scan_state = 0;
int kb_sense = 0;
int saved_latchout1_shadow = 0;
int keycode = 0;
char keychar = 0;      // What the key code is
int gotkey = 0;       // We have a key

#define NUM_SCAN_STATES  20
#define SCAN_STATE_DRIVE 0
#define SCAN_STATE_READ  10

struct _KEYDEF
{
  int  code;
  char ch;
} menukeys[] =
    {
     {0x1002, 'A'},
     {0x1004, 'B'},
     {0x1008, 'C'},
     {0x1040, 'D'},
     {0x1010, 'E'},
     {0x1020, 'F'},
     {0x0802, 'G'},
     {0x0804, 'H'},
     {0x0808, 'I'},
     {0x0840, 'J'},
     {0x0810, 'K'},
     {0x0820, 'L'},
     {0x0402, 'M'},
     {0x0404, 'N'},
     {0x0408, 'O'},
     {0x0440, 'P'},
     {0x0410, 'Q'},
     {0x0420, 'R'},
     {0x0202, 'S'},
     {0x0204, 'T'},
     {0x0208, 'U'},
     {0x0240, 'V'},
     {0x0210, 'W'},
     {0x0220, 'X'},
     {0x0108, 'Y'},
     {0x0140, 'Z'},
     {0x0101, 'm'},
     {0x0120, 'x'},
     {0x0102, 's'},
     {0x0104, '-'},
     {0x0801, 'l'},
     {0x1001, 'r'},
     {0x0201, 'u'},
     {0x0401, 'd'},
     {0x0110, ' '},
    };

#define NUM_KEYCODES ((sizeof(menukeys))/(sizeof(struct _KEYDEF)))

void scan_keys(void)
{
  switch(scan_state)
    {
    case SCAN_STATE_DRIVE:
      // Drive scan lines
      latchout1_shadow &= 0x80;
      latchout1_shadow |= scan_drive;
      write_595(PIN_LATCHOUT1, latchout1_shadow, 8);

      // Store the scan drive for the building of the keycode before
      // scan drive is updated
      keycode = scan_drive;
      
      scan_drive <<= 1;
      if( scan_drive == 0x80 )
	{
	  scan_drive = 0x1;
	}
      break;

    case SCAN_STATE_READ:
      // Read port5
      kb_sense = (read_165(PIN_LATCHIN) & 0xFC) >> 2;

      if( kb_sense & 0x20 )
	{
	  keychar = 'o';
	  gotkey = 1;
	}
      else
	{
	  if( (kb_sense & 0x3F) != 0 )
	    {
	      // Build keycode
	      //printxy_hex(5, 2, keycode);
	      //printxy_hex(1, 2, kb_sense);
	      
	      keycode |= kb_sense << 8;
	      
	      // Find key from code
	      for(int i=0; i<NUM_KEYCODES; i++)
		{
		  if( menukeys[i].code == keycode )
		    {
		      keychar = menukeys[i].ch;
		      gotkey = 1;
		      break;
		    }
		}
	    }
	}
      break;

    default:
      break;
    }
  
  scan_state = (scan_state + 1) % NUM_SCAN_STATES;
}

typedef enum _MENU_ID
  {
   MENU_TOP = 1,
   MENU_SCAN_TEST,
   MENU_INSTANT_OFF,
  } MENU_ID;

MENU_ID active_menu = MENU_TOP;

int menu_done = 0;
int menu_init = 0;

////////////////////////////////////////////////////////////////////////////////

void goto_menu(int menu)
{
  menu_init = 1;
  active_menu = menu;
}

int do_menu_init(void)
{
  if( menu_init )
    {
      menu_init = 0;
      return(1);
    }
  
  return(0);
}

////////////////////////////////////////////////////////////////////////////////

void menu_top(void)
{
  if( do_menu_init() )
    {
      display_clear();
      printxy_str(0, 0, "Meta Menu");
      printxy_str(0,1, "Keytest Instantoff");
    }
  
  if( gotkey )
    {
      switch(keychar)
	{
	case 'o':
	  menu_done = 1;
	  return;
	  break;

	case 'K':
	  goto_menu(MENU_SCAN_TEST);
	  return;
	  break;

	case 'I':
	  goto_menu(MENU_INSTANT_OFF);
	  return;
	  break;
	}
      
      gotkey = 0;
    }
}

//------------------------------------------------------------------------------

void menu_scan_test(void)
{
  if( do_menu_init() )
    {
      display_clear();
      printxy_str(0,0, "Key test    ");
    }
  
  if( gotkey )
    {
      printxy(2, 1, '=');
      printxy(3, 1, keychar);
      gotkey = 0;

      // Exit on ON key, exiting demonstrates it is working...
      if( keychar == 'o' )
	{
	  goto_menu(MENU_TOP);
	  return;
	}
    }
}

//------------------------------------------------------------------------------

// Turn off immediately, with no dump

void menu_instant_off(void)
{
  if( do_menu_init() )
    {
      display_clear();
      printxy_str(0, 0, "Instant Off");
    }

  handle_power_off();
}

////////////////////////////////////////////////////////////////////////////////
//
//

void enter_menu(void)
{
  // Save the bit pattern the serial latch is generating so keyboard
  // scan doesn't affect the organiser code
  saved_latchout1_shadow = latchout1_shadow;
  
  // Save the display for the menu exit
  display_save();
  
  display_clear();
  
  printxy_str(0,0, "Menu");

  menu_done = 0;

  goto_menu(MENU_TOP);
  
  while(!menu_done)
    {
      scan_keys();

      switch(active_menu)
	{
	case MENU_TOP:
	  menu_top();
	  break;
	  
	case MENU_SCAN_TEST:
	  menu_scan_test();
	  break;

	case MENU_INSTANT_OFF:
	  menu_instant_off();
	  break;
	}

      gotkey = 0;
    }

  display_restore();

  // Restore latch data
  write_595(PIN_LATCHOUT1, saved_latchout1_shadow, 8);
  latchout1_shadow = saved_latchout1_shadow;
}

//------------------------------------------------------------------------------
//
// Menu detection

// How many presses of the SHIFT key have been seen
// Let the organiser scan the keyboard and check the flag in memory

int shift_edge_counter = 0;
int last_shift[NUM_LAST] = { 2, 2, 2, 2, 2, };
int shift = 0;
int shift_counter = 0;

void check_menu_launch(void)
{
  shift_counter++;
  
  if( (shift_counter % META_MENU_SCAN_COUNT) == 0 )
    {
      if( ramdata[KBB_STAT] & 0x80 )
	{
	  // Shift pressed
	  shift = 1;
	  //	  printxy(0,2, '*');
	}
      else
	{
	  // Shift not pressed
	  shift = 0;
	  //	  printxy(0,2, ' ');
	}
      
      for(int i = 1; i<NUM_LAST; i++)
	{
	  last_shift[i-1] = last_shift[i];
	}
      
      last_shift[NUM_LAST-1] = shift;
       
      if( (last_shift[NUM_LAST-1] == 1) &&
	  (last_shift[NUM_LAST-2] == 1) &&
	  (last_shift[NUM_LAST-3] == 0) &&
	  (last_shift[NUM_LAST-4] == 0)
	  )
	{
	  // Another edge
	  shift_edge_counter++;
	  //printxy_hex(0,1, shift_edge_counter);
	  
	  if( shift_edge_counter == 4 )
	    {
	      enter_menu();
	      shift_edge_counter = 0;
	    }
	}
    }
      
  // Any other key pressed?
  if( ramdata[KBB_NKEYS] )
    {
      // Another key pressed, reset shift_edge_counter
      shift_edge_counter = 0;
    }
}

