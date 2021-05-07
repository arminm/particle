/*
 * Project ble_mesh
 * Description: BLE Mesh network
 * Author: Armin Mahmoudi
 * Date: 9/26/2020
 */

SYSTEM_THREAD(ENABLED);

// SYSTEM_MODE(AUTOMATIC);
SYSTEM_MODE(MANUAL);
SerialLogHandler logHandler;

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
void checkBattery(BleCharacteristic batteryLevelCharacteristic);
void setBrightness(int b);
float readBattery();
int chargingMode();
bool isCharging();

// UUID for battery service
BleUuid batteryServiceUUID = BleUuid(0x180F);
BleUuid batteryCharUUID = BleUuid(0x2A19);

// Batt char
BleCharacteristic batteryLevelCharacteristic;

const char *serviceUuid = "52FBD5CA-8C9E-4C84-B3F7-E674BB439420";
const char *mode = "52FBD5CE-8C9B-4C84-B3F7-E674BB439421";
const char *delayMs = "52FBD5CE-8C9C-4C84-B3F7-E674BB439422";
const char *rank = "52FBD5CE-8C9D-4C84-B3F7-E674BB439423";
const char *brightness = "52FBD5CF-8C9D-4C84-B3F7-E674BB439424";
const char *ledOn = "52FBD5CG-8C9D-4C84-B3F7-E674BB439425";
const char *wifiOn = "52FBD5CH-8C9D-4C84-B3F7-E674BB439426";
const char *wifiSSID = "52FBD5CI-8C9D-4C84-B3F7-E674BB439427";
const char *wifiPassword = "52FBD5CJ-8C9D-4C84-B3F7-E674BB439428";
const char *meshOn = "52FBD5CK-8C9D-4C84-B3F7-E674BB439429";
const char *caneName = "52FBD5CL-8C9D-4C84-B3F7-E674BB439430";

bool IS_LEADER = false;
int MODE = 2; // rainbow
bool AUTO_MODE = true;
int DELAY_MS = 20;
unsigned int MODE_INDEX = 0;
int RANK = 0;
int BRIGHTNESS = 10;
int LED_ON = 1;
int MESH_ON = 0;
int WIFI_ON = 0;
char WIFI_SSID[50];
char WIFI_PASS[50];

system_tick_t startTime = 0;
system_tick_t lastPublishedCommands = 0;
system_tick_t lastPublishedRank = 0;
system_tick_t lastDecidedOnLeader = 0;
bool leaderIsAlive = false;

// Set the mesh BLE service
BleUuid bleMeshService(serviceUuid);

// Set up characteristics
BleCharacteristic modeCharacteristic("mode", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, mode, serviceUuid, onDataReceived, (void *)mode);
BleCharacteristic delayMsCharacteristic("delayMs", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, delayMs, serviceUuid, onDataReceived, (void *)delayMs);
BleCharacteristic brightnessCharacteristic("brightness", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, brightness, serviceUuid, onDataReceived, (void *)brightness);
BleCharacteristic meshOnCharacteristic("meshOn", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, meshOn, serviceUuid, onDataReceived, (void *)meshOn);
BleCharacteristic ledOnCharacteristic("ledOn", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, ledOn, serviceUuid, onDataReceived, (void *)ledOn);
BleCharacteristic caneNameCharacteristic("caneName", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, caneName, serviceUuid, onDataReceived, (void *)caneName);
BleCharacteristic wifiOnCharacteristic("wifiOn", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, wifiOn, serviceUuid, onDataReceived, (void *)wifiOn);
BleCharacteristic wifiSSIDCharacteristic("wifiSSID", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, wifiSSID, serviceUuid, onDataReceived, (void *)wifiSSID);
BleCharacteristic wifiPasswordCharacteristic("wifiPassword", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, wifiPassword, serviceUuid, onDataReceived, (void *)wifiPassword);
BleCharacteristic leaderCharacteristic("isLeader", BleCharacteristicProperty::READ, NULL);
BleCharacteristic rankCharacteristic("rank", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, rank, serviceUuid, onDataReceived, (void *)rank);

