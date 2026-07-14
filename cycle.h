#ifndef CYCLE_H
#define CYCLE_H

#include "config.h"
#include "globals.h"
#include "pcf_io.h"
#include "motor.h"

// ── Cycle Start ───────────────────────────────────────────────────────────────
void cycle_start()
{
    pulseCount       = 0;
    pulseState       = LOW;
    lastPulseTime    = micros();
    motorRunning     = true;
    lastRejectCode   = REJECT_NONE;
    cycleRejectFlags = REJECT_NONE;

    all_off();  // clears buzzers from previous reject + any other outputs
    motor_enable();
    systemState = STATE_RUNNING;
    Serial.println("--- Cycle Started ---");
}

// ── Cycle Result ──────────────────────────────────────────────────────────────
void cycle_result(bool ok)
{
    motor_disable();
    motorRunning = false;

    if (ok)
    {
        Serial.println("--- Result: OK ---");
        all_off();
        indicator_on();
        partRemoveTimer = 0;
        okShortTimer    = millis();
        systemState     = STATE_OK_ENABLE;
        Serial.println("[INFO] Part OK — indicator ON for "
            + String(SMALL_TIME_ON / 1000UL) + " sec");
    }
    else
    {
        rejectStats.total++;
        if      (lastRejectCode == REJECT_RING) rejectStats.ring++;
        else if (lastRejectCode == REJECT_HUB)  rejectStats.hub++;
        else if (lastRejectCode == REJECT_BOTH) rejectStats.both++;

        Serial.print("--- Result: REJECT — Cause: ");
        Serial.println(reject_str(lastRejectCode));
        Serial.println("[INFO] REJECT latched - Supervisor bypass key required to reset");

        buzzers_for_reject(lastRejectCode);  // buzzers ON until bypass reset
        systemState = STATE_RESULT_REJECT;
    }
}

// ── Bypass ────────────────────────────────────────────────────────────────────
void enter_bypass()
{
    motor_disable();
    motorRunning = false;
    all_off();
    relay_on();
    indicator_on();
    systemState = STATE_BYPASS;
    Serial.println("[BYPASS] Mode ON — Relay ON, Indicator solid green");
}

void exit_bypass()
{
    all_off();
    systemState = STATE_IDLE;
    Serial.println("[BYPASS] Mode OFF — Returning to IDLE");
}

// ── PCF Poll Handler ──────────────────────────────────────────────────────────
void pcf_poll_handler()
{
    if (millis() - lastPollTime < PCF_POLL_INTERVAL) return;
    lastPollTime = millis();

    pcfInputCache = pcf_read();

    bool btnNow     = (pcfInputCache >> CYCLE_START) & 1;
    bool btnPressed = !btnNow && lastButtonState;     // NPN falling edge
    bool bypassNow  = bypass_active(pcfInputCache);

    switch (systemState)
    {
        case STATE_IDLE:
            if (bypassNow) { enter_bypass(); break; }
            if (btnPressed)
            {
                Serial.println("[BTN] Cycle Start pressed");
                cycle_start();
            }
            break;

        case STATE_RESULT_REJECT:
            // only bypass key can reset a reject
            if (bypassNow)
            {
                Serial.println("[RESET] Bypass key reset accepted - returning to IDLE");
                all_off();
                systemState = STATE_IDLE;
            }
            else if (btnPressed)
            {
                Serial.println("[LOCK] Reject latched - Supervisor bypass key required");
            }
            break;

        case STATE_OK_ENABLE:
        case STATE_WAIT_PART_REMOVE:
        case STATE_MACHINE_ENABLE:
            break;  // ignore all inputs during OK flow

        case STATE_BYPASS:
            if (!bypassNow) exit_bypass();
            break;

        case STATE_RUNNING:
            break;
    }

    lastButtonState = btnNow;
    lastBypassState = bypassNow;
}

