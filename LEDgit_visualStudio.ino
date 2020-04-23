#include "Arduino.h"
#include "Math.h"
#include <FastLED.h>		// https://github.com/FastLED/FastLED 
#include <LEDMatrix.h>      // https://github.com/Jorgen-VikingGod/LEDMatrix
#include <FastLED_GFX.h>	// https://github.com/Jorgen-VikingGod/FastLED-GFX
//#include <TimerOne.h>

// Change the next defines to match your matrix type and size
#define TEST_PIN_D7         6  // internal LED
#define DATA_PIN            12 // C2 (alt arduino: 3)
#define MIDI_RX_PIN         2  // D2
#define LIPO_PIN            40 // 40 // F2
#define SECONDSFORVOLTAGE	5

// initial matrix layout (to get led strip index by x/y)
#define MATRIX_WIDTH        22
#define MATRIX_HEIGHT       23
#define MATRIX_TYPE         HORIZONTAL_ZIGZAG_MATRIX
#define MATRIX_SIZE         (MATRIX_WIDTH * MATRIX_HEIGHT)
#define NUMPIXELS           MATRIX_SIZE
#define COLOR_ORDER         GRB
#define CHIPSET             WS2812B
#define anz_LEDs			278
#define BRIGTHNESS			5

// paths for progOutlinePath
const static int outlinePath1[] = { 30, 31, 29, 28, 27, 26, 36, 42, 43, 44, 45, 46, 25, 9, 8, 0, 1, 2, 4, 3, 16, 17, 56, 57, 91, 92, 101, 102, 111, 112, 121, 122, 162, 193, 229, 230, 262, 263, 274, 275, 276, 277, 270, 269, 254, 239, 240, 241, 242, 243, 244, 253, 252, 251, 250, 249, 211, 210, 176, 177, 178, 179, 175, 161, 152, 151, 142, 141, 132, 131, 77, 72, 73, 74, 75, 76, 37, 31 };
const static int outlinePath2[] = { 32, 33, 34, 35, 41, 71, 70, 69, 68, 67, 47, 24, 10, 7, 6, 5, 14, 15, 18, 55, 58, 90, 93, 100, 103, 110, 113, 120, 123, 163, 192, 194, 228, 231, 261, 264, 273, 272, 271, 268, 255, 238, 220, 219, 218, 217, 216, 215, 245, 246, 247, 248, 212, 209, 208, 207, 180, 174, 160, 153, 150, 143, 140, 133, 130, 77, 72, 73, 74, 38 };
const static int outlinePath3[] = { 39, 40, 72, 77, 78, 79, 80, 81, 66, 48, 23, 11, 12, 13, 19, 54, 59, 89, 94, 99, 104, 109, 114, 119, 124, 164, 191, 195, 227, 232, 260, 265, 266, 267, 256, 237, 221, 202, 203, 204, 205, 206, 215, 214, 213, 181, 173, 159, 154, 149, 144, 139, 134, 129 };
const static int outlinePath4[] = { 81, 82, 65, 49, 22, 21, 20, 53, 60, 88, 95, 98, 105, 108, 115, 118, 125, 164, 191, 195, 227, 233, 258, 257, 236, 222, 201, 202, 183, 172, 158, 155, 148, 145, 138, 135, 128 };
const static int outlinePath5[] = { 82, 65, 49, 50, 51, 61, 87, 96, 97, 106, 107, 116, 117, 126, 165, 190, 196, 226, 234, 235, 236, 222, 201, 184, 171, 157, 156, 147, 146, 137, 136, 127 };
const static int outlinePath6[] = { 82, 65, 64, 63, 62, 87, 96, 97, 106, 107, 116, 117, 126, 165, 190, 196, 225, 224, 223, 222, 201, 184, 171, 157, 156, 147, 146, 137, 136, 127 };
const static int outlinePath7[] = { 82, 83, 84, 85, 86, 96, 97, 106, 107, 116, 117, 126, 165, 190, 197, 198, 199, 200, 185, 171, 157, 156, 147, 146, 137, 136, 127 };
const static int outlinePath8[] = { 82, 83, 84, 85, 86, 96, 97, 106, 107, 116, 117, 126, 165, 189, 188, 187, 186, 185, 171, 157, 156, 147, 146, 137, 136, 127 };
const static int outlinePath9[] = { 82, 83, 84, 85, 86, 96, 97, 106, 107, 116, 117, 126, 166, 167, 168, 169, 170, 157, 156, 147, 146, 137, 136, 127 };

// create our matrix based on matrix definition
cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

unsigned long lastTimestamp = millis();

byte songID = 0; // 0 -> default loop

byte red2;
byte blue2;
#define green2 		255	//byte green2;
#define center_x 	10	//byte center_x;
#define center_y 	10	//byte center_y;

byte incomingMidiByte;
byte midiStatusByte;
byte midiDataByte1;
byte midiDataByte2;

byte voltageSmooth;