// Advertising data
BleAdvertisingData advData;

// publish rank every second
void publishRank()
{
  // Reset if overflow
  if (millis() < lastPublishedRank)
  {
    lastPublishedRank = millis();
  }

  if (millis() > (lastPublishedRank + 1000))
  {
    Mesh.publish("rank", String::format("%d", RANK));
    lastPublishedRank = millis();
  }
}

// decide on leader every 2 seconds
void checkForLeader()
{
  // Reset if overflow
  if (millis() < lastDecidedOnLeader)
  {
    lastDecidedOnLeader = millis();
  }

  if (millis() > (lastDecidedOnLeader + 2000))
  {
    if (!leaderIsAlive)
    {
      IS_LEADER = true;
      // Start advertising!
      BLE.advertise(&advData);
    }
    else
    {
      leaderIsAlive = false;
    }
    leaderCharacteristic.setValue(IS_LEADER);
    lastDecidedOnLeader = millis();
  }
}

int prevPublishedBrightness = BRIGHTNESS + 1; // so we don't init to the same value
int prevMode = MODE + 1;                      // so we don't init to the same value

// publish every 5 seconds if you're the leader
void publishCommands(bool force = false)
{
  if (!IS_LEADER)
  {
    return;
  }
  // Reset if overflow
  if (millis() < lastPublishedCommands)
  {
    lastPublishedCommands = millis();
  }

  if (force || (millis() > (lastPublishedCommands + 5000)))
  {
    Mesh.publish("state", String::format(",%d,%d,%d,%d,%d,", LED_ON, BRIGHTNESS, DELAY_MS, MODE, MODE_INDEX));
    lastPublishedCommands = millis();
  }
}

void handleMeshState(const int index, const int value)
{
  switch (index)
  {
  case 0: // LED on/off
    LED_ON = value;
    break;
  case 1: // Brightness
    setBrightness(value);
    break;
  case 2: // DELAY_MS
    DELAY_MS = value;
    delayMsCharacteristic.setValue(DELAY_MS);
    break;
  case 3: // MODE
    MODE = value;
    modeCharacteristic.setValue(MODE);
    break;
  case 4: // MODE_INDEX
    MODE_INDEX = value;
    break;

  default:
    break;
  }
}

// Mesh event handler
void meshHandler(const char *event, const char *data)
{
  // Convert to String for useful conversion and comparison functions
  String eventString = String(event);
  String dataString = String(data);

  if (eventString.equals("state"))
  {
    unsigned int index = 0;
    int count = 0;
    while (count < 5)
    {
      int endIndex = dataString.indexOf(",", index + 1);
      String word = dataString.substring(index + 1, endIndex);
      handleMeshState(count, word.toInt());
      index = endIndex;
      count++;
    }
  }
  else if (eventString.equals("rank"))
  {
    const int incomingRank = dataString.toInt();
    if (incomingRank > RANK)
    {
      leaderIsAlive = true;
      IS_LEADER = false;
      // Stop advertising!
      BLE.stopAdvertising();
    }
  }
}

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
  else if (context == rank)
  {
    RANK = data[0];
  }
  else if (context == brightness)
  {
    BRIGHTNESS = data[0];
    setBrightness(BRIGHTNESS);
  }
  else if (context == meshOn)
  {
    MESH_ON = data[0];
  }
  else if (context == ledOn)
  {
    LED_ON = data[0];
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

  if (Mesh.ready())
  {
    publishCommands(true);
  }
}

