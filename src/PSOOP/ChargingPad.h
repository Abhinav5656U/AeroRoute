/*
 *
 * FILE        : ChargingPad.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 CO1 | Task 2 CO2
 *
 */

#ifndef CHARGING_PAD_H
#define CHARGING_PAD_H

#include "Location.h"
using namespace std;

// ════════════════════════════════════════════════════════════
// [PSOOP CO1 CO2 - ChargingPad]
// Mid-route battery charging station for drones.
// ════════════════════════════════════════════════════════════
class ChargingPad : public Location {
private:
    float  chargeRatePerMinute; // percent per minute
    float  currentPowerLevel;   // 0.0 to 100.0
    int    dronesServedTotal;
    bool   isOccupied;          // one drone at a time
    int    chargingSlots;       // how many drones at once

public:
    ChargingPad(int row, int col,
                const string& label,
                float chargeRate = 15.0f,
                int   slots      = 2);

    ~ChargingPad();

    // [PSOOP CO2 - Task 2: Mandatory overrides]
    void        display()      const override;
    string getTypeName()  const override;
    int         getTypeCode()  const override;
    bool        canDroneDock() const override;

    // ChargingPad-specific operations
    bool  startCharging();
    void  finishCharging();
    float estimateChargeTime(float currentBattery) const;

    // Getters
    float getChargeRate()       const;
    float getPowerLevel()       const;
    int   getDronesServed()     const;
    bool  getIsOccupied()       const;
};

#endif // CHARGING_PAD_H