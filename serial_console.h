#ifndef SERIAL_CONSOLE_H
#define SERIAL_CONSOLE_H


#include "config.h"
#include "globals.h"
#include "pcf_io.h"
#include "motor.h"
#include "cycle.h"
#include "persist_eeprom.h"

// ── Print Status 
void print_status()
{
    Serial.println("--------- STATUS ---------");
    Serial.print("State       : ");
    switch (systemState)
    {
        case STATE_IDLE:             Serial.println("IDLE");             break;
        case STATE_RUNNING:          Serial.println("RUNNING");          break;
        case STATE_OK_ENABLE:        Serial.println("OK ENABLE");        break;
        case STATE_WAIT_PART_REMOVE: Serial.println("WAIT PART REMOVE"); break; 
        case STATE_MACHINE_ENABLE:   Serial.println("MACHINE ENABLED");  break;
        case STATE_RESULT_REJECT:    Serial.println("REJECT");           break;
        case STATE_BYPASS:           Serial.println("BYPASS MODE");      break;
    }
    Serial.print("S1 Ring (p7): "); Serial.println((pcfInputCache >> S1_IN) & 1);
    Serial.print("S2 Hub  (p6): "); Serial.println((pcfInputCache >> S2_IN) & 1);
    Serial.print("BTN     (p5): "); Serial.println((pcfInputCache >> CYCLE_START) & 1);
    Serial.print("BYPASS  (p4): "); Serial.println((pcfInputCache >> BYPASS_MODE) & 1);
    Serial.print("Pulses      : "); Serial.print(pulseCount);
    Serial.print(" / ");            Serial.println(TOTAL_PULSES);
    Serial.print("Last Reject : "); Serial.println(reject_str(lastRejectCode));
    Serial.println("-------------------");
}

// ── Print Stats 
void print_stats()
{
    Serial.println(" --------- STATS ----------");
    Serial.print("Total Rejects : "); Serial.println(rejectStats.total);
    Serial.print("Ring  Rejects : "); Serial.println(rejectStats.ring);
    Serial.print("Hub   Rejects : "); Serial.println(rejectStats.hub);
    Serial.print("Both  Rejects : "); Serial.println(rejectStats.both);
    Serial.println("----------------------------");
}

// ── Serial Command Handler
void handleSerialCommands()
{
    if (awaitingConfirm && millis() - confirmStartTime > CONFIRM_TIMEOUT_MS)
    {
        awaitingConfirm = false;
        pendingCommand  = "";
        Serial.println("[TIMEOUT] Confirmation cancelled");
    }

    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) return;

    Serial.print("> "); Serial.println(cmd);

    // ── Confirmation response 
    if (awaitingConfirm)
    {
        if (cmd == "yes")
        {
            awaitingConfirm = false;
            if (pendingCommand == "reset_stats")
            {
                rejectStats      = {0, 0, 0, 0};
                lastRejectCode   = REJECT_NONE;
                cycleRejectFlags = REJECT_NONE;
                Serial.println("[OK] Stats reset");
                print_stats();
            }
            pendingCommand = "";
        }
        else
        {
            awaitingConfirm = false;
            pendingCommand  = "";
            Serial.println("[CANCELLED]");
        }
        return;
    }

    // ── Commands 
    if (cmd == "help")
    {
        Serial.println("Commands:");
        Serial.println("  help         — show this list");
        Serial.println("  status       — system state + sensor readings");
        Serial.println("  stats        — reject statistics");
        Serial.println("  reset_stats  — reset counters (requires yes)");
        Serial.println("  start        — trigger cycle start");
        Serial.println("  stop         — emergency stop → IDLE");
        Serial.println("  boot         — restart ESP32");
        Serial.println("  eeprom       — print EEPROM state");
    }
    else if (cmd == "status") { print_status(); }
    else if (cmd == "stats")  { print_stats();  }
    else if (cmd == "reset_stats")
    {
        pendingCommand   = "reset_stats";
        awaitingConfirm  = true;
        confirmStartTime = millis();
        Serial.println("[CONFIRM] Send 'yes' within 10 sec to reset stats");
    }
    else if (cmd == "start")
    {
        if (systemState == STATE_BYPASS)
        {
            Serial.println("[WARN] System in bypass mode");
        }
        else if (systemState == STATE_RUNNING)
        {
            Serial.println("[WARN] Cycle already running");
        }
        else if (systemState == STATE_RESULT_REJECT)
        {
            Serial.println("[LOCK] Reject latched - Only bypass key can reset");
        }
        else if (systemState == STATE_IDLE)
        {
            cycle_start();
        }
        else
        {
            Serial.println("[WARN] Cannot start now — send stop first");
        }
    }
    else if (cmd == "stop")
    {
        if (systemState == STATE_RESULT_REJECT)
        {
            Serial.println("[LOCK] Reject latched - Only bypass key can reset");
            return;
        }

        motor_disable();
        motorRunning    = false;
        awaitingConfirm = false;
        pendingCommand  = "";
        all_off();
        systemState = STATE_IDLE;
        Serial.println("[STOP] System reset to IDLE");
    }
    else if (cmd == "boot")
    {
        Serial.println("Restarting...");
        delay(500);
        ESP.restart();
    }
    else if(cmd== "eeprom")
    {
        persist_print_state();
    }
    else
    {
        Serial.print("[?] Unknown: "); Serial.println(cmd);
        Serial.println("    Type 'help' for command list");
    }
  
}

#endif