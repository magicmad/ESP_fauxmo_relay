
#define SERIAL_BAUDRATE                 115200
#define LED                             2

#define WIFI_SSID "xxx"
#define WIFI_PASS "yyy"

// uncomment the following lines to use a static IP instead of DHCP
const IPAddress ip(192, 168, 1, 111); // device IP
const IPAddress gateway(192, 168, 1, 1); // gateway IP
const IPAddress subnet(255, 255, 255, 0); // subnet mask


// set number of used relays, their pin number and names
const unsigned int RelayCount = 3;
const unsigned int RelayPins[] = { 4, 5, 6 };
const char* RelayNames[] = { "Wasser eins", "Wasser zwei", "Wasser drei" };
// set automatic shutdown times in seconds for each relay (0 disables the auto power down feature)
const int ShutDownSeconds[] = { 600, 600, 0 };



// a button for each relay
const bool UseButtons = true;
const int ButtonPins[] = { 7, 8, 9 };          // use 0 to disable button
const unsigned int ButtonDelay = 50;          // debounce delay (increase if button toggles during press)




