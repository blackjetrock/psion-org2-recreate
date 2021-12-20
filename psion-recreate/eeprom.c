////////////////////////////////////////////////////////////////////////////////
//
// Serial EEPROM
//
////////////////////////////////////////////////////////////////////////////////
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"

#include "psion_recreate.h"

// Read a block of data from the EEPROM
// 
// Returns:
// 1 for success
// 0 for failure
//

int read_eeprom(int slave_addr, int start, int len, BYTE *dest)
{
  BYTE a[2];

  // Force slave address to have correct RDWR bit
  slave_addr |= 1;
  
  a[0] = start >> 8;
  a[1] = start & 0xFF;
  
  // Set up the write address
  i2c_send_bytes(slave_addr, 2, a);
  
  return(i2c_read_bytes(slave_addr, len, dest));
}

int write_eeprom(int slave_addr, int start, int len, BYTE *src)
{
  BYTE a[2];
  int i;

  // Force slave address to have correct RDWR bit
  slave_addr &= 0xFE;
  
  a[0] = start >> 8;
  a[1] = start & 0xFF;

  i2c_start();

  // Send slave address with write bit
  i2c_send_byte(slave_addr);

  // Address
  i2c_send_byte(a[0]);
  i2c_send_byte(a[1]);

  // Write the data
    for (i = 0; i < len; i++)
    {
      i2c_send_byte(*(src++));
    }

  i2c_stop();
}


////////////////////////////////////////////////////////////////////////////////
//
// test mode
//
#define TEST_LEN 4
#define TEST_START 0x1000

void eeprom_test(void)
{
  BYTE data[TEST_LEN];
  int i;

  //DEBUG_STOP;
  
  for(i=0; i<TEST_LEN; i++)
    {
      data[i] = 10+i;
    }

  printxy_str(0,0,"EEPROM Test");
  
  // Write some bytes to an eeprom, then read it back
  write_eeprom(EEPROM_1_ADDR_WR, TEST_START, TEST_LEN, data);

  for(i=0; i<TEST_LEN; i++)
    {
      printxy_hex(i*3, 1, data[i]);
    }

    for(i=0; i<TEST_LEN; i++)
    {
      data[i] = 0xAA;
    }

  read_eeprom(EEPROM_1_ADDR_RD, TEST_START, TEST_LEN, data);
  
  for(i=0; i<TEST_LEN; i++)
    {
      printxy_hex(i*3, 2, data[i]);
    }

  while(1)
    {
    }
}
