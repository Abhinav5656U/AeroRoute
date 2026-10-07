/*
 *
 * FILE        : Drone.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 CO1 | Task 2 CO2
 *
 */

#include "Drone.h"
#include <cmath>
#include <iomanip>
#include <stdexcept>
using namespace std;

int Drone::idCounter = 1;

// ════════════════════════════════════════════════════════════
// Drone Base Class Implementation
// ════════════════════════════════════════════════════════════
Drone::Drone(const string& serial,
             double maxBattery,
             int startRow, int startCol)
    : serialNumber(serial),
      batteryPct(100.0),
      maxBatteryWh(maxBattery),
      currentRow(startRow),
      currentCol(startCol),
      headingDeg(0.0),
      altitudeM(0.0),
      speedMs(0.0),
      status("IDLE"),
      loadedPackage(nullptr)
{
    droneId = idCounter++;
    cout << "[PSOOP] Drone created"
              << " | Serial: " << serial
              << " | ID: " << droneId << "\n";
}

Drone::~Drone() {
    cout << "[PSOOP] Drone destroyed: "
              << serialNumber << "\n";
}

// ════════════════════════════════════════════════════════════
// [PSOOP CO2 - Shared concrete methods]
// Implemented ONCE in base class.
// All three drone models inherit and use these directly.
// This eliminates code duplication — CO2 objective.
// ════════════════════════════════════════════════════════════

// Load package onto drone
bool Drone::loadPackage(Package* pkg) {
    if (!pkg) {
        throw invalid_argument("Cannot load a null package.");
    }
    if (loadedPackage != nullptr) {
        cout << "[PSOOP] " << serialNumber
                  << " already carrying a package.\n";
        return false;
    }
    if (!pkg->isWithinDroneLimit(getMaxPayloadKg())) {
        cout << "[PSOOP] Package too heavy for "
                  << getModelName()
                  << " (max " << getMaxPayloadKg() << "kg).\n";
        return false;
    }
    loadedPackage = pkg;
    pkg->markLoaded();
    status = "LOADED";
    cout << "[PSOOP] " << serialNumber
              << " loaded: " << pkg->getTrackingCode() << "\n";
    return true;
}

// Unload package at destination
bool Drone::unloadPackage() {
    if (!loadedPackage) {
        cout << "[PSOOP] No package to unload.\n";
        return false;
    }
    loadedPackage->markDelivered();
    loadedPackage = nullptr;
    status = "IDLE";
    cout << "[PSOOP] " << serialNumber
              << " package delivered and unloaded.\n";
    return true;
}

// Charge battery
void Drone::chargeBattery(float percent) {
    batteryPct = min(100.0, batteryPct + percent);
    cout << "[PSOOP] " << serialNumber
              << " charged to " << batteryPct << "%\n";
}

// Drain battery (per flight unit)
void Drone::drainBattery(double amount) {
    batteryPct = max(0.0, batteryPct - amount);
}

// Check if charging is needed
bool Drone::needsCharging(double threshold) const {
    return batteryPct < threshold;
}

// ── Base display ──────────────────────────────────────────────
// [PSOOP CO2 - CO2: Shared display used by all models]
void Drone::display() const {
    cout << "\n  ┌─────────────────────────────┐\n";
    cout << "  │  " << left << setw(27)
              << getModelName() << "│\n";
    cout << "  └─────────────────────────────┘\n";
    cout << "  ID       : " << droneId           << "\n";
    cout << "  Serial   : " << serialNumber      << "\n";
    cout << "  Position : (" << currentRow
                                  << "," << currentCol << ")\n";
    cout << "  Altitude : " << altitudeM << " m" << "\n";
    cout << "  Heading  : " << headingDeg << " deg\n";
    cout << "  Battery  : " << fixed
              << setprecision(1)
              << batteryPct << "%\n";
    cout << "  Status   : " << status            << "\n";
    cout << "  Payload  : max "
              << getMaxPayloadKg() << " kg\n";
    cout << "  Range    : max "
              << getMaxRangeUnits() << " cells\n";
    cout << "  Package  : "
              << (loadedPackage
                  ? loadedPackage->getTrackingCode()
                  : "none") << "\n";
    displaySpecs();
}

