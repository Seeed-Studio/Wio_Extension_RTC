#pragma once

#include <Wire.h>

class WioRTC
{
private:
	TwoWire* _Wire;

public:
	WioRTC()
	{
		_Wire = &Wire;
	}

	WioRTC(TwoWire* wire)
	{
		_Wire = wire;
	}

	bool begin();
	bool SetWakeupPeriod(int sec);
	bool Shutdown();

	void EepromWrite(uint16_t address, const void* data, int dataSize);
	bool EepromRead(uint16_t address, void* data, int dataSize);

private:
	void Write(int slaveAddress, const uint8_t* data, int dataSize);
	int Read(int slaveAddress, uint8_t* data, int dataSize);
	void WriteReg8(int slaveAddress, uint8_t reg, uint8_t data);
	int ReadReg8(int slaveAddress, uint8_t reg, uint8_t* data);
	bool ChangeReg8(int slaveAddress, uint8_t reg, uint8_t andVal, uint8_t orVal);

};