/*
 Name:		ESP32APRS T-TWR Plus
 Created:	13-10-2023 14:27:23
 Author:	HS5TQA/Atten
 Github:	https://github.com/nakhonthai
 Facebook:	https://www.facebook.com/atten
 Support IS: host:aprs.dprns.com port:14580 or aprs.hs5tqa.ampr.org:14580
 Support IS monitor: http://aprs.dprns.com:14501 or http://aprs.hs5tqa.ampr.org:14501
*/

#ifndef GUI_LCD_H
#define GUI_LCD_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "main.h"
#include <parse_aprs.h>
#include <TimeLib.h>
#include <time.h>
#include <TinyGPSPlus.h>
#include <pbuf.h>
#include "parse_aprs.h"
#include "MenuSystem.h"
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/Seven_Segment24pt7b.h>
#include <Fonts/Picopixel.h>

#include <Wire.h>
#include "Adafruit_SSD1306.h"
#include <Adafruit_GFX.h>
#include <Adafruit_I2CDevice.h>
#include "cppQueue.h"

#include "wireguard_vpn.h"

#include "XPowersLib.h"

#include "cppQueue.h"

#define keyA ENCODER_B_PIN
#define keyB ENCODER_A_PIN
#define keyPush ENCODER_OK_PIN

#define CON_WIFI 0
#define CON_SERVER 1
#define CON_WEB 2
#define CON_NORMAL 3
#define CON_MENU 4
#define CON_RF 5

// #define PKGLISTSIZE 50

#define PKG_ALL 0		// Packet is of position type
#define PKG_OBJECT 1	// packet is an object
#define PKG_ITEM 2		// packet is an item
#define PKG_MESSAGE 3	// packet is a message
#define PKG_WX 4		// packet is WX data
#define PKG_TELEMETRY 5 // packet is telemetry
#define PKG_QUERY 6		// packet is a query
#define PKG_STATUS 7	// packet is status

const char APRS_PATH[9][16] = {"", "WIDE1-1", "WIDE1-1,WIDE2-1", "TRACK3-3", "RS0ISS", "YBOX-1", "W3ADO-1", "BJ1SI", "PSAT2-1"};

const unsigned char LOGO[] PROGMEM =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80,
		0xC0, 0x00, 0x00, 0xC0, 0xC0, 0xE0, 0xE0, 0xF0, 0xF8, 0xF8,
		0xFC, 0xFC, 0xFE, 0x7E, 0x7F, 0x7C, 0x70, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xE0, 0xE0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF8, 0xF8, 0xF8, 0xFC,
		0x7C, 0x3C, 0x3C, 0x18, 0x83, 0x9F, 0x9F, 0x9F, 0x8F, 0x8F,
		0xCC, 0x41, 0x63, 0x13, 0x01, 0x81, 0xE1, 0x21, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
		0x07, 0x07, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
		0x08, 0x08, 0x08, 0x09, 0x19, 0x19, 0x09, 0x08, 0x08, 0x0C,
		0x04, 0x86, 0x83, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x8C, 0x80, 0x84, 0xC2, 0x80, 0x07, 0x08,
		0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xF8, 0x3E, 0x43, 0x7D,
		0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
		0x7E, 0x7E, 0x7E, 0x7E, 0x79, 0x07, 0x7C, 0xF0, 0xC0, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xC3, 0xFF,
		0xFF, 0xFF, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0xFF, 0xFF, 0xFF,
		0xE7, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x1F, 0x3F, 0x3F, 0x3F, 0x1F, 0x03, 0x03, 0x03, 0x03,
		0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x1F,
		0x3F, 0x3F, 0x3F, 0x1F, 0x03, 0x00, 0x00, 0x00};

