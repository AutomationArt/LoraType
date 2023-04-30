#include <Arduino.h>
#include "ledFunction.h"
#include "main.h"
#include "Adafruit_NeoPixel.h"

Adafruit_NeoPixel pixels(1, PinLed, NEO_GRB);
uint8_t maxLedWarnBrightness = 20;

int clearLed()
{
	pixels.clear();
	pixels.setPixelColor(0, pixels.Color(0, 0, 0));
	pixels.show();
	return 0;
}

int blinkLed()
{
	pixels.clear();

	for (uint8_t i = 0; i <= 20; i++)
	{
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(253, 166, 0));
		pixels.show();
		delay(5);
	}
	for (uint8_t i = 20; i <= 0; i--)
	{
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(253, 166, 0));
		pixels.show();
		delay(5);
	}

	pixels.setPixelColor(0, pixels.Color(0, 0, 0));
	delay(5);
	pixels.show();
	return 0;
}

int heartLed()
{
	pixels.clear();
	for (int i = 0; i < 4; i++)
	{
		delay(20);
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(128, 128, 128));
		pixels.show();
	}

	for (int i = 4; i > 0; i--)
	{
		delay(20);
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(128, 128, 128));
		pixels.show();
	}
	clearLed();

	return 0;
}

// Orange
int LedSystemWarning()
{

	pixels.clear();
	for (uint8_t i = 0; i <= maxLedWarnBrightness; i++)
	{
		pixels.setBrightness(i);

		pixels.setPixelColor(0, pixels.Color(253, 166, 0));
		pixels.show();
		delay(3);
	}
	for (uint8_t i = maxLedWarnBrightness; i <= 0; i--)
	{
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(253, 166, 0));
		pixels.show();
		delay(3);
	}

	pixels.setPixelColor(0, pixels.Color(0, 0, 0));
	delay(3);
	pixels.show();

	return 0;
}

// Red
int LedLoraWarning(bool state = false)
{

	uint8_t r, g, b;
	if (state == false)
	{
		r = 201;
		g = 0;
		b = 0;
	}
	else
	{
		r = 50;
		g = 174;
		b = 0;
	}

	pixels.clear();
	for (uint8_t i = 0; i <= maxLedWarnBrightness; i++)
	{
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(r, g, b));
		pixels.show();
		delay(3);
	}
	for (uint8_t i = maxLedWarnBrightness; i <= 0; i--)
	{
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(r, g, b));
		pixels.show();
		delay(3);
	}

	pixels.setPixelColor(0, pixels.Color(0, 0, 0));
	delay(3);
	pixels.show();

	return 0;
}

int LedUpperCase(bool state)
{
	if (state == true)
	{
		pixels.clear();
		pixels.setBrightness(10);
		pixels.setPixelColor(0, pixels.Color(255, 255, 255));
		delay(5);
		pixels.show();
	}
	else
	{
		pixels.clear();
		pixels.setPixelColor(0, pixels.Color(0, 0, 0));
		delay(5);
		pixels.show();
	}
	return 0;
}

int LedLoraInputMess(bool state = false)
{

	uint8_t r, g, b;
	if (state == false)
	{
		r = 255;
		g = 255;
		b = 255;
	}
	else
	{
		r = 253;
		g = 166;
		b = 0;
	}

	pixels.clear();
	for (uint8_t i = 0; i <= maxLedWarnBrightness; i++)
	{
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(r, g, b));
		pixels.show();
		delay(3);
	}
	for (uint8_t i = maxLedWarnBrightness; i <= 0; i--)
	{
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(r, g, b));
		pixels.show();
		delay(3);
	}

	pixels.setPixelColor(0, pixels.Color(0, 0, 0));
	delay(3);
	pixels.show();

	return 0;
}

int LedStart()
{
	pixels.begin();
	clearLed();
	return 0;
}

int LedSystemStart()
{
	pixels.clear();

	for (int i = 0; i < 20; i++)
	{
		vTaskDelay(10/portTICK_PERIOD_MS);
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(255, 160, 16));
		pixels.show();
	}

	for (int i = 20; i > 0; i--)	
	{
		vTaskDelay(10/portTICK_PERIOD_MS);
		pixels.setBrightness(i);
		pixels.setPixelColor(0, pixels.Color(255, 160, 16));
		pixels.show();
	}
	clearLed();

	return 0;
}