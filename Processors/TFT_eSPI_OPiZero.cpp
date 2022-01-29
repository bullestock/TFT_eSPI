////////////////////////////////////////////////////
//    TFT_eSPI Orange Pi Zero driver functions    //
////////////////////////////////////////////////////

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void digitalWrite(int pin, int value)
{
    char s[32];
    sprintf(s, "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(s, O_WRONLY);
    if (fd == -1) {
        int err = errno;
        printf("Unable to open %s: %s", s, strerror(err));
        exit(1);
    }

    if (write(fd, value ? "1" : "0", 1) != 1) {
        int err = errno;
        printf("Error writing to %s: %s", s, strerror(err));
        exit(1);
    }
}

int digitalRead(int pin)
{
    char s[32];
    sprintf(s, "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(s, O_WRONLY);
    if (fd == -1) {
        int err = errno;
        printf("Unable to open %s: %s", s, strerror(err));
        exit(1);
    }

    char c;
    if (read(fd, &c, 1) != 1) {
        int err = errno;
        printf("Error reading from %s: %s", s, strerror(err));
        exit(1);
    }
    if (c != '0' && c != '1')
    {
        printf("Bad input value from %s: %d", s, (int) c);
        exit(1);
    }
    return c == '1';
}

////////////////////////////////////////////////////////////////////////////////////////
#if defined (TFT_SDA_READ) && !defined (TFT_PARALLEL_8_BIT)
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           tft_Read_8
** Description:             Bit bashed SPI to read bidirectional SDA line
***************************************************************************************/
uint8_t TFT_eSPI::tft_Read_8(void)
{
  uint8_t  ret = 0;

  for (uint8_t i = 0; i < 8; i++) {  // read results
    ret <<= 1;
    SCLK_L;
    if (digitalRead(TFT_MOSI)) ret |= 1;
    SCLK_H;
  }

  return ret;
}

/***************************************************************************************
** Function name:           beginSDA
** Description:             Detach SPI from pin to permit software SPI
***************************************************************************************/
void TFT_eSPI::begin_SDA_Read(void)
{
  // Release configured SPI port for SDA read
  spi.end();
}

/***************************************************************************************
** Function name:           endSDA
** Description:             Attach SPI pins after software SPI
***************************************************************************************/
void TFT_eSPI::end_SDA_Read(void)
{
  // Configure SPI port ready for next TFT access
  spi.begin();
}

////////////////////////////////////////////////////////////////////////////////////////
#endif // #if defined (TFT_SDA_READ)
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
#if defined (TFT_PARALLEL_8_BIT) // Code for generic (i.e. any) processor
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           pushBlock - for generic processor and parallel display
** Description:             Write a block of pixels of the same colour
***************************************************************************************/
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len){

  while (len>1) {tft_Write_32D(color); len-=2;}
  if (len) {tft_Write_16(color);}
}

/***************************************************************************************
** Function name:           pushPixels - for gereric processor and parallel display
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len){

  uint16_t *data = (uint16_t*)data_in;
  if(_swapBytes) {
    while (len>1) {tft_Write_16(*data); data++; tft_Write_16(*data); data++; len -=2;}
    if (len) {tft_Write_16(*data);}
    return;
  }

  while (len>1) {tft_Write_16S(*data); data++; tft_Write_16S(*data); data++; len -=2;}
  if (len) {tft_Write_16S(*data);}
}

#define EXPORT_PIN(pin)                                         \
    if (::write(fd, #pin, 2) != 2)                              \
    {                                                           \
        perror("Error writing to /sys/class/gpio/export" #pin); \
        exit(1);                                                \
    }

#define SET_PIN_MODE(pin, mode)                                         \
    fd = open("/sys/class/gpio/gpio" #pin "/direction", O_WRONLY);      \
    if (fd == -1)                                                       \
    {                                                                   \
        perror("Unable to open /sys/class/gpio" #pin "/direction");     \
        exit(1);                                                        \
    }                                                                   \
    if (::write(fd,                                                     \
              mode == direction::input ? "in" : "out",                  \
              mode == direction::input ? 2 : 3) !=                      \
        (mode == direction::input ? 2 : 3))                             \
    {                                                                   \
        perror("Error writing to /sys/class/gpio/" #pin "/direction");  \
        exit(1);                                                        \
    }

/***************************************************************************************
** Function name:           GPIO direction control  - supports class functions
** Description:             Set parallel bus to direction::input or direction::output
***************************************************************************************/
void TFT_eSPI::busDir(uint32_t mask, direction mode)
{
    // mask is unused for generic processor
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1)
    {
        perror("Unable to open /sys/class/gpio/export");
        exit(1);
    }
    EXPORT_PIN(TFT_D0);
    EXPORT_PIN(TFT_D1);
    EXPORT_PIN(TFT_D2);
    EXPORT_PIN(TFT_D3);
    EXPORT_PIN(TFT_D4);
    EXPORT_PIN(TFT_D5);
    EXPORT_PIN(TFT_D6);
    EXPORT_PIN(TFT_D7);
    SET_PIN_MODE(TFT_D0, mode);
    SET_PIN_MODE(TFT_D1, mode);
    SET_PIN_MODE(TFT_D2, mode);
    SET_PIN_MODE(TFT_D3, mode);
    SET_PIN_MODE(TFT_D4, mode);
    SET_PIN_MODE(TFT_D5, mode);
    SET_PIN_MODE(TFT_D6, mode);
    SET_PIN_MODE(TFT_D7, mode);
}

