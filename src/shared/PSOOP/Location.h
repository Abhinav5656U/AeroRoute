/*
 * ============================================================
 * FILE        : Location.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 — Class Hierarchy Design (CO1)
 * COURSE OUT  : CO1 — Design modular class hierarchies
 *               representing real-world entities.
 *               Encapsulate data with proper access specifiers.
 * DESCRIPTION : Abstract base class for all location types
 *               in the AeroRoute city grid.
 *               Warehouse, CustomerSite, ChargingPad all
 *               inherit from this class.
 * AUTHOR      : [Your Name] | MIT Academy of Engineering
 * DATE        : 2026-2027
 * ============================================================
 */

#ifndef LOCATION_H
#define LOCATION_H

#include <string>
#include <iostream>
#include <iomanip>
#include "../shared/Constants.h"

// ════════════════════════════════════════════════════════════
// [PSOOP CO1 - Task 1: Abstract Base Class — Location]
//
// Represents any physical location in the delivery grid.
// Cannot be instantiated directly (pure virtual methods).
// All derived classes MUST implement display() and getType().
//
// Access specifiers used:
//   private   → internal data no subclass should touch
//   protected → data subclasses can access directly
//   public    → interface exposed to the rest of the system
// ════════════════════════════════════════════════════════════
class Location {
private:
    // [PSOOP CO1 - private: unique ID auto-assigned]
    // No subclass or outside code should change this directly
    int locationId;

    // Static counter to generate unique IDs
    static int idCounter;

protected:
    // [PSOOP CO1 - protected: subclasses can read/write these]
    int         row;        // grid row position
    int         col;        // grid column position
    std::string label;      // user-defined name e.g. "WH-Main"
    bool        isActive;   // whether this location is in use
    int         cellTypeCode; // matches Constants.h cell codes

public:
    // ── Constructor ──────────────────────────────────────────
    // [PSOOP CO1 - Task 1: Constructor with encapsulated init]
    Location(int r, int c,
             const std::string& lbl,
             int typeCode);

    // ── Virtual Destructor ───────────────────────────────────
    // [PSOOP CO1 - Must be virtual in base class]
    // Ensures correct destructor is called for derived objects
    virtual ~Location();

    // ════════════════════════════════════════════════════════
    // [PSOOP CO1 CO2 - Pure Virtual Methods]
    // Every derived class MUST override these.
    // This is what makes Location abstract.
    // ════════════════════════════════════════════════════════

    // Display complete information about this location
    virtual void display() const = 0;

    // Return the type name as a string
    virtual std::string getTypeName() const = 0;

    // Return the cell type code (matches grid constants)
    virtual int getTypeCode() const = 0;

    // Check if a drone can land/dock at this location
    virtual bool canDroneDock() const = 0;

    // ── Concrete Public Methods ───────────────────────────────
    // [PSOOP CO1 - Task 1: Encapsulated getters]
    int         getId()       const;
    int         getRow()      const;
    int         getCol()      const;
    std::string getLabel()    const;
    bool        getIsActive() const;

    // [PSOOP CO1 - Task 1: Encapsulated setters with validation]
    void setLabel(const std::string& newLabel);
    void setActive(bool active);

    // Common display helper used by all subclasses
    void displayBase() const;

    // Operator overload for printing
    friend std::ostream& operator<<(std::ostream& os,
                                    const Location& loc);
};

#endif // LOCATION_H