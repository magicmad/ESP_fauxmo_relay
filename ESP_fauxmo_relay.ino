#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
#else
    #include <ESP8266WiFi.h>
#endif
#include "fauxmoESP.h"
#include <Bounce2.h>
#include "config.h"


// buttons
Bounce * buttons = new Bounce[RelayCount];

// this library does all the wemo things
fauxmoESP fauxmo;

// variables used for auto power off; they store the turn-on-time for each relay.
unsigned long durations[RelayCount]; // 0 is off; other values are the ticks at turn on



// setup WIFI connection
void wifiSetup()
{
    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);
    
    // set IP - there is contriting info on when to call config. before or after begin
    WiFi.config(ip, gateway, subnet);
    
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    // connect to wifi
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait until connected
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

  // setup Buttons
	if(UseButtons)
	{
    Serial.println("setup buttons:");
    
    for (int i = 0; i < RelayCount; i++) 
    {
      if(ButtonPins[i] != 0)
      {
        Serial.print("setup pin as button: ");
        Serial.print(ButtonPins[i]);
        Serial.print(" for device: ");
        Serial.print(RelayNames[i]);
        Serial.println();
        
        buttons[i].attach( ButtonPins[i] , INPUT_PULLUP  );     //setup the bounce instance for the current button
        buttons[i].interval(ButtonDelay);                       // interval in ms
      }
    }
	}
	
	// start FAUXMO library
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
	if(UseButtons)
	{
		for (int i = 0; i < RelayCount; i = i + 1)
		{
      // do not check uninitialized button pins
      if(ButtonPins[i] != 0)
      {
        // Update the Bounce instance
        buttons[i].update();
        
        // If it fell (it was pressed for x ms and was now released), flag the need to toggle
        if ( buttons[i].fell() )
        {
          if(durations[i] == 0)
          {
            // turn on
            setRelay(i, true);
          }
          else
          {
            // turn off
            setRelay(i, false);
          }
        }
      }
		}
	}

	// check timers for auto power off
	for (int i = 0; i < RelayCount; i = i + 1)
	{
		// if relay is active and there is a shutdown time for this relay
		if(durations[i] > 0 && ShutDownSeconds[i] > 0)
		{
			unsigned long duration = timeNow - durations[i];
      // above shutdown duration?
			if(duration > ShutDownSeconds[i] * 1000)
			{
				Serial.print("Auto Shutdown Time exceeded for Relay: ");
				Serial.println(RelayNames[i]);
				Serial.print("Shutdown after ");
				Serial.print(duration / 1000);
				Serial.println(" seconds.");
        
        // turn off
				setRelay(i, false);
			}
		}
	}

	// make sure the ESP has some time to do whatever it wants to do (wifi..)
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

