#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"


#define SERIAL_BAUDRATE                 115200
#define LED                             2

#define WIFI_SSID "GnetFritz"
#define WIFI_PASS "singstarsingstar"


// set number of used relays, their pin number and names
const int RelayCount = 2;
const int RelayPins[] = { 4, 5 };
const char* RelayNames[] = { "Wasser eins", "Wasser zwei" };
// set automatic shutdown times in seconds for each relay (0 disables the auto power down feature)
const int shutDownSeconds[] = { 600, 600 };



// a button for each relay
const bool useButtons = false;
const int ButtonPins[] = { 6, 7 };
const unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers




// this library does all the wemo things
fauxmoESP fauxmo;

// variables used for auto power off; they store the turn-on-time for each relay.
unsigned long durations[RelayCount]; // 0 is off; other values are the ticks at turn on
int lastButtonState[RelayCount];
unsigned long lastDebounceTime[RelayCount];  // the last time the output pin was toggled



// -----------------------------------------------------------------------------
// Wifi..........Pls CHANGE YOUR WIFI CONFIGURATION!!!!!!!!!!!
// -----------------------------------------------------------------------------
void wifiSetup()
{
    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);

    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    WiFi.config(IPAddress(192,168,1,99), IPAddress(192,168,1,1), IPAddress(255,255,255,0), IPAddress(192,168,1,1));

    // Wait
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();

    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void setup()
{
	// setup serial port and clean garbage
	Serial.begin(SERIAL_BAUDRATE);
	Serial.println();
	Serial.println();

	// setup Wifi
	wifiSetup();

	// setup LED
	pinMode(LED, OUTPUT);
	pinMode(LED, HIGH);

	// setup relays
	for (int i = 0; i < RelayCount; i = i + 1)
	{
		Serial.print("setup pin: ");
		Serial.print(RelayPins[i]);
		Serial.print(" for device: ");
		Serial.print(RelayNames[i]);
		Serial.println();
		
		pinMode(RelayPins[i], OUTPUT);
		digitalWrite(RelayPins[i], HIGH);
		durations[i] = 0;
	}
	
	if(useButtons)
	{
		for (int i = 0; i < RelayCount; i = i + 1)
		{
			pinMode(ButtonPins[i], INPUT);
			lastButtonState[i] = LOW;
			lastDebounceTime[i] = 0;
		}
	}
	
	// start library
	fauxmo.enable(true);

	// Add virtual devices
	for (int i = 0; i < RelayCount; i = i + 1)
	{
		fauxmo.addDevice(RelayNames[i]);
	}

	// fauxmoESP 2.0.0 has changed the callback signature to add the device_id,
	// this way it's easier to match devices to action without having to compare strings.
	fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state)
	{
		Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
		digitalWrite(LED, !state);
		setRelay(device_id, state);
	});

	// Callback to retrieve current state (for GetBinaryState queries)
	fauxmo.onGetState([](unsigned char device_id, const char * device_name)
	{
		if(device_id < RelayCount)
		{
			return !digitalRead(RelayPins[device_id]);
		}
		else
		{
			Serial.print("Error: got invalid device id: ");
			Serial.println(device_id);
		}
	});
}

void loop()
{
	// Let the Library do its thing
	fauxmo.handle();

	unsigned long timeNow = millis();

	// check buttons
	if(useButtons)
	{
		for (int i = 0; i < RelayCount; i = i + 1)
		{
			int reading  = digitalRead(ButtonPins[i]);
			
			// If the switch changed, due to noise or pressing:
			if (reading  != lastButtonState[i])
			{
				// reset the debouncing timer
				lastDebounceTime[i] = millis();
			}

			if((timeNow - lastDebounceTime[i]) > debounceDelay)
			{
				// whatever the reading is at, it's been there for longer than the debounce
				// delay, so take it as the actual current state:
				
				// button is pressed. invert relay state.
				Serial.println("Button pressed");
				setRelay(i, durations[i] == 0);
			}

			// lastButtonState[i] = val;
		}
	}

	// check timers for auto power off
	for (int i = 0; i < RelayCount; i = i + 1)
	{
		// is there a shutdown time for this relay? and is relay active?
		if(shutDownSeconds[i] > 0 && durations[i] > 0)
		{
			unsigned long duration = timeNow - durations[i];
			if(duration > shutDownSeconds[i] * 1000)
			{
				Serial.print("Auto Shutdown Time exceeded for Relay: ");
				Serial.println(RelayNames[i]);
				Serial.print("Shutdown after ");
				Serial.print(duration / 1000);
				Serial.println(" seconds.");
				setRelay(i, false);
			}
		}
	}

	// make sure the ESP has some time to do whatever it wants to do
	yield();
}


// turns a relay on or off
void setRelay(unsigned char device_id, bool state)
{
	if(device_id < RelayCount)
	{
		Serial.print("Set Relay: ");
		Serial.print(RelayNames[device_id]);

		if (state)
		{
			digitalWrite(RelayPins[device_id], LOW);
			Serial.println(" -> on");
			// store turn-on time
			durations[device_id] = millis();;
		}
		else
		{
			digitalWrite(RelayPins[device_id], HIGH);
			Serial.println(" -> off");
			// reset turn-on time (its off)
			durations[device_id] = 0;
		}
	}
	else
	{
		Serial.print("Error: got invalid device id: ");
		Serial.println(device_id);
	}
}