boolean progStroboIsBlack = false;	// for strobo
byte secondsForVoltage = 0;
volatile unsigned int millisForVoltage = 0;
volatile unsigned int millisSelf = 0;		// achtung!! -> kann nur bis 65.536 zaehlen!!
volatile unsigned int nextChangeMillis = 100000;		// start value = 10 sec
volatile int millisForLED = 0;
volatile int millisForFastLED = 0;
volatile boolean flag_processFastLED = false;
volatile boolean nextChangeMillisAlreadyCalculated = false;
volatile byte nextSongPart = 0;
volatile byte prog = 0;


unsigned int lastLEDchange = millis();
int ledState = LOW;             // ledState used to set the LED --TODO: nur test mit interner LED



//=====================================================================
//=========== HELPER FUNCTIONS ========================================
//=====================================================================

int getRandomColorValue() {
	int farbZahl = random(1, 4);
	int farbe = 0;
	switch (farbZahl) {
	case 1:
		farbe = 0; // 0
		break;
	case 2:
		farbe = 127; // 127
		break;
	case 3:
		farbe = 255;
		break;
	}
	return farbe;
}

byte r = getRandomColorValue();
byte g = getRandomColorValue();
byte b = getRandomColorValue();

//==================================================================
//=========== FX programs ==========================================
//==================================================================
// TODO:
// snake, tetris, pong, pac man, scrolltext -> FastLED GFX zum laufen bekommen!!
// myRemapping table  von hier aus an die lib übergeben!

//--- progBlingBlingColoring -----
// leds werden zufällig mit der selben farbe eingeschaltet und einige wenige zufällig ausgeschaltet
// alle x sekunden wird die eine der drei farbkomponenten zufällig geändert
int progBlingBlingColoring_rounds = 0;

void progBlingBlingColoring(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		nextChangeMillis = durationMillis;
		//nextChangeMillis = round((float)durationMillis / (float)9.0f);	// TODO: diesen wert eurieren und anpassen!!
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	if (progBlingBlingColoring_rounds == 0) {
		r = getRandomColorValue();
		g = getRandomColorValue();
		b = getRandomColorValue();
	}

	//set random pixel to defined color
	leds.m_LED[random(0, anz_LEDs)] = CRGB(r, g, b);

	// delete 1 pixel sometimes
	if (random(0, 3) == 1) leds.m_LED[random(0, anz_LEDs)] = CRGB::Black;

	FastLED.show();

	// after 15 seconds change 1 part of the color randomly
	if (millis() - lastTimestamp > 15000) {

		randomSeed(analogRead(4));	// damit werden die pseudo zufallszahlen neu "gemischt" // analogread(4) erzeugt irgendeinen wert mit dem dann der zeiger in der random tabelle neu positioniert wird

		progBlingBlingColoring_rounds++;
		if (progBlingBlingColoring_rounds == 4) progBlingBlingColoring_rounds = 1;

		r = getRandomColorValue();
		g = getRandomColorValue();
		b = getRandomColorValue();

		lastTimestamp = millis();
	}
}

void progFastBlingBling(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)9.65f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	FastLED.clear();
	FastLED.setBrightness(255);
	//set random pixel to defined color
	leds.m_LED[random(0, anz_LEDs)] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
	FastLED.show();
}

void progFullColors(unsigned int durationMillis, byte nextPart, unsigned int del) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)1.0f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	if (millis() - lastTimestamp > del) {

		r = getRandomColorValue();
		g = getRandomColorValue();
		b = getRandomColorValue();

		if (r == 0 && g == 0 && b == 0) {
			r = getRandomColorValue();
			g = getRandomColorValue();
			b = getRandomColorValue();
		}
		FastLED.showColor(CRGB(r, g, b));
		lastTimestamp = millis();	// restart timer
	}
}

void progFullColorsWithFading(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)2.25f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	if (millis() - lastTimestamp > 5000) {
		byte lastRed = r;
		byte lastGreen = g;
		byte lastBlue = b;

		r = getRandomColorValue();
		g = getRandomColorValue();
		b = getRandomColorValue();

		int diff_r = r - lastRed;
		int diff_g = g - lastGreen;
		int diff_b = b - lastBlue;

		for (int i = 0; i < abs(diff_r); i++) {
			if (diff_r < 0) FastLED.showColor(CRGB(lastRed - i, lastGreen, lastBlue));
			else FastLED.showColor(CRGB(lastRed + i, lastGreen, lastBlue));
		}
		for (int i = 0; i < abs(diff_g); i++) {
			if (diff_g < 0) FastLED.showColor(CRGB(r, lastGreen - i, lastBlue));
			else FastLED.showColor(CRGB(r, lastGreen + i, lastBlue));
		}
		for (int i = 0; i < abs(diff_b); i++) {
			if (diff_b < 0) FastLED.showColor(CRGB(r, g, lastBlue - i));
			else FastLED.showColor(CRGB(r, g, lastBlue + i));
		}

		FastLED.showColor(CRGB(r, g, b));
		lastTimestamp = millis();	// restart timer
	}
}