// ── Motor Handler ─────────────────────────────────────────────────────────────
void motor_handler()
{
    if (!motorRunning) return;

    if (micros() - lastPulseTime >= PULSE_HALF_PERIOD)
    {
        lastPulseTime = micros();
        pulseState    = !pulseState;
        digitalWrite(PUL_PIN, pulseState);

        if (pulseState == LOW)
        {
            pulseCount++;

            if (pulseCount % SENSOR_READ_INTERVAL == 0)
            {
                uint8_t input = pcf_read();
                bool s1 = (input >> S1_IN) & 1;
                bool s2 = (input >> S2_IN) & 1;

                uint8_t sampledReject = REJECT_NONE;
                if (s1) sampledReject |= REJECT_RING;
                if (s2) sampledReject |= REJECT_HUB;

                uint8_t newReject = sampledReject & ~cycleRejectFlags;
                if (newReject)
                {
                    cycleRejectFlags |= newReject;
                    Serial.print("[REJECT] Detected: ");
                    Serial.print(reject_str(cycleRejectFlags));
                    Serial.print(" at pulse ");
                    Serial.println(pulseCount);
                }
            }

            if (pulseCount % 100 == 0)
            {
                Serial.print("[MOTOR] Pulse ");
                Serial.print(pulseCount);
                Serial.print(" / ");
                Serial.println(TOTAL_PULSES);
            }

            if (pulseCount >= TOTAL_PULSES)
            {
                Serial.println("[MOTOR] Pulses complete");
                lastRejectCode = cycleRejectFlags;
                cycle_result(cycleRejectFlags == REJECT_NONE);
            }
        }
    }
}

// ── OK Enable Handler ─────────────────────────────────────────────────────────
void ok_enable_handler()
{
    if (systemState != STATE_OK_ENABLE) return;

    if (millis() - okShortTimer >= SMALL_TIME_ON)
    {
        indicator_off();
        partRemoveTimer = 0;
        systemState     = STATE_WAIT_PART_REMOVE;
        Serial.println("[INFO] Indicator OFF — waiting for part removal");
    }
}

// ── Part Remove Handler ───────────────────────────────────────────────────────
void part_remove_handler()
{
    if (systemState != STATE_WAIT_PART_REMOVE) return;

    uint8_t input   = pcf_read();
    bool    s1      = (input >> S1_IN) & 1;
    bool    s2      = (input >> S2_IN) & 1;
    bool    removed = s1 && s2;

    if (removed)
    {
        if (partRemoveTimer == 0)
        {
            partRemoveTimer = millis();
            Serial.println("[OK] Part removed — waiting "
                + String(PART_REMOVE_STABLE_DELAY / 1000UL)
                + " sec stable before machine enable");
        }

        if (millis() - partRemoveTimer >= PART_REMOVE_STABLE_DELAY)
        {
            relay_on();
            indicator_on();
            okTimer         = millis();
            partRemoveTimer = 0;
            systemState     = STATE_MACHINE_ENABLE;
            Serial.println("[OK] Part removal stable — machine enabled for "
                + String(MACHINE_ENABLE_TIME / 1000UL) + " sec");
        }
    }
    else
    {
        if (partRemoveTimer != 0)
        {
            Serial.println("[INFO] Part not stable — removal timer reset");
            partRemoveTimer = 0;
        }
    }
}

// ── OK Timer Handler ──────────────────────────────────────────────────────────
void ok_timer_handler()
{
    if (systemState != STATE_MACHINE_ENABLE) return;

    uint8_t input        = pcf_read();
    bool    s1           = (input >> S1_IN) & 1;
    bool    s2           = (input >> S2_IN) & 1;
    bool    stillRemoved = s1 && s2;

    if (!stillRemoved)
    {
        Serial.println("[WARN] Part put back — machine disabled immediately");
        all_off();
        systemState = STATE_IDLE;
        return;
    }

    if (millis() - okTimer >= MACHINE_ENABLE_TIME)
    {
        Serial.println("[TIMER] Machine-enable expired — Cycle Complete");
        all_off();
        systemState = STATE_IDLE;
        Serial.println("[INFO] System returned to IDLE");
    }
}


#endif