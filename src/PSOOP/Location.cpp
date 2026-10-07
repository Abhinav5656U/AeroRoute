/*
 *
 * FILE        : Location.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 — CO1
 *
 */

#include "Location.h"
using namespace std;

// Static member definition — starts at 1
// [PSOOP CO1 - Task 1: Static member for unique ID generation]
int Location::idCounter = 1;

// ── Constructor ───────────────────────────────────────────────
// [PSOOP CO1 - Task 1: Base class constructor]
Location::Location(int r, int c,
                   const string& lbl,
                   int typeCode)
    : row(r), col(c), label(lbl),
      isActive(true), cellTypeCode(typeCode)
{
    // Auto-assign unique ID from static counter
    locationId = idCounter++;

    cout << "[PSOOP] Location created: "
              << label
              << " ID=" << locationId
              << " at (" << row << "," << col << ")\n";
}

// ── Virtual Destructor ────────────────────────────────────────
Location::~Location() {
    cout << "[PSOOP] Location destroyed: "
              << label << " (ID=" << locationId << ")\n";
}

// ── Getters ───────────────────────────────────────────────────
// [PSOOP CO1 - Task 1: Encapsulated read access]
int         Location::getId()       const { return locationId; }
int         Location::getRow()      const { return row; }
int         Location::getCol()      const { return col; }
string Location::getLabel()    const { return label; }
bool        Location::getIsActive() const { return isActive; }

// ── Setters with validation ───────────────────────────────────
// [PSOOP CO1 - Task 1: Encapsulated write access]
void Location::setLabel(const string& newLabel) {
    if (newLabel.empty()) {
        cout << "[PSOOP] WARNING: empty label rejected.\n";
        return;
    }
    label = newLabel;
}

void Location::setActive(bool active) {
    isActive = active;
    cout << "[PSOOP] " << label
              << (active ? " activated." : " deactivated.")
              << "\n";
}

// ── displayBase ───────────────────────────────────────────────
// [PSOOP CO1 - Task 1: Shared display logic for subclasses]
void Location::displayBase() const {
    cout << "  ID       : " << locationId        << "\n";
    cout << "  Label    : " << label              << "\n";
    cout << "  Position : (" << row
                                  << "," << col << ")\n";
    cout << "  Active   : "
              << (isActive ? "Yes" : "No") << "\n";
    cout << "  TypeCode : " << cellTypeCode       << "\n";
}

// ── Operator overload ─────────────────────────────────────────
// [PSOOP CO1 - Task 1: Friend operator overload]
ostream& operator<<(ostream& os,
                          const Location& loc) {
    os << "[" << loc.getTypeName()
       << "|" << loc.label
       << "|(" << loc.row << "," << loc.col << ")]";
    return os;
}