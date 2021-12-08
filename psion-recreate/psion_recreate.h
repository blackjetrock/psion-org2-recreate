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

extern uint8_t latchout1_shadow;
extern uint8_t latchout2_shadow;

uint8_t read_165(const uint latchpin);

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

#define LAT2PIN_MASK_DRV_HV    0x0100
#define LAT2PIN_MASK_ESP_ON    0x0200
#define LAT2PIN_MASK_BUZZER    0x0400
#define LAT2PIN_MASK_5V_ON     0x0800
#define LAT2PIN_MASK_P_SPGM    0x1000
#define LAT2PIN_MASK_VBSW_ON   0x2000
#define LAT2PIN_MASK_SD_OE     0x4000
#define LAT2PIN_MASK_SC_OE     0x8000

#define LAT2PIN_MASK_SS1       0x0008
#define LAT2PIN_MASK_SS2       0x0010
#define LAT2PIN_MASK_SS3       0x0020


