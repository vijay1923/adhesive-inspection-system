#ifndef PCF_IO_H
#define PCF_IO_H


#include <Wire.h>
#include "config.h"
#include "globals.h"

// ── PCF Helpers ───────────────────────────────────────────────────────────────
uint8_t pcf_read()
{
    Wire.requestFrom(IN_PCF, 1);
    if (Wire.available())
    return Wire.read();
    Serial.println("[WARN] I2C read failed — using cache");
    return pcfInputCache;
}

void pcf_write(uint8_t val)
{
    outputByte = val;
    Wire.beginTransmission(OUT_PCF);
    Wire.write(outputByte);
    uint8_t err = Wire.endTransmission();
    if (err != 0)
    {
        Serial.print("[WARN] I2C write failed — error: ");
        Serial.println(err);
    }
}

void pcf_set_bit(uint8_t bit)   { pcf_write(outputByte |  (1 << bit)); }
void pcf_clear_bit(uint8_t bit) { pcf_write(outputByte & ~(1 << bit)); }

// ── Output Helpers ────────────────────────────────────────────────────────────
void relay_on()      { pcf_set_bit(OUT_0);   Serial.println("[OUT] Relay ON");      }
void relay_off()     { pcf_clear_bit(OUT_0); Serial.println("[OUT] Relay OFF");     }
void indicator_on()  { pcf_set_bit(OUT_2);   Serial.println("[OUT] Indicator ON");  }
void indicator_off() { pcf_clear_bit(OUT_2); Serial.println("[OUT] Indicator OFF"); }
void all_off()       { pcf_write(0x00);      Serial.println("[OUT] All OFF");       }

// ── Bypass Check ──────────────────────────────────────────────────────────────
bool bypass_active(uint8_t input)
{
    return !((input >> BYPASS_MODE) & 1);
}

// ── Reject Helpers ────────────────────────────────────────────────────────────
String reject_str(uint8_t code)
{
    switch (code)
    {
        case REJECT_RING: return String(PART_RING);
        case REJECT_HUB:  return String(PART_HUB);
        case REJECT_BOTH: return String(PART_RING) + " + " + String(PART_HUB);
        default:          return "None";
    }
}

void buzzers_for_reject(uint8_t code)
{
    // clear both buzzer bits, preserve all other output bits
    uint8_t nextOutput = outputByte & ~((1 << OUT_3) | (1 << OUT_4));

    switch (code)
    {
        case REJECT_RING: nextOutput |= (1 << OUT_3);                 break;
        case REJECT_HUB:  nextOutput |= (1 << OUT_4);                 break;
        case REJECT_BOTH: nextOutput |= (1 << OUT_3) | (1 << OUT_4); break;
        default:                                                       break;
    }

    pcf_write(nextOutput);  // single I2C write — both outputs latch simultaneously

    switch (code)
    {
        case REJECT_RING: Serial.println("[OUT] Ring Buzzer ON");        break;
        case REJECT_HUB:  Serial.println("[OUT] Hub Buzzer ON");         break;
        case REJECT_BOTH: Serial.println("[OUT] Ring + Hub Buzzers ON"); break;
        default:          Serial.println("[OUT] All Buzzers OFF");       break;
    }
}


#endif