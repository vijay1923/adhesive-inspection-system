#include "config.h"
#include "types.h"
#include "globals.h"
#include "pcf_io.h"
#include "motor.h"
#include "persist_eeprom.h"
#include "cycle.h"
#include "serial_console.h"
#include "persist_eeprom.h"

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

    if (persist_begin())
    {
        Serial.println("EEPROM persistence initialized");
    }
    else
    {
        Serial.println("[WARN] EEPROM init failed — reject latch will not survive reboot");
    }

    all_off();
    pcfInputCache = pcf_read();

    lastButtonState = (pcfInputCache >> CYCLE_START) & 1;
    lastBypassState = bypass_active(pcfInputCache);

    uint8_t restoredRejectCode = REJECT_NONE;
    if (persist_is_reject_latched(restoredRejectCode))
    {
        lastRejectCode = restoredRejectCode;
        systemState    = STATE_RESULT_REJECT;
        buzzers_for_reject(lastRejectCode);

        Serial.print("[BOOT] Restored REJECT latch from EEPROM — Cause: ");
        Serial.println(reject_str(lastRejectCode));
        Serial.println("[LOCK] Reject latched - Supervisor bypass key required");
        return;
    }

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