// const uint8_t iconBluetooth[] = {0x06, 0x00, 0x07, 0x00, 0x07, 0x80, 0x16, 0x80, 0x0f, 0x00, 0x06, 0x00, 0x06, 0x00, 0x0f, 0x00, 0x16, 0x80, 0x07, 0x80, 0x07, 0x00, 0x06, 0x00};
// icon 11x11
const uint8_t iconBluetooth[] = {0x04, 0x00, 0x06, 0x00, 0x25, 0x00, 0x15, 0x00, 0x0e, 0x00, 0x04, 0x00, 0x0e, 0x00, 0x15, 0x00, 0x25, 0x00, 0x06, 0x00, 0x04, 0x00};
// icon 12x12
const uint8_t iconClound[] = {0x00, 0x00, 0x3e, 0x00, 0x77, 0x00, 0xc1, 0x80, 0xc0, 0xe0, 0x86, 0x30, 0xc6, 0x10, 0xcf, 0x10, 0x56, 0xa0, 0x06, 0x20, 0x06, 0x00, 0x00, 0x00};
const uint8_t iconLocation[] = {0x0f, 0x00, 0x19, 0x80, 0x20, 0x40, 0x66, 0x60, 0x4f, 0x20, 0x4f, 0x20, 0x66, 0x60, 0x60, 0x40, 0x30, 0xc0, 0x19, 0x80, 0x0f, 0x00, 0x06, 0x00};
const uint8_t iconLink[] = {0x00, 0x80, 0x03, 0xe0, 0x06, 0x20, 0x04, 0x30, 0x1e, 0x20, 0x3a, 0x60, 0x65, 0xc0, 0x47, 0x80, 0xc2, 0x00, 0x46, 0x00, 0x7c, 0x00, 0x10, 0x00};

