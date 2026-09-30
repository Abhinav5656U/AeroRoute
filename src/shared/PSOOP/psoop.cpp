/*
 * ============================================================
 * FILE        : Subject4_Main.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 (CO1) + Task 2 (CO2) — Full Demo
 * DESCRIPTION : Standalone entry point for PSOOP subject.
 *               Demonstrates full OOP class hierarchy,
 *               encapsulation, inheritance, and polymorphism.
 *               Writes entity data to website mission.json.
 * HOW TO RUN  :
 *   cd src/PSOOP
 *   g++ Subject4_Main.cpp Location.cpp Warehouse.cpp CustomerSite.cpp ChargingPad.cpp Package.cpp Drone.cpp -o ..\..\build\PSOOP.exe -std=c++17
 *   ..\..\build\PSOOP.exe
 * ============================================================
 */

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <limits>
#include "Location.h"
#include "Warehouse.h"
#include "CustomerSite.h"
#include "ChargingPad.h"
#include "Package.h"
#include "Drone.h"
#include "../shared/JsonWriter.h"

// ── Forward declarations ──────────────────────────────────────
void showMenu();
void runTask1Demo();
void runTask2Demo();
void runFullMissionDemo();
void runInteractiveMode();
void exportEntitiesToJson(
    const std::vector<Location*>& locations,
    const std::vector<Drone*>&    drones,
    const std::vector<Package*>&  packages);
void clearInput();

// ── Main ──────────────────────────────────────────────────────
int main() {

    std::cout << "\n";
    std::cout << "  ================================================\n";
    std::cout << "   AeroRoute — PSOOP: Problem Solving Using OOP\n";
    std::cout << "   Course Code: 2310261L\n";
    std::cout << "   CO1: Class Hierarchy & Encapsulation\n";
    std::cout << "   CO2: Inheritance & Polymorphism\n";
    std::cout << "   MIT Academy of Engineering | Sem III 2026-27\n";
    std::cout << "  ================================================\n\n";

    int choice = 0;
    do {
        showMenu();
        std::cin >> choice;
        clearInput();

        switch(choice) {
            case 1: runTask1Demo();       break;
            case 2: runTask2Demo();       break;
            case 3: runFullMissionDemo(); break;
            case 4: runInteractiveMode(); break;
            case 0:
                std::cout << "\n  Exiting PSOOP.\n\n";
                break;
            default:
                std::cout << "\n  Invalid choice.\n\n";
        }
    } while(choice != 0);

    return 0;
}

void showMenu() {
    std::cout << "  ------------------------------------------------\n";
    std::cout << "   PSOOP — MAIN MENU\n";
    std::cout << "  ------------------------------------------------\n";
    std::cout << "   [1] Task 1 — Class Hierarchy (CO1)\n";
    std::cout << "       Location Warehouse CustomerSite\n";
    std::cout << "       ChargingPad Package — all displayed\n";
    std::cout << "   [2] Task 2 — Polymorphism Demo (CO2)\n";
    std::cout << "       Drone base + DeliveryDrone ScoutDrone\n";
    std::cout << "       HeavyDrone — virtual dispatch shown\n";
    std::cout << "   [3] Full Mission Demo (CO1 + CO2)\n";
    std::cout << "       Complete delivery mission simulation\n";
    std::cout << "   [4] Interactive Mode\n";
    std::cout << "   [0] Exit\n";
    std::cout << "  ------------------------------------------------\n";
    std::cout << "   Choice: ";
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(), '\n');
}