void progStrobo(unsigned int durationMillis, byte nextPart, unsigned int del, int red, int green, int blue) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)1.3f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	if (millis() - lastTimestamp > del) {

		//--- switch color ---
		if (progStroboIsBlack) {
			FastLED.showColor(CRGB(red, green, blue));
			progStroboIsBlack = false;
		}
		else {
			FastLED.showColor(CRGB::Black);
			progStroboIsBlack = true;
		}
		lastTimestamp = millis();	// restart timer
	}
}

void progMatrixScanner(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)3.95f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	for (int i = -3; i < 23; i++) {
		leds.DrawFilledRectangle(i - 1, 0, i + 2, MATRIX_HEIGHT, CRGB::Red);
		leds.DrawFilledRectangle(i, 0, i + 1, MATRIX_HEIGHT, CRGB::White);
		leds.DrawFilledRectangle(i - 5, 0, i - 2, MATRIX_HEIGHT, CRGB::Black);
		FastLED.show();
		//		delay((10 +i));
	}
	for (int i = 20; i > -3; i--) {
		leds.DrawFilledRectangle(i, 0, i + 2, MATRIX_HEIGHT, CRGB::Red);
		leds.DrawFilledRectangle(i + 2, 0, i + 4, MATRIX_HEIGHT, CRGB::Black);
		FastLED.show();
		//		delay((10 +i));
	}
}

void progStern_initialize() {
	r = 255;
	g = getRandomColorValue();
	b = 127;
	red2 = getRandomColorValue();
	//	green2 = 255;
	blue2 = getRandomColorValue();
	//	center_x = 10;
	//	center_y = 10;
}
void progStern(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)5.85f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	for (int i = 0; i < 10; i++) {

		FastLED.clear();

		leds.DrawLine(center_x - i, 0, center_x + i, 22, CRGB(r, g, b)); 			// 10, 0, 10, 22
		leds.DrawLine(center_x - i + 1, 0, center_x + i + 1, 22, CRGB(red2, green2, blue2));  	// 11, 0, 11, 22
		leds.DrawLine(0, i + 1, 21, 22 - i, CRGB(r, g, b));
		leds.DrawLine(0, i, 21, 21 - i, CRGB(red2, green2, blue2));
		leds.DrawLine(0, center_y + i + 1, 21, center_y - i + 1, CRGB(r, g, b));
		leds.DrawLine(0, center_y + i, 21, center_y - i, CRGB(red2, green2, blue2));
		leds.DrawLine(i, 22, 22 - i, 0, CRGB(r, g, b));
		leds.DrawLine(i - 1, 22, 21 - i, 0, CRGB(red2, green2, blue2));

		FastLED.show();
		//		delay(10);
	}
}

void progBlack(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)1.0f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	FastLED.clear();	// TODO fixen
	FastLED.show();
}

void progCircles(unsigned int durationMillis, byte nextPart, unsigned int msForChange) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)1.0f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	if (millis() - lastTimestamp > msForChange) {
		leds.DrawFilledCircle(random(0, 21), random(0, 22), random(3, 10), CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue()));
		FastLED.show();

		lastTimestamp = millis();
	}
}

void progRandomLines(unsigned int durationMillis, byte nextPart, unsigned int msForChange) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)1.05f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	if (millis() - lastTimestamp > msForChange) {
		byte x1 = random(0, 21);
		byte x2 = random(0, 21);
		r = getRandomColorValue();
		g = getRandomColorValue();
		b = getRandomColorValue();

		if (r == 0 && g == 0 && b == 0) {
			r = getRandomColorValue();
			g = getRandomColorValue();
			b = getRandomColorValue();
		}

		FastLED.clear();
		leds.DrawLine(x1 - 1, 0, x2 - 1, 22, CRGB(r, g, b));
		leds.DrawLine(x1, 0, x2, 22, CRGB(r, g, b));
		leds.DrawLine(x1 + 1, 0, x2 + 1, 22, CRGB(r, g, b));
		FastLED.show();

		lastTimestamp = millis();
	}
}

void progMovingLines(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)9.1f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	for (int i = 0; i < 26; i++) {
		FastLED.clear();
		leds.DrawLine(i, 0, 25 - i, 22, CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue()));
		FastLED.show();
	}
	for (int i = 0; i < 12; i++) {
		FastLED.clear();
		leds.DrawLine(25, i, 0, 22 - i, CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue()));
		FastLED.show();
	}
	for (int i = 12; i > -1; i--) {
		FastLED.clear();
		leds.DrawLine(25, i, 0, 22 - i, CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue()));
		FastLED.show();
	}
	for (int i = 25; i > -1; i--) {
		FastLED.clear();
		leds.DrawLine(i, 0, 25 - i, 22, CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue()));
		FastLED.show();
	}
	for (int i = 0; i < 11; i++) {
		FastLED.clear();
		leds.DrawLine(0, i, 25, 22 - i, CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue()));
		FastLED.show();
	}
	for (int i = 10; i > -1; i--) {
		FastLED.clear();
		leds.DrawLine(0, i, 25, 22 - i, CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue()));
		FastLED.show();
	}
}

