#include <Arduino.h>
#include <Wire.h>

#define PIN_REG_3V3 22

const int32_t serialBaud = 115200;

void i2c_cal(void) {
    String address;
    String command;
    String first;

    // reserve a bunch of bytes for the strings
    first.reserve(40);
    command.reserve(40);
    address.reserve(40);

    // check for "I2C" from serial buffer
    first = Serial.readStringUntil('\ ');
    first.trim();
    if (first == "I2C") {
        // next string is address
        address = Serial.readStringUntil('\ ');
        address.trim();
        // next string is command
        command = Serial.readStringUntil('\n');
        command.trim();
        Serial.println("i2c string recognized");
    } else {
        // if I2C not found, flush the buffer
        (Serial.flush());
        Serial.println(first);
        Serial.println("i2c string not recognized");
        return;
    }

    int i2c_address;
    if (address == "DO") {
        i2c_address = 0x61;
    } else if (address == "ORP") {
        i2c_address = 0x62;
    } else if (address == "PH") {
        i2c_address = 0x63;
    } else if (address == "COND") {
        i2c_address = 0x64;
    } else if (address == "RTD") {
        i2c_address = 0x66;
    } else {
        Serial.println("i2c address not recognized");
        return;
    }

    Serial.print("i2c address is: ");
    Serial.print(i2c_address, HEX);
    Serial.print("\n");

    Serial.print("i2c command is: ");
    Serial.print(command);
    Serial.print("\n");

    // send I2C command
    Wire.beginTransmission(i2c_address);
    Wire.write(command.c_str());
    Wire.endTransmission();
    // requesting 40 bytes, longer than needed but ok
    Wire.requestFrom(i2c_address, 40, true);
    // wait 900ms for EZO to do its thing
    delay(900);
    // read status from EZO
    while (Wire.available()) {
        char c = Wire.read();
        Serial.print(c);
    }
}

void setup() {
    // turn on regulator to power EZO
   digitalWrite(PIN_REG_3V3, HIGH);
   Serial.begin(serialBaud);
   Serial.setTimeout(2000);
}
void loop() {

    while(Serial.available() >0){
      i2c_cal();
    }
}