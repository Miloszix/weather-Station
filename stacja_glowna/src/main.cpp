#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WiFi.h>
#include "sensor.h"
#include "displayBMP.h"
#include "bitmaps.h"

// Definicje pinów
#define I2C_SDA 27
#define I2C_SCL 26
#define CE_PIN 5
#define CS_PIN 21
#define RESET_PIN 22
#define DC_PIN 23
#define CNS_PIN 16
#define SPI_MISO 19
#define SPI_MOSI 17
#define SPI_SCK 18
#define IRQ 4

uint16_t tm[7200] = {0};
uint16_t dt[4800] = {0};
uint16_t st1[9100] = {0};
uint16_t st2[9100] = {0};

// Dane Wi-Fi
const char *ssid = "Moto";
const char *password = "123456780";

// NTP i RTC
const char *timeZone = "CET-1CEST,M3.5.0/2,M10.5.0/3";
const char *ntpServer = "pool.ntp.org";

// Obiekty i flagi
RTC_DS3231 rtc;
RF24 radio(CE_PIN, CNS_PIN);
const byte address[6] = "00001";

hw_timer_t *timerReadSensor = NULL;
hw_timer_t *timerDispaly = NULL;
volatile bool dataReceived = false;
volatile bool readSensors = false;
volatile bool displayTime = false;

bool wifiConnected = false;    // Flaga Wi-Fi
bool rtcInitialized = false;   // Flaga RTC
bool radioInitialized = false; // Flaga RF24

float temperature;
float humidity;
float pressure;

// Funkcje ISR
void IRAM_ATTR DataReceived()
{
  dataReceived = true;
}

void IRAM_ATTR onTimerRead()
{
  readSensors = true;
}

void IRAM_ATTR onTimerDisplay()
{
  displayTime = true;
}

// Funkcja synchronizacji RTC z NTP
void syncRTCWithNTP()
{
  configTime(0, 0, ntpServer);
  setenv("TZ", timeZone, 1);
  tzset();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Błąd: Nie udało się pobrać czasu z serwera NTP");
    return;
  }

  rtc.adjust(DateTime(
      timeinfo.tm_year + 1900,
      timeinfo.tm_mon + 1,
      timeinfo.tm_mday,
      timeinfo.tm_hour,
      timeinfo.tm_min,
      timeinfo.tm_sec));

  Serial.println("Czas pobrany z NTP:");
  Serial.printf("%04d-%02d-%02d %02d:%02d:%02d\n",
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec);
}

void setup()
{
  Serial.begin(115200);

  // Inicjalizacja SPI i I2C
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  Wire.begin(I2C_SDA, I2C_SCL);

  // Inicjalizacja czujników
  initBME280();

  // Inicjalizacja rf
  if (radio.begin()){
    radioInitialized = true;
    radio.maskIRQ(1, 1, 0);
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_250KBPS);
    radio.startListening();
    pinMode(IRQ, INPUT_PULLUP);
    attachInterrupt(IRQ, DataReceived, FALLING);
    Serial.println("Moduł RF24 działa.");
  }
  else{
    Serial.println("Błąd: Nie udało się zainicjalizować modułu RF24!");
  }

  // Timery
  timerReadSensor = timerBegin(1, 80, true);
  timerAttachInterrupt(timerReadSensor, &onTimerRead, true);
  timerAlarmWrite(timerReadSensor, 5000000, true);
  timerAlarmEnable(timerReadSensor);

  timerDispaly = timerBegin(2, 80, true);
  timerAttachInterrupt(timerDispaly, &onTimerDisplay, true);
  timerAlarmWrite(timerDispaly, 1000000, true);
  timerAlarmEnable(timerDispaly);

  //inicjalizacja wyświetlacza
  tft.init();
  tft.setRotation(2); // Ustawienie orientacji ekranu
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(0, 0);

  // Inicjalizacja Wi-Fi
  WiFi.begin(ssid, password);
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 10)
  {
    delay(1000);
    Serial.println("Łączenie z Wi-Fi...");
    wifiAttempts++;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    wifiConnected = true;
    Serial.println("Połączono z Wi-Fi!");
    tft.println("Polaczono z Wi-Fi!");
    Serial.print("Adres IP: ");
    tft.println("Adres IP: ");
    Serial.println(WiFi.localIP());
    tft.println(WiFi.localIP());
    delay(1000);
  }
  else
  {
    Serial.println("Błąd: Nie udało się połączyć z Wi-Fi.");
  }

  // Inicjalizacja RTC
  if (rtc.begin())
  {
    rtcInitialized = true;
    if (rtc.lostPower())
    {
      Serial.println("RTC stracił zasilanie");
    }
    else
    {
      Serial.println("RTC DS3231 działa poprawnie.");
    }
    syncRTCWithNTP();
  }
  else
  {
    Serial.println("Błąd: Nie wykryto modułu RTC DS3231!");
  }

  // Inicjalizacja SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Wyświetl obraz na ekranie
  drawBMP("/background.bmp", 0, 0); // Rysowanie obrazu od współrzędnych (0, 0)
  tft.readRect(0, 20, 240, 30, tm);
  tft.readRect(50, 190, 70, 130, st1);
  tft.readRect(170, 190, 70, 130, st2);
  tft.readRect(0, 50, 240, 20, dt);
  // Inicjalizacja RF24
  
}