/***************************************************************************************
** Function name:           GPIO direction control  - supports class functions
** Description:             Faster GPIO pin input/output switch
***************************************************************************************/
void TFT_eSPI::gpioMode(uint8_t gpio, uint8_t mode)
{
  // No fast port based generic approach available
}

/***************************************************************************************
** Function name:           read byte  - supports class functions
** Description:             Read a byte - parallel bus only
***************************************************************************************/
uint8_t TFT_eSPI::readByte(void)
{
  uint8_t b = 0;

  busDir(0, direction::input);
  digitalWrite(TFT_RD, 0);

  b |= digitalRead(TFT_D0) << 0;
  b |= digitalRead(TFT_D1) << 1;
  b |= digitalRead(TFT_D2) << 2;
  b |= digitalRead(TFT_D3) << 3;
  b |= digitalRead(TFT_D4) << 4;
  b |= digitalRead(TFT_D5) << 5;
  b |= digitalRead(TFT_D6) << 6;
  b |= digitalRead(TFT_D7) << 7;

  digitalWrite(TFT_RD, 1);
  busDir(0, direction::output); 

  return b;
}

////////////////////////////////////////////////////////////////////////////////////////
#elif defined (RPI_WRITE_STROBE)  // For RPi TFT with write strobe                      
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           pushBlock - for ESP32 or STM32 RPi TFT
** Description:             Write a block of pixels of the same colour
***************************************************************************************/
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len){

  if(len) { tft_Write_16(color); len--; }
  while(len--) {WR_L; WR_H;}
}

/***************************************************************************************
** Function name:           pushPixels - for ESP32 or STM32 RPi TFT
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len)
{
  uint16_t *data = (uint16_t*)data_in;

  if (_swapBytes) while ( len-- ) {tft_Write_16S(*data); data++;}
  else while ( len-- ) {tft_Write_16(*data); data++;}
}

////////////////////////////////////////////////////////////////////////////////////////
#elif defined (SPI_18BIT_DRIVER) // SPI 18 bit colour                         
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           pushBlock - for STM32 and 3 byte RGB display
** Description:             Write a block of pixels of the same colour
***************************************************************************************/
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len)
{
  // Split out the colours
  uint8_t r = (color & 0xF800)>>8;
  uint8_t g = (color & 0x07E0)>>3;
  uint8_t b = (color & 0x001F)<<3;

  while ( len-- ) {tft_Write_8(r); tft_Write_8(g); tft_Write_8(b);}
}

/***************************************************************************************
** Function name:           pushPixels - for STM32 and 3 byte RGB display
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len){

  uint16_t *data = (uint16_t*)data_in;
  if (_swapBytes) {
    while ( len-- ) {
      uint16_t color = *data >> 8 | *data << 8;
      tft_Write_8((color & 0xF800)>>8);
      tft_Write_8((color & 0x07E0)>>3);
      tft_Write_8((color & 0x001F)<<3);
      data++;
    }
  }
  else {
    while ( len-- ) {
      tft_Write_8((*data & 0xF800)>>8);
      tft_Write_8((*data & 0x07E0)>>3);
      tft_Write_8((*data & 0x001F)<<3);
      data++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////
#else //                   Standard SPI 16 bit colour TFT                               
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           pushBlock - for STM32
** Description:             Write a block of pixels of the same colour
***************************************************************************************/
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len){

  while ( len-- ) {tft_Write_16(color);}
}

/***************************************************************************************
** Function name:           pushPixels - for STM32
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len){

  uint16_t *data = (uint16_t*)data_in;

  if (_swapBytes) while ( len-- ) {tft_Write_16(*data); data++;}
  else while ( len-- ) {tft_Write_16S(*data); data++;}
}

////////////////////////////////////////////////////////////////////////////////////////
#endif // End of display interface specific functions
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
//                                DMA FUNCTIONS                                         
////////////////////////////////////////////////////////////////////////////////////////

//                Placeholder for DMA functions

/*
Minimal function set to support DMA:

bool TFT_eSPI::initDMA(void)
void TFT_eSPI::deInitDMA(void)
bool TFT_eSPI::dmaBusy(void)
void TFT_eSPI::pushPixelsDMA(uint16_t* image, uint32_t len)
void TFT_eSPI::pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* image)

*/
