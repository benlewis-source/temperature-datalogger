#include <OneWire.h>
#include <SPI.h>
#include <SD.h>

OneWire sensorBus(A0);  // on pin A0


void logMeasurements(void);

void setup(void) {
    Serial.begin(115200);
    Serial.println("Initializing SD card...");

    // see if the card is present and can be initialized:
    if (!SD.begin(10)) {
        Serial.println("Card failed, or not present");
        while (1);
    }
}

void loop(void) {
    logMeasurements();
    delay(300000);
}

void logMeasurements(void){
    Serial.println("Reading sensors");
    uint8_t data[9];
    uint8_t addr[8];
    float celsius;
    String fileLine;
    
    while (sensorBus.search(addr)) {
        fileLine = String(millis())+",";
        celsius = -100;

        for( uint8_t i=0; i<8; i++){
            fileLine += String(addr[i]);
        }
        fileLine += ",";

        sensorBus.reset();
        sensorBus.select(addr);
        // write to 0x44 to initiate temperature conversion
        sensorBus.write(0x44, 1);
        delay(1000);
        
        sensorBus.reset();
        sensorBus.select(addr); 
        // write to 0xBE to read scratchpad  
        sensorBus.write(0xBE);
        for (uint8_t i = 0; i < 9; i++) {
            data[i] = sensorBus.read();
        }

        // Convert the data to actual temperature
        int16_t raw = (data[1] << 8) | data[0];
        byte cfg = (data[4] & 0x60);

        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        // default is 12 bit resolution, 750 ms conversion time
        celsius = (float)raw / 16.0;

        fileLine += String(celsius)+",C";

        
        Serial.println(fileLine);
        File dataFile = SD.open("datalog.csv", FILE_WRITE);
        // if the file is available, write to it:
        if (dataFile) {
            dataFile.println(fileLine);
            dataFile.close();
        }
    }
    sensorBus.reset_search();
    Serial.println();
}
