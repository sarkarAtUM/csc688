/*
 * BMP180Module.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: Saminda
 */

#include "BMP180Module.h"

#if defined(ENERGIA)
#include "Wire.h"
#endif

void BMP180Module::init()
{
  calibration();
}

void BMP180Module::update(BMP180Representation& theBMP180Representation)
{
#if defined(ENERGIA)
  calculation(theBMP180Representation);             // run calculations for temperature and pressure

  //Serial.print("Temperature: ");
  //Serial.println(theBMP180Representation.fTemperature, 3);
  //Serial.print("Pressure:    ");
  //Serial.println(theBMP180Representation.fPressure, 3);
  //Serial.print("Altitude: ");
  //Serial.println(theBMP180Representation.fAltitude, 3);

#endif

}

void BMP180Module::calibration()
{
#if defined(ENERGIA)
  I2CMRead(BMP180_O_AC1_MSB);
  parameters.i16AC1 = (int16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);
  I2CMRead(BMP180_O_AC2_MSB);
  parameters.i16AC2 = (int16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);
  I2CMRead(BMP180_O_AC3_MSB);
  parameters.i16AC3 = (int16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);

  I2CMRead(BMP180_O_AC4_MSB);
  parameters.ui16AC4 = (uint16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);
  I2CMRead(BMP180_O_AC5_MSB);
  parameters.ui16AC5 = (uint16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);
  I2CMRead(BMP180_O_AC6_MSB);
  parameters.ui16AC6 = (uint16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);

  I2CMRead(BMP180_O_B1_MSB);
  parameters.i16B1 = (int16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);
  I2CMRead(BMP180_O_B2_MSB);
  parameters.i16B2 = (int16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);

  I2CMRead(BMP180_O_MC_MSB);
  parameters.i16MC = (int16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);
  I2CMRead(BMP180_O_MD_MSB);
  parameters.i16MD = (int16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);

  //Serial.print("i16AC1=");
  //Serial.println(parameters.i16AC1);
  //Serial.print("i16AC2=");
  //Serial.println(parameters.i16AC2);
  //Serial.print("i16AC3=");
  //Serial.println(parameters.i16AC3);
  //Serial.print("ui16AC4=");
  //Serial.println(parameters.ui16AC4);
  //Serial.print("ui16AC5=");
  //Serial.println(parameters.ui16AC5);
  //Serial.print("ui16AC6=");
  //Serial.println(parameters.ui16AC6);
  //Serial.print("i16B1=");
  //Serial.println(parameters.i16B1);
  //Serial.print("i16B2=");
  //Serial.println(parameters.i16B2);
  //Serial.print("i16MC=");
  //Serial.println(parameters.i16MC);
  //Serial.print("i16MD=");
  //Serial.println(parameters.i16MD);

#endif
}

// read 16-bits from I2C
void BMP180Module::I2CMRead(const uint8_t& addr, const uint8_t& bytes)
{
#if defined(ENERGIA)
  Wire.beginTransmission(parameters.ui8Addr);
  Wire.write(addr);
  Wire.endTransmission(false);
  Wire.requestFrom(parameters.ui8Addr, bytes);
  while (Wire.available() < bytes - 1)
    ;
  parameters.pui8Data[0] = Wire.read();
  parameters.pui8Data[1] = Wire.read();
  if (bytes == 3)
    parameters.pui8Data[2] = Wire.read();
#endif
}

void BMP180Module::cmdI2CMRead(const uint8_t& cmd, const uint8_t& addr, const uint8_t& bytes)
{
#if defined(ENERGIA)
  Wire.beginTransmission(parameters.ui8Addr);
  Wire.write(BMP180_O_CTRL_MEAS);
  Wire.write(cmd);
  Wire.endTransmission();
  delay(5); //<< To ready data (Read manufacturer data sheet: TODO)
  I2CMRead(addr, bytes);
#endif
}

void BMP180Module::calculation(BMP180Representation& theBMP180Representation)
{
#if defined(ENERGIA)
  // Temperature
  cmdI2CMRead((BMP180_CTRL_MEAS_SCO | BMP180_CTRL_MEAS_TEMPERATURE), BMP180_O_OUT_MSB, 2);

  float fUT, fX1, fX2, fB5;

  //
  // Get the uncompensated temperature.
  //
  fUT = (float) (uint16_t) ((parameters.pui8Data[0] << 8) | parameters.pui8Data[1]);

  //
  // Calculate the true temperature.
  //
  fX1 = ((fUT - (float) parameters.ui16AC6) * (float) parameters.ui16AC5) / 32768.f;
  fX2 = ((float) parameters.i16MC * 2048.f) / (fX1 + (float) parameters.i16MD);
  fB5 = fX1 + fX2;
  theBMP180Representation.fTemperature = fB5 / 160.f;

  // This is with no sampling
  cmdI2CMRead((BMP180_CTRL_MEAS_SCO | BMP180_CTRL_MEAS_PRESSURE | parameters.ui8Mode),
  BMP180_O_OUT_MSB, 3);

  float fUP, fX3, fB3, fB4, fB6, fB7, fP;
  int_fast8_t i8Oss;

  uint32_t rawPressure = (int32_t) ((parameters.pui8Data[0] << 16) | (parameters.pui8Data[1] << 8)
      | (parameters.pui8Data[2] & BMP180_OUT_XLSB_M));

  // Get the oversampling ratio.
  //
  i8Oss = parameters.ui8Mode >> BMP180_CTRL_MEAS_OSS_S;

  //
  // Retrieve the uncompensated pressure.
  //
  fUP = ((float) (int32_t) ((parameters.pui8Data[0] << 16) | (parameters.pui8Data[1] << 8)
      | (parameters.pui8Data[2] & BMP180_OUT_XLSB_M)) / (1 << (8 - i8Oss)));

  //
  // Calculate the true temperature.
  //
  fX1 = ((fUT - (float) parameters.ui16AC6) * (float) parameters.ui16AC5) / 32768.f;
  fX2 = ((float) parameters.i16MC * 2048.f) / (fX1 + (float) parameters.i16MD);
  fB5 = fX1 + fX2;

  //
  // Calculate the true pressure.
  //
  fB6 = fB5 - 4000;
  fX1 = ((float) parameters.i16B2 * ((fB6 * fB6) / 4096)) / 2048;
  fX2 = ((float) parameters.i16AC2 * fB6) / 2048;
  fX3 = fX1 + fX2;
  fB3 = ((((float) parameters.i16AC1 * 4) + fX3) * (1 << i8Oss)) / 4;
  fX1 = ((float) parameters.i16AC3 * fB6) / 8192;
  fX2 = ((float) parameters.i16B1 * ((fB6 * fB6) / 4096)) / 65536;
  fX3 = (fX1 + fX2) / 4;
  fB4 = (float) parameters.ui16AC4 * ((fX3 / 32768) + 1);
  fB7 = (fUP - fB3) * (50000 >> i8Oss);
  fP = (fB7 * 2) / fB4;
  fX1 = (fP / 256) * (fP / 256);
  fX1 = (fX1 * 3038) / 65536;
  fX2 = (fP * -7357) / 65536;
  fP += (fX1 + fX2 + 3791) / 16;
  theBMP180Representation.fPressure = fP;

  //
  // Calculate the altitude.
  //
  theBMP180Representation.fAltitude = 44330.0f
      * (1.0f - powf(theBMP180Representation.fPressure / 101325.0f, 1.0f / 5.255f));

#endif
}

MAKE_MODULE(BMP180Module)