// icon 28x28
const uint8_t iconList[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x7f, 0xff, 0x80, 0x10, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x7f, 0xff, 0x80, 0x10, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x7f, 0xff, 0x80, 0x10, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconAlert[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x30, 0xc0, 0x00, 0x00, 0x30, 0xc0, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x01, 0x86, 0x18, 0x00, 0x01, 0x86, 0x18, 0x00, 0x03, 0x06, 0x0c, 0x00, 0x03, 0x06, 0x0c, 0x00, 0x06, 0x06, 0x06, 0x00, 0x0c, 0x06, 0x03, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x18, 0x00, 0x01, 0x80, 0x18, 0x00, 0x01, 0x80, 0x30, 0x06, 0x00, 0xc0, 0x60, 0x06, 0x00, 0xe0, 0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x70, 0x00, 0x00, 0xe0, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconCalendar[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xff, 0x80, 0x18, 0xc0, 0x31, 0x80, 0x10, 0x40, 0x20, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x3f, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xff, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x18, 0x00, 0x01, 0x80, 0x1c, 0x00, 0x03, 0x80, 0x1f, 0xff, 0xff, 0x80, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconCPU[] = {0x00, 0x20, 0x40, 0x00, 0x00, 0x70, 0xe0, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x20, 0x40, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x03, 0x00, 0xfc, 0x3f, 0xc3, 0xf0, 0x0c, 0x20, 0x43, 0x00, 0x0c, 0x20, 0x43, 0x00, 0x0c, 0x20, 0x43, 0x00, 0x0c, 0x20, 0x43, 0x00, 0x7c, 0x20, 0x43, 0xe0, 0xfc, 0x20, 0x43, 0xf0, 0x0c, 0x3f, 0xc3, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x20, 0x40, 0x00};
const uint8_t iconGlobe[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0x9f, 0x9c, 0x00, 0x07, 0x19, 0x8e, 0x00, 0x0c, 0x30, 0xc3, 0x00, 0x1c, 0x30, 0xc3, 0x80, 0x18, 0x60, 0x61, 0x80, 0x30, 0x60, 0x60, 0xc0, 0x30, 0x60, 0x60, 0xc0, 0x30, 0x60, 0x60, 0xc0, 0x20, 0xc0, 0x20, 0x40, 0x7f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 0x60, 0x40, 0x20, 0x40, 0x30, 0x60, 0x60, 0xc0, 0x30, 0x60, 0x60, 0xc0, 0x30, 0x60, 0x60, 0xc0, 0x18, 0x60, 0x61, 0x80, 0x18, 0x30, 0xc1, 0x80, 0x0c, 0x30, 0xc3, 0x00, 0x07, 0x19, 0x8e, 0x00, 0x03, 0x99, 0x9c, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconHome[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x39, 0xc0, 0x00, 0x00, 0xe0, 0x70, 0x00, 0x01, 0xc0, 0x38, 0x00, 0x03, 0x80, 0x1c, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x1c, 0x00, 0x03, 0x80, 0x18, 0x00, 0x01, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x3f, 0xc0, 0x80, 0x10, 0x3f, 0xc0, 0x80, 0x10, 0x20, 0x40, 0x80, 0x10, 0x20, 0x40, 0x80, 0x10, 0x20, 0x40, 0x80, 0x10, 0x20, 0x40, 0x80, 0x10, 0x20, 0x40, 0x80, 0x10, 0x20, 0x40, 0x80, 0x10, 0x20, 0x40, 0x80, 0x10, 0x20, 0x40, 0x80, 0x18, 0x20, 0x41, 0x80, 0x1c, 0x20, 0x43, 0x80, 0x1f, 0xff, 0xff, 0x80, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconInfo[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x01, 0xf9, 0xf8, 0x00, 0x03, 0x80, 0x1c, 0x00, 0x07, 0x00, 0x0e, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x1c, 0x00, 0x03, 0x80, 0x18, 0x00, 0x01, 0x80, 0x30, 0x06, 0x00, 0xc0, 0x30, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00, 0xc0, 0x20, 0x00, 0x00, 0x40, 0x60, 0x06, 0x00, 0x60, 0x60, 0x06, 0x00, 0x60, 0x60, 0x06, 0x00, 0x40, 0x30, 0x06, 0x00, 0xc0, 0x30, 0x06, 0x00, 0xc0, 0x30, 0x06, 0x00, 0xc0, 0x18, 0x06, 0x01, 0x80, 0x18, 0x00, 0x01, 0x80, 0x0c, 0x00, 0x03, 0x00, 0x07, 0x00, 0x0e, 0x00, 0x03, 0x80, 0x1c, 0x00, 0x01, 0xf9, 0xf8, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconLayer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x03, 0xc0, 0x3c, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x3c, 0x00, 0x03, 0xc0, 0x78, 0x00, 0x01, 0xe0, 0x1e, 0x00, 0x07, 0x80, 0x07, 0x80, 0x1e, 0x00, 0x01, 0xe0, 0x78, 0x00, 0x00, 0x79, 0xe0, 0x00, 0x20, 0x1f, 0x80, 0x40, 0x38, 0x06, 0x01, 0xc0, 0x1e, 0x00, 0x07, 0x80, 0x07, 0xc0, 0x3e, 0x00, 0x01, 0xf0, 0xf8, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x70, 0x1f, 0x80, 0xe0, 0x3c, 0x06, 0x03, 0xc0, 0x0f, 0x00, 0x0f, 0x00, 0x03, 0xc0, 0x3c, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconMail[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xc0, 0x70, 0x00, 0x00, 0xe0, 0x78, 0x00, 0x01, 0xe0, 0x7c, 0x00, 0x03, 0xe0, 0x6f, 0x00, 0x0f, 0x60, 0x63, 0x80, 0x1c, 0x60, 0x61, 0xc0, 0x38, 0x60, 0x60, 0xf0, 0xf0, 0x60, 0x60, 0x39, 0xc0, 0x60, 0x60, 0x1f, 0x80, 0x60, 0x60, 0x06, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x3f, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconMenu[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconNavigation[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x1e, 0xc0, 0x00, 0x00, 0x79, 0x80, 0x00, 0x01, 0xe1, 0x80, 0x00, 0x0f, 0x83, 0x00, 0x00, 0x3e, 0x03, 0x00, 0x00, 0xf8, 0x06, 0x00, 0x03, 0xc0, 0x06, 0x00, 0x0f, 0x00, 0x0c, 0x00, 0x1c, 0x00, 0x0c, 0x00, 0x1f, 0xc0, 0x18, 0x00, 0x03, 0xf8, 0x18, 0x00, 0x00, 0x3c, 0x30, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x06, 0xe0, 0x00, 0x00, 0x06, 0xc0, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconPower[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x06, 0x06, 0x06, 0x00, 0x06, 0x06, 0x06, 0x00, 0x0e, 0x06, 0x07, 0x00, 0x1c, 0x06, 0x03, 0x80, 0x18, 0x06, 0x01, 0x80, 0x18, 0x06, 0x01, 0x80, 0x18, 0x06, 0x01, 0x80, 0x18, 0x06, 0x01, 0x80, 0x18, 0x00, 0x01, 0x80, 0x18, 0x00, 0x01, 0x80, 0x18, 0x00, 0x01, 0x80, 0x18, 0x00, 0x01, 0x80, 0x1c, 0x00, 0x03, 0x80, 0x0c, 0x00, 0x03, 0x00, 0x0e, 0x00, 0x07, 0x00, 0x07, 0x00, 0x0e, 0x00, 0x03, 0x80, 0x1c, 0x00, 0x01, 0xfb, 0xf8, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconFile[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xf0, 0x00, 0x1d, 0x80, 0x38, 0x00, 0x19, 0x80, 0x1c, 0x00, 0x11, 0x80, 0x0e, 0x00, 0x11, 0x80, 0x07, 0x00, 0x11, 0xff, 0xc3, 0x80, 0x11, 0xff, 0xc1, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x80, 0x10, 0xff, 0xf0, 0x80, 0x11, 0xff, 0xf8, 0x80, 0x11, 0x80, 0x18, 0x80, 0x11, 0x80, 0x18, 0x80, 0x11, 0x80, 0x18, 0x80, 0x11, 0x80, 0x18, 0x80, 0x11, 0x80, 0x18, 0x80, 0x11, 0x80, 0x18, 0x80, 0x19, 0x80, 0x19, 0x80, 0x1d, 0x80, 0x1b, 0x80, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconSetting[] = {0x00, 0x07, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x19, 0x80, 0x00, 0x02, 0x19, 0x84, 0x00, 0x0f, 0xb0, 0xff, 0x00, 0x1d, 0xf0, 0xfb, 0x00, 0x18, 0x40, 0x61, 0x80, 0x18, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x06, 0x00, 0x0c, 0x1f, 0x83, 0x00, 0x7c, 0x39, 0xc3, 0xe0, 0x70, 0x30, 0xc1, 0xe0, 0xc0, 0x30, 0xc0, 0x30, 0xc0, 0x30, 0xc0, 0x30, 0x70, 0x30, 0xc0, 0xe0, 0x7c, 0x39, 0xc3, 0xe0, 0x0c, 0x1f, 0x83, 0x00, 0x06, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x01, 0x80, 0x18, 0x40, 0x21, 0x80, 0x0c, 0xf0, 0xfb, 0x00, 0x0f, 0xf0, 0xdf, 0x00, 0x07, 0x19, 0x8e, 0x00, 0x00, 0x19, 0x80, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x06, 0x00, 0x00};
const uint8_t iconTool[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x07, 0x8c, 0x00, 0x00, 0x0e, 0x18, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x18, 0x60, 0xc0, 0x00, 0x18, 0xc1, 0xc0, 0x00, 0x18, 0xc3, 0x60, 0x00, 0x18, 0xe6, 0x60, 0x00, 0x18, 0x7c, 0x60, 0x00, 0x18, 0x38, 0xc0, 0x00, 0x18, 0x00, 0xc0, 0x00, 0x30, 0x01, 0xc0, 0x00, 0x60, 0x03, 0x80, 0x00, 0xc1, 0xff, 0x00, 0x01, 0x83, 0xfc, 0x00, 0x03, 0x06, 0x00, 0x00, 0x06, 0x0c, 0x00, 0x00, 0x0c, 0x18, 0x00, 0x00, 0x18, 0x30, 0x00, 0x00, 0x38, 0x60, 0x00, 0x00, 0x38, 0xc0, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t iconWebservice[] = {0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0, 0xe4, 0xe0, 0x00, 0x70, 0xe4, 0xe0, 0x00, 0x70, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xf0, 0x80, 0x06, 0x00, 0x10, 0x80, 0x0f, 0x00, 0x10, 0x80, 0x9f, 0x90, 0x10, 0x81, 0xff, 0xf8, 0x10, 0x83, 0xff, 0xfc, 0x10, 0x83, 0xe0, 0x7c, 0x10, 0x83, 0xc0, 0x3c, 0x10, 0x87, 0x90, 0x9e, 0x10, 0x8f, 0x30, 0xcf, 0x10, 0x9f, 0x30, 0xcf, 0x90, 0x9f, 0x39, 0xcf, 0x90, 0x9f, 0x3f, 0xcf, 0x90, 0x8f, 0x3f, 0xcf, 0x10, 0x87, 0x9f, 0x9e, 0x10, 0x83, 0xcf, 0x3c, 0x10, 0x83, 0xef, 0x7c, 0x10, 0x83, 0xef, 0x7c, 0x10, 0x81, 0xef, 0x78, 0x10, 0x80, 0x8f, 0x10, 0x10, 0x80, 0x0f, 0x00, 0x10, 0x80, 0x00, 0x00, 0x10, 0x7f, 0xff, 0xff, 0xe0};
const uint8_t iconStation[] = {0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x1e, 0x00, 0x07, 0x80, 0x3c, 0x80, 0x13, 0xc0, 0x38, 0xe0, 0x71, 0xc0, 0x71, 0xc0, 0x38, 0xe0, 0x71, 0xc0, 0x38, 0xe0, 0x73, 0xc6, 0x3c, 0xe0, 0xf3, 0x8f, 0x1c, 0xf0, 0xe3, 0x9f, 0x9c, 0x70, 0xf3, 0x8f, 0x1c, 0xf0, 0x71, 0xc6, 0x38, 0xe0, 0x71, 0xc0, 0x38, 0xe0, 0x70, 0xe0, 0x70, 0xe0, 0x38, 0x80, 0x11, 0xc0, 0x3c, 0x06, 0x03, 0xc0, 0x1e, 0x0f, 0x07, 0x80, 0x0c, 0x0f, 0x03, 0x00, 0x08, 0x1f, 0x81, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xfc, 0x00, 0x07, 0xff, 0xfe, 0x00};

#define TXCH_TCP 0
#define TXCH_RF 1
#define TXCH_DIGI 2
#define TXCH_3PTY 3
typedef struct txDispStruct
{
	uint8_t tx_ch;
	char name[12];
	char info[50];
} txDisp;

typedef struct menuListName
{
	const uint8_t *icon;
	const char *name;
} menuListName;

#define MAX_MENU 7
const menuListName menuList[MAX_MENU] PROGMEM = {
	{iconList, "STATISTICS"},
	{iconStation, "STATION LIST"},
	{iconNavigation, "GNSS INFORMATION"},
	{iconSetting, "SYSTEM INFO"},
	{iconTool, "SETTING"},
	{iconWebservice, "QR Web Service"},
	{iconInfo, "ABOUT"},
};

// uint8_t *menuList[5]={iconList,iconNavigation,iconSetting,iconTool,iconInfo};

// Create icon from https://rickkas7.github.io/DisplayGenerator
const uint8_t bluetooth_icon[] = {0x10, 0x5c, 0x38, 0x10, 0x38, 0x5c, 0x10};

extern Adafruit_SSD1306 display;
extern cppQueue dispBuffer;
extern int conStat;
extern int conStatNetwork;
extern int raw_count;
// extern pkgListType pkgList[PKGLISTSIZE];
extern pkgListType *pkgList;
// extern TelemetryType Telemetry[TLMLISTSIZE];
extern statusType status;
extern TelemetryType *Telemetry;
extern Configuration config;
extern int16_t SB_HEADING;
extern unsigned char SB_SPEED;
extern TinyGPSPlus gps;
extern WiFiClient aprsClient;
extern uint16_t tx_interval;	// How often we transmit, in seconds
extern unsigned int tx_counter; // Incremented every second
extern char send_aprs_table;
extern char send_aprs_symbol;
extern ParseAPRS aprsParse;
extern float vbat;
extern time_t systemUptime;

extern XPowersAXP2101 PMU;

// extern uint8_t dispFlagTX;

void mainDisp(void *pvParameters);
void dispWindow(String line, uint8_t mode, bool filter);
void msgBox(String msg);
void topBar(int ws);
void compass_label(signed int startx, signed int starty, unsigned int length, double angle, unsigned int color);
void compass_arrow(signed int startx, signed int starty, unsigned int length, double angle, unsigned int color);
void dispTxWindow(txDisp txs);
void setOLEDLock(bool lck);
void pushTxDisp(uint8_t ch, const char *name, char *info);

// const char *str_status[] = {
// 	"IDLE_STATUS",
// 	"NO_SSID_AVAIL",
// 	"SCAN_COMPLETED",
// 	"CONNECTED",
// 	"CONNECT_FAILED",
// 	"CONNECTION_LOST",
// 	"DISCONNECTED"
// };

#define ALL 0
#define NUMBER 1
#define ALPHA 2
class MyTextBox
{
private:
	int curr_cursor;

public:
	// struct textboxType {
	char text[50];
	int x;
	int y;
	int length;
	bool isSelect;
	char type = 0; // 0:All,1:Number,2:Alpha Upper
	char char_max = 0x7F;
	char char_min = 0x20;

	void TextBox();
	void TextBoxShow();
};

class MyCheckBox
{
public:
	// struct textboxType {
	char text[20];
	int x;
	int y;
	int length;
	bool isSelect;
	bool Checked;

	void Toggle();

	void CheckBoxShow();
};

class MyButtonBox
{
public:
	// struct textboxType {
	char text[30];
	int x;
	int y;
	int length;
	bool isSelect;
	bool Checked;
	bool Border;

	void Toggle();
	void Show();
};

class MyComboBox
{
private:
	unsigned char current_index = 0;
	long current = 0;

public:
	// struct textboxType {
	char text[30];
	char item[10][30];
	int x;
	int y;
	unsigned int length;
	bool isSelect;
	bool isValue = false;
	char type = 0; // 0:All,1:Number,2:Alpha Upper,3:Calculator
	unsigned int char_max = 9;
	unsigned int char_min = 0;

	void SelectValue(long val_min, long val_max, long step);
	void AddItem(int index, char *str);
	void AddItem(int index, const char *str);
	void GetItem(int index, char *str);
	void maxItem(unsigned char index);
	unsigned long GetValue();
	unsigned char GetIndex();
	void SetIndex(unsigned int i);
	void SelectItem();
	void Show();
};

class MySymbolBox
{
private:
	unsigned char current_index = 0;

public:
	// struct textboxType {
	String title;
	char tableMode = 0;
	char table;
	char symbol;
	char text[30];
	char item[2][32];
	int x;
	int y;
	unsigned int length;
	bool isSelect;
	bool onSelect = false;
	char type = 0; // 0:All,1:Number,2:Alpha Upper,3:Calculator
	unsigned char char_max = 0x80;
	unsigned char char_min = 0x21;

	unsigned char GetTable();
	unsigned char GetSymbol();
	unsigned char GetIndex();
	void SetIndex(unsigned char i);
	void SelectItem();
	void Show();
};

#endif
