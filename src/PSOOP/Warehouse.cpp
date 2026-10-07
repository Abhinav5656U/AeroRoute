/*
 *
 * FILE        : Warehouse.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 CO1 | Task 2 CO2
 *
 */

#include "Warehouse.h"
using namespace std;

// ── Constructor ───────────────────────────────────────────────
// [PSOOP CO1 - Task 1: Call base class constructor first]
// Base class Location is initialised via initialiser list
// before Warehouse-specific members are set.
Warehouse::Warehouse(int row, int col,
                     const string& label,
                     int cap,
                     const string& manager,
                     bool mainHub)
    : Location(row, col, label, CELL_WAREHOUSE),
      capacity(cap),
      currentStock(0),
      managerName(manager),
      isMainHub(mainHub)
{
    cout << "[PSOOP] Warehouse constructed: "
              << label << "\n";
}

Warehouse::~Warehouse() {
    cout << "[PSOOP] Warehouse destructed: "
              << getLabel() << "\n";
}

// ════════════════════════════════════════════════════════════
// [PSOOP CO2 - Task 2: Overriding virtual display()]
// This override provides Warehouse-specific display.
// Calls displayBase() from Location to avoid code duplication.
// ════════════════════════════════════════════════════════════
void Warehouse::display() const {
    cout << "\n  ┌─────────────────────────────┐\n";
    cout << "  │  WAREHOUSE                  │\n";
    cout << "  └─────────────────────────────┘\n";
    displayBase(); // reuse base class common display
    cout << "  Manager  : " << managerName   << "\n";
    cout << "  Capacity : " << capacity      << " packages\n";
    cout << "  Stock    : " << currentStock  << " / "
              << capacity << " ("
              << fixed << setprecision(1)
              << getStockPercent() << "%)\n";
    cout << "  Main Hub : "
              << (isMainHub ? "YES" : "NO") << "\n";
}

// [PSOOP CO2 - Task 2: Override getTypeName()]
string Warehouse::getTypeName() const {
    return "Warehouse";
}

// [PSOOP CO2 - Task 2: Override getTypeCode()]
int Warehouse::getTypeCode() const {
    return CELL_WAREHOUSE;
}

// [PSOOP CO2 - Task 2: Override canDroneDock()]
// Drones can dock at warehouse only if it is active
bool Warehouse::canDroneDock() const {
    return getIsActive();
}

// ── Stock management ──────────────────────────────────────────
bool Warehouse::addStock(int quantity) {
    if (currentStock + quantity > capacity) {
        cout << "[PSOOP] Warehouse " << getLabel()
                  << " is full. Cannot add "
                  << quantity << " packages.\n";
        return false;
    }
    currentStock += quantity;
    cout << "[PSOOP] Stock added: +"
              << quantity << " | Total: "
              << currentStock << "\n";
    return true;
}

bool Warehouse::removeStock(int quantity) {
    if (currentStock - quantity < 0) {
        cout << "[PSOOP] Not enough stock in "
                  << getLabel() << "\n";
        return false;
    }
    currentStock -= quantity;
    cout << "[PSOOP] Stock removed: -"
              << quantity << " | Remaining: "
              << currentStock << "\n";
    return true;
}

// ── Getters ───────────────────────────────────────────────────
int         Warehouse::getCapacity()     const { return capacity; }
int         Warehouse::getCurrentStock() const { return currentStock; }
string Warehouse::getManagerName()  const { return managerName; }
bool        Warehouse::getIsMainHub()    const { return isMainHub; }
float       Warehouse::getStockPercent() const {
    if (capacity == 0) return 0.0f;
    return (float)currentStock / capacity * 100.0f;
}