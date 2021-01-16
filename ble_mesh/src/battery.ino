
#define MAX_BATT_V 4.1
#define MIN_BATT_V 3.1
#define MEASUREMENT_INTERVAL_MS 10000

// Timer for batt measurement
system_tick_t lastMeasurementMs = 0;

void checkBattery(BleCharacteristic batteryLevelCharacteristic)
{
    // Reset if overflow
    if (millis() < lastMeasurementMs)
    {
        lastMeasurementMs = millis();
    }

    // Check if it's time to make a measurement
    if (millis() > (lastMeasurementMs + MEASUREMENT_INTERVAL_MS))
    {
        lastMeasurementMs = millis();

        float voltage = analogRead(BATT) * 0.0011224;
        float normalized = (voltage - MIN_BATT_V) / (MAX_BATT_V - MIN_BATT_V) * 100;

        // If normalized goes above or below the min/max, set to the min/max
        if (normalized > 100)
        {
            normalized = 100;
        }
        else if (normalized < 0)
        {
            normalized = 0;
        }

        // Set the battery value
        batteryLevelCharacteristic.setValue((uint8_t)normalized);

        // Print the results
        Log.info("Batt level: %d", (uint8_t)normalized);
    }
}