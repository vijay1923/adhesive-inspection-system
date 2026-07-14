#ifndef CONFIG_H
#define CONFIG_H

// ── Pin & Address Definitions ─────────────────────────────────────────────────
#define PIN_SDA 21
#define PIN_SCL 22

#define IN_PCF  0x25
#define OUT_PCF 0x26

#define S1_IN       7  // Ring Sensor → PCF pin 7
#define S2_IN       6  // Hub Sensor  → PCF pin 6
#define CYCLE_START 5  // Cycle Start Button → PCF pin 5 (NPN: LOW = pressed)
#define BYPASS_MODE 4  // Bypass Toggle → PCF pin 4 (LOW = bypass ON)

#define OUT_0 0  // Machine Enable Relay Output
#define OUT_1 1  // Spare
#define OUT_2 2  // Green Indicator
#define OUT_3 3  // Ring Buzzer
#define OUT_4 4  // Hub Buzzer

#define PUL_PIN 26  // Pulse Output GPIO → Motor Driver (PWM)
#define ENA_PIN 27  // Enable Output GPIO → Motor Driver (LOW = enabled, HIGH = disabled)

#define PART_RING "Ring"
#define PART_HUB  "Hub"

#define REJECT_NONE 0  // no reject
#define REJECT_RING 1  // ring reject
#define REJECT_HUB  2  // hub reject
#define REJECT_BOTH 3  // both reject

#define SMALL_TIME_ON            3000UL   // ms — indicator ON after OK
#define PART_REMOVE_STABLE_DELAY 5000UL   // ms — stable removal delay
#define MACHINE_ENABLE_TIME      30000UL  // ms — relay ON after stable removal
#define TOTAL_PULSES             1700     // total pulses for full 360° derotation
#define PULSE_HALF_PERIOD        781      // us — 640Hz → 781us half-period for 1.28ms full period
#define PCF_POLL_INTERVAL        20       // ms — poll PCF every N ms
#define SENSOR_READ_INTERVAL     10       // read sensors every N pulses
#define CONFIRM_TIMEOUT_MS       10000UL  // ms — serial confirmation timeout


#endif