void Drone::displayFlightLog() const {
    cout << "\n  Flight Log (" << serialNumber << "):\n";
    if (flightLog.empty()) {
        cout << "  No flights recorded.\n";
        return;
    }
    for (int i = 0; i < (int)flightLog.size(); i++) {
        cout << "  [" << i << "] ("
                  << flightLog[i].first << ","
                  << flightLog[i].second << ")\n";
    }
}

// ── Getters / Setters ─────────────────────────────────────────
int         Drone::getId()           const { return droneId; }
string Drone::getSerialNumber() const { return serialNumber; }
double      Drone::getBatteryPct()   const { return batteryPct; }
int         Drone::getCurrentRow()   const { return currentRow; }
int         Drone::getCurrentCol()   const { return currentCol; }
double      Drone::getHeadingDeg()   const { return headingDeg; }
double      Drone::getAltitudeM()    const { return altitudeM; }
string Drone::getStatus()       const { return status; }
bool        Drone::hasPackage()      const {
    return loadedPackage != nullptr;
}

void Drone::setStatus(const string& s)  { status = s; }
void Drone::setAltitude(double alt)          { altitudeM = alt; }
void Drone::setHeading(double deg)           { headingDeg = deg; }

// ════════════════════════════════════════════════════════════
// DeliveryDrone Implementation
// [PSOOP CO2 - Task 2: Override fly() for delivery logic]
// ════════════════════════════════════════════════════════════
DeliveryDrone::DeliveryDrone(const string& serial,
                             int startRow, int startCol,
                             bool parachute)
    : Drone(serial, 150.0, startRow, startCol),
      hasParachuteDrop(parachute)
{
    cout << "[PSOOP] DeliveryDrone ready: "
              << serial << "\n";
}

DeliveryDrone::~DeliveryDrone() {}

// [PSOOP CO2 - Task 2: Polymorphic fly() for delivery]
void DeliveryDrone::fly(int targetRow, int targetCol) {
    cout << "\n[PSOOP] DeliveryDrone " << serialNumber
              << " flying: (" << currentRow << ","
              << currentCol << ") -> ("
              << targetRow  << "," << targetCol << ")\n";

    // Calculate distance
    double dx = targetCol - currentCol;
    double dy = targetRow - currentRow;
    double dist = sqrt(dx*dx + dy*dy);

    // Update heading
    headingDeg = atan2(dy, dx) * 180.0 / 3.14159265;

    // Drain battery proportional to distance
    double drain = dist * 2.5;
    drainBattery(drain);
    status = "FLYING";
    altitudeM = 120.0;

    // Update position
    flightLog.push_back({currentRow, currentCol});
    currentRow = targetRow;
    currentCol = targetCol;
    altitudeM  = 0.0;
    status     = "IDLE";

    cout << "[PSOOP] DeliveryDrone arrived. "
              << "Battery: " << fixed
              << setprecision(1)
              << batteryPct << "%\n";

    if (loadedPackage) loadedPackage->markInTransit();
}

string DeliveryDrone::getModelName()    const { return "AR-D1 DeliveryDrone"; }
double      DeliveryDrone::getMaxPayloadKg() const { return 15.0; }
int         DeliveryDrone::getMaxRangeUnits()const { return 8; }

void DeliveryDrone::displaySpecs() const {
    cout << "  Parachute: "
              << (hasParachuteDrop ? "Yes" : "No") << "\n";
}

// ════════════════════════════════════════════════════════════
// ScoutDrone Implementation
// [PSOOP CO2 - Task 2: Different fly() — lighter, faster]
// ════════════════════════════════════════════════════════════
ScoutDrone::ScoutDrone(const string& serial,
                       int startRow, int startCol,
                       int radius)
    : Drone(serial, 80.0, startRow, startCol),
      hasCameraFeed(true),
      scanRadius(radius)
{
    cout << "[PSOOP] ScoutDrone ready: "
              << serial << "\n";
}

ScoutDrone::~ScoutDrone() {}

