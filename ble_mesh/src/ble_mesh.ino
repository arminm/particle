/*
 * Project ble_light_cane
 * Description: BLE Light Cane
 * Author: Armin Mahmoudi
 * Date: 9/26/2020
 */

SYSTEM_THREAD(ENABLED);

// SYSTEM_MODE(AUTOMATIC);
SYSTEM_MODE(MANUAL);
SerialLogHandler logHandler;

int MODE = 2; // rainbow
bool AUTO_MODE = true;
int DELAY_MS = 30;
unsigned int MODE_INDEX = 0;
int BRIGHTNESS = 10;
int WIFI_ON = 0;
bool BLE_ON = true;
uint16_t BLE_ADVERTISING_TIMEOUT = 5 * 60 * 1000 / 10; // unit is 10ms
system_tick_t BLE_TIMEOUT_MS = BLE_ADVERTISING_TIMEOUT * 10;
char WIFI_SSID[50];
char WIFI_PASS[50];

system_tick_t startTime = 0;

// Persisted Object
const unsigned int MAX_NAME_LEN = 72;
struct PersistedObject
{
  uint8_t version;
  char name[MAX_NAME_LEN];
};
PersistedObject DEFAULT_PROPS = {0, "Light Cane"};
PersistedObject props;

// declare other functions
void dispayModeAtIndex(int mode, int index);
unsigned int hexToUnsignedInt(String hex);
void setBrightness(int b);
int chargingMode();
void checkBattery(BleCharacteristic batteryLevelCharacteristic);
bool isCharging();
float readBattery();
uint32_t wheel(int angle);

// UUID for battery service
BleUuid batteryServiceUUID = BleUuid(0x180F);
BleUuid batteryCharUUID = BleUuid(0x2A19);

// Batt char
BleCharacteristic batteryLevelCharacteristic;

const char *serviceUuid = "52FBD5CA-8C9E-4C84-B3F7-E674BB439420";
const char *mode = "52FBD5CE-8C9B-4C84-B3F7-E674BB439421";
const char *delayMs = "52FBD5CE-8C9C-4C84-B3F7-E674BB439422";
const char *brightness = "52FBD5CF-8C9D-4C84-B3F7-E674BB439424";
const char *wifiOn = "52FBD5CH-8C9D-4C84-B3F7-E674BB439426";
const char *wifiSSID = "52FBD5CI-8C9D-4C84-B3F7-E674BB439427";
const char *wifiPassword = "52FBD5CJ-8C9D-4C84-B3F7-E674BB439428";
const char *caneName = "52FBD5CL-8C9D-4C84-B3F7-E674BB439430";

// Set the BLE service
BleUuid bleService(serviceUuid);

// Set up BLE characteristics
BleCharacteristic modeCharacteristic("mode", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, mode, serviceUuid, onDataReceived, (void *)mode);
BleCharacteristic delayMsCharacteristic("delayMs", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, delayMs, serviceUuid, onDataReceived, (void *)delayMs);
BleCharacteristic brightnessCharacteristic("brightness", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, brightness, serviceUuid, onDataReceived, (void *)brightness);
BleCharacteristic caneNameCharacteristic("caneName", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, caneName, serviceUuid, onDataReceived, (void *)caneName);
BleCharacteristic wifiOnCharacteristic("wifiOn", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, wifiOn, serviceUuid, onDataReceived, (void *)wifiOn);
BleCharacteristic wifiSSIDCharacteristic("wifiSSID", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, wifiSSID, serviceUuid, onDataReceived, (void *)wifiSSID);
BleCharacteristic wifiPasswordCharacteristic("wifiPassword", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, wifiPassword, serviceUuid, onDataReceived, (void *)wifiPassword);

