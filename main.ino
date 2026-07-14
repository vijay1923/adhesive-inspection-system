#include "config.h"
#include "types.h"
#include "globals.h"
#include "pcf_io.h"
#include "motor.h"
#include "cycle.h"
#include "serial_console.h"

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("WELCOME ESP32 : Adhesive Inspection System"); 
    Serial.println("Initializing...");

    pinMode(PUL_PIN, OUTPUT);
    pinMode(ENA_PIN, OUTPUT);
    digitalWrite(PUL_PIN, LOW);
    motor_disable();
    Serial.println("Motor Initialized");

    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(400000);
    Wire.setTimeOut(10);
    Serial.println("I2C Initialized");

    Wire.beginTransmission(IN_PCF);
    Wire.write(0xFF);
    Wire.endTransmission();
    Serial.println("PCF Initialized");

    all_off();
    pcfInputCache = pcf_read();

    lastButtonState = (pcfInputCache >> CYCLE_START) & 1;
    lastBypassState = bypass_active(pcfInputCache);

    if (bypass_active(pcfInputCache))
    {
        Serial.println("[BOOT] Bypass ON at startup");
        enter_bypass();
    }
    else
    {
        systemState = STATE_IDLE;
        Serial.println("System Ready — Press cycle start or type 'help'");
    }
}

void loop()
{
    handleSerialCommands();
    pcf_poll_handler();
    motor_handler();
    ok_enable_handler();
    part_remove_handler();
    ok_timer_handler();
}
