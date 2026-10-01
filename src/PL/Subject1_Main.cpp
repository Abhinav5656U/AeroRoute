/*
 * ============================================================
 * FILE        : Subject1_Main.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : Programming Lab | Course Code: 2310214L
 * TASK        : Task 1 (CO1) + Task 2 (CO2) — Full Demo
 * DESCRIPTION : Standalone entry point for Subject 1.
 *               Demonstrates all array CRUD operations and
 *               all three linked list route types.
 *               Writes real data to website JSON files.
 * HOW TO RUN  :
 *   cd src/PL
 *   g++ Subject1_Main.cpp CityGrid.cpp RouteTracker.cpp DeliveryScheduler.cpp -o ../../build/subject1.exe -std=c++17
 *   ../../build/subject1.exe
 * ============================================================
 */

#include <iostream>
#include <string>
#include <limits>
#include "CityGrid.h"
#include "RouteTracker.h"
#include "DeliveryScheduler.h"
#include "../shared/Constants.h"
#include "../shared/JsonWriter.h"

// ── Forward declarations ──────────────────────────────────────
void showMenu();
void runTask1Demo(CityGrid& grid);
void runTask2Demo(RouteTracker& tracker);
void runTask3Demo();
void runInteractiveMode(CityGrid& grid, RouteTracker& tracker);
void clearInput();

// ── Main ──────────────────────────────────────────────────────
int main() {

    std::cout << "\n";
    std::cout << "  ================================================\n";
    std::cout << "   AeroRoute — Subject 1: Programming Lab\n";
    std::cout << "   Course Code: 2310214L\n";
    std::cout << "   CO1: City Grid Arrays\n";
    std::cout << "   CO2: Linked List Route Tracking\n";
    std::cout << "   MIT Academy of Engineering | Sem III 2026-27\n";
    std::cout << "  ================================================\n\n";

    CityGrid     grid;
    RouteTracker tracker;

    int choice = 0;
    do {
        showMenu();
        std::cin >> choice;
        clearInput();

        switch (choice) {
            case 1:
                runTask1Demo(grid);
                break;
            case 2:
                runTask2Demo(tracker);
                break;
            case 3:
                runInteractiveMode(grid, tracker);
                break;
            case 4:
                // Export current state to JSON for website
                grid.exportToJson();
                tracker.exportToJson();
                std::cout << "\n  Both JSON files updated.\n";
                std::cout << "  Check website/data/ folder.\n\n";
                break;
            case 5:
                runTask3Demo();
                break;
            case 0:
                std::cout << "\n  Exiting Subject 1.\n\n";
                break;
            default:
                std::cout << "\n  Invalid choice.\n\n";
        }
    } while (choice != 0);

    return 0;
}