// Static function for handling Bluetooth Low Energy callbacks
static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{
  if (context == mode)
  {
    MODE = data[0];
    MODE_INDEX = 0;
    AUTO_MODE = false;
  }
  else if (context == delayMs)
  {
    DELAY_MS = data[0];
  }
  else if (context == brightness)
  {
    BRIGHTNESS = data[0];
    setBrightness(BRIGHTNESS);
  }
  else if (context == wifiOn)
  {
    WIFI_ON = data[0];
  }
  else if (context == wifiSSID)
  {
    for (size_t i = 0; i < len; i++)
    {
      WIFI_SSID[i] = static_cast<char>(data[i]);
    }
  }
  else if (context == wifiPassword)
  {
    for (size_t i = 0; i < len; i++)
    {
      WIFI_PASS[i] = static_cast<char>(data[i]);
    }
    WiFi.clearCredentials();
    WiFi.setCredentials(WIFI_SSID, WIFI_PASS);
  }
  else if (context == caneName)
  {
    if (len <= MAX_NAME_LEN)
    {
      for (size_t i = 0; i < len; i++)
      {
        props.name[i] = static_cast<char>(data[i]);
      }
      EEPROM.put(0, props);
    }
  }
}

void setWiFi(int on)
{
  if (on == 1 && !WiFi.ready() && !WiFi.connecting())
  {
    RGB.control(false);
    WiFi.on();
    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);
    waitUntil(WiFi.ready);
    if (WiFi.ready())
    {
      Particle.connect();
    }
    else
    {
      WiFi.off();
      RGB.control(true);
      WIFI_ON = 0;
    }
  }
  else if (on == 0 && WiFi.ready())
  {
    Particle.disconnect();
    WiFi.off();
    RGB.control(true);
  }
}

void setupBLE(char *name)
{
  BLE.on();
  BLE.setDeviceName(name);
  BLE.setAdvertisingTimeout(BLE_ADVERTISING_TIMEOUT);

  // Add the characteristics
  BLE.addCharacteristic(modeCharacteristic);
  modeCharacteristic.setValue(MODE);
  BLE.addCharacteristic(delayMsCharacteristic);
  delayMsCharacteristic.setValue(DELAY_MS);
  BLE.addCharacteristic(brightnessCharacteristic);
  brightnessCharacteristic.setValue(BRIGHTNESS);
  BLE.addCharacteristic(wifiOnCharacteristic);
  wifiOnCharacteristic.setValue(WIFI_ON);
  BLE.addCharacteristic(wifiSSIDCharacteristic);
  wifiSSIDCharacteristic.setValue(WIFI_SSID);
  BLE.addCharacteristic(wifiPasswordCharacteristic);
  wifiPasswordCharacteristic.setValue(WIFI_PASS);
  BLE.addCharacteristic(caneNameCharacteristic);

  batteryLevelCharacteristic = BleCharacteristic("bat", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, batteryCharUUID, batteryServiceUUID);
  BLE.addCharacteristic(batteryLevelCharacteristic);

  // start advertising!
  BLE.advertise();
}

void debugLogs()
{
  Log.info("MODE: %d, DELAY_MS: %d, BRIGHTNESS: %d", MODE, DELAY_MS, BRIGHTNESS);
}

// setup() runs once, when the device is first turned on.
void setup()
{
  startTime = millis();

  pinMode(PWR, INPUT);
  pinMode(CHG, INPUT);

  RGB.control(true);

  // load persisted props
  EEPROM.get(0, props); // 0 is the address (up to 4096 bytes on Argon)
  if (props.version != 0)
  {
    // EEPROM was empty -> initialize props
    PersistedObject defaultObj = {0, "Light Cane"};
    props = defaultObj;
  }
  setupBLE(props.name);

  // Set LED Strip brightness
  setBrightness(BRIGHTNESS);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  if (millis() < (startTime + 1000))
  {
    return; // not ready yet. I think bluetooth setup takes a second or two
  }
  else if (BLE_ON && millis() > (startTime + BLE_TIMEOUT_MS))
  {
    BLE.off();
    BLE_ON = false;
  }

  setWiFi(WIFI_ON);

  MODE_INDEX++;
  RGB.color(wheel(MODE_INDEX % 360));
  unsigned int mode = MODE;
  if (AUTO_MODE)
  {
    static unsigned int offset = 2;
    static unsigned int modes = 7;
    static unsigned int delay = 30000; // 30 seconds
    mode = (((millis() - startTime) / delay) % modes) + offset;
  }

  if (isCharging())
  {
    dispayModeAtIndex(chargingMode(), MODE_INDEX);
  }
  else
  {
    dispayModeAtIndex(mode, MODE_INDEX);
  }

  checkBattery(batteryLevelCharacteristic);
  // debugLogs();
  delay(DELAY_MS);
}
