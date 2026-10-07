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
using namespace std;

// ── Forward declarations ──────────────────────────────────────
void showMenu();
void runTask1Demo(CityGrid& grid);
void runTask2Demo(RouteTracker& tracker);
void runTask3Demo();
void runInteractiveMode(CityGrid& grid, RouteTracker& tracker);
void clearInput();

// ── Main ──────────────────────────────────────────────────────
int main() {

    cout << "\n";
    cout << "  ================================================\n";
    cout << "   AeroRoute — Subject 1: Programming Lab\n";
    cout << "   Course Code: 2310214L\n";
    cout << "   CO1: City Grid Arrays\n";
    cout << "   CO2: Linked List Route Tracking\n";
    cout << "   MIT Academy of Engineering | Sem III 2026-27\n";
    cout << "  ================================================\n\n";

    CityGrid     grid;
    RouteTracker tracker;

    int choice = 0;
    do {
        showMenu();
        cin >> choice;
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
                cout << "\n  Both JSON files updated.\n";
                cout << "  Check website/data/ folder.\n\n";
                break;
            case 5:
                runTask3Demo();
                break;
            case 0:
                cout << "\n  Exiting Subject 1.\n\n";
                break;
            default:
                cout << "\n  Invalid choice.\n\n";
        }
    } while (choice != 0);

    return 0;
}

