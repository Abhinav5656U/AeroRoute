/*
 *
 * FILE        : Drone.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 CO1 | Task 2 CO2
 * COURSE OUT  : CO2 — Abstract Drone base class with virtual
 *               methods. DeliveryDrone, ScoutDrone, HeavyDrone
 *               inherit and override fly() and getModelName().
 *               Demonstrates polymorphism eliminating
 *               redundant code across drone models.
 *
 */

#ifndef DRONE_H
#define DRONE_H

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "Package.h"
#include "Location.h"
using namespace std;

// ════════════════════════════════════════════════════════════
// [PSOOP CO2 - Task 2: Abstract Drone Base Class]
// Cannot be instantiated directly.
// All drone models share this interface.
// ════════════════════════════════════════════════════════════
class Drone {
private:
    int         droneId;
    static int  idCounter;

protected:
    // [PSOOP CO2 - protected: accessible to all drone models]
    string serialNumber;
    double      batteryPct;
    double      maxBatteryWh;
    int         currentRow;
    int         currentCol;
    double      headingDeg;
    double      altitudeM;
    double      speedMs;
    string status;  // IDLE FLYING CHARGING DOCKED ERROR

    // Package currently being carried (nullptr if empty)
    Package*    loadedPackage;

    // Flight history (list of visited positions)
    vector<pair<int,int>> flightLog;

public:
    Drone(const string& serial,
          double maxBattery,
          int startRow, int startCol);

    virtual ~Drone();

    // ════════════════════════════════════════════════════════
    // [PSOOP CO2 - Task 2: Pure Virtual Methods]
    // Each drone model implements these differently.
    // This is the core of polymorphism in this project.
    // ════════════════════════════════════════════════════════

    // Fly to a target location
    // Different drone models handle this differently
    virtual void fly(int targetRow, int targetCol) = 0;

    // Return model name string
    virtual string getModelName() const = 0;

    // Return max payload in kg
    virtual double getMaxPayloadKg() const = 0;

    // Return max range in grid units
    virtual int getMaxRangeUnits() const = 0;

    // Display drone-specific info
    virtual void displaySpecs() const = 0;

    // ── Concrete shared methods ───────────────────────────────
    // [PSOOP CO2 - Eliminate redundant code across models]
    // These are implemented ONCE here, used by ALL subclasses.

    bool   loadPackage(Package* pkg);
    bool   unloadPackage();
    void   chargeBattery(float percent);
    void   drainBattery(double amount);
    bool   needsCharging(double threshold = 20.0) const;
    void   display() const;
    void   displayFlightLog() const;

    // ── Getters ──────────────────────────────────────────────
    int         getId()            const;
    string getSerialNumber()  const;
    double      getBatteryPct()    const;
    int         getCurrentRow()    const;
    int         getCurrentCol()    const;
    double      getHeadingDeg()    const;
    double      getAltitudeM()     const;
    string getStatus()        const;
    bool        hasPackage()       const;

    // ── Setters ──────────────────────────────────────────────
    void setStatus(const string& s);
    void setAltitude(double alt);
    void setHeading(double deg);
};

// ════════════════════════════════════════════════════════════
// [PSOOP CO2 - Task 2: DeliveryDrone]
// Standard delivery model. Balanced payload and range.
// Inherits from Drone, overrides all pure virtual methods.
// ════════════════════════════════════════════════════════════
class DeliveryDrone : public Drone {
private:
    bool hasParachuteDrop; // can drop without landing

public:
    DeliveryDrone(const string& serial,
                  int startRow, int startCol,
                  bool parachute = false);
    ~DeliveryDrone();

    // [PSOOP CO2 - Override pure virtual methods]
    void        fly(int targetRow, int targetCol) override;
    string getModelName()    const override;
    double      getMaxPayloadKg() const override;
    int         getMaxRangeUnits()const override;
    void        displaySpecs()    const override;
};

// ════════════════════════════════════════════════════════════
// [PSOOP CO2 - Task 2: ScoutDrone]
// Lightweight recon model. No payload. Long range.
// ════════════════════════════════════════════════════════════
class ScoutDrone : public Drone {
private:
    bool hasCameraFeed;
    int  scanRadius;

public:
    ScoutDrone(const string& serial,
               int startRow, int startCol,
               int radius = 3);
    ~ScoutDrone();

    void        fly(int targetRow, int targetCol) override;
    string getModelName()    const override;
    double      getMaxPayloadKg() const override;
    int         getMaxRangeUnits()const override;
    void        displaySpecs()    const override;

    // Scout-only
    void scanArea(int row, int col);
};

// ════════════════════════════════════════════════════════════
// [PSOOP CO2 - Task 2: HeavyDrone]
// High-capacity model. Heavy payload. Short range.
// ════════════════════════════════════════════════════════════
class HeavyDrone : public Drone {
private:
    int  numRotors;        // more rotors for heavy lift
    bool hasStabiliser;

public:
    HeavyDrone(const string& serial,
               int startRow, int startCol,
               int rotors = 8);
    ~HeavyDrone();

    void        fly(int targetRow, int targetCol) override;
    string getModelName()    const override;
    double      getMaxPayloadKg() const override;
    int         getMaxRangeUnits()const override;
    void        displaySpecs()    const override;
};

#endif // DRONE_H