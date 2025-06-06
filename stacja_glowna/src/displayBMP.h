#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

uint16_t read16(fs::File &f)
{
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read(); // MSB
    return result;
}

uint32_t read32(fs::File &f)
{
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read(); // MSB
    return result;
}

void drawBMP(const char *filename, int16_t x, int16_t y)
{

    if ((x >= tft.width()) || (y >= tft.height()))
        return;

    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = SPIFFS.open(filename, "r");

    if (!bmpFS)
    {
        Serial.print("File not found");
        return;
    }

    uint32_t seekOffset;
    uint16_t w, h, row, col;
    uint8_t r, g, b;

    uint32_t startTime = millis();

    if (read16(bmpFS) == 0x4D42)
    {
        read32(bmpFS);
        read32(bmpFS);
        seekOffset = read32(bmpFS);
        read32(bmpFS);
        w = read32(bmpFS);
        h = read32(bmpFS);

        if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
        {
            y += h - 1;

            bool oldSwapBytes = tft.getSwapBytes();
            tft.setSwapBytes(true);
            bmpFS.seek(seekOffset);

            uint16_t padding = (4 - ((w * 3) & 3)) & 3;
            uint8_t lineBuffer[w * 3 + padding];

            for (row = 0; row < h; row++)
            {

                bmpFS.read(lineBuffer, sizeof(lineBuffer));
                uint8_t *bptr = lineBuffer;
                uint16_t *tptr = (uint16_t *)lineBuffer;
                // Convert 24 to 16-bit colours
                for (uint16_t col = 0; col < w; col++)
                {
                    b = *bptr++;
                    g = *bptr++;
                    r = *bptr++;
                    *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }

                // Push the pixel row to screen, pushImage will crop the line if needed
                // y is decremented as the BMP image is drawn bottom up
                tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer);
            }
            tft.setSwapBytes(oldSwapBytes);
            Serial.print("Loaded in ");
            Serial.print(millis() - startTime);
            Serial.println(" ms");
        }
        else
            Serial.println("BMP format not recognized.");
    }
    bmpFS.close();
}

void drawFragment(const char *filename, int16_t start_x, int16_t start_y, int16_t width, int16_t height) {
    if ((start_x >= tft.width()) || (start_y >= tft.height()))
        return;

    fs::File bmpFS;

    // Open requested file on SPIFFS
    bmpFS = SPIFFS.open(filename, "r");

    if (!bmpFS) {
        Serial.print("File not found");
        return;
    }

    uint32_t seekOffset;
    uint16_t w, h, row;
    uint8_t r, g, b;

    uint32_t startTime = millis();

    if (read16(bmpFS) == 0x4D42) { // Check BMP header
        read32(bmpFS);             // Skip file size
        read32(bmpFS);             // Skip reserved fields
        seekOffset = read32(bmpFS);
        read32(bmpFS);             // Skip header size
        w = read32(bmpFS);         // Image width
        h = read32(bmpFS);         // Image height

        if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0)) { // Validate BMP format
            bool oldSwapBytes = tft.getSwapBytes();
            tft.setSwapBytes(true);
            bmpFS.seek(seekOffset);

            uint16_t padding = (4 - ((w * 3) & 3)) & 3; // BMP row padding
            uint8_t lineBuffer[w * 3 + padding];

            // Iterate only over rows within the specified range
            for (row = 0; row < h; row++) {
                int16_t screen_y = h - 1 - row; // Map BMP rows to screen rows
                if (screen_y < start_y || screen_y >= (start_y + height)) {
                    // Skip rows outside the vertical range
                    bmpFS.seek(bmpFS.position() + sizeof(lineBuffer));
                    continue;
                }

                bmpFS.read(lineBuffer, sizeof(lineBuffer));

                uint8_t *bptr = lineBuffer + start_x * 3; // Skip to the start_x column
                uint16_t croppedBuffer[width];

                // Convert only the specified range of columns
                for (uint16_t col = 0; col < width; col++) {
                    b = *bptr++;
                    g = *bptr++;
                    r = *bptr++;
                    croppedBuffer[col] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }

                // Draw the cropped row on screen
                tft.pushImage(start_x, screen_y, width, 1, croppedBuffer);
            }

            tft.setSwapBytes(oldSwapBytes);
            Serial.print("Loaded in ");
            Serial.print(millis() - startTime);
            Serial.println(" ms");
        } else {
            Serial.println("BMP format not recognized.");
        }
    }
    bmpFS.close();
}



