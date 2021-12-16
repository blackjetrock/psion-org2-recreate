#define uint unsigned int
#define uchar unsigned char

#define DEBUG_STOP {volatile int x = 1; while (x) {} }


#define FN_OLED_DEMO      0
#define FN_KEYBOARD_TEST  0
#define FN_FLASH_LED      0
#define SLOT_TEST         0
#define SLOT_TEST_MASK    LAT2PIN_MASK_SS2
#define SLOT_TEST_G       0
#define SLOT_TEST_GPIO    PIN_SD0
#define TEST_PORT2        0
#define PACK_TEST         0
#define WIFI_TEST         0
#define RTC_TEST          0
#define NEW_I2C           1

typedef u_int8_t BYTE;

// Do we use two cores?
// If yes then the second core handles:
//    Display update

#define MULTI_CORE        1

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

extern const uint PIN_SDAOUT;
extern const uint PIN_LATCHOUT2;
extern const uint PIN_I2C_SDA;
extern const uint PIN_I2C_SCL;
extern const uint PIN_LS_DIR;
extern const uint PIN_LATCHIN;
extern const uint PIN_SCLKIN;
extern const uint PIN_SDAIN;
extern const uint PIN_LATCHOUT1;
extern const uint PIN_SCLKOUT;
extern const uint PIN_VBAT_SW_ON;

extern uint16_t latchout1_shadow;
extern uint16_t latch2_shadow;

void latch2_set_mask(int value);
void latch2_clear_mask(int value);

uint8_t read_165(const uint latchpin);
void write_595(const uint latchpin, int value, int n);

// Latch pins

// Pseudo pins, code needed to handle them
#define PSEUDO_PIN             100

#define PIN_5V_ON              100
#define PIN_SS1                101
#define PIN_SS2                102
#define PIN_SS3                103
#define PIN_SPGM               104

#define LAT1PIN_MASK_K1        0x01
#define LAT1PIN_MASK_K2        0x02
#define LAT1PIN_MASK_K3        0x04
#define LAT1PIN_MASK_K4        0x08
#define LAT1PIN_MASK_K5        0x10
#define LAT1PIN_MASK_K6        0x20
#define LAT1PIN_MASK_K7        0x40
#define LAT1PIN_MASK_OLED_RES  0x80

#define LAT2PIN_MASK_DRV_HV    0x0001
#define LAT2PIN_MASK_ESP_ON    0x0002
#define LAT2PIN_MASK_BUZZER    0x0004
#define LAT2PIN_MASK_5V_ON     0x0008
#define LAT2PIN_MASK_P_SPGM    0x0010
#define LAT2PIN_MASK_VBSW_ON   0x0020
#define LAT2PIN_MASK_SD_OE     0x0040
#define LAT2PIN_MASK_SC_OE     0x0080

#define LAT2PIN_MASK_SS1       0x0800
#define LAT2PIN_MASK_SS2       0x1000
#define LAT2PIN_MASK_SS3       0x2000

#define MAX_DDRAM 0xFF
#define MAX_CGRAM (5*16)
#define DISPLAY_NUM_LINES   4
#define DISPLAY_NUM_CHARS  21
#define DISPLAY_NUM_EXTRA   4

extern unsigned char font_5x7_letters[];
extern char lcd_display_buffer[MAX_DDRAM+2];
extern char lcd_display[MAX_DDRAM+2];;
extern char display_line[DISPLAY_NUM_LINES][DISPLAY_NUM_CHARS+1];
void _nop_(void);

void put_display_char(int x,int y, int ch);

void write_port2(u_int8_t value);
u_int8_t read_port2(void);

void printxy(int x, int y, int ch);
void printxy_str(int x, int y, char *str);
void printxy_hex(int x, int y, int value);

void i_printxy(int x, int y, int ch);
void i_printxy_str(int x, int y, char *str);

void write_display_extra(int i, int ch);

void wireless_init(void);
void wireless_loop(void);
void wireless_taskloop(void);

////////////////////////////////////////////////////////////////////////////////
//
// MCP7940 Registers
extern int read_seconds;
extern int read_minutes;
extern int read_hours;

extern int rtc_seconds;
extern int rtc_minutes;
extern int rtc_hours;

#define MCP_RTCSEC_REG   0x00
#define MCP_ST_MASK      0x80

#define MCP_RTCMIN_REG   0x01

#define MCP_RTCHOUR_REG  0x02

#define MCP_RTCWKDAY_REG 0x03
#define MCP_VBATEN_MASK  0x08

#define MCP_RTCDATE_REG  0x04

#define MCP_RTCMTH_REG   0x05

#define MCP_RTCYEAR_REG  0x06

////////////////////////////////////////////////////////////////////////////////

void set_st_bit();
void set_vbaten_bit();
void write_mcp7940(int r, int n, BYTE data[]);
int read_mcp7940(int r);
void Delay1(uint n);
void Write_number(uchar *n,uchar k,uchar station_dot);
void display_Contrast_level(uchar number);
void adj_Contrast(void);
void Delay(uint n);
void Set_Page_Address(unsigned char add);
void Set_Column_Address(unsigned char add);
void Set_Contrast_Control_Register(unsigned char mod);
void initialise_oled(void);
void Display_Chess(unsigned char value);
void Display_Chinese(unsigned char ft[]);
void Display_Chinese_Column(unsigned char ft[]);
void Display_Picture(unsigned char pic[]);
void SentByte(unsigned char Byte);
void Check_Ack(void);//Acknowledge
void Stop(void);
void Start(void);
void Send_ACK(void);
unsigned char ReceiveByte(void);
void clear_oled(void);

void oledmain(void);


void dump_lcd(void);

// Core 1
void core1_main(void);
  
// Emulator
void initialise_emulator(void);
void loop_emulator(void);

// RTC tasks
void rtc_tasks(void);
extern int rtc_set_st;

#define RAM_SIZE 65536
#define ROM_SIZE (sizeof(romdata))
#define ROM_START (0x8000)

extern u_int8_t ramdata[RAM_SIZE];

// I2C functions
void i2c_release(void);
void i2c_delay(void);
void i2c_sda_low(void);
void i2c_sda_high(void);
void i2c_scl_low(void);
void i2c_scl_high(void);
void i2c_start(void);
void i2c_stop(void);
int i2c_send_byte(BYTE b);
int i2c_read_bytes(BYTE slave_addr, int n, BYTE *data);
void i2c_send_bytes(BYTE slave_addr, int n, BYTE *data);
