#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "psion_recreate.h"
#include "emulator.h"
#include "wireless.h"
#include "eeprom.h"
#include "menu.h"
#include "rtc.h"

////////////////////////////////////////////////////////////////////////////////

volatile int core1_in_menu = 0;

////////////////////////////////////////////////////////////////////////////////

void initialise_oled(void);
void clear_oled(void);

////////////////////////////////////////////////////////////////////////////////
//
// GPIOs
//
//
////////////////////////////////////////////////////////////////////////////////

#define PIN_SD0 0
#define PIN_SD1 1
#define PIN_SD2 2
#define PIN_SD3 3
#define PIN_SD4 4
#define PIN_SD5 5
#define PIN_SD6 6
#define PIN_SD7 7

#define PIN_SCLK       10
#define PIN_SOE        11
#define PIN_SMR        12
#define PIN_P57        13

const uint PIN_SDAOUT     = 14;
const uint PIN_LATCHOUT2  = 15;
const uint PIN_I2C_SDA    = 16;
const uint PIN_I2C_SCL    = 17;
const uint PIN_LS_DIR     = 18;
const uint PIN_LATCHIN    = 19;
const uint PIN_SCLKIN     = 20;
const uint PIN_SDAIN      = 21;
const uint PIN_LATCHOUT1  = 22;

const uint PIN_SCLKOUT    = 26;
const uint PIN_VBAT_SW_ON = 27;

uint16_t latch2_shadow    = 0;
uint16_t latchout1_shadow = 0;

uint16_t csum_calc_on_restore = 0;
uint16_t csum_in_eeprom       = 0;
uint16_t csum_calc_on_dump    = 0;

void latch2_set_mask(int value)
{
    if( latch2_shadow == 0x2009 )
    {
      volatile int x = 0;

      while(x)
	{
	}
    }

  latch2_shadow |= value;
  write_595(PIN_LATCHOUT2, latch2_shadow, 16);

  if( latch2_shadow == 0x2009 )
    {
      volatile int x = 0;

      while(x)
	{
	}
    }

}

