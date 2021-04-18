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

// declare other functions
void dispayModeAtIndex(int mode, int index);
uint32_t wheel(byte WheelPos);
unsigned int hexToUnsignedInt(String hex);
void checkBattery(BleCharacteristic batteryLevelCharacteristic);
void setBrightness(int b);
float readBattery();
int lowPowerMode();

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

bool IS_LEADER = false;
int MODE = 1;
int DELAY_MS = 100;
int MODE_INDEX = 0;
int RANK = 0;
int BRIGHTNESS = 50;
int LED_ON = 1;

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
BleCharacteristic rankCharacteristic("rank", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, rank, serviceUuid, onDataReceived, (void *)rank);
BleCharacteristic brightnessCharacteristic("brightness", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, brightness, serviceUuid, onDataReceived, (void *)brightness);
BleCharacteristic ledOnCharacteristic("ledOn", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, ledOn, serviceUuid, onDataReceived, (void *)ledOn);
BleCharacteristic leaderCharacteristic("isLeader", BleCharacteristicProperty::READ, NULL);

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

void setRGBColor(int b)
{
  // RGB.color(wheel(MODE_INDEX & 255));
  if (IS_LEADER)
  {
    RGB.color(b & 255, 0, 0);
  }
  else
  {
    RGB.color(0, 0, b & 255);
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
  else if (context == ledOn)
  {
    LED_ON = data[0];
  }
  publishCommands(true);
}

// setup() runs once, when the device is first turned on.
void setup()
{
  startTime = millis();
  lastDecidedOnLeader = millis();

  RGB.control(true);
  Mesh.on();
  // WiFi.off();

  Mesh.connect();
  waitUntil(Mesh.ready);
  Log.info("Mesh is ready!");
  const char *localIP = Mesh.localIP().toString().c_str();
  Log.info("localIP: %s", localIP);
  char subArr[4];
  const int length = strlen(localIP);
  Log.info("length: %d", length);
  const int startIndex = length - 4;
  for (int i = startIndex; i < length; i++)
  {
    Log.info("localIP[%d]: %c", i, localIP[i]);
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

  // Add the characteristics
  BLE.addCharacteristic(modeCharacteristic);
  modeCharacteristic.setValue(MODE);
  BLE.addCharacteristic(delayMsCharacteristic);
  delayMsCharacteristic.setValue(DELAY_MS);
  BLE.addCharacteristic(rankCharacteristic);
  rankCharacteristic.setValue(RANK);
  BLE.addCharacteristic(brightnessCharacteristic);
  brightnessCharacteristic.setValue(BRIGHTNESS);
  BLE.addCharacteristic(ledOnCharacteristic);
  ledOnCharacteristic.setValue(LED_ON);
  BLE.addCharacteristic(leaderCharacteristic);
  leaderCharacteristic.setValue(IS_LEADER);

  batteryLevelCharacteristic = BleCharacteristic("bat", BleCharacteristicProperty::NOTIFY, batteryCharUUID, batteryServiceUUID);
  BLE.addCharacteristic(batteryLevelCharacteristic);

  advData.appendLocalName(String::format("Light Cane - %d", RANK));
  // Add the RGB LED service
  advData.appendServiceUUID(bleMeshService);
  // Add the battery service
  advData.appendServiceUUID(batteryServiceUUID);

  // Set LED Strip brightness
  setBrightness(BRIGHTNESS);
}

void debugLogs()
{
  Log.info("MODE: %d, DELAY_MS: %d, RANK: %d, BRIGHTNESS: %d, LED_ON: %d, IS_LEADER: %d", MODE, DELAY_MS, RANK, BRIGHTNESS, LED_ON, IS_LEADER);
}

bool previousLedOn = true;

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // rainbow run
  delay(DELAY_MS);
  MODE_INDEX++;
  publishRank();
  checkForLeader();
  publishCommands();
  setRGBColor(MODE_INDEX);
  if (LED_ON == 1)
  {
    // if (readBattery() < 10)
    // {
    //   dispayModeAtIndex(lowPowerMode(), MODE_INDEX);
    // }
    // else
    // {
    dispayModeAtIndex(MODE, MODE_INDEX);
    // }
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
