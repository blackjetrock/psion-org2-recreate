
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
//
// RTC support for MCP7940

#define ADDR_MCP7940    0xdf
#define MCP_READ_ADDR   (ADDR_MCP7940  | 0x01)
#define MCP_WRITE_ADDR  (ADDR_MCP7940 & 0xFE)



////////////////////////////////////////////////////////////////////////////////
//
//
// Read a register from the RTC
// 

int read_mcp7940(int r)
{
  int b;
  
  Start();
  
  SentByte(MCP_WRITE_ADDR);
  SentByte(r);
  Stop();
  
  Start();
  SentByte(MCP_READ_ADDR);  
  b = ReceiveByte();
  Stop();

  return(b);
}

// Write a register to the RTC
void write_mcp7940(int r, int n, int data[])
{
  Start();

  SentByte(MCP_WRITE_ADDR);
  SentByte(r);

  for(int i=0; i<n; i++)
    {
      SentByte(data[i]);      
    }

  Stop();
}

// Sets the VBATEN bit
void set_vbaten_bit()
{
  int reg0[1];

  reg0[0] = read_mcp7940(MCP_RTCWKDAY_REG);
  reg0[0] |= MCP_VBATEN_MASK;

  write_mcp7940(MCP_RTCWKDAY_REG, 1, reg0);
}

// Sets the ST bit
void set_st_bit()
{
  int reg0[1];

  reg0[0] = read_mcp7940(MCP_RTCSEC_REG);
  reg0[0] |= MCP_ST_MASK;

  write_mcp7940(MCP_RTCSEC_REG, 1, reg0);

}


////////////////////////////////////////////////////////////////////////////////
//
//  RTC tasks concerning the I2C bus are done here.
// 

int read_seconds = 0;
int read_minutes = 0;
int read_hours = 0;
int rtc_set_st = 0;

int rtc_seconds = 0;
int rtc_minutes = 0;
int rtc_hours = 0;

void rtc_tasks(void)
{
  if( rtc_set_st )
    {
      set_st_bit();
      rtc_set_st = 0;
    }
  
  if( read_seconds )
    {
      rtc_seconds = read_mcp7940( MCP_RTCSEC_REG);
      read_seconds = 0;
    }

  if( read_minutes )
    {
      rtc_minutes = read_mcp7940( MCP_RTCMIN_REG);
      read_minutes = 0;
    }

  if( read_hours )
    {
      rtc_hours = read_mcp7940( MCP_RTCHOUR_REG);
      read_hours = 0;
    }
}
