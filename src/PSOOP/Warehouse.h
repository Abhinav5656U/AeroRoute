/*
 *
 * FILE        : Warehouse.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 — CO1 | Task 2 — CO2
 * COURSE OUT  : CO1 — Derived class with encapsulated data
 *               CO2 — Inherits from Location, overrides
 *                     virtual methods (polymorphism)
 *
 */

#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include "Location.h"
#include <vector>
using namespace std;

// ════════════════════════════════════════════════════════════
// [PSOOP CO1 CO2 - Task 1 Task 2: Warehouse class]
// Derives from Location (public inheritance).
// A warehouse is a dispatch hub — drones depart from here
// carrying packages to customer sites.
// ════════════════════════════════════════════════════════════
class Warehouse : public Location {
private:
    // [PSOOP CO1 - private data specific to Warehouse]
    int         capacity;        // max packages storable
    int         currentStock;    // packages currently in stock
    string managerName;     // warehouse manager
    bool        isMainHub;       // is this the primary hub?

public:
    // ── Constructor ──────────────────────────────────────────
    Warehouse(int row, int col,
              const string& label,
              int capacity       = 100,
              const string& manager = "Unassigned",
              bool mainHub       = false);

    // ── Destructor ───────────────────────────────────────────
    ~Warehouse();

    // ════════════════════════════════════════════════════════
    // [PSOOP CO2 - Task 2: Override pure virtual methods]
    // These are mandatory overrides from Location base class.
    // virtual keyword in base, override keyword here.
    // ════════════════════════════════════════════════════════
    void        display()      const override;
    string getTypeName()  const override;
    int         getTypeCode()  const override;
    bool        canDroneDock() const override;

    // ── Warehouse-specific methods ────────────────────────────
    bool addStock(int quantity);
    bool removeStock(int quantity);

    // ── Getters ──────────────────────────────────────────────
    int         getCapacity()    const;
    int         getCurrentStock()const;
    string getManagerName() const;
    bool        getIsMainHub()   const;
    float       getStockPercent()const;
};

#endif // WAREHOUSE_H