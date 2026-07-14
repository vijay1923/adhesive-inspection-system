#ifndef MOTOR_H
#define MOTOR_H

#include "config.h"

// ── Motor Helpers ─────────────────────────────────────────────────────────────
void motor_enable()
{
    digitalWrite(ENA_PIN, LOW);
    Serial.println("[MOTOR] Enabled");
}

void motor_disable()
{
    digitalWrite(ENA_PIN, HIGH);
    digitalWrite(PUL_PIN, LOW);
    Serial.println("[MOTOR] Disabled");
}


#endif