void progOutline(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)2.15f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	int anz;
	//	const int dauer = 60;

	for (int y = 0; y < 9; y++) {
		FastLED.clear();
		switch (y) {
		case 0:
			anz = (sizeof(outlinePath1) / sizeof(outlinePath1[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath1[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 1:
			anz = (sizeof(outlinePath2) / sizeof(outlinePath2[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath2[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 2:
			anz = (sizeof(outlinePath3) / sizeof(outlinePath3[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath3[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 3:
			anz = (sizeof(outlinePath4) / sizeof(outlinePath4[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath4[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 4:
			anz = (sizeof(outlinePath5) / sizeof(outlinePath5[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath5[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 5:
			anz = (sizeof(outlinePath6) / sizeof(outlinePath6[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath6[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 6:
			anz = (sizeof(outlinePath7) / sizeof(outlinePath7[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath7[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 7:
			anz = (sizeof(outlinePath8) / sizeof(outlinePath8[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath8[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 8:
			anz = (sizeof(outlinePath9) / sizeof(outlinePath9[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath9[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		}
		FastLED.show();
		//		delay(dauer);
	}

	for (int y = 8; y > -1; y--) {
		FastLED.clear();
		switch (y) {
		case 0:
			anz = (sizeof(outlinePath1) / sizeof(outlinePath1[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath1[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 1:
			anz = (sizeof(outlinePath2) / sizeof(outlinePath2[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath2[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 2:
			anz = (sizeof(outlinePath3) / sizeof(outlinePath3[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath3[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 3:
			anz = (sizeof(outlinePath4) / sizeof(outlinePath4[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath4[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 4:
			anz = (sizeof(outlinePath5) / sizeof(outlinePath5[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath5[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 5:
			anz = (sizeof(outlinePath6) / sizeof(outlinePath6[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath6[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 6:
			anz = (sizeof(outlinePath7) / sizeof(outlinePath7[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath7[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 7:
			anz = (sizeof(outlinePath8) / sizeof(outlinePath8[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath8[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		case 8:
			anz = (sizeof(outlinePath9) / sizeof(outlinePath9[0]));
			for (int i = 0; i < anz; i++) {
				int test = outlinePath9[i];
				leds.m_LED[test] = CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
			}
			break;
		}
		FastLED.show();
		//		delay(dauer);
	}
}

void progRunningPixel(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)1.0f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	int last_x = -1;
	int last_y = -1;
	FastLED.setBrightness(255);
	FastLED.clear();

	for (int y = 0; y < MATRIX_HEIGHT; y++) {
		for (int x = 0; x < MATRIX_WIDTH; x++) {
			leds.DrawLine(x, y, x, y, CRGB(getRandomColorValue(), getRandomColorValue(), getRandomColorValue()));
			leds.DrawLine(last_x, last_y, last_x, last_y, CRGB::Black);
			last_x = x;
			last_y = y;
			FastLED.show();
			//			delay(15);
		}
	}
}

//=== progCLED =====================
uint8_t progCLED_hue;
int16_t progCLED_counter;

void progCLED(unsigned int durationMillis, byte nextPart) {

	//--- standard-part um dauer und naechstes programm zu speichern ----
	if (!nextChangeMillisAlreadyCalculated) {
		FastLED.clear(true);
		// workaround: die eigentlichen millis werden korrigiert auf die faktische dauer
		//nextChangeMillis = round((float)durationMillis / (float)2.25f);	// TODO: diesen wert eurieren und anpassen!!
		nextChangeMillis = durationMillis;
		nextSongPart = nextPart;
		nextChangeMillisAlreadyCalculated = true;
		//		Serial.println(nextChangeMillis);
	}
	//---------------------------------------------------------------------

	int16_t x, y;
	uint8_t h;
	FastLED.clear();

	h = progCLED_hue;
	if (progCLED_counter < 1125) {
		// ** Fill LED's with diagonal stripes
		for (x = 0; x < (leds.Width() + leds.Height()); ++x)
		{
			leds.DrawLine(x - leds.Height(), leds.Height() - 1, x, 0, CHSV(h, 255, 255));
			h += 16;
		}
	}
	else {
		// ** Fill LED's with horizontal stripes
		for (y = 0; y < leds.Height(); ++y)
		{
			leds.DrawLine(0, y, leds.Width() - 1, y, CHSV(h, 255, 255));
			h += 16;
		}
	}
	progCLED_hue += 4;

	if (progCLED_counter < 375) leds.HorizontalMirror();
	else if (progCLED_counter < 625) leds.VerticalMirror();
	else if (progCLED_counter < 875) leds.QuadrantMirror();
	else if (progCLED_counter < 1125) leds.QuadrantRotateMirror();
	else if (progCLED_counter < 1500) leds.TriangleTopMirror();
	else if (progCLED_counter < 1750) leds.TriangleBottomMirror();
	else if (progCLED_counter < 2000) leds.QuadrantTopTriangleMirror();
	else if (progCLED_counter < 2250) leds.QuadrantBottomTriangleMirror();

	progCLED_counter++;
	if (progCLED_counter >= 2250) progCLED_counter = 0;

	FastLED.show();
}

//==========================================================================

void switchToSong(byte song) {

	millisSelf = 0;
	lastTimestamp = millis();
	//	timer_start = millis();

		//--- initializeValues ---
	progBlingBlingColoring_rounds = 0;
	progCLED_hue = 0;
	progCLED_counter = 0;
	progStern_initialize();

	//--- start song ----
	songID = song;
	prog = 0;
	nextChangeMillisAlreadyCalculated = false;	// bool wieder fuer naechstes programm freigeben
	
	//FastLED.clear(true); // TODO: brauchen wir das????

	Serial.println("switched program");
}

//========================================================

////---- check voltage @ PIN A2 as lipo safer ------
//void setupTimer() {
//	//set timer1 interrupt at 1hz
//	tccr1a = 0; 	// set entire tccr1a register to 0
//	tccr1b = 0; 	// same for tccr1b
//	tcnt1 = 0; 	//initialize counter value to 0
//	// set compare match register for 1hz increments
//	ocr1a = 1999; //1996;	// = (16.000.000 / (prescaler 8 * 1000hz)) - 1 = 1999 (must be <65536)
//	tccr1b |= (1 << wgm12); 	// turn on ctc mode
//	tccr1b |= (0 << cs12) | (1 << cs11) | (0 >> cs10); //prescaler: 8
//	timsk1 |= (1 << ocie1a); 	// enable timer compare interrupt
//}


void setupInterrupt() {
	TCCR3A	= 0;
	TCCR3B	= 0x0B;      // WGM32 (CTC) , Prescaler: // 0x0C = 256 // 0x0B = 64
	OCR3A	= 248;       // 16M/256/248 = 1 ms
	TIMSK3	= 0x02;      // enable compare interrupt
}

#define INCREMENT	1

ISR(TIMER3_COMPA_vect) {
	
	millisSelf = millisSelf + INCREMENT;
	millisForVoltage = millisForVoltage + INCREMENT;
	millisForLED = millisForLED + INCREMENT;
	millisForFastLED = millisForFastLED + INCREMENT;

	if (millisForFastLED >= 10) {
		millisForFastLED = 0;
		flag_processFastLED = true;	// process FastLED-loops only every 10 ms
	}

	if (millisForLED >= 500) {
		millisForLED = 0;
		PORTD ^= 0x40;	// toggle LED
	}

	if (millisSelf >= nextChangeMillis) {
		prog = nextSongPart;
		millisSelf = 0;
		nextChangeMillisAlreadyCalculated = false; // bool wieder fuer naechstes programm freigeben
	}
}

//// Interruptbehandlungsroutine für den Timer Compare Interrupt: VoltageMeterTimer
//void interruptHandler() {
//	millisSelf++;
//	millisForVoltage++;
//
//	if (millisSelf >= nextChangeMillis) {
//		//		Serial.println(nextChangeMillis);
//		prog = nextSongPart;
//		millisSelf = 0;
//		nextChangeMillisAlreadyCalculated = false; // bool wieder fuer naechstes programm freigeben
//	}
//}

//========================================================

void setup() {
	Serial.begin(31250);	// for midi

	//----- to be deleted
	pinMode(TEST_PIN_D7, OUTPUT);// TODO: nur test mit interner LED
	//---------------------

	//---- check voltage @ PIN A2 as lipo safer ------
	pinMode(LIPO_PIN, INPUT); //---- check voltage @ PIN F2 as lipo safer ------
	voltageSmooth = map(analogRead(LIPO_PIN), 0, 1023, 0, 120);	// zu beginn mit startwert initialisieren, damit nicht mit NULL gemittelt wird

	// initial LEDs
	FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds[0], leds.Size()).setCorrection(TypicalLEDStrip);
	FastLED.setBrightness(BRIGTHNESS);
	FastLED.clear(true);

	noInterrupts();				// Alle Interrupts temporär abschalten
		setupInterrupt();
	interrupts();				// alle Interrupts scharf schalten

	switchToSong(0);
}
//==============================================

void loop() {

	boolean debug = false;

	//---- start loop only when voltage is high enough
	if (voltageSmooth > 102) {	// only fire LEDs if voltage is > 10.2V

		if (flag_processFastLED) {	// LED loop only in certain time-slots to make ms-counter more accurate

			switch (songID) {

			case 0:
				defaultLoop();
				break;

	//		case 1:
	//			LearnToFly();
	//			break;

			case 2:
				Castle();
				break;

			case 3:
				TooClose();
				break;
			}

			flag_processFastLED = false;

			checkIncomingMIDI();

			//---- check voltage as lipo safer ------
			if (millisForVoltage >= 1000) { // TODO: zurueck auf 1.000
				millisForVoltage = 0;
				secondsForVoltage++;
			}
			/*
				if (secondsForVoltage >= SECONDSFORVOLTAGE) {
					secondsForVoltage = 0;
					Serial.println(analogRead(LIPO_PIN)); // TODO JUST TESTING
					voltageSmooth = 0.7 * voltageSmooth + 0.3 * map(analogRead(LIPO_PIN), 0, 1023, 0, 120);	// glaettungsfunktion um zittern zu vermeiden
				}
			*/
			if (debug) {
				Serial.print(voltageSmooth);
				Serial.print("\t");
				Serial.print(millisForVoltage);
				Serial.print("\t");
				Serial.println(secondsForVoltage);
			}
			//--------------------------------------------
		}
	}
	else {	// if voltage is too low let LED 0 blink red
		FastLED.clear();
		FastLED.show();
		delay(1000);
		FastLED.setBrightness(BRIGTHNESS);
		leds.m_LED[0] = CRGB::Red;
		FastLED.show();
		delay(1000);
	}
}

//---- scheint zu gehen ----
//		progFullColors();
//		progRandomLines(500);
//		progCircles(1000);
//		progCLED();
//		progStrobo(50, getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
//		progStrobo(50, 255, 255, 255);	// Weisser strobo

//---- PROBLEMATISCH!!!
//		progBlack()
//		progMovingLines();
//		progOutline(); // mehrere peaks, dann wieder mehrere pause
//		progFastBlingBling();
//		progFullColorsWithFading	// bei farbaenderung geht es nicht .... sonst schon!!
//		progBlingBlingColoring();
//		progStern();
//		progMatrixScanner();
//-----------------------

void checkIncomingMIDI() {

	boolean debug = false;

	do {
		if (Serial.available()) {
			incomingMidiByte = Serial.read(); //read first byte

			if (debug) Serial.println(incomingMidiByte);

			if (incomingMidiByte != 255) {	// wir filtern eigenartige 255er eingänge raus

				if (incomingMidiByte > 127) {	// es wurde ein statusbyte erkannt
					midiStatusByte = incomingMidiByte;
					midiDataByte1 = 0;	// neues Statusbyte => lösche alte datenbytes
					midiDataByte2 = 0;  // neues Statusbyte => lösche alte datenbytes

					if (debug) Serial.println(midiStatusByte);
				}
				else if (incomingMidiByte < 128) {	// es wurde ein datenbyte erkannt
					if (midiDataByte1 == 0) midiDataByte1 = incomingMidiByte;
					else if (midiDataByte2 == 0) midiDataByte2 = incomingMidiByte;

					if (debug) {
						Serial.print(midiStatusByte);
						Serial.print("\t");
						Serial.print(midiDataByte1);
						Serial.print("\t");
						Serial.println(midiDataByte2);
					}

					if (midiStatusByte == 185 && midiDataByte1 == 22 && midiDataByte2 > 0) {
						switchToSong(midiDataByte2);
						if (debug) {
							Serial.print("switch to song: ");
							Serial.println(midiDataByte2);
						}
					}
				}
			}
		}
	} while (Serial.available());
}
//==============================================

//void defaultLoop() {
//	FastLED.setBrightness(BRIGTHNESS); // zur sicherheit in jedem loop neu auf default setzen. ggf. kann einzelner fx das überschreiben
////	long duration_since_start = millis() - timer_start;
//
//	switch (prog) {
//
//	case 0:
//		progBlingBlingColoring(60000, 1);//3    59,5hz
//		break;
//
//	case 1:
//		progStern(15000, 2);
//		break;
//
//	case 2:	// random farbiger strobo
//		progStrobo(10000, 3, 50, getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
//		break;
//
//	case 3:
//		progCLED(50000, 4);	// matrix colors
//		break;
//
//	case 4:
//		progMatrixScanner(15000, 5);
//		break;
//
//	case 5:
//		progFullColors(20000, 6, 2000);
//		break;
//
//	case 6:
//		progStrobo(10000, 7, 50, 255, 255, 255);	// Weisser strobo
//		break;
//
//	case 7:
//		progCircles(30000, 8, 1000);
//		break;
//
//	case 8:
//		progFastBlingBling(15000, 9); //20s -> 3:13
//		break;
//
//	case 9:
//		progOutline(15000, 10);
//		break;
//
//	case 10:
//		progMovingLines(10000, 11);
//		break;
//
//	case 11:
//		progRandomLines(30000, 100, 500);
//		break;
//
//	case 100:
//		FastLED.clear();
//		switchToSong(0);	// SongID 0 == DEFAULT loop
//		break;
//	}
//}
////==============================================

void defaultLoop() {
	FastLED.setBrightness(BRIGTHNESS); // zur sicherheit in jedem loop neu auf default setzen. ggf. kann einzelner fx das überschreiben
//	long duration_since_start = millis() - timer_start;

	switch (prog) {

	case 0:
		progBlingBlingColoring(10000, 1);//3    59,5hz
		break;

	case 1:
		progStern(10000, 2);
		break;

	case 2:	// random farbiger strobo
		progStrobo(10000, 3, 50, getRandomColorValue(), getRandomColorValue(), getRandomColorValue());
		break;

	case 3:
		progCLED(10000, 4);	// matrix colors
		break;

	case 4:
		progMatrixScanner(10000, 5);
		break;

	case 5:
		progFullColors(10000, 6, 2000);
		break;

	case 6:
		progStrobo(10000, 7, 50, 255, 255, 255);	// Weisser strobo
		break;

	case 7:
		progCircles(10000, 8, 1000);
		break;

	case 8:
		progFastBlingBling(10000, 9); //20s -> 3:13
		break;

	case 9:
		progOutline(10000, 10);
		break;

	case 10:
		progMovingLines(10000, 11);
		break;

	case 11:
		progRandomLines(10000, 100, 500);
		break;

	case 100:
		FastLED.clear();
		switchToSong(0);	// SongID 0 == DEFAULT loop
		break;
	}
}
//==============================================

void LearnToFly() {
	FastLED.setBrightness(BRIGTHNESS); // zur sicherheit in jedem loop neu auf default setzen. ggf. kann einzelner fx das überschreiben
//	long duration_since_start = millis() - timer_start;

	switch (prog) {

	case 0:
		progCircles(30476, 1, 952); 		// 30476
		break;

	case 1:
		progRandomLines(45714, 2, 476); 	// 43537
		break;

	case 2:
		progFullColors(30476, 3, 952); 		// 30476
		break;

	case 3:
		progCircles(15238, 4, 952); 		// 15238
		break;

	case 4:
		progRandomLines(30476, 5, 476);		// 29025
		break;

	case 5:
		progFastBlingBling(3810, 6);		// 395
		break;

	case 6:
		progStern(26667, 7);				// 4558
		break;

	case 7:
		progStrobo(15238, 8, 50, 255, 255, 255); // Weisser strobo // 11722
		break;

	case 8:
		progFullColors(15238, 9, 952);		// 15238
		break;

	case 9:
		progCLED(7619, 10);					// 3386
		break;

	case 10:
		progFullColors(7619, 11, 952);		// 7619
		break;

	case 11:
		progStrobo(7619, 12, 50, getRandomColorValue(), getRandomColorValue(), getRandomColorValue()); // 5861
		break;

	case 12:
		progBlingBlingColoring(65535, 100); // 65535 is max for unsigned int!
		break;

	case 100:
		FastLED.clear();
		switchToSong(0);	// SongID 0 == DEFAULT loop
		break;
	}
}
//==============================================

void Castle() {
	FastLED.setBrightness(BRIGTHNESS); // zur sicherheit in jedem loop neu auf default setzen. ggf. kann einzelner fx das überschreiben
//	long duration_since_start = millis() - timer_start;

	switch (prog) {

	case 0:
		progBlingBlingColoring(20308, 1); // 65535 is max for unsigned int!
		break;

	case 1:
		progRandomLines(7385, 2, 462);
		break;

	case 2:
		progMatrixScanner(29538, 3);
		break;

	case 3:
		progFastBlingBling(18462, 4);
		break;

	case 4:
		progStern(29538, 5);
		break;

	case 5:
		progCircles(29538, 6, 923);
		break;

	case 6:
		progFastBlingBling(14769, 7);
		break;

	case 7:
		progStern(29538, 8);
		break;

	case 8:
		progCLED(44308, 9);
		break;

	case 9:
		progStrobo(3692, 10, 50, 255, 255, 255); // Weisser strobo
		break;

	case 10:
		progStern(29538, 11);
		break;

	case 11:
		progOutline(12880, 12);
		break;

	case 12:
		progBlingBlingColoring(65535, 100); // 65535 is max for unsigned int!
		break;

	case 100:
		FastLED.clear();
		switchToSong(0);	// SongID 0 == DEFAULT loop
		break;
	}
}
//==============================================

void TooClose() {
	FastLED.setBrightness(BRIGTHNESS); // zur sicherheit in jedem loop neu auf default setzen. ggf. kann einzelner fx das überschreiben
//	long duration_since_start = millis() - timer_start;

	switch (prog) {

	case 0:
		progCircles(30476, 1, 952); 		// 30476
		break;

	case 1:
		progRandomLines(45714, 2, 476); 	// 43537
		break;

	case 2:
		progFullColors(30476, 3, 952); 		// 30476
		break;

	case 3:
		progCircles(15238, 4, 952); 		// 15238
		break;

	case 4:
		progRandomLines(30476, 5, 476);		// 29025
		break;

	case 5:
		progFastBlingBling(3810, 6);		// 395
		break;

	case 6:
		progStern(26667, 7);				// 4558
		break;

	case 7:
		progStrobo(15238, 8, 50, 255, 255, 255); // Weisser strobo // 11722
		break;

	case 8:
		progFullColors(15238, 9, 952);		// 15238
		break;

	case 9:
		progCLED(7619, 10);					// 3386
		break;

	case 10:
		progFullColors(7619, 11, 952);		// 7619
		break;

	case 11:
		progStrobo(7619, 12, 50, getRandomColorValue(), getRandomColorValue(), getRandomColorValue()); // 5861
		break;

	case 12:
		progBlingBlingColoring(65535, 100); // 65535 is max for unsigned int!
		break;

	case 100:
		FastLED.clear();
		switchToSong(0);	// SongID 0 == DEFAULT loop
		break;
	}
}
////==============================================

//=====================================================
//=====================================================


//#include "Arduino.h"
//
//#define TEST_PIN_D7         6  // internal LED
//#define MIDI_RX_PIN         2  // D2
//byte incomingMidiByte;
//byte midiStatusByte;
//byte midiDataByte1;
//byte midiDataByte2;
//
//unsigned int lastLEDchange = millis();
//int ledState = LOW;             // ledState used to set the LED --TODO: nur test mit interner LED
//
//
////========================================================
//
//void setup() {
//	Serial.begin(31250);	// for midi
//	pinMode(MIDI_RX_PIN, INPUT);// TODO: nur test mit interner LED
//	pinMode(TEST_PIN_D7, OUTPUT);// TODO: nur test mit interner LED
//}
////==============================================
//
//void loop() {
//
//	if (millis() - lastLEDchange >= 500) {
//		//Serial.print("led change: ");
//		//Serial.println(ledState);
//		if (ledState == LOW) ledState = HIGH;
//		else ledState = LOW;
//		lastLEDchange = millis();
//		digitalWrite(TEST_PIN_D7, ledState);
//	}
//
//	checkIncomingMIDI();
//}
//
//
//void checkIncomingMIDI() {
//
//	boolean debug = true;
//
//	do {
//		if (Serial.available()) {
//			incomingMidiByte = Serial.read(); //read first byte
//
//			Serial.println(incomingMidiByte);
//
//			if (incomingMidiByte != 255) {	// wir filtern eigenartige 255er eingänge raus
//
//				if (incomingMidiByte > 127) {	// es wurde ein statusbyte erkannt
//					midiStatusByte = incomingMidiByte;
//					midiDataByte1 = 0;	// neues Statusbyte => lösche alte datenbytes
//					midiDataByte2 = 0;  // neues Statusbyte => lösche alte datenbytes
//
//					if (debug) Serial.println(midiStatusByte);
//				}
//				else if (incomingMidiByte < 128) {	// es wurde ein datenbyte erkannt
//					if (midiDataByte1 == 0) midiDataByte1 = incomingMidiByte;
//					else if (midiDataByte2 == 0) midiDataByte2 = incomingMidiByte;
//
//					if (debug) {
//						Serial.print(midiStatusByte);
//						Serial.print("\t");
//						Serial.print(midiDataByte1);
//						Serial.print("\t");
//						Serial.println(midiDataByte2);
//					}
//
//					if (midiStatusByte == 185 && midiDataByte1 == 22 && midiDataByte2 > 0) {
//						//switchToSong(midiDataByte2);
//						if (debug) {
//							Serial.print("switch to song: ");
//							Serial.println(midiDataByte2);
//						}
//					}
//				}
//			}
//		}
//	} while (Serial.available());
//}


//=====================================================


//const byte teensyLED = 6;   // 6 = helle LED // 0 der 11 = dunkle LED // PD6 Teensy LED
//volatile int milliseconds = 0;
//volatile long ms = millis();
//int ms_flag;
//
//void setup() {
//    pinMode(teensyLED, OUTPUT);
//
//    Serial.begin(115200);
//
//    noInterrupts();
//    TCCR3A = 0;
//    TCCR3B = 0x0B;      // WGM32 (CTC) , Prescaler = 256 // 0x0C = 256 // 0x0B = 64
//    OCR3A = 248;        // 16M/256/31250 = 0,5 seconds
//    TIMSK3 = 0x02;      // enable compare interrupt
//    interrupts();
//}
//
//ISR(TIMER3_COMPA_vect) {
//    //PIND = PIND | 0x40;    // toggle pin PD6 Teensy LED (helle LED)
//    milliseconds = milliseconds + 1;
//    if (milliseconds >= 1000) {
//        PIND = PIND | 0x40;    // toggle pin PD6 Teensy LED (helle LED)
//        milliseconds = 0;
//        ms_flag = 1;
//    }
//}
//
//void loop() {
//    //if (milliseconds >= 1000) {
//        if (ms_flag) {
//            Serial.print("1 ms");
//            ms_flag = 0;
//        }
//    //}
//}

////---- LED TESTROUTINE ----
//const int ledPin = 6;
//int ledState = LOW;
//long previousMillis = 0;
//long interval = 1000;
//
//void setup() {
//    pinMode(ledPin, OUTPUT);
//}
//
//void loop() {
//    unsigned long currentMillis = millis();
//    if (currentMillis - previousMillis > interval) {
//        previousMillis = currentMillis;
//        if (ledState == LOW) ledState = HIGH;
//        else ledState = LOW;
//        digitalWrite(ledPin, ledState);
//    }
//}
////---- ENDE LED TESTROUTINE ----
