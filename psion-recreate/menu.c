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
#include "emulator.h"
#include "psion_recreate.h"

void enter_menu(void)
{
  printxy_str(0,0, "Menu");

  while(1)
    {
    }
}


//------------------------------------------------------------------------------
//
// Menu detection

// How many presses of the SHIFT key have been seen
int shift_edge_counter = 0;
int last_shift = 0;
int shift = 0;

void check_menu_launch(void)
{
  if( !(ramdata[PORT5] & 0x04) && !(sca_counter & 0x02) )
    {
      // Shift pressed
      shift = 1;
    }
  else
    {
      // Shift not pressed
      shift = 0;
      
      // Any other key pressed?
      if( (ramdata[PORT5] & 0x7C) != 0x7C )
	{
	  // Another key pressed, reset shift_edge_counter
	  shift_edge_counter = 0;
	}
    }
  
  if( (last_shift == 0) && (shift == 1) )
    {
      // Another edge
      shift_edge_counter++;
      
      if( shift_edge_counter == 50 )
	{
    	  enter_menu();
	  shift_edge_counter = 0;
	}
    }
  
  last_shift = shift;
}
