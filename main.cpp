#include <TFT_eSPI.h>

#define TFT_GREY 0x5AEB // New colour

uint32_t readRegister(TFT_eSPI& tft, uint8_t reg, int16_t bytes, uint8_t index)
{
  uint32_t  data = 0;

  while (bytes > 0) {
    bytes--;
    data = (data << 8) | tft.readcommand8(reg, index);
    index++;
  }

  printf("Register 0x%02X: 0x%08X\n", reg, data);

  return data;
}

void debug(const std::string& s)
{
    printf("%s\n", s.c_str());
    delay(100);
}

int main()
{
    TFT_eSPI tft = TFT_eSPI();  // Invoke library

    tft.init();

    tft.setRotation(2);

    while (1)
    {
        readRegister(tft, ILI9341_RDDID, 3, 1);
        readRegister(tft, ILI9341_RDDST, 4, 1);
        readRegister(tft, ILI9341_RDMODE, 1, 1);
        readRegister(tft, ILI9341_RDMADCTL, 1, 1);
        readRegister(tft, ILI9341_RDPIXFMT, 1, 1);
        readRegister(tft, ILI9341_RDSELFDIAG, 1, 1);
        readRegister(tft, ILI9341_RAMRD, 3, 1);

        readRegister(tft, ILI9341_RDID1, 1, 1);
        readRegister(tft, ILI9341_RDID2, 1, 1);
        readRegister(tft, ILI9341_RDID3, 1, 1);
        readRegister(tft, ILI9341_RDIDX, 1, 1); // ?
        readRegister(tft, ILI9341_RDID4, 3, 1);  // ID

        debug("fill");
        // Fill screen with grey so we can see the effect of printing with and without 
        // a background colour defined
        tft.fillScreen(TFT_GREY);

        debug("hello");
        
        // Set "cursor" at top left corner of display (0,0) and select font 2
        // (cursor will move to next line automatically during printing with 'tft.println'
        //  or stay on the line is there is room for the text with tft.print)
        tft.setCursor(0, 0, 2);
        // Set the font colour to be white with a black background, set text size multiplier to 1
        tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
        // We can now plot text on screen using the "print" class
        tft.println("Hello World!");
#if 0
        debug("number");

        // Set the font colour to be yellow with no background, set to font 7
        tft.setTextColor(TFT_YELLOW); tft.setTextFont(7);
        tft.println(1234.56);
  
        // Set the font colour to be red with black background, set to font 4
        tft.setTextColor(TFT_RED,TFT_BLACK);    tft.setTextFont(4);
        //tft.println(3735928559L, HEX); // Should print DEADBEEF

        debug("groop");

        // Set the font colour to be green with black background, set to font 4
        tft.setTextColor(TFT_GREEN,TFT_BLACK);
        tft.setTextFont(4);
        tft.println("Groop");
        tft.println("I implore thee,");

        // Change to font 2
        tft.setTextFont(2);
        tft.println("my foonting turlingdromes.");
        tft.println("And hooptiously drangle me");
        tft.println("with crinkly bindlewurdles,");
        // This next line is deliberately made too long for the display width to test
        // automatic text wrapping onto the next line
        tft.println("Or I will rend thee in the gobberwarts with my blurglecruncheon, see if I don't!");
#endif  
        delay(10000);
    }
}