// ════════════════════════════════════════════════════════════
// TASK 1 DEMO — Class Hierarchy
// [PSOOP CO1 - Task 1: Demonstrates all entity classes]
// ════════════════════════════════════════════════════════════
void runTask1Demo() {
    std::cout << "\n  --- TASK 1: OOP Class Hierarchy Demo ---\n\n";

    std::cout << "  Creating Location subclass objects:\n\n";

    // [PSOOP CO1 - Warehouse object]
    Warehouse wh1(0, 0, "WH-Main", 200, "Ramesh Kumar", true);
    wh1.addStock(50);
    wh1.display();

    // [PSOOP CO1 - CustomerSite object]
    CustomerSite cs1(9, 9, "CS-Alpha",
                     "Priya Sharma", "Sector 7", true);
    cs1.addPendingDelivery();
    cs1.display();

    // [PSOOP CO1 - ChargingPad object]
    ChargingPad cp1(4, 4, "CP-Central", 20.0f, 2);
    cp1.display();
    std::cout << "  Charge time estimate (30% battery): "
              << cp1.estimateChargeTime(30.0f)
              << " minutes\n";

    // [PSOOP CO1 - Package object]
    Package pkg1(4.5, 30.0, 20.0, 15.0,
                 0, 0, 9, 9, "Priya Sharma");
    pkg1.display();

    // Export to JSON
    std::vector<Location*> locs = {&wh1, &cs1, &cp1};
    std::vector<Drone*>    drones;
    std::vector<Package*>  pkgs  = {&pkg1};
    exportEntitiesToJson(locs, drones, pkgs);

    std::cout << "\n  Task 1 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// TASK 2 DEMO — Polymorphism
// [PSOOP CO2 - Task 2: Demonstrates virtual dispatch]
//
// KEY POINT for professor:
// We store all drones as Drone* (base class pointer).
// When we call drone->fly() or drone->display(),
// C++ automatically calls the correct subclass version.
// This is RUNTIME POLYMORPHISM via virtual dispatch table.
// ════════════════════════════════════════════════════════════
void runTask2Demo() {
    std::cout << "\n  --- TASK 2: Polymorphism Demo ---\n\n";

    std::cout << "  [PSOOP CO2] Creating drones as base class"
              << " pointers (Drone*):\n\n";

    // All stored as base class pointer — key OOP concept
    std::vector<Drone*> fleet;

    fleet.push_back(new DeliveryDrone("AR-D1-001", 0, 0));
    fleet.push_back(new ScoutDrone   ("AR-S1-001", 0, 0));
    fleet.push_back(new HeavyDrone   ("AR-H1-001", 0, 0, 8));

    std::cout << "\n  [PSOOP CO2] Calling display() on each drone\n";
    std::cout << "  SAME function call -> DIFFERENT behaviour\n";
    std::cout << "  This is runtime polymorphism:\n\n";

    // Same call, different result — polymorphism
    for (Drone* d : fleet) {
        d->display();
        std::cout << "\n";
    }

    std::cout << "  [PSOOP CO2] Calling fly() on each drone\n";
    std::cout << "  SAME call -> different flight behaviour:\n\n";

    // Same fly() call — each drone behaves differently
    for (Drone* d : fleet) {
        std::cout << "  Calling: drone->fly(5, 5)  ["
                  << d->getModelName() << "]\n";
        d->fly(5, 5);
    }

    // Check payload capacity polymorphically
    std::cout << "\n  [PSOOP CO2] Payload capacity check"
              << " (polymorphic getMaxPayloadKg()):\n";
    Package heavyPkg(30.0, 50.0, 40.0, 30.0,
                     0, 0, 9, 9, "Factory");

    for (Drone* d : fleet) {
        bool fits = heavyPkg.isWithinDroneLimit(
                        d->getMaxPayloadKg());
        std::cout << "  " << d->getModelName()
                  << " (max " << d->getMaxPayloadKg() << "kg)"
                  << ": 30kg package "
                  << (fits ? "FITS" : "TOO HEAVY") << "\n";
    }

    // Free memory
    for (Drone* d : fleet) delete d;

    std::cout << "\n  Task 2 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// FULL MISSION DEMO — CO1 + CO2 Together
// [PSOOP - Runs a complete delivery scenario]
// ════════════════════════════════════════════════════════════
void runFullMissionDemo() {
    std::cout << "\n  --- Full Mission Demo ---\n\n";

    // ── Create locations ──────────────────────────────────────
    Warehouse    wh("WH-Main",  0, 0, "WH-Main", 100,
                    "Manager", true);
    CustomerSite cs(9, 9, "CS-Alpha", "Amit Patel",
                    "Sector 12");
    ChargingPad  cp(4, 4, "CP-Mid", 25.0f);

    wh.addStock(10);
    cs.addPendingDelivery();

    // ── Create drone and package ──────────────────────────────
    DeliveryDrone drone("AR-D1-MISSION", 0, 0);
    Package       pkg(3.5, 25.0, 15.0, 10.0,
                      0, 0, 9, 9, "Amit Patel");

    std::cout << "\n  === MISSION START ===\n\n";

    // Step 1: Load package
    std::cout << "  Step 1: Load package onto drone\n";
    drone.loadPackage(&pkg);

    // Step 2: Fly to charging pad (mid-route)
    std::cout << "\n  Step 2: Fly to charging pad\n";
    drone.fly(cp.getRow(), cp.getCol());
    cp.startCharging();
    drone.chargeBattery(20.0f);
    cp.finishCharging();

    // Step 3: Fly to destination
    std::cout << "\n  Step 3: Fly to customer site\n";
    drone.fly(cs.getRow(), cs.getCol());

    // Step 4: Deliver package
    std::cout << "\n  Step 4: Deliver package\n";
    drone.unloadPackage();
    cs.receiveDelivery();

    // Step 5: Display final state
    std::cout << "\n  === MISSION COMPLETE ===\n";
    drone.display();
    cs.display();
    pkg.display();
    drone.displayFlightLog();

    // Export to JSON
    std::vector<Location*> locs = {&wh,   &cs,  &cp};
    std::vector<Drone*>    drns = {&drone};
    std::vector<Package*>  pkgs = {&pkg};
    exportEntitiesToJson(locs, drns, pkgs);

    std::cout << "\n  Full mission demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// INTERACTIVE MODE
// ════════════════════════════════════════════════════════════
void runInteractiveMode() {
    std::cout << "\n  --- Interactive Mode ---\n\n";

    std::vector<Location*> locations;
    std::vector<Drone*>    drones;
    std::vector<Package*>  packages;

    int subChoice = 0;
    do {
        std::cout << "  [1] Add Warehouse\n";
        std::cout << "  [2] Add Customer Site\n";
        std::cout << "  [3] Add Charging Pad\n";
        std::cout << "  [4] Add Package\n";
        std::cout << "  [5] Add Drone\n";
        std::cout << "  [6] Display all entities\n";
        std::cout << "  [7] Fly drone to location\n";
        std::cout << "  [8] Export to JSON\n";
        std::cout << "  [0] Back\n";
        std::cout << "  Choice: ";
        std::cin >> subChoice;
        clearInput();

        if (subChoice == 1) {
            int r, c, cap;
            std::string lbl, mgr;
            std::cout << "  Row: "; std::cin >> r;
            std::cout << "  Col: "; std::cin >> c;
            std::cout << "  Capacity: "; std::cin >> cap;
            clearInput();
            std::cout << "  Label: ";
            std::getline(std::cin, lbl);
            std::cout << "  Manager: ";
            std::getline(std::cin, mgr);
            locations.push_back(
                new Warehouse(r, c, lbl, cap, mgr));
        }
        else if (subChoice == 2) {
            int r, c;
            std::string lbl, name;
            std::cout << "  Row: "; std::cin >> r;
            std::cout << "  Col: "; std::cin >> c;
            clearInput();
            std::cout << "  Label: ";
            std::getline(std::cin, lbl);
            std::cout << "  Customer name: ";
            std::getline(std::cin, name);
            locations.push_back(
                new CustomerSite(r, c, lbl, name));
        }
        else if (subChoice == 3) {
            int r, c;
            std::string lbl;
            float rate;
            std::cout << "  Row: "; std::cin >> r;
            std::cout << "  Col: "; std::cin >> c;
            std::cout << "  Charge rate (%/min): ";
            std::cin >> rate;
            clearInput();
            std::cout << "  Label: ";
            std::getline(std::cin, lbl);
            locations.push_back(
                new ChargingPad(r, c, lbl, rate));
        }
        else if (subChoice == 4) {
            double wt;
            int sr, sc, dr, dc;
            std::string recip;
            std::cout << "  Weight (kg): "; std::cin >> wt;
            std::cout << "  From row: ";    std::cin >> sr;
            std::cout << "  From col: ";    std::cin >> sc;
            std::cout << "  To row: ";      std::cin >> dr;
            std::cout << "  To col: ";      std::cin >> dc;
            clearInput();
            std::cout << "  Recipient: ";
            std::getline(std::cin, recip);
            packages.push_back(
                new Package(wt,10,10,10,sr,sc,dr,dc,recip));
        }
        else if (subChoice == 5) {
            int type, r, c;
            std::string serial;
            std::cout << "  Type (1=Delivery 2=Scout 3=Heavy): ";
            std::cin >> type;
            std::cout << "  Start row: "; std::cin >> r;
            std::cout << "  Start col: "; std::cin >> c;
            clearInput();
            std::cout << "  Serial number: ";
            std::getline(std::cin, serial);
            if      (type == 1)
                drones.push_back(new DeliveryDrone(serial,r,c));
            else if (type == 2)
                drones.push_back(new ScoutDrone(serial,r,c));
            else
                drones.push_back(new HeavyDrone(serial,r,c));
        }
        else if (subChoice == 6) {
            std::cout << "\n  Locations:\n";
            for (auto* l : locations) l->display();
            std::cout << "\n  Drones:\n";
            for (auto* d : drones) d->display();
            std::cout << "\n  Packages:\n";
            for (auto* p : packages) p->display();
        }
        else if (subChoice == 7) {
            if (drones.empty()) {
                std::cout << "  No drones added yet.\n";
            } else {
                int tr, tc;
                std::cout << "  Target row: "; std::cin >> tr;
                std::cout << "  Target col: "; std::cin >> tc;
                drones[0]->fly(tr, tc);
                exportEntitiesToJson(locations, drones, packages);
            }
        }
        else if (subChoice == 8) {
            exportEntitiesToJson(locations, drones, packages);
        }

    } while(subChoice != 0);

    // Cleanup
    for (auto* l : locations) delete l;
    for (auto* d : drones)    delete d;
    for (auto* p : packages)  delete p;
}

// ════════════════════════════════════════════════════════════
// JSON EXPORT — sends all OOP entities to mission.json
// Website reads this and shows them in the entities panel
// ════════════════════════════════════════════════════════════
void exportEntitiesToJson(
    const std::vector<Location*>& locations,
    const std::vector<Drone*>&    drones,
    const std::vector<Package*>&  packages)
{
    std::vector<EntityData> entities;

    // Add location entities
    for (const auto* loc : locations) {
        std::string typeStr;
        switch(loc->getTypeCode()) {
            case CELL_WAREHOUSE: typeStr = "warehouse"; break;
            case CELL_CUSTOMER:  typeStr = "customer";  break;
            case CELL_CHARGING:  typeStr = "charging";  break;
            default:             typeStr = "location";  break;
        }
        entities.push_back({
            typeStr,
            loc->getLabel(),
            loc->getRow(),
            loc->getCol(),
            0.0
        });
    }

    // Add drone entities
    for (const auto* d : drones) {
        entities.push_back({
            "drone",
            d->getModelName(),
            d->getCurrentRow(),
            d->getCurrentCol(),
            0.0
        });
    }

    // Add package entities
    for (const auto* p : packages) {
        entities.push_back({
            "package",
            p->getTrackingCode(),
            p->getDstRow(),
            p->getDstCol(),
            p->getWeightKg()
        });
    }

    GraphicsData gd = {0, 0, 0, 1.0, 0};
    writeMissionJson(entities, gd);
    std::cout << "\n[PSOOP] Exported "
              << entities.size()
              << " entities to mission.json\n";
}