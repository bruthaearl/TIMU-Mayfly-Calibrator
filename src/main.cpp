// TIMU-Mayfly_Calibrator
// @bruthaearl (jake jones)

#include <Arduino.h>
#include <Wire.h>
#define PIN_REG_3V3 22
#define BUFFER_SIZE 32

// make sure serial monitor baud rate is set the same!
const int32_t serialBaud = 57600;

char rx_buffer[BUFFER_SIZE];
uint8_t bytes_read = 0;
bool serial_flag = false;  // true when data in serial buffer

char *sensor;   // buffer for sensor string
char *command;  // buffer for command string

uint8_t address = 0;  // sensor address number

uint8_t response_code = 0;  // first byte sent from ezo
char tx_buffer[BUFFER_SIZE];
char tx_byte;
uint8_t tx_buffer_index = 0;

// interrupt called whenever there's data on the rx buffer
void serialEvent() {
    // read bytes from the buffer
    bytes_read = Serial.readBytesUntil('\r', rx_buffer, BUFFER_SIZE);
    // the last byte read is the CR, remove it
    rx_buffer[bytes_read] = 0;
    serial_flag = true;
}

void print_help(void) {
    Serial.println("======================");
    Serial.println("TIMU Mayfly Calibrator");
    Serial.println("Syntax is [SENSOR] [COMMAND]");
    Serial.println("Available sensors are DO, ORP, PH, COND, and RTD");
    Serial.println("Refer to the EZO datasheets for commands");
    Serial.println("Example: 'COND R' takes a reading from the electrical conductivity sensor");
    Serial.println("======================");
    Serial.println();
}

// convert sensor string into address number
uint8_t parse_sensor(char *sensor) {
    if (!strcmp(sensor, "DO")) {
        return 97;
    }
    if (!strcmp(sensor, "ORP")) {
        return 98;
    }
    if (!strcmp(sensor, "PH")) {
        return 99;
    }
    if (!strcmp(sensor, "COND")) {
        return 100;
    }
    if (!strcmp(sensor, "RTD")) {
        return 102;
    }
    if (!strcmp(sensor, "HELP")) {
        return 1;
    }
    // if we made it here, sensor not recognized
    return 0;
}

// send command to I2C bus
uint8_t send_command(uint8_t address, char *command) {
    Wire.beginTransmission(address);
    Wire.write(command);
    Wire.endTransmission();

    // give the ezo time to process
    delay(900);

    Wire.requestFrom(address, BUFFER_SIZE, 1);

    // first byte read is ezo response code
    response_code = Wire.read();
    switch (response_code) {
        case 1:
            Serial.println("Request successful.");
            break;
        case 2:
            Serial.println("Syntax error.");
            Serial.println("Type HELP for syntax help");
            break;
        case 3:
            Serial.println("Still processing, not ready.");
            break;
        case 4:
            Serial.println("No data to send.");
            break;
        default:
            Serial.print("Sensor not available.");
            break;
    }

    // read bytes one at a time, placing them into the tx buffer
    tx_buffer_index = 0;
    while (Wire.available()) {
        tx_byte = Wire.read();
        tx_buffer[tx_buffer_index] = tx_byte;
        tx_buffer_index++;
        if (tx_byte == 0) {
            break;
        }
    }

    // print data from the tx buffer
    Serial.println(tx_buffer);
    Serial.println();
    return 1;
}

void setup() {
    // turn on regulator to power EZO
    digitalWrite(PIN_REG_3V3, HIGH);
    Serial.begin(serialBaud);
    Wire.begin();
    print_help();
}
void loop() {
    // if serial_flag is set then data has been pulled from serial line and placed into rx_buffer
    if (serial_flag == true) {
        // extract sensor and command string
        sensor = strtok(rx_buffer, " ");
        command = strtok(NULL, " ");
        address = parse_sensor(sensor);

        // if sensor is recognized, send the command
        if (address == 1) {
            print_help();
        } else if (address) {
            tx_buffer[0] = 0;
            Serial.print("Sending command \"");
            Serial.print(command);
            Serial.print("\" to the ");
            Serial.print(sensor);
            Serial.print(" sensor... \n");
            send_command(address, command);

        } else {
            Serial.print("Sensor ");
            Serial.print(sensor);
            Serial.print(" not recognized \n");
            Serial.print("Type HELP for syntax help \n\n");
        }

        serial_flag = false;
    }
}