void setMesh(int on)
{
  if (on == 1 && !Mesh.ready() && !Mesh.connecting())
  {
    Mesh.on();
    Mesh.connect();
    waitUntil(Mesh.ready);
    const char *localIP = Mesh.localIP().toString().c_str();
    char subArr[4];
    const int length = strlen(localIP);
    const int startIndex = length - 4;
    for (int i = startIndex; i < length; i++)
    {
      subArr[i - startIndex] = localIP[i];
    }
    RANK = strtoul(subArr, 0, 16);

    // Set the subscription for Mesh updates
    int error = Mesh.subscribe("rank", meshHandler);
    if (error != SYSTEM_ERROR_NONE)
    {
      // TODO: have a function for subscription with error checking
      Log.error("Failed to subscribe with error code: %d", error);
    }
    Mesh.subscribe("state", meshHandler);
  }
  else if (on == 0 && Mesh.ready())
  {
    Mesh.disconnect();
    Mesh.off();
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

  // Add the characteristics
  BLE.addCharacteristic(modeCharacteristic);
  modeCharacteristic.setValue(MODE);
  BLE.addCharacteristic(delayMsCharacteristic);
  delayMsCharacteristic.setValue(DELAY_MS);
  BLE.addCharacteristic(rankCharacteristic);
  rankCharacteristic.setValue(RANK);
  BLE.addCharacteristic(brightnessCharacteristic);
  brightnessCharacteristic.setValue(BRIGHTNESS);
  BLE.addCharacteristic(meshOnCharacteristic);
  meshOnCharacteristic.setValue(MESH_ON);
  BLE.addCharacteristic(ledOnCharacteristic);
  ledOnCharacteristic.setValue(LED_ON);
  BLE.addCharacteristic(wifiOnCharacteristic);
  wifiOnCharacteristic.setValue(WIFI_ON);
  BLE.addCharacteristic(wifiSSIDCharacteristic);
  wifiSSIDCharacteristic.setValue(WIFI_SSID);
  BLE.addCharacteristic(wifiPasswordCharacteristic);
  wifiPasswordCharacteristic.setValue(WIFI_PASS);
  BLE.addCharacteristic(caneNameCharacteristic);
  BLE.addCharacteristic(leaderCharacteristic);
  leaderCharacteristic.setValue(IS_LEADER);

  batteryLevelCharacteristic = BleCharacteristic("bat", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, batteryCharUUID, batteryServiceUUID);
  BLE.addCharacteristic(batteryLevelCharacteristic);

  advData.appendLocalName(name);
  // Add the RGB LED service
  advData.appendServiceUUID(bleMeshService);
  // Add the battery service
  advData.appendServiceUUID(batteryServiceUUID);

  // Start advertising!
  BLE.advertise(&advData);
}

void setRGBColor(int b)
{
  if (IS_LEADER)
  {
    RGB.color(b & 255, 0, 0);
  }
  else
  {
    RGB.color(0, 0, b & 255);
  }
}

void debugLogs()
{
  Log.info("MODE: %d, DELAY_MS: %d, RANK: %d, BRIGHTNESS: %d, LED_ON: %d, IS_LEADER: %d", MODE, DELAY_MS, RANK, BRIGHTNESS, LED_ON, IS_LEADER);
}

// setup() runs once, when the device is first turned on.
void setup()
{
  startTime = millis();
  lastDecidedOnLeader = millis();

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

bool previousLedOn = true;

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  delay(DELAY_MS);
  if (millis() < (startTime + 1000))
  {
    return; // not ready yet
  }
  MODE_INDEX++;
  if (MESH_ON && Mesh.ready())
  {
    publishRank();
    checkForLeader();
    publishCommands();
  }
  setRGBColor(MODE_INDEX);
  setWiFi(WIFI_ON);
  setMesh(MESH_ON);
  unsigned int mode = MODE;
  if (AUTO_MODE)
  {
    static unsigned int offset = 2;
    static unsigned int modes = 6;
    static unsigned int delay = 30000; // 30 seconds
    mode = (((millis() - startTime) / delay) % modes) + offset;
  }
  if (LED_ON == 1)
  {
    if (isCharging())
    {
      dispayModeAtIndex(chargingMode(), MODE_INDEX);
    }
    else
    {
      dispayModeAtIndex(mode, MODE_INDEX);
    }
    previousLedOn = true;
  }
  else
  {
    if (previousLedOn)
    {
      dispayModeAtIndex(0, MODE_INDEX);
      previousLedOn = false;
    }
  }
  checkBattery(batteryLevelCharacteristic);
  // debugLogs();
}