// ── Menu ──────────────────────────────────────────────────────
void showMenu() {
    cout << "  ------------------------------------------------\n";
    cout << "   SUBJECT 1 — MAIN MENU\n";
    cout << "  ------------------------------------------------\n";
    cout << "   [1] Task 1 Demo — City Grid (CO1)\n";
    cout << "       Arrays: insert delete update search display\n";
    cout << "   [2] Task 2 Demo — Route Tracker (CO2)\n";
    cout << "       SLL forward | DLL return | CLL patrol\n";
    cout << "   [3] Interactive Mode — User Input\n";
    cout << "       You choose nodes and routes manually\n";
    cout << "   [4] Export to JSON (update website)\n";
    cout << "   [5] Task 3 Demo — Delivery Queue and Completed Stack\n";
    cout << "   [0] Exit\n";
    cout << "  ------------------------------------------------\n";
    cout << "   Choice: ";
}

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ════════════════════════════════════════════════════════════
// TASK 1 DEMO — City Grid Array Operations
// [PL CO1 - Task 1: Demonstrates all CRUD operations]
// ════════════════════════════════════════════════════════════
void runTask1Demo(CityGrid& grid) {

    cout << "\n  --- TASK 1: 2D Array City Grid Demo ---\n\n";

    // INSERT operations
    cout << "  [INSERT] Adding nodes to the grid:\n";
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
    cout << "\n  [DISPLAY] Grid after insertions:\n";
    grid.displayGrid();

    // SEARCH operation
    cout << "  [SEARCH] Looking for CS-Alpha:\n";
    int foundR, foundC;
    grid.searchByLabel("CS-Alpha", foundR, foundC);

    // UPDATE operation
    cout << "\n  [UPDATE] Changing CS-Beta to a Charging Pad:\n";
    grid.updateNode(5, 5, CELL_CHARGING, "CP-3");
    grid.displayGrid();

    // DELETE operation
    cout << "  [DELETE] Removing OB-3 at (6,2):\n";
    grid.deleteNode(6, 2);
    grid.displayGrid();

    // ADJACENCY MATRIX
    cout << "  [ADJACENCY] Flight corridors:\n";
    grid.displayAdjacency();

    // Export to JSON so website updates
    grid.exportToJson();

    cout << "  Task 1 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// TASK 2 DEMO — Linked List Route Tracking
// [PL CO2 - Task 2: Demonstrates SLL, DLL, CLL]
// ════════════════════════════════════════════════════════════
void runTask2Demo(RouteTracker& tracker) {

    cout << "\n  --- TASK 2: Linked List Route Tracking Demo ---\n\n";

    // ── SLL Demo ─────────────────────────────────────────────
    cout << "  === Singly Linked List (Forward Delivery) ===\n";
    tracker.setRouteType(ROUTE_SLL);
    tracker.addWaypoint(0, 0, "WH-Main");
    tracker.addWaypoint(0, 3, "WP-1");
    tracker.addWaypoint(2, 5, "WP-2");
    tracker.addWaypoint(4, 7, "WP-3");
    tracker.addWaypoint(9, 9, "CS-Alpha");
    tracker.displayRoute();

    cout << "  Advancing drone (SLL traversal):\n";
    for (int i = 0; i < 3; i++) {
        tracker.advanceDrone();
        tracker.displayRoute();
    }
    tracker.exportToJson();

    // ── DLL Demo ─────────────────────────────────────────────
    cout << "  === Doubly Linked List (Return Path) ===\n";
    RouteTracker dll_tracker;
    dll_tracker.setRouteType(ROUTE_DLL);
    dll_tracker.addWaypoint(9, 9, "CS-Alpha");
    dll_tracker.addWaypoint(6, 6, "WP-R1");
    dll_tracker.addWaypoint(3, 3, "WP-R2");
    dll_tracker.addWaypoint(0, 0, "WH-Main");
    dll_tracker.displayRoute();

    // ── CLL Demo ─────────────────────────────────────────────
    cout << "  === Circular Linked List (Patrol Zone) ===\n";
    RouteTracker cll_tracker;
    cll_tracker.setRouteType(ROUTE_CLL);
    cll_tracker.addWaypoint(2, 2, "Zone-A");
    cll_tracker.addWaypoint(2, 7, "Zone-B");
    cll_tracker.addWaypoint(7, 7, "Zone-C");
    cll_tracker.addWaypoint(7, 2, "Zone-D");
    cll_tracker.displayRoute();

    cout << "  Cycling patrol (CLL wraps around):\n";
    for (int i = 0; i < 6; i++) {
        cll_tracker.advanceDrone();
        cll_tracker.displayRoute();
    }
    cll_tracker.exportToJson();

    cout << "  Task 2 demo complete.\n\n";
}

// [PL CO3 - Task 3: queue-based FIFO dispatch and completed-drone LIFO stack]
void runTask3Demo() {
    cout << "\n  --- TASK 3: Delivery Queue and Completed Stack ---\n\n";

    DeliveryScheduler scheduler;
    scheduler.enqueueDelivery("AR-D1-001", "CS-Alpha");
    scheduler.enqueueDelivery("AR-D1-002", "CS-Beta");
    scheduler.enqueueDelivery("AR-D1-003", "CS-Gamma");
    scheduler.displayPendingDeliveries();

    cout << "\n  Completing the next queued delivery:\n";
    scheduler.completeNextDelivery();
    scheduler.displayPendingDeliveries();
    scheduler.displayCompletedDrones();

    cout << "\n  Adding a new delivery while earlier deliveries remain queued:\n";
    scheduler.enqueueDelivery("AR-D1-004", "CS-Delta");
    scheduler.displayPendingDeliveries();

    cout << "\n  Completing the next queued delivery:\n";
    scheduler.completeNextDelivery();
    scheduler.displayPendingDeliveries();
    scheduler.displayCompletedDrones();
    cout << "\n  Pending: " << scheduler.getPendingCount()
              << " | Completed: " << scheduler.getCompletedCount() << "\n";
    cout << "\n  Empty-queue boundary check:\n";
    while (scheduler.completeNextDelivery()) {}
    if (scheduler.completeNextDelivery()) {
        cout << "  ERROR: empty queue reported a completed delivery.\n";
    } else {
        cout << "  Empty queue safely reports no work to complete.\n";
    }
    scheduler.displayCompletedDrones();
    cout << "  Task 3 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// INTERACTIVE MODE — reads from website user_input.json
// Also accepts direct terminal input
// ════════════════════════════════════════════════════════════
void runInteractiveMode(CityGrid& grid, RouteTracker& tracker) {

    cout << "\n  --- Interactive Mode ---\n";
    cout << "  Add your own nodes and build a route.\n\n";

    int subChoice = 0;
    do {
        cout << "  [1] Add grid node\n";
        cout << "  [2] Remove grid node\n";
        cout << "  [3] Search grid node\n";
        cout << "  [4] Set route type (SLL/DLL/CLL)\n";
        cout << "  [5] Add route waypoint\n";
        cout << "  [6] Advance drone one step\n";
        cout << "  [7] Display grid and route\n";
        cout << "  [8] Export to JSON (update website)\n";
        cout << "  [0] Back to main menu\n";
        cout << "  Choice: ";
        cin >> subChoice;
        clearInput();

        if (subChoice == 1) {
            int r, c, t;
            string lbl;
            cout << "  Row (0-9): "; cin >> r;
            cout << "  Col (0-9): "; cin >> c;
            cout << "  Type (1=WH 2=CS 3=CP 4=OB): ";
            cin >> t;
            clearInput();
            cout << "  Label: ";
            getline(cin, lbl);
            grid.insertNode(r, c, t, lbl);
            grid.exportToJson();
        }
        else if (subChoice == 2) {
            int r, c;
            cout << "  Row: "; cin >> r;
            cout << "  Col: "; cin >> c;
            grid.deleteNode(r, c);
            grid.exportToJson();
        }
        else if (subChoice == 3) {
            string lbl;
            clearInput();
            cout << "  Label to search: ";
            getline(cin, lbl);
            int fr, fc;
            grid.searchByLabel(lbl, fr, fc);
        }
        else if (subChoice == 4) {
            string rt;
            cout << "  Route type (SLL/DLL/CLL): ";
            cin >> rt;
            tracker.setRouteType(rt);
        }
        else if (subChoice == 5) {
            int r, c;
            string lbl;
            cout << "  Row: "; cin >> r;
            cout << "  Col: "; cin >> c;
            clearInput();
            cout << "  Label: ";
            getline(cin, lbl);
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
            cout << "  JSON files updated. Website will reflect changes.\n";
        }

    } while (subChoice != 0);
}