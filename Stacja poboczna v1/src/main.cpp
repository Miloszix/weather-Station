#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "sensor.h"
#include "bitmaps.h" // Plik z Twoimi bitmapami
#include <nRF24L01.h>
#include <RF24.h>

// Parametry wyświetlacza
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset pin (nie używany z ESP32)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define I2C_SDA 13
#define I2C_SCL 14
#define SPI_MISO 25
#define SPI_MOSI 27
#define SPI_SCK 33
#define CE 32
#define CSN 26

RF24 radio(CE, CSN);
const byte address[6] = "00001";

hw_timer_t *timerDisplay = NULL; // Timer do zmiany wyświetlacza
hw_timer_t *timerReadSensor = NULL;    // Timer do odczytu danych

volatile bool updateDisplay = false;
volatile bool readSensors = false;

volatile int currentDisplay = 0;

void IRAM_ATTR onTimerRead() {
    readSensors = true; // Ustaw flagę do odczytu danych
}

void IRAM_ATTR onTimerDisplay() {
    currentDisplay = (currentDisplay + 1) % 3; // Zmieniaj ekran cyklicznie
    updateDisplay = true; // Ustaw flagę do aktualizacji
}

void setup()
{
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);
  initBME280();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x78 >> 1))
  {
    Serial.println(F("SSD1306 allocation failed"));
  }

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();

  // Inicjalizacja drugiego timera (odczyt danych, co 5 sekund)
  timerReadSensor = timerBegin(1, 80, true);                 // Timer 1, preskaler 80 (1 tick = 1 µs)
  timerAttachInterrupt(timerReadSensor, &onTimerRead, true); // ISR do odczytu danych
  timerAlarmWrite(timerReadSensor, 5000000, true);           // Alarm co 5 sekund
  timerAlarmEnable(timerReadSensor);                         // Włącz timer

  timerDisplay = timerBegin(0, 80, true);                    // Timer 0, preskaler 80 (1 tick = 1 µs)
  timerAttachInterrupt(timerDisplay, &onTimerDisplay, true); // ISR do zmiany wyświetlacza
  timerAlarmWrite(timerDisplay, 3000000, true);              // Alarm co 3 sekundy
  timerAlarmEnable(timerDisplay);                            // Włącz timer
}

void showTemp(float temperature)
{
  display.clearDisplay();
  display.drawBitmap(8, 8, bit_temp, 48, 48, SSD1306_WHITE);
  display.setCursor(64, 15);
  display.print(temperature);
  display.setCursor(80, 35);
  display.printf("%cC");
  display.display();
}

void showHum(float humidity)
{
  display.clearDisplay();
  display.drawBitmap(8, 8, bit_hum, 48, 48, SSD1306_WHITE);
  display.setCursor(64, 15); // Ustawienie kursora na odpowiednie miejsce
  display.print(humidity);
  display.setCursor(85, 35);
  display.printf("%%");
  display.display(); // Aktualizacja wyświetlacza
}

void showPress(float pressure)
{
  display.clearDisplay();
  display.drawBitmap(8, 8, bit_press, 48, 48, SSD1306_WHITE);
  display.setCursor(80, 15);
  display.printf("%0.f", pressure);
  display.setCursor(80, 35);
  display.printf("hpa");
  display.display();
}

struct SensorData {
    float temperature;
    float pressure;
    float humidity;
};

SensorData sensorData; // Obiekt do przechowywania danych

void loop()
{
  if (readSensors) {
    readSensors = false; // Zresetuj flagę
    readRawData();
    sensorData.temperature = getTemp();
    sensorData.pressure = getPress();
    sensorData.humidity = getHum();
    radio.write(&sensorData, sizeof(sensorData));
  }

  if (updateDisplay)
  {
    updateDisplay = false; // Zresetuj flagę

    switch (currentDisplay)
    {
    case 0:
      showTemp(sensorData.temperature);
      break;
    case 1:
      showPress(sensorData.pressure);
      break;
    case 2:
      showHum(sensorData.humidity);
      break;
    }
  }
}