// [PSOOP CO2 - Scout fly() — lighter drain, higher altitude]
void ScoutDrone::fly(int targetRow, int targetCol) {
    cout << "\n[PSOOP] ScoutDrone " << serialNumber
              << " scouting: (" << currentRow << ","
              << currentCol << ") -> ("
              << targetRow  << "," << targetCol << ")\n";

    double dx   = targetCol - currentCol;
    double dy   = targetRow - currentRow;
    double dist = sqrt(dx*dx + dy*dy);

    headingDeg = atan2(dy, dx) * 180.0 / 3.14159265;
    drainBattery(dist * 1.2); // lighter drain
    status    = "FLYING";
    altitudeM = 180.0; // flies higher for recon

    flightLog.push_back({currentRow, currentCol});
    currentRow = targetRow;
    currentCol = targetCol;
    altitudeM  = 0.0;
    status     = "IDLE";

    cout << "[PSOOP] ScoutDrone arrived. Battery: "
              << fixed << setprecision(1)
              << batteryPct << "%\n";
}

void ScoutDrone::scanArea(int row, int col) {
    cout << "[PSOOP] ScoutDrone scanning radius "
              << scanRadius << " around ("
              << row << "," << col << ")\n";
    for (int dr = -scanRadius; dr <= scanRadius; dr++) {
        for (int dc = -scanRadius; dc <= scanRadius; dc++) {
            int r = row + dr, c = col + dc;
            if (r >= 0 && r < GRID_ROWS &&
                c >= 0 && c < GRID_COLS)
                cout << "  Scanned: (" << r
                          << "," << c << ")\n";
        }
    }
}

string ScoutDrone::getModelName()    const { return "AR-S1 ScoutDrone"; }
double      ScoutDrone::getMaxPayloadKg() const { return 0.5; }
int         ScoutDrone::getMaxRangeUnits()const { return 15; }

void ScoutDrone::displaySpecs() const {
    cout << "  Camera   : "
              << (hasCameraFeed ? "Live feed" : "None") << "\n";
    cout << "  Scan R   : " << scanRadius << " cells\n";
}

// ════════════════════════════════════════════════════════════
// HeavyDrone Implementation
// [PSOOP CO2 - Task 2: Heaviest payload, shortest range]
// ════════════════════════════════════════════════════════════
HeavyDrone::HeavyDrone(const string& serial,
                       int startRow, int startCol,
                       int rotors)
    : Drone(serial, 300.0, startRow, startCol),
      numRotors(rotors),
      hasStabiliser(true)
{
    cout << "[PSOOP] HeavyDrone ready: "
              << serial << " (" << rotors << " rotors)\n";
}

HeavyDrone::~HeavyDrone() {}

// [PSOOP CO2 - Heavy fly() — most battery drain]
void HeavyDrone::fly(int targetRow, int targetCol) {
    cout << "\n[PSOOP] HeavyDrone " << serialNumber
              << " lifting: (" << currentRow << ","
              << currentCol << ") -> ("
              << targetRow  << "," << targetCol << ")\n";

    double dx   = targetCol - currentCol;
    double dy   = targetRow - currentRow;
    double dist = sqrt(dx*dx + dy*dy);

    headingDeg = atan2(dy, dx) * 180.0 / 3.14159265;
    drainBattery(dist * 4.5); // heaviest drain
    status    = "FLYING";
    altitudeM = 80.0; // flies lower due to weight

    flightLog.push_back({currentRow, currentCol});
    currentRow = targetRow;
    currentCol = targetCol;
    altitudeM  = 0.0;
    status     = "IDLE";

    cout << "[PSOOP] HeavyDrone arrived. Battery: "
              << fixed << setprecision(1)
              << batteryPct << "%\n";

    if (loadedPackage) loadedPackage->markInTransit();
}

string HeavyDrone::getModelName()    const { return "AR-H1 HeavyDrone"; }
double      HeavyDrone::getMaxPayloadKg() const { return 50.0; }
int         HeavyDrone::getMaxRangeUnits()const { return 4; }

void HeavyDrone::displaySpecs() const {
    cout << "  Rotors   : " << numRotors     << "\n";
    cout << "  Stabiliser: "
              << (hasStabiliser ? "Yes" : "No") << "\n";
}