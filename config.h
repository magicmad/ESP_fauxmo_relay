
#define SERIAL_BAUDRATE                 115200
#define LED                             2

#define WIFI_SSID "xxx"
#define WIFI_PASS "yyy"

// uncomment the following lines to use a static IP instead of DHCP
const IPAddress ip(192, 168, 1, 111); // device IP
const IPAddress gateway(192, 168, 1, 1); // gateway IP
const IPAddress subnet(255, 255, 255, 0); // subnet mask


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



