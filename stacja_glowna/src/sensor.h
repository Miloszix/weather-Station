#include <Arduino.h>
#include <Wire.h>

typedef int32_t BME280_S32_t;
typedef int64_t BME280_S64_t;
typedef uint32_t BME280_U32_t;

#define BME280_ADDR 0x76

uint16_t dig_T1;
int16_t dig_T2, dig_T3;
uint16_t dig_P1;
int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
uint8_t dig_H1, dig_H3;
int16_t dig_H2, dig_H4, dig_H5, dig_H6;

float temp;
float press;
float hum;

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(BME280_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// Funkcja do odczytu danych z rejestru
uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(BME280_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDR, 1);
  return Wire.read();
}

void readCalibrationData() {
  dig_T1 = (readRegister(0x89) << 8) | readRegister(0x88);
  dig_T2 = (readRegister(0x8B) << 8) | readRegister(0x8A);
  dig_T3 = (readRegister(0x8D) << 8) | readRegister(0x8C);

  dig_P1 = (readRegister(0x8F) << 8) | readRegister(0x8E);
  dig_P2 = (readRegister(0x91) << 8) | readRegister(0x90);
  dig_P3 = (readRegister(0x93) << 8) | readRegister(0x92);
  dig_P4 = (readRegister(0x95) << 8) | readRegister(0x94);
  dig_P5 = (readRegister(0x97) << 8) | readRegister(0x96);
  dig_P6 = (readRegister(0x99) << 8) | readRegister(0x98);
  dig_P7 = (readRegister(0x9B) << 8) | readRegister(0x9A);
  dig_P8 = (readRegister(0x9D) << 8) | readRegister(0x9C);
  dig_P9 = (readRegister(0x9F) << 8) | readRegister(0x9E);

  dig_H1 = readRegister(0xA1);
  dig_H2 = (readRegister(0xE2) << 8) | readRegister(0xE1);
  dig_H3 = readRegister(0xE3);
  dig_H4 = (readRegister(0xE4) << 4) | (readRegister(0xE5) & 0x0F);
  dig_H5 = (readRegister(0xE6) << 4) | (readRegister(0xE5) >> 4);
  dig_H6 = (int8_t)readRegister(0xE7);
}

void initBME280() {
  // Sprawdź ID czujnika
  uint8_t id = readRegister(0xD0);
  if (id != 0x60) {
    Serial.println("BME280 nie wykryte!");
  }
  else{
    Serial.println("BME280 podłączone poprawnie");
  }
  // Ustawienia konfiguracji
  writeRegister(0xF2, 0b101);  // Oversampling wilgotności x4
  writeRegister(0xF4, 0b01101101);  // Tryb forced, oversampling x4 dla temp i ciśnienia
  writeRegister(0xF5, 0b10101100);  // Konfiguracja filtra i standby (standby jest używany w trybie normalnym wieć nie ma to znaczenia)
  readCalibrationData();
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.   
// t_fine carries fine temperature as global value  
BME280_S32_t t_fine;  
BME280_S32_t BME280_compensate_T_int32( BME280_S32_t adc_T)  
{  
  BME280_S32_t var1, var2, T;  
  var1  = ((((adc_T>>3) - (( BME280_S32_t )dig_T1<<1))) * (( BME280_S32_t )dig_T2)) >> 11;  
  var2  = (((((adc_T>>4) - (( BME280_S32_t )dig_T1)) * ((adc_T>>4) - ((BME280_S32_t )dig_T1))) >> 12) *   
    (( BME280_S32_t )dig_T3)) >>  14;  
  t_fine = var1 + var2;  
  T = (t_fine * 5 + 128) >> 8;  
  return T;  
}  

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).  
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa  
BME280_U32_t BME280_compensate_P_int64(BME280_S32_t   adc_P)  
{  
  BME280_S64_t   var1, var2, p;  
  var1 = (( BME280_S64_t )t_fine) - 128000;  
  var2 = var1 * var1 * ( BME280_S64_t )dig_P6;  
  var2 = var2 + ((var1*( BME280_S64_t )dig_P5)<<17);  
  var2 = var2 + ((( BME280_S64_t )dig_P4)<<35);  
  var1 = ((var1 * var1 * ( BME280_S64_t )dig_P3)>>8) + ((var1 * ( BME280_S64_t )dig_P2)<<12);  
  var1 = ((((( BME280_S64_t )1)<<47)+var1))*(( BME280_S64_t )dig_P1)>>33;  
  if   (var1 == 0)  
  {  
    return 0;  // avoid exception caused by division by zero  
  }  
  p = 1048576 - adc_P;  
  p = (((p<<31) - var2)*3125)/var1;  
  var1 = ((( BME280_S64_t )dig_P9) * (p>>13) * (p>>13)) >> 25;  
  var2 = ((( BME280_S64_t )dig_P8) * p) >> 19;  
  p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)dig_P7)<<4);  
  return (BME280_U32_t)p;  
}  

// Returns humidity in %RH as unsigned 32 bit integer in Q2.10 format (22 integer and 10 fractional bits).  
// Output value of “47445” represents 47445 / 1024 = 46.33%RH  
BME280_U32_t BME280_compensate_H_int32(BME280_S32_t adc_H)  
{  
  BME280_S32_t   v_x1_u32r;  
  v_x1_u32r = (t_fine - (( BME280_S32_t )76800));  
  v_x1_u32r = (((((adc_H << 14) -  ((( BME280_S32_t )dig_H4) << 20) - ((( BME280_S32_t )dig_H5) * v_x1_u32r)) + 
    (( BME280_S32_t )16384)) >> 15) * (((((((v_x1_u32r * (( BME280_S32_t )dig_H6)) >> 10) * (((v_x1_u32r * (( BME280_S32_t )dig_H3)) >> 11) + 
    (( BME280_S32_t )32768))) >> 10) + (( BME280_S32_t )2097152)) * (( BME280_S32_t )dig_H2) + 8192) >> 14));  
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >>   7) * (( BME280_S32_t )dig_H1)) >> 4));  
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);   
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);    
  return ( BME280_U32_t )(v_x1_u32r>>12);  
}  

void readRawData() {
  writeRegister(0xF4, 0b01101101);  // włączenie trybu forced dla kolejnego pomiaru
  delay(10);  // Dodaj opóźnienie, aby czujnik zdążył przeprowadzić pomiar
  Wire.beginTransmission(BME280_ADDR);
  Wire.write(0xF7);  // Adres pierwszego rejestru danych
  Wire.endTransmission();
  Wire.requestFrom(BME280_ADDR, 8);
  if (Wire.available() == 8) {
    uint32_t press_raw = (Wire.read() << 12) | (Wire.read() << 4) | (Wire.read() >> 4);
    uint32_t temp_raw = (Wire.read() << 12) | (Wire.read() << 4) | (Wire.read() >> 4);
    uint16_t hum_raw = (Wire.read() << 8) | Wire.read();

    temp = BME280_compensate_T_int32(temp_raw)/100.0;
    press = BME280_compensate_P_int64(press_raw)/256.0/100.0;  // hPa
    hum = BME280_compensate_H_int32(hum_raw)/1024.0;
    
  } else {
    Serial.println("Error: Not enough data available!");
  } 
}

float getTemp() {
    return temp;
}

float getHum() {
    return hum;
}

float getPress() {
    return press;
}
