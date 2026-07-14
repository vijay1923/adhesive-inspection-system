#ifndef GLOBALS_H
#define GLOBALS_H


#include "config.h"
#include "types.h"

// ── Runtime State ──────────────────────────────────────────────────────────────
SystemState systemState = STATE_IDLE;
RejectStats rejectStats = {0, 0, 0, 0};

// ── PCF / I2C Cache ────────────────────────────────────────────────────────────
uint8_t  pcfInputCache = 0xFF;
uint32_t lastPollTime  = 0;
uint8_t  outputByte    = 0x00;

// ── Motor / Pulse State ────────────────────────────────────────────────────────
bool     motorRunning  = false;
bool     pulseState    = LOW;
uint32_t lastPulseTime = 0;
int      pulseCount    = 0;

// ── Timers ──────────────────────────────────────────────────────────────────────
uint32_t okTimer         = 0;
uint32_t okShortTimer    = 0;
uint32_t partRemoveTimer = 0;

// ── Input Edge Tracking ────────────────────────────────────────────────────────
bool lastButtonState = true;
bool lastBypassState = true;

// ── Reject Tracking ────────────────────────────────────────────────────────────
uint8_t lastRejectCode   = REJECT_NONE;
uint8_t cycleRejectFlags = REJECT_NONE;

// ── Serial Confirmation State ──────────────────────────────────────────────────
bool     awaitingConfirm  = false;
String   pendingCommand   = "";
uint32_t confirmStartTime = 0;



#endif