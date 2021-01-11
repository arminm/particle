/*
 * Project ble_mesh
 * Description: BLE Mesh network
 * Author: Armin Mahmoudi
 * Date: 9/26/2020
 */

// SYSTEM_MODE(AUTOMATIC);
SYSTEM_MODE(MANUAL);
SerialLogHandler logHandler;

// declare other functions
void dispayModeAtIndex(int mode, int index);
uint32_t wheel(byte WheelPos);
unsigned int hexToUnsignedInt(String hex);

const char *serviceUuid = "52FBD5CA-8C9E-4C84-B3F7-E674BB439420";
const char *mode = "52FBD5CE-8C9B-4C84-B3F7-E674BB439420";
const char *delayMs = "52FBD5CE-8C9C-4C84-B3F7-E674BB439420";
const char *rank = "52FBD5CE-8C9D-4C84-B3F7-E674BB439421";

bool IS_LEADER = false;
int MODE = 1;
int DELAY_MS = 10;
int RAINBOW_INDEX = 0;
int RANK = 0;

int startTime = 0;
int lastPublishedCommands = 0;
int lastPublishedRank = 0;
int lastDecidedOnLeader = 0;
bool leaderIsAlive = false;

// Set the mesh BLE service
BleUuid bleMeshService(serviceUuid);

// Set up characteristics
BleCharacteristic modeCharacteristic("mode", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, mode, serviceUuid, onDataReceived, (void *)mode);
BleCharacteristic delayMsCharacteristic("delayMs", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, delayMs, serviceUuid, onDataReceived, (void *)delayMs);
BleCharacteristic rankCharacteristic("rank", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, rank, serviceUuid, onDataReceived, (void *)rank);
BleCharacteristic leaderCharacteristic("isLeader", BleCharacteristicProperty::READ, NULL);

// Advertising data
BleAdvertisingData advData;

// publish rank every second
void publishRank()
{
  if (millis() - lastPublishedRank > 1000)
  {
    Mesh.publish("rank", String::format("%d", RANK));
    lastPublishedRank = millis();
  }
}

// decide on leader every 2 seconds
void checkForLeader()
{
  if (millis() - lastDecidedOnLeader > 2000)
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

// publish every 5 seconds if you're the leader
void publishCommands(bool force = false)
{
  if (!IS_LEADER)
  {
    return;
  }

  if (force || millis() - lastPublishedCommands > 5000)
  {
    Mesh.publish("delayMs", String::format("%d", DELAY_MS));
    Mesh.publish("mode", String::format("%d", MODE));
    Mesh.publish("modeIndex", String::format("%d", RAINBOW_INDEX));
    lastPublishedCommands = millis();
  }
}

void setRGBColor()
{
  // RGB.color(wheel(RAINBOW_INDEX & 255));
  if (IS_LEADER)
  {
    RGB.color(255, 0, 0);
  }
  else
  {
    RGB.color(0, 0, 255);
  }
}

// Mesh event handler
static void meshHandler(const char *event, const char *data)
{
  // Convert to String for useful conversion and comparison functions
  String eventString = String(event);
  String dataString = String(data);

  if (eventString.equals("mode"))
  {
    MODE = dataString.toInt();
    modeCharacteristic.setValue(MODE);
  }
  else if (eventString.equals("modeIndex"))
  {
    RAINBOW_INDEX = dataString.toInt();
  }
  else if (eventString.equals("delayMs"))
  {
    DELAY_MS = dataString.toInt();
    delayMsCharacteristic.setValue(DELAY_MS);
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
  int error = Mesh.subscribe("mode", meshHandler);
  if (error != SYSTEM_ERROR_NONE)
  {
    // TODO: have a function for subscription with error checking
    Log.error("Failed to subscribe with error code: %d", error);
  }
  Mesh.subscribe("modeIndex", meshHandler);
  Mesh.subscribe("delayMs", meshHandler);
  Mesh.subscribe("rank", meshHandler);

  // Add the characteristics
  BLE.addCharacteristic(modeCharacteristic);
  modeCharacteristic.setValue(MODE);
  BLE.addCharacteristic(delayMsCharacteristic);
  delayMsCharacteristic.setValue(DELAY_MS);
  BLE.addCharacteristic(rankCharacteristic);
  rankCharacteristic.setValue(RANK);
  BLE.addCharacteristic(leaderCharacteristic);
  leaderCharacteristic.setValue(IS_LEADER);

  // Add the RGB LED service
  advData.appendServiceUUID(bleMeshService);
  advData.appendLocalName("BLE Mesh Node");
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // rainbow run
  delay(DELAY_MS);
  RAINBOW_INDEX++;
  if (RAINBOW_INDEX >= 255)
  {
    RAINBOW_INDEX = 0;
  }
  publishRank();
  checkForLeader();
  publishCommands();
  setRGBColor();
  dispayModeAtIndex(MODE, RAINBOW_INDEX);
}
