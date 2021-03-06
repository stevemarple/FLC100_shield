#include <AsyncDelay.h>
#include <Wire.h>
#include <RTC.h>
#include <MCP342x.h>
#include <FLC100.h>


MCP342x adc;
// List of possible addresses for the ADC; 0x68 and 0x6F omitted since
// they clash with the address used by the DS1307 and similar
// real-time clocks and the MCP7941x.
const uint8_t adcAddressList[] = {0x69, 0x6A, 0x6B,
				  0x6C, 0x6D, 0x6E};
bool haveMCP342x = false;

uint8_t xrfSleepState = 0;
uint8_t xrfResetState = 0;
uint8_t adcPowerState = 1;

void setup(void)
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(XRF_RESET, OUTPUT);
  pinMode(XRF_SLEEP, OUTPUT);
  pinMode(FLC100_POWER, OUTPUT);

  // Turn on 5V supply so that the ADC can be probed
  adcPowerState = 1;
  digitalWrite(FLC100_POWER, adcPowerState);

  // Reset all MCP342x devices
  adc.generalCallReset();
  haveMCP342x = adc.autoprobe(adcAddressList, sizeof(adcAddressList));
  if (haveMCP342x) {
    Serial.print("Detected ADC at 0x");
    Serial.println(adc.getAddress(), HEX);
  }
  else
    Serial.println("Could not detect MCP7941x");
}


const uint8_t bufLen = 30;
char buffer[bufLen + 1] = {'\0'};
uint8_t bufPos = 0;
unsigned long last = 0;
MCP342x::Gain gain = MCP342x::gain1;

void loop(void)
{
  while (Serial.available()) {
    char c = Serial.read();
    if ((c == '\r' || c == '\n')) {
      buffer[bufPos] = '\0';
      if (bufPos <= bufLen) {
	if (strcmp_P(buffer, PSTR("adcpower")) == 0) { 
	  adcPowerState = !adcPowerState;
	  digitalWrite(FLC100_POWER, adcPowerState);
	}
	else if (strcmp_P(buffer, PSTR("xrfsleep")) == 0) {
	  xrfSleepState = !xrfSleepState;
	  digitalWrite(XRF_SLEEP, xrfSleepState);
	}
	else if (strcmp_P(buffer, PSTR("xrfreset")) == 0) {
	  xrfSleepState = !xrfResetState;
	  digitalWrite(XRF_RESET, xrfResetState);
	}
	else if (strcmp_P(buffer, PSTR("gain1")) == 0) {
	  gain = adc.gain1;
	}
	else if (strcmp_P(buffer, PSTR("gain2")) == 0) {
	  gain = adc.gain2;
	}
	else if (strcmp_P(buffer, PSTR("gain4")) == 0) {
	  gain = adc.gain4;
	}
	else if (strcmp_P(buffer, PSTR("gain8")) == 0) {
	  gain = adc.gain8;
	}
	else {
	  Serial.print(buffer);
	  Serial.println(": ERROR");
	}
      }
      bufPos = 0;
    }
    else if (bufPos < bufLen)
      // Store character
      buffer[bufPos++] = c; 
  }

  if (millis() - last > 2000) {
    last = millis();
    Serial.println("--------------");
    Serial.print("ADC power: ");
    Serial.println(adcPowerState ? '1' : '0');
    Serial.print("ADC gain: ");
    Serial.print((1 << gain), DEC);
    Serial.println('x');
    Serial.print("XRF sleep: ");
    Serial.println(xrfSleepState ? '1' : '0');
    Serial.print("XRF reset: ");
    Serial.println(xrfResetState ? '1' : '0');

    Serial.print("Vin: ");
    Serial.print((3.3 * analogRead(BATTERY_ADC)) / 1024);
    Serial.println(" V");
    if (haveMCP342x && adcPowerState) {
      // Sample channel 0
      int32_t x;
      MCP342x::Config status;
      adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
			 MCP342x::resolution18, gain,
			 300000UL, x, status);
      Serial.print("X: ");
      Serial.println(x);
    }

  }

  
}