void latch2_clear_mask(int value)
{
  if( value == 0x2009 )
    {
      volatile int x = 0;

      while(x)
	{
	}
    }

  latch2_shadow &= ~value;
  write_595(PIN_LATCHOUT2, latch2_shadow, 16);

  if( latch2_shadow == 0x2009 )
    {
      volatile int x = 0;

      while(x)
	{
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Read an 8 bit value from a 165 latch
//
////////////////////////////////////////////////////////////////////////////////

uint8_t read_165(const uint latchpin)
{
  uint8_t value = 0;
  
  // Latch the data
  gpio_put(latchpin, 0);
  gpio_put(latchpin, 0);  
  gpio_put(latchpin, 1);

  // Clock the data out of the latch
  for(int i=0; i<8; i++)
    {
      gpio_put(PIN_SCLKIN, 0);
      gpio_put(PIN_SCLKIN, 1);
      
      // Read data
      value <<= 1;
      if( gpio_get(PIN_SDAIN) )
	{
	  value |= 1;
	}
    }

#if XP_DEBUG  
  printxy_hex(0,3, value);
#endif
  return(value);
}

////////////////////////////////////////////////////////////////////////////////
//
// Write an N bit pattern to a 595 latch or latches
//
////////////////////////////////////////////////////////////////////////////////

void write_595(const uint latchpin, int value, int n)
{
  
  // Latch pin low
  gpio_put(latchpin, 0);
  
  for(int i = 0; i<n; i++)
    {
      // Clock low
      gpio_put(PIN_SCLKOUT, 0);
      
      // Set data up
      if( value & (1<< (n-1)) )
	{
	  gpio_put(PIN_SDAOUT, 1);
	}
      else
	{
	  gpio_put(PIN_SDAOUT, 0);
	}

      // Shift to next data bit
      value <<= 1;
      
      // Clock data in
      gpio_put(PIN_SCLKOUT, 1);
    }

  // Now latch value (update outputs)
  gpio_put(latchpin, 1);

}

////////////////////////////////////////////////////////////////////////////////
//
// Test the keyboard
//

void keyboard_test(void)
{
  uint8_t drive = 0;
  uint8_t port5 = 0;

  printxy_str(0,0, "Keyboard Test");

    while(1)
    {
      if( drive == 0 )
	{
	  drive = 0x08;
	}

      // Drive KB line (keep display out of reset)
      write_595(PIN_LATCHOUT1, drive | 0x80, 8);

      // read port5
      port5 = (read_165(PIN_LATCHIN) ^ 0x7f);

      // Display value
      printxy_hex(0, 2, port5);
    }
}



////////////////////////////////////////////////////////////////////////////////
//
// To improve performance we use the second core. It performs:
//
// Display update of buffer to OLED over I2C
// Wireless task manager
// RTC tasks
// EEPROM accesses
// EEPROM dump and restore
//
//
// As the core peforms OLED updates over I2C it also has to do all other
// accesses or we'd have to have locks to prevent two cores using the same
// I2C bus.

typedef enum
  {
   MS_ENTER = 1,
   MS_INIT,
   MS_RUNNING,
   MS_LEAVE,
   MS_IDLE,
  } MENU_STATE;
  
MENU_STATE ms = MS_IDLE;

void menu_tasks(void)
{
  switch(ms)
    {
    case MS_IDLE:
      if( core1_in_menu )
	{
	  ms = MS_ENTER;
	}
      break;

    case MS_ENTER:
      menu_enter();
      ms = MS_RUNNING;
      break;

    case MS_RUNNING:
      menu_loop();

      if( menu_done )
	{
	  menu_done = 0;
	  ms = MS_LEAVE;
	}
      break;

    case MS_LEAVE:
      menu_leave();
      core1_in_menu = 0;
      ms = MS_IDLE;
      break;

    }
}

//------------------------------------------------------------------------------

// These are the tasks the menu function sneed to perform in order to
// keep the display, wireless and so on running.

void menu_loop_tasks(void)
{
  scan_keys();
  dump_lcd();
  rtc_tasks();
  eeprom_tasks();
  wireless_taskloop();
}


// This is the main function for the second core. It handles
// everything apart from emulationm
void core1_main(void)
{
  while(1)
    {
      dump_lcd();
      rtc_tasks();
      eeprom_tasks();

      menu_tasks();
      
#if !WIFI_TEST      
      wireless_taskloop();
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//


int main() {

#if 0
    // I2C bus
  #define PIN_I2C_SDA 19
  
  gpio_init(PIN_I2C_SCL);
  gpio_init(PIN_I2C_SDA);

  // Use pull ups
  gpio_pull_up(PIN_I2C_SDA);
  gpio_pull_up(PIN_I2C_SCL);

  gpio_set_dir(PIN_I2C_SDA, GPIO_OUT);
  gpio_set_dir(PIN_I2C_SCL, GPIO_OUT);

  gpio_put(PIN_I2C_SDA, 1);
  gpio_put(PIN_I2C_SCL, 1);
  
  while(1)
    {
    }
  #endif
  // Set up the GPIOs
  gpio_init(PIN_VBAT_SW_ON);
  gpio_set_dir(PIN_VBAT_SW_ON, GPIO_OUT);

  // Take power on high so we latch on
  gpio_put(PIN_VBAT_SW_ON, 1);

  // ON key
  gpio_init(PIN_P57);
  
  // Set GPIOs up
  gpio_init(PIN_SDAOUT);
  gpio_init(PIN_LATCHOUT2);
  gpio_init(PIN_LATCHOUT1);
  gpio_init(PIN_SDAIN);
  gpio_init(PIN_SCLKOUT);
  gpio_init(PIN_LATCHIN);
  gpio_init(PIN_SCLKIN);
  gpio_init(PIN_LS_DIR);
  
  gpio_init(PIN_SD0);
  gpio_init(PIN_SD1);
  gpio_init(PIN_SD2);
  gpio_init(PIN_SD3);
  gpio_init(PIN_SD4);
  gpio_init(PIN_SD5);
  gpio_init(PIN_SD6);
  gpio_init(PIN_SD7);

  // Turn on pull downs on the data bus
  gpio_pull_down(PIN_SD0);
  gpio_pull_down(PIN_SD1);
  gpio_pull_down(PIN_SD2);
  gpio_pull_down(PIN_SD3);
  gpio_pull_down(PIN_SD4);
  gpio_pull_down(PIN_SD5);
  gpio_pull_down(PIN_SD6);
  gpio_pull_down(PIN_SD7);
  
  gpio_init(PIN_SCLK);
  gpio_init(PIN_SOE);
  gpio_init(PIN_SMR);

  // I2C bus
  gpio_init(PIN_I2C_SCL);
  gpio_init(PIN_I2C_SDA);

  // Use pull ups
  gpio_pull_up(PIN_I2C_SDA);
  gpio_pull_up(PIN_I2C_SCL);
  
  gpio_set_dir(PIN_SDAOUT,    GPIO_OUT);
  gpio_set_dir(PIN_P57,       GPIO_IN);
  gpio_set_dir(PIN_SDAOUT,    GPIO_OUT);
  gpio_set_dir(PIN_LATCHOUT2, GPIO_OUT);
  gpio_set_dir(PIN_LATCHOUT1, GPIO_OUT);
  gpio_set_dir(PIN_SDAIN,     GPIO_IN);
  gpio_set_dir(PIN_SCLKOUT,   GPIO_OUT);
  gpio_set_dir(PIN_LATCHIN,   GPIO_OUT);
  gpio_set_dir(PIN_SCLKIN,    GPIO_OUT);
  gpio_set_dir(PIN_LS_DIR,    GPIO_OUT);
  
  gpio_set_dir(PIN_SCLK, GPIO_OUT);
  gpio_set_dir(PIN_SOE,  GPIO_OUT);
  gpio_set_dir(PIN_SMR,  GPIO_OUT);

  // Unlatch input latch
  //  gpio_put(PIN_SDAIN,  1);
  gpio_put(PIN_SCLKIN, 1);

#if OVERCLOCK
  //  set_sys_clock_khz(260000, false);   // Works
  set_sys_clock_khz(300000, false);  
  
#endif
  //------------------------------------------------------------------------------
  //
  // Display test
  //
  
#if FN_OLED_DEMO  
  oledmain();
#endif
  //------------------------------------------------------------------------------
  //
  // Initialise display

  // Turn on the 12V supply
  // This signal does nothing now. OLED turned on when 3V3 turned on
  
  latch2_set_mask(LAT2PIN_MASK_DRV_HV);
  
  //  latchout2_shadow |= LAT2PIN_MASK_DRV_HV;
  //write_595(PIN_LATCHOUT2, latchout2_shadow, 16);

  // Initialise I2C
  //i2c_fn_initialise();
  
  // Wait for it to start up
  sleep_ms(10);
  initialise_oled();
  
  // Clear screen
  clear_oled();
  stdio_init_all();

  // Restore RAM from EEPROM

  // Initialise emulator
  initialise_emulator();
  
  // Initialise wifi
  wireless_init();

  // EEPROM tests before core 1 starts, so display won't work but tests
  // won't have interference from that core on I2C
  
#if EEPROM_TEST
  eeprom_test();
#endif

#if EEPROM_TEST2
  eeprom_test2();
#endif

#if EEPROM_TEST3
  eeprom_test3();
#endif

#if MULTI_CORE
  // If multi core then we run the LCD update on the other core
  multicore_launch_core1(core1_main);

#endif

#if RAM_RESTORE
#if !DISABLE_RESTORE_ONLY  
  // Ask core1 to restore the eeprom
  eeprom_perform_restore();
  
  after_ram_restore_init();
#endif
#endif

  
#if WIFI_TEST
  printxy_str(0,0,"Wifi Test Mode");

  while(1)
    {
#if !MULTI_CORE
      core1_main();
#endif
      wireless_taskloop();
    }
#endif
  //------------------------------------------------------------------------------
  //
  // Drop into optional functions here, or fall through to perform emulation
  //
  
#if FN_KEYBOARD_TEST
  keyboard_test();
#endif
  
    // Test the slot lines?
#if SLOT_TEST
    while(1)
      {
	latch2_set_mask(SLOT_TEST_MASK);
	latch2_set_mask(SLOT_CLEAR_MASK);
      }
    
#endif

#if PACK_TEST

    volatile u_int8_t portbytes[20];
    
    while(1)
      {
	// Read a byte from a pack

	// Inputs for ports
	port2_ddr(0x00);    // Port 2 inputs
	port6_ddr(0x00);    // Port 6 inputs

	write_port6(0x74);   // Set up port 6 for later
	port6_ddr(0x80);     // Power up 5V supply
	sleep_ms(50);        // Wait for power to stabilise.

	port6_ddr(0xFF);      // Port 6 outputs
	write_port6(0x76);    // SMR set
	write_port6(0x7e);    // SOE set

	port2_ddr(0xFF);      // Port 2 outputs
	write_port2(0x00);    // Write segmnt register

	write_port6(0x7a);    // PGM clear
	write_port6(0x5a);    // SS2 clear
	write_port6(0x7a);    // SS2 set

	write_port6(0x72);    // SOE clear
	write_port6(0x70);    // SMR clear
	write_port6(0x74);    // PGM set

	port2_ddr(0x00);      // Port 2 inputs
	write_port6(0x54);    // SS2 clear

	int j = 0;
	for(int i = 0; i<20; i++)
	  {
	    portbytes[j++] = read_port2();
	    write_port6(0x55);              // Clock data
	    portbytes[j++] = read_port2();
	    write_port6(0x54);
	  }
	port6_ddr(0x00);
      }
    
#endif

#if TEST_PORT2
    gpio_init(PIN_SD0);
    
    while(1)
      {
#if 0
	gpio_set_dir(PIN_SD0, GPIO_OUT);
	gpio_put(PIN_SD0, 1);
	gpio_put(PIN_SD0, 0);
#endif
#if 1	
	port2_ddr(0xFF);
	gpio_set_dir(PIN_SD0, GPIO_OUT);
	write_port2(0xFF);

	port2_ddr(0x00);
	read_port2();
	
	port2_ddr(0xFF);
	gpio_set_dir(PIN_SD0, GPIO_OUT);
	write_port2(0x00);

	port2_ddr(0x00);
	read_port2();
#endif
      }
#endif


#if SLOT_TEST_G
    while(1)
      {
	gpio_put(SLOT_TEST_GPIO, 0);
	gpio_put(SLOT_TEST_GPIO, 1);
	
      }
#endif

#if BUZZER_TEST
    int d = 0;
    while(1)
      {


	for(int i=0; i<10; i++)
	  {
	    latch2_set_mask( LAT2PIN_MASK_BUZZER);
	    sleep_us(d);
	    latch2_clear_mask( LAT2PIN_MASK_BUZZER);
	    sleep_us(d);
	  }
	d+=1;
	if( d > 200 )
	  {
	    d = 0;
	  }
      }
    
#endif
    
#if RTC_TEST

    printxy_str(0, 0, "RTC Test");

    
    while(1)
      {
	int s, m, h;

	// Set the clock running
	rtc_set_st = 1;

	read_seconds = 1;
	s = rtc_seconds;
	printxy_hex(7, 1, s);

	read_minutes = 1;
	m = rtc_minutes;
	printxy_hex(4, 1, m);

	read_hours = 1;
	h = rtc_hours;
	printxy_hex(1, 1, h);
      }
    
#endif

#if TRACE_ADDR
    // Trace a number of execution addresses
    int tracing            = 0;
    u_int16_t trigger_addr = 0x80c8;
    int addr_trace_i       = 0;

    // Trace from a trigger address until trace full
    volatile u_int16_t addr_trace_from[NUM_ADDR_TRACE];

    // Trace continuously until trigger address seen
    u_int16_t trace_stop_addr = 0x7fba;
    volatile u_int16_t addr_trace_to[NUM_ADDR_TRACE];
    int addr_trace_to_i       = 0;
    int tracing_to            = 1;
#endif

    // Before we start the emulator and sit in the emulation loop we
    // scan the keyboard for a while. This allows entry into the
    // meta menu for things like dump recovery and forced cold
    // start
    
    for(int i=0; i<NUM_SCAN_STATES*80; i++)
      {
	scan_keys();
      }

    // If any key is pressed then we enter the menu
    if( gotkey )
      {
	// get core 1 to run the menu
	core1_in_menu = 1;
	
	// Wait for the core to exit the menu before we continue
	while(core1_in_menu)
	  {
	  }
      }
    
    // Main loop
    while(1)
      {
#if TRACE_ADDR
	// Trace from when we see trigger address
	if( REG_PC == trigger_addr )
	  {
	    tracing = 1;
	  }
	
	if( (addr_trace_i < NUM_ADDR_TRACE) && tracing )
	  {
	    addr_trace_from[addr_trace_i++] = REG_PC;
	  }

	// Trace continuously until we see the stop address
	if( REG_PC == trace_stop_addr )
	  {
	    tracing_to = 0;
	  }
	
	if( tracing_to )
	  {
	    addr_trace_to[addr_trace_to_i++] = REG_PC;
	    addr_trace_to_i %= NUM_ADDR_TRACE;
	  }
#endif
	
#if DISABLE_AUTO_OFF
	// Continuously reset the auto off flag
	if( (time_us_64() % 20000000) == 0 )
	  {
	    ramdata[0x007c] = 0;
	  }
#endif
	loop_emulator();
	//dump_lcd();
	
	//wireless_taskloop();
	//wireless_loop();
      }
    
}

