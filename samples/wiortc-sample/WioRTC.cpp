#include "WioRTC.h"

#define PCF8523_ADDRESS				(0xd0 >> 1)

#define PCF8523_CONTROL_1			(0x00)
#define PCF8523_CONTROL_2			(0x01)
#define PCF8523_CONTROL_3			(0x02)
#define PCF8523_SECONDS				(0x03)
#define PCF8523_MINUTES				(0x04)
#define PCF8523_HOURS				(0x05)
#define PCF8523_DAYS				(0x06)
#define PCF8523_WEEKDAYS			(0x07)
#define PCF8523_MONTHS				(0x08)
#define PCF8523_YEARS				(0x09)
#define PCF8523_MINUTE_ALARM		(0x0a)
#define PCF8523_HOUR_ALARM			(0x0b)
#define PCF8523_DAY_ALARM			(0x0c)
#define PCF8523_WEEKDAY_ALARM		(0x0d)
#define PCF8523_OFFSET				(0x0e)
#define PCF8523_TMR_CLOCKOUT_CTRL	(0x0f)
#define PCF8523_TMR_A_FREQ_CTRL		(0x10)
#define PCF8523_TMR_A_REG			(0x11)
#define PCF8523_TMR_B_FREQ_CTRL		(0x12)
#define PCF8523_TMR_B_REG			(0x13)

#define EEPROM_ADDRESS				(0xa0 >> 1)

bool WioRTC::begin()
{
	if (!ChangeReg8(PCF8523_ADDRESS, PCF8523_TMR_CLOCKOUT_CTRL, 0b11000111, 0b00000000)) return false;	// CLKOUT is 32768Hz
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// RTC functions

bool WioRTC::SetWakeupPeriod(int sec)
{
	if (sec <= 0 || 255 < sec / 3600) return false;

	if (!ChangeReg8(PCF8523_ADDRESS, PCF8523_TMR_CLOCKOUT_CTRL, 0b11111110, 0b00000000)) return false;	// timer B is disabled

	if (sec <= 255)
	{
		WriteReg8(PCF8523_ADDRESS, PCF8523_TMR_B_FREQ_CTRL, 0b00000010);								// source for timer B is 1Hz
		WriteReg8(PCF8523_ADDRESS, PCF8523_TMR_B_REG, sec);												// timer B value
	}
	else if (sec / 60 <= 255)
	{
		WriteReg8(PCF8523_ADDRESS, PCF8523_TMR_B_FREQ_CTRL, 0b00000011);								// source for timer B is 1/60Hz
		WriteReg8(PCF8523_ADDRESS, PCF8523_TMR_B_REG, sec / 60);										// timer B value

	}
	else
	{
		WriteReg8(PCF8523_ADDRESS, PCF8523_TMR_B_FREQ_CTRL, 0b00000100);								// source for timer B is 1/3600Hz
		WriteReg8(PCF8523_ADDRESS, PCF8523_TMR_B_REG, sec / 3600);										// timer B value
	}

	if (!ChangeReg8(PCF8523_ADDRESS, PCF8523_CONTROL_2, 0b00000000, 0b00000001)) return false;			// countdown timer B interrupt is enabled

	if (!ChangeReg8(PCF8523_ADDRESS, PCF8523_TMR_CLOCKOUT_CTRL, 0b11111111, 0b00000001)) return false;	// timer B is enabled

	return true;
}

bool WioRTC::Shutdown()
{
	if (!ChangeReg8(PCF8523_ADDRESS, PCF8523_TMR_CLOCKOUT_CTRL, 0b11111111, 0b00111000)) return false;	// CLKOUT disabled

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// EEPROM functions

void WioRTC::EepromWrite(uint16_t address, const void* data, int dataSize)
{
	uint8_t writeBuffer[2 + dataSize];
	writeBuffer[0] = address >> 8;
	writeBuffer[1] = address;
	memcpy(&writeBuffer[2], data, dataSize);
	Write(EEPROM_ADDRESS, writeBuffer, 2 + dataSize);
}

bool WioRTC::EepromRead(uint16_t address, void* data, int dataSize)
{
	uint8_t writeBuffer[2];
	writeBuffer[0] = address >> 8;
	writeBuffer[1] = address;
	Write(EEPROM_ADDRESS, writeBuffer, 2);

	auto readSize = Read(EEPROM_ADDRESS, (uint8_t*)data, dataSize);

	return readSize == dataSize;
}

////////////////////////////////////////////////////////////////////////////////
// I2C functions

void WioRTC::Write(int slaveAddress, const uint8_t* data, int dataSize)
{
	_Wire->beginTransmission(slaveAddress);
	while (dataSize--)
	{
		_Wire->write(*data++);
	}
	_Wire->endTransmission();
}

int WioRTC::Read(int slaveAddress, uint8_t* data, int dataSize)
{
	auto readSize = _Wire->requestFrom(slaveAddress, dataSize);
	dataSize = readSize;
	while (dataSize--)
	{
		*data++ = _Wire->read();
	}

	return readSize;
}

void WioRTC::WriteReg8(int slaveAddress, uint8_t reg, uint8_t data)
{
	uint8_t writeData[2];
	writeData[0] = reg;
	writeData[1] = data;

	Write(slaveAddress, writeData, sizeof(writeData));
}

int WioRTC::ReadReg8(int slaveAddress, uint8_t reg, uint8_t* data)
{
	Write(slaveAddress, &reg, 1);

	return Read(slaveAddress, data, 1);
}

bool WioRTC::ChangeReg8(int slaveAddress, uint8_t reg, uint8_t andVal, uint8_t orVal)
{
	uint8_t val;
	if (!ReadReg8(slaveAddress, reg, &val)) return false;

	val = val & andVal | orVal;

	WriteReg8(slaveAddress, reg, val);

	return true;
}

////////////////////////////////////////////////////////////////////////////////