// ── Menu ──────────────────────────────────────────────────────
void showMenu() {
    std::cout << "  ------------------------------------------------\n";
    std::cout << "   SUBJECT 1 — MAIN MENU\n";
    std::cout << "  ------------------------------------------------\n";
    std::cout << "   [1] Task 1 Demo — City Grid (CO1)\n";
    std::cout << "       Arrays: insert delete update search display\n";
    std::cout << "   [2] Task 2 Demo — Route Tracker (CO2)\n";
    std::cout << "       SLL forward | DLL return | CLL patrol\n";
    std::cout << "   [3] Interactive Mode — User Input\n";
    std::cout << "       You choose nodes and routes manually\n";
    std::cout << "   [4] Export to JSON (update website)\n";
    std::cout << "   [5] Task 3 Demo — Delivery Queue and Completed Stack\n";
    std::cout << "   [0] Exit\n";
    std::cout << "  ------------------------------------------------\n";
    std::cout << "   Choice: ";
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// ════════════════════════════════════════════════════════════
// TASK 1 DEMO — City Grid Array Operations
// [PL CO1 - Task 1: Demonstrates all CRUD operations]
// ════════════════════════════════════════════════════════════
void runTask1Demo(CityGrid& grid) {

    std::cout << "\n  --- TASK 1: 2D Array City Grid Demo ---\n\n";

    // INSERT operations
    std::cout << "  [INSERT] Adding nodes to the grid:\n";
    grid.insertNode(0, 0, CELL_WAREHOUSE, "WH-Main");
    grid.insertNode(0, 9, CELL_WAREHOUSE, "WH-East");
    grid.insertNode(9, 9, CELL_CUSTOMER,  "CS-Alpha");
    grid.insertNode(5, 5, CELL_CUSTOMER,  "CS-Beta");
    grid.insertNode(3, 3, CELL_CHARGING,  "CP-1");
    grid.insertNode(7, 7, CELL_CHARGING,  "CP-2");
    grid.insertNode(2, 4, CELL_OBSTACLE,  "OB-1");
    grid.insertNode(4, 6, CELL_OBSTACLE,  "OB-2");
    grid.insertNode(6, 2, CELL_OBSTACLE,  "OB-3");

    // DISPLAY after inserts
    std::cout << "\n  [DISPLAY] Grid after insertions:\n";
    grid.displayGrid();

    // SEARCH operation
    std::cout << "  [SEARCH] Looking for CS-Alpha:\n";
    int foundR, foundC;
    grid.searchByLabel("CS-Alpha", foundR, foundC);

    // UPDATE operation
    std::cout << "\n  [UPDATE] Changing CS-Beta to a Charging Pad:\n";
    grid.updateNode(5, 5, CELL_CHARGING, "CP-3");
    grid.displayGrid();

    // DELETE operation
    std::cout << "  [DELETE] Removing OB-3 at (6,2):\n";
    grid.deleteNode(6, 2);
    grid.displayGrid();

    // ADJACENCY MATRIX
    std::cout << "  [ADJACENCY] Flight corridors:\n";
    grid.displayAdjacency();

    // Export to JSON so website updates
    grid.exportToJson();

    std::cout << "  Task 1 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// TASK 2 DEMO — Linked List Route Tracking
// [PL CO2 - Task 2: Demonstrates SLL, DLL, CLL]
// ════════════════════════════════════════════════════════════
void runTask2Demo(RouteTracker& tracker) {

    std::cout << "\n  --- TASK 2: Linked List Route Tracking Demo ---\n\n";

    // ── SLL Demo ─────────────────────────────────────────────
    std::cout << "  === Singly Linked List (Forward Delivery) ===\n";
    tracker.setRouteType(ROUTE_SLL);
    tracker.addWaypoint(0, 0, "WH-Main");
    tracker.addWaypoint(0, 3, "WP-1");
    tracker.addWaypoint(2, 5, "WP-2");
    tracker.addWaypoint(4, 7, "WP-3");
    tracker.addWaypoint(9, 9, "CS-Alpha");
    tracker.displayRoute();

    std::cout << "  Advancing drone (SLL traversal):\n";
    for (int i = 0; i < 3; i++) {
        tracker.advanceDrone();
        tracker.displayRoute();
    }
    tracker.exportToJson();

    // ── DLL Demo ─────────────────────────────────────────────
    std::cout << "  === Doubly Linked List (Return Path) ===\n";
    RouteTracker dll_tracker;
    dll_tracker.setRouteType(ROUTE_DLL);
    dll_tracker.addWaypoint(9, 9, "CS-Alpha");
    dll_tracker.addWaypoint(6, 6, "WP-R1");
    dll_tracker.addWaypoint(3, 3, "WP-R2");
    dll_tracker.addWaypoint(0, 0, "WH-Main");
    dll_tracker.displayRoute();

    // ── CLL Demo ─────────────────────────────────────────────
    std::cout << "  === Circular Linked List (Patrol Zone) ===\n";
    RouteTracker cll_tracker;
    cll_tracker.setRouteType(ROUTE_CLL);
    cll_tracker.addWaypoint(2, 2, "Zone-A");
    cll_tracker.addWaypoint(2, 7, "Zone-B");
    cll_tracker.addWaypoint(7, 7, "Zone-C");
    cll_tracker.addWaypoint(7, 2, "Zone-D");
    cll_tracker.displayRoute();

    std::cout << "  Cycling patrol (CLL wraps around):\n";
    for (int i = 0; i < 6; i++) {
        cll_tracker.advanceDrone();
        cll_tracker.displayRoute();
    }
    cll_tracker.exportToJson();

    std::cout << "  Task 2 demo complete.\n\n";
}

void runTask3Demo() {
    std::cout << "\n  --- TASK 3: Delivery Queue and Completed Stack ---\n\n";

    DeliveryScheduler scheduler;
    scheduler.enqueueDelivery("AR-D1-001", "CS-Alpha");
    scheduler.enqueueDelivery("AR-D1-002", "CS-Beta");
    scheduler.enqueueDelivery("AR-D1-003", "CS-Gamma");
    scheduler.displayPendingDeliveries();

    std::cout << "\n  Completing the next queued delivery:\n";
    scheduler.completeNextDelivery();
    scheduler.displayPendingDeliveries();
    scheduler.displayCompletedDrones();

    std::cout << "\n  Adding a new delivery while earlier deliveries remain queued:\n";
    scheduler.enqueueDelivery("AR-D1-004", "CS-Delta");
    scheduler.displayPendingDeliveries();

    std::cout << "\n  Completing the next queued delivery:\n";
    scheduler.completeNextDelivery();
    scheduler.displayPendingDeliveries();
    scheduler.displayCompletedDrones();
    std::cout << "\n  Pending: " << scheduler.getPendingCount()
              << " | Completed: " << scheduler.getCompletedCount() << "\n";
    std::cout << "  Task 3 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// INTERACTIVE MODE — reads from website user_input.json
// Also accepts direct terminal input
// ════════════════════════════════════════════════════════════
void runInteractiveMode(CityGrid& grid, RouteTracker& tracker) {

    std::cout << "\n  --- Interactive Mode ---\n";
    std::cout << "  Add your own nodes and build a route.\n\n";

    int subChoice = 0;
    do {
        std::cout << "  [1] Add grid node\n";
        std::cout << "  [2] Remove grid node\n";
        std::cout << "  [3] Search grid node\n";
        std::cout << "  [4] Set route type (SLL/DLL/CLL)\n";
        std::cout << "  [5] Add route waypoint\n";
        std::cout << "  [6] Advance drone one step\n";
        std::cout << "  [7] Display grid and route\n";
        std::cout << "  [8] Export to JSON (update website)\n";
        std::cout << "  [0] Back to main menu\n";
        std::cout << "  Choice: ";
        std::cin >> subChoice;
        clearInput();

        if (subChoice == 1) {
            int r, c, t;
            std::string lbl;
            std::cout << "  Row (0-9): "; std::cin >> r;
            std::cout << "  Col (0-9): "; std::cin >> c;
            std::cout << "  Type (1=WH 2=CS 3=CP 4=OB): ";
            std::cin >> t;
            clearInput();
            std::cout << "  Label: ";
            std::getline(std::cin, lbl);
            grid.insertNode(r, c, t, lbl);
            grid.exportToJson();
        }
        else if (subChoice == 2) {
            int r, c;
            std::cout << "  Row: "; std::cin >> r;
            std::cout << "  Col: "; std::cin >> c;
            grid.deleteNode(r, c);
            grid.exportToJson();
        }
        else if (subChoice == 3) {
            std::string lbl;
            clearInput();
            std::cout << "  Label to search: ";
            std::getline(std::cin, lbl);
            int fr, fc;
            grid.searchByLabel(lbl, fr, fc);
        }
        else if (subChoice == 4) {
            std::string rt;
            std::cout << "  Route type (SLL/DLL/CLL): ";
            std::cin >> rt;
            tracker.setRouteType(rt);
        }
        else if (subChoice == 5) {
            int r, c;
            std::string lbl;
            std::cout << "  Row: "; std::cin >> r;
            std::cout << "  Col: "; std::cin >> c;
            clearInput();
            std::cout << "  Label: ";
            std::getline(std::cin, lbl);
            tracker.addWaypoint(r, c, lbl);
            tracker.exportToJson();
        }
        else if (subChoice == 6) {
            tracker.advanceDrone();
            tracker.displayRoute();
            tracker.exportToJson();
        }
        else if (subChoice == 7) {
            grid.displayGrid();
            tracker.displayRoute();
        }
        else if (subChoice == 8) {
            grid.exportToJson();
            tracker.exportToJson();
            std::cout << "  JSON files updated. Website will reflect changes.\n";
        }

    } while (subChoice != 0);
}