#ifndef PERSIST_EEPROM_H
#define PERSIST_EEPROM_H

#include <EEPROM.h>
#include "config.h"

struct PersistState
{
    uint32_t magic;
    uint8_t  version;
    uint8_t  lastResult;   // 1 = OK/CLEARED, 2 = REJECT LATCHED
    uint8_t  rejectCode;   // REJECT_* code
    uint8_t  reserved;
};

static const uint8_t PERSIST_RESULT_OK     = 1;
static const uint8_t PERSIST_RESULT_REJECT = 2;

static bool persistReady = false;

bool persist_is_valid_reject_code(uint8_t code)
{
    return code == REJECT_NONE || code == REJECT_RING || code == REJECT_HUB || code == REJECT_BOTH;
}

PersistState persist_default_state()
{
    PersistState s;
    s.magic      = EEPROM_MAGIC;
    s.version    = EEPROM_VERSION;
    s.lastResult = PERSIST_RESULT_OK;
    s.rejectCode = REJECT_NONE;
    s.reserved   = 0;
    return s;
}

bool persist_write_state(const PersistState& s)
{
    if (!persistReady) return false;
    EEPROM.put(EEPROM_STATE_ADDR, s);
    return EEPROM.commit();
}

PersistState persist_read_state_raw()
{
    PersistState s;
    EEPROM.get(EEPROM_STATE_ADDR, s);
    return s;
}

bool persist_begin()
{
    persistReady = EEPROM.begin(EEPROM_SIZE_BYTES);
    if (!persistReady) return false;

    PersistState s = persist_read_state_raw();

    bool invalid = false;
    if (s.magic != EEPROM_MAGIC) invalid = true;
    if (s.version != EEPROM_VERSION) invalid = true;
    if (s.lastResult != PERSIST_RESULT_OK && s.lastResult != PERSIST_RESULT_REJECT) invalid = true;
    if (!persist_is_valid_reject_code(s.rejectCode)) invalid = true;

    if (invalid)
    {
        PersistState defaults = persist_default_state();
        persist_write_state(defaults);
    }

    return true;
}

void persist_save_ok()
{
    PersistState s = persist_default_state();
    persist_write_state(s);
}

void persist_save_reject(uint8_t rejectCode)
{
    PersistState s = persist_default_state();
    s.lastResult = PERSIST_RESULT_REJECT;
    s.rejectCode = persist_is_valid_reject_code(rejectCode) ? rejectCode : REJECT_NONE;
    persist_write_state(s);
}

void persist_clear_reject()
{
    persist_save_ok();
}

bool persist_is_reject_latched(uint8_t& rejectCode)
{
    rejectCode = REJECT_NONE;
    if (!persistReady) return false;

    PersistState s = persist_read_state_raw();
    if (s.magic != EEPROM_MAGIC || s.version != EEPROM_VERSION) return false;
    if (!persist_is_valid_reject_code(s.rejectCode)) return false;
    if (s.lastResult != PERSIST_RESULT_REJECT) return false;

    rejectCode = s.rejectCode;
    return true;
}

/// EEPOM HELTH MONITERING FUNCTION ////

void persist_print_state()
{
    if (!persistReady)
    {
        Serial.println("[EEPROM] Not initialized");
        return;
    }

    PersistState s = persist_read_state_raw();
    Serial.println("[EEPROM] State:");
    Serial.print("  Magic: 0x"); Serial.println(s.magic, HEX);
    Serial.print("  Version: "); Serial.println(s.version);
    Serial.print("  Last Result: "); Serial.println(s.lastResult == PERSIST_RESULT_OK ? "OK" : "REJECT");
    Serial.print("  Reject Code: "); Serial.println(reject_str(s.rejectCode));
}

#endif