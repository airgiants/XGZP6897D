/*
  XGZP6897D.cpp - Library for using a familly of pressure sensors from CFSensor.com
  I2C sensors
  Created by Francis Sourbier
  GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  This library is free software; 
  Released into the public domain
*/
#include <XGZP6897D.h>
#include <Wire.h>
//  Descriptor. K depends on the exact model of the sensor. See datasheet and documentation
XGZP6897D::XGZP6897D(uint16_t K)
{
  _K = K;
  _I2C_address = I2C_device_address;
}
//
//  
bool XGZP6897D::begin()
{
  Wire.begin();
  // A basic scanner, see if it ACK's
  Wire.beginTransmission(_I2C_address);
  if (Wire.endTransmission() == 0) {
    return true;  // Ok device is responding
  }
  return false; // device not r
}
//
//  Read temperature (degres Celsius), and pressure (PA)
void XGZP6897D::readSensor(float &temperature, float &pressure)
{
  int32_t pressure_adc;
  int16_t  temperature_adc ;
  uint8_t pressure_H, pressure_M, pressure_L, temperature_H, temperature_L;
  uint8_t CMD_reg;
  // start conversion
  Wire.beginTransmission(_I2C_address);
  Wire.write(0x30);
  Wire.write(0x0A);   //start combined conversion pressure and temperature
  Wire.endTransmission();
  // wait until the end of conversion (Sco bit in CMD_reg. bit 3)
  do {
    Wire.beginTransmission(_I2C_address);
    Wire.write(0x30);                       //send 0x30 CMD register address
    Wire.endTransmission();
    Wire.requestFrom(_I2C_address, byte(1));
    CMD_reg = Wire.read();                //read 0x30 register value
  } while ((CMD_reg & 0x08) > 0);        //loop while bit 3 of 0x30 register copy is 1
  // read temperature and pressure
  Wire.beginTransmission(_I2C_address);
  Wire.write(0x06);                        //send pressure high byte register address
  Wire.endTransmission();
  Wire.requestFrom(_I2C_address, byte(5)); // read 3 bytes for pressure and 2 for temperature
  pressure_H = Wire.read();
  pressure_M = Wire.read();
  pressure_L = Wire.read();
  temperature_H = Wire.read();
  temperature_L = Wire.read();
  pressure_adc = ((uint32_t)pressure_H << 8) + (uint32_t) pressure_M;
  pressure_adc = (pressure_adc << 8) + (uint32_t) pressure_L;
  temperature_adc = ((uint16_t)temperature_H << 8) + (uint16_t) temperature_L;
  // pressure is a signed2 complement style value, on 24bits.
  // need to extend the bit sign on the full 32bits.
  pressure = ((pressure_adc << 8) >> 8) / _K;
  temperature = float(temperature_adc) / 256;
#ifdef debugFS
  Serial.print(String(pressure_H, HEX));
  Serial.print("," + String(pressure_M, HEX));
  Serial.print("," + String(pressure_L, HEX));
  Serial.print(":" + String(pressure_adc, HEX));
  Serial.print(" – " + String(temperature_H, HEX));
  Serial.print("," + String(temperature_L, HEX));
  Serial.print(":" + String(temperature_adc, HEX));
  Serial.print(" - " + String(temperature) + ":" + String(pressure));
  Serial.println();
#endif
  return ;
}
