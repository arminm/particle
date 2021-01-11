/*
 * Project thermostat
 * Description:
 * Author:
 * Date:
 */
// #include "dht.h"
#include "dht_particle.h"
#include "TM1637.h"

// Pins definitions for TM1637 and can be changed to other ports
const int CLK = D4;
const int DIO = D5;
TM1637 tm1637(CLK, DIO);

// DHT
#define DHTPIN D2
     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);
SerialLogHandler logHandler;

#if defined(ARDUINO_ARCH_AVR)
    #define debug  Serial
#elif defined(ARDUINO_ARCH_SAMD) ||  defined(ARDUINO_ARCH_SAM)
    #define debug  SerialUSB
#else
    #define debug  Serial
#endif

void setup() {

    // debug.begin(115200);
    // debug.println("DHTxx test!");
    Log.info("TEST!");
    // Wire.begin();

    /*if using WIO link,must pull up the power pin.*/
    // pinMode(PIN_GROVE_POWER, OUTPUT);
    // digitalWrite(PIN_GROVE_POWER, 1);

    tm1637.init();
    tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
    tm1637.start();
    tm1637.displayNum(12);

    dht.begin();
    delay(2000);
}

void loop() {
  // Wait a few seconds between measurements.
//	delay(2000);

// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a 
// very slow sensor)
	float h = dht.getHumidity();
// Read temperature as Celsius
	float t = dht.getTempCelcius();
// Read temperature as Farenheit
	float f = dht.getTempFarenheit();
  
  Log.info("Humidity: %f %\tTemperature: %f *C %f *F", h, t, f);
// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t) || isnan(f)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

tm1637.displayNum(f);



    // float temp_hum_val[2] = {0};
    // // Reading temperature or humidity takes about 250 milliseconds!
    // // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    // float temp = dht.readTemperature();
    // float hum = dht.readHumidity();
    // tm1637.displayNum(hum);

    // if (!dht.readTempAndHumidity(temp_hum_val)) {
    //     // debug.print("Humidity: ");
    //     // debug.print(temp_hum_val[0]);
    //     // debug.print(" %\t");
    //     // debug.print("Temperature: ");
    //     // debug.print(temp_hum_val[1]);
    //     // debug.println(" *C");
    //     Log.info("Humidity: %f %\tTemperature: %f *C", temp_hum_val[0], temp_hum_val[1]);
        
    // } else {
    //     // debug.println("Failed to get temprature and humidity value.");
    //     Log.info("Failed to get temprature and humidity value.");
    // }

    delay(1000);
}