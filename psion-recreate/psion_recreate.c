#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

#include "psion_recreate.h"

////////////////////////////////////////////////////////////////////////////////

#define FN_OLED_DEMO      0
#define FN_KEYBOARD_TEST  0
#define FN_FLASH_LED      0
#define SLOT_TEST         0
#define SLOT_TEST_MASK    LAT2PIN_MASK_SS2
#define SLOT_TEST_G       0
#define SLOT_TEST_GPIO    PIN_SD0
#define TEST_PORT2        0
#define PACK_TEST         0


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

uint16_t latch2_shadow = 0;
uint16_t latchout1_shadow = 0;

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

void oledmain(void);

////////////////////////////////////////////////////////////////////////////////
//
//


int main() {

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
  
  gpio_init(PIN_I2C_SCL);
  gpio_init(PIN_I2C_SDA);
  
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
  latch2_set_mask(LAT2PIN_MASK_DRV_HV);
  
  //  latchout2_shadow |= LAT2PIN_MASK_DRV_HV;
  //write_595(PIN_LATCHOUT2, latchout2_shadow, 16);

  // Wait for it to start up
  sleep_ms(10);
  initialise_oled();
  
  // Clear screen
  clear_oled();
  stdio_init_all();
  
  //------------------------------------------------------------------------------
  //
  // Drop into optional functions here, or fall through to perform emulation
  //
  
#if FN_KEYBOARD_TEST
  keyboard_test();
#endif
  
  //------------------------------------------------------------------------------

  
#if 0
  #if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/lcd_1602_i2c example requires a board with I2C pins
#else
    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
#endif
#endif

    // Initialise emulator
    initialise_emulator();

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

	  //         SCLK           =0
	  //         SMR            =1
	  //         SPGM           =2
	  //         SOE_B          =3
	  //         SS1_B          =4
	  //         SS2_B          =5
	  //         SS3_B          =6
	  //         PACON_B        =7

    
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
    
    // Main loop
    while(1)
      {
	loop_emulator();	
      }
    
}