//dane z drugiego urządzenia
struct SensorData
{
  float temperature;
  float pressure;
  float humidity;
};

SensorData sensorData;

void loop(){
  // Obsługa odbioru danych RF24
  if (radioInitialized && dataReceived){
    dataReceived = false;
    radio.read(&sensorData, sizeof(sensorData));
  }

  // Odczyt czujników
  if (readSensors){
    readSensors = false;
    readRawData();
    humidity = getHum();
    temperature = getTemp();
    pressure = getPress();

    Serial.println("Podstacja:");
    Serial.printf("Aktualna temperatura stacji: %.1f°C\n", sensorData.temperature);
    Serial.printf("Aktualna wilgotność stacji: %.1f%%\n", sensorData.humidity);
    Serial.printf("Aktualne ciśnienie stacji: %.1fHpa\n", sensorData.pressure);

    Serial.println("Stacja główna:");
    Serial.printf("Aktualna temperatura: %.1f°C\n", temperature);
    Serial.printf("Aktualna wilgotność: %.1f%%\n", humidity);
    Serial.printf("Aktualne ciśnienie: %.1fHpa\n", pressure);

    if (rtcInitialized){
      DateTime now = rtc.now();
      Serial.printf("Czas z DS3231: %04d-%02d-%02d %02d:%02d:%02d\n",
                    now.year(),
                    now.month(),
                    now.day(),
                    now.hour(),
                    now.minute(),
                    now.second());
    }
  }

  if (displayTime){
    displayTime = false; // Zresetowanie flagi

    // Wyświetlanie godziny
    tft.setTextColor(TFT_BLACK); // Kolor tekstu (czarny)
    tft.setTextSize(3);          // Rozmiar tekstu dla godziny
    DateTime now = rtc.now();

    char timeBuffer[10];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d",
             now.hour(),
             now.minute(),
             now.second());

    // Wyśrodkowanie godziny na górze
    tft.setCursor((tft.width() - (tft.textWidth(timeBuffer))) / 2, 20);
    tft.pushRect(0, 20, 240, 30, tm);
    tft.print(timeBuffer); // Wyświetlenie godziny

    // Wyświetlanie daty pod godziną
    tft.setTextSize(2); // Rozmiar tekstu dla daty
    char dateBuffer[20];
    snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d",
             now.year(),
             now.month(),
             now.day());
    // Wyśrodkowanie daty poniżej godziny
    tft.setCursor((tft.width() - (tft.textWidth(dateBuffer))) / 2, 50);
    tft.pushRect(0, 50, 240, 20, dt);
    tft.print(dateBuffer); // Wyświetlenie daty

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 160);
    tft.print("Ta stacja");

    tft.setCursor(130, 160);
    tft.print("Druga stacja");

    //wyświetlenie ikonek
    tft.drawBitmap(10, 180, bit_temp, 32, 32, TFT_WHITE);
    tft.drawBitmap(130, 180, bit_temp, 32, 32, TFT_WHITE);
    tft.drawBitmap(10, 220, bit_press, 32, 32, TFT_WHITE);
    tft.drawBitmap(130, 220, bit_press, 32, 32, TFT_WHITE);
    tft.drawBitmap(10, 260, bit_hum, 32, 32, TFT_WHITE);
    tft.drawBitmap(130, 260, bit_hum, 32, 32, TFT_WHITE);

    //wyświetlanie wartości
    tft.setTextSize(2);
    tft.pushRect(50, 190, 70, 130, st1);
    tft.setCursor(50, 190);
    tft.printf("%.1fC", temperature);
    tft.setCursor(50, 230);
    tft.printf("%.1f%%", humidity);
    tft.setCursor(50, 260);
    tft.printf("%.1f", pressure);
    tft.setCursor(50, 280);
    tft.print("Hpa");

    tft.pushRect(170, 190, 70, 130, st2);
    tft.setCursor(170, 190);
    tft.printf("%.1fC", sensorData.temperature);
    tft.setCursor(170, 230);
    tft.printf("%.1f%%", sensorData.humidity);
    tft.setCursor(170, 260);
    tft.printf("%.1f", sensorData.pressure);
    tft.setCursor(170, 280);
    tft.print("Hpa");
  }

  // Synchronizacja RTC co jakiś czas, jeśli Wi-Fi działa
  static unsigned long lastSync = 0;
  if (wifiConnected && millis() - lastSync > (24 * 3600000)){ // Co godzinę
    lastSync = millis();
    syncRTCWithNTP();
  }
}
