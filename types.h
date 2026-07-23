#ifndef TYPES_H 
#define TYPES_H

#include <Arduino.h>

// ── System States 
enum SystemState
{
    STATE_IDLE,              // system waiting for cycle start
    STATE_RUNNING,           // motor running, sensors being monitored
    STATE_OK_ENABLE,         // motor stopped, indicator ON, waiting for part removal
    STATE_WAIT_PART_REMOVE,  // motor stopped, indicator OFF, waiting for part removal
    STATE_MACHINE_ENABLE,    // motor stopped, relay ON, waiting for machine enable timer to expire
    STATE_RESULT_REJECT,     // buzzers ON — waiting for bypass reset
    STATE_BYPASS
};
// ── Reject Statistics 
struct RejectStats
{
    uint32_t total;
    uint32_t ring;
    uint32_t hub;
    uint32_t both;
};

    
#endif