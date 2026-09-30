/*
 * ============================================================
 * FILE        : ChargingPad.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 CO1 | Task 2 CO2
 * ============================================================
 */

#include "ChargingPad.h"

ChargingPad::ChargingPad(int row, int col,
                         const std::string& label,
                         float chargeRate,
                         int   slots)
    : Location(row, col, label, CELL_CHARGING),
      chargeRatePerMinute(chargeRate),
      currentPowerLevel(100.0f),
      dronesServedTotal(0),
      isOccupied(false),
      chargingSlots(slots)
{
    std::cout << "[PSOOP] ChargingPad constructed: "
              << label << "\n";
}

ChargingPad::~ChargingPad() {
    std::cout << "[PSOOP] ChargingPad destructed: "
              << getLabel() << "\n";
}

// [PSOOP CO2 - Task 2: Override display()]
void ChargingPad::display() const {
    std::cout << "\n  ┌─────────────────────────────┐\n";
    std::cout << "  │  CHARGING PAD               │\n";
    std::cout << "  └─────────────────────────────┘\n";
    displayBase();
    std::cout << "  Charge Rate : "
              << chargeRatePerMinute << "% / min\n";
    std::cout << "  Power Level : "
              << currentPowerLevel  << "%\n";
    std::cout << "  Slots       : "
              << chargingSlots      << "\n";
    std::cout << "  Occupied    : "
              << (isOccupied ? "YES" : "NO") << "\n";
    std::cout << "  Total Served: "
              << dronesServedTotal  << " drones\n";
}

std::string ChargingPad::getTypeName() const { return "ChargingPad"; }
int         ChargingPad::getTypeCode() const { return CELL_CHARGING; }

// [PSOOP CO2 - Dock allowed only if not fully occupied]
bool ChargingPad::canDroneDock() const {
    return getIsActive() && !isOccupied;
}

bool ChargingPad::startCharging() {
    if (isOccupied) {
        std::cout << "[PSOOP] ChargingPad "
                  << getLabel() << " is occupied.\n";
        return false;
    }
    isOccupied = true;
    std::cout << "[PSOOP] Charging started at "
              << getLabel() << "\n";
    return true;
}

void ChargingPad::finishCharging() {
    isOccupied = false;
    dronesServedTotal++;
    std::cout << "[PSOOP] Charging complete at "
              << getLabel()
              << " | Total served: "
              << dronesServedTotal << "\n";
}

float ChargingPad::estimateChargeTime(
    float currentBattery) const
{
    if (chargeRatePerMinute <= 0) return -1.0f;
    return (100.0f - currentBattery) / chargeRatePerMinute;
}

float ChargingPad::getChargeRate()   const { return chargeRatePerMinute; }
float ChargingPad::getPowerLevel()   const { return currentPowerLevel; }
int   ChargingPad::getDronesServed() const { return dronesServedTotal; }
bool  ChargingPad::getIsOccupied()   const { return isOccupied; }