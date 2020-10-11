/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/arminmahmoudi/dev/lights/particle/ble_mesh/src/ble_mesh.ino"
/*
 * Project ble_mesh
 * Description: BLE Mesh network
 * Author: Armin Mahmoudi
 * Date: 9/26/2020
 */

static void meshHandler(const char *event, const char *data);
static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);
void setup();
void loop();
#line 8 "/Users/arminmahmoudi/dev/lights/particle/ble_mesh/src/ble_mesh.ino"
SYSTEM_MODE(MANUAL);
SerialLogHandler logHandler;

// Variables for keeping state
typedef struct
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} led_level_t;

led_level_t rainbow[7] = {
  {148, 0, 211},
  {75, 0, 130},
  {0, 0, 255},
  {0, 255, 0},
  {255, 255, 0},
  {255, 127, 0},
  {255, 0 , 0},
};

// Static level tracking
static led_level_t m_led_level;

// Tracks when to publish to Mesh
static bool m_publish;

const char *serviceUuid = "52FBD5CA-8C9E-4C84-B3F7-E674BB439420";
const char *red = "52FBD5CB-8C9E-4C84-B3F7-E674BB439420";
const char *green = "52FBD5CC-8C9E-4C84-B3F7-E674BB439420";
const char *blue = "52FBD5CD-8C9E-4C84-B3F7-E674BB439420";

// Set the RGB BLE service
BleUuid rgbService(serviceUuid);

// Mesh event handler
static void meshHandler(const char *event, const char *data)
{
  // Convert to String for useful conversion and comparison functions
  String eventString = String(event);
  String dataString = String(data);

  // Determine which event we recieved
  if (eventString.equals("red"))
  {
    m_led_level.red = dataString.toInt();
  }
  else if (eventString.equals("green"))
  {
    m_led_level.green = dataString.toInt();
  }
  else if (eventString.equals("blue"))
  {
    m_led_level.blue = dataString.toInt();
  }
  else
  {
    return;
  }

  // Set RGB color
  RGB.color(m_led_level.red, m_led_level.green, m_led_level.blue);
}

// Static function for handling Bluetooth Low Energy callbacks
static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{
  // We're only looking for one byte
  if (len != 1)
  {
    return;
  }
  // Sets the global level
  if (context == red)
  {
    m_led_level.red = data[0];
  }
  else if (context == green)
  {
    m_led_level.green = data[0];
  }
  else if (context == blue)
  {
    m_led_level.blue = data[0];
  }

  // Set RGB color
  RGB.color(m_led_level.red, m_led_level.green, m_led_level.blue);

  // Set to publish
  m_publish = true;
}

// setup() runs once, when the device is first turned on.
void setup()
{
  RGB.control(true);
  Mesh.on();
  // WiFi.off();

  Mesh.connect();
  Log.info("Mesh connect called");
  Log.info("localIP: %s", Mesh.localIP().toString().c_str());
  waitUntil(Mesh.ready);
  Log.info("Mesh is ready!");

  // Set to false at first
  m_publish = false;

  // Set the subscription for Mesh updates
  int error = Mesh.subscribe("red", meshHandler);
  if (error != SYSTEM_ERROR_NONE) {
    // TODO: have a function for subscription with error checking
    Log.error("Failed to subscribe with error code: %d", error);
  }
  Mesh.subscribe("green", meshHandler);
  Mesh.subscribe("blue", meshHandler);

  // Set up characteristics
  BleCharacteristic redCharacteristic("red", BleCharacteristicProperty::WRITE_WO_RSP, red, serviceUuid, onDataReceived, (void *)red);
  BleCharacteristic greenCharacteristic("green", BleCharacteristicProperty::WRITE_WO_RSP, green, serviceUuid, onDataReceived, (void *)green);
  BleCharacteristic blueCharacteristic("blue", BleCharacteristicProperty::WRITE_WO_RSP, blue, serviceUuid, onDataReceived, (void *)blue);

  // Add the characteristics
  BLE.addCharacteristic(redCharacteristic);
  BLE.addCharacteristic(greenCharacteristic);
  BLE.addCharacteristic(blueCharacteristic);

  // Advertising data
  BleAdvertisingData advData;

  // Add the RGB LED service
  advData.appendServiceUUID(rgbService);

  // Start advertising!
  BLE.advertise(&advData);
}



int rainbowIndex = 0;

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // rainbow run
  delay(500);
  rainbowIndex++;
  if (rainbowIndex >= 7) {
    rainbowIndex = 0;
  }

  // Set RGB color
  m_led_level = rainbow[rainbowIndex];
  RGB.color(m_led_level.red, m_led_level.green, m_led_level.blue);
  m_publish = true;

  if (m_publish)
  {
    // Reset flag
    m_publish = false;

    // Publish to Mesh
    Mesh.publish("red", String::format("%d", m_led_level.red));
    Mesh.publish("green", String::format("%d", m_led_level.green));
    Mesh.publish("blue", String::format("%d", m_led_level.blue));
  }
}
