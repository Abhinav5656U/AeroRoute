#include "Co4.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "ChargingPad.h"
#include "CustomerSite.h"
#include "Drone.h"
#include "Package.h"
#include "Warehouse.h"

using namespace std;

void runPsoopCo4() {
    cout << "\n--- PSOOP CO4: Decision-based integrated delivery ---\n";
    struct DeliveryCase {
        double packageWeight;
        int routeCells;
        double startingBattery;
        const char* recipient;
    };
    const DeliveryCase cases[] = {
        {3.5, 6, 80.0, "Central Recipient"},
        {25.0, 4, 15.0, "Heavy Recipient"},
        {2.0, 9, 60.0, "Out-of-range Recipient"}
    };
    Warehouse warehouse(0, 0, "WH-Main", 250, "AeroRoute", true);
    ChargingPad pad(0, 0, "CP-Dispatch");
    int deliveredCount = 0;
    int rejectedCount = 0;
    const size_t caseCount = sizeof(cases) / sizeof(cases[0]);

    for (size_t index = 0; index < caseCount; ++index) {
        const DeliveryCase& delivery = cases[index];
        cout << "  Case " << index + 1 << ": "
             << delivery.packageWeight << " kg, "
             << delivery.routeCells << " route cells, "
             << delivery.startingBattery << "% battery -> ";

        enum class DroneChoice { Delivery, Heavy, None };
        DroneChoice choice = DroneChoice::None;
        if (delivery.packageWeight <= 15.0 && delivery.routeCells <= 8) {
            choice = DroneChoice::Delivery;
        } else if (delivery.packageWeight <= 50.0 &&
                   delivery.routeCells <= 4) {
            choice = DroneChoice::Heavy;
        }

        if (choice == DroneChoice::None) {
            ++rejectedCount;
            cout << "no safe drone matches payload and range; dispatch rejected.\n";
            continue;
        }

        unique_ptr<Drone> drone;
        if (choice == DroneChoice::Delivery) {
            drone = make_unique<DeliveryDrone>(
                "AR-D1-CO4-" + to_string(index + 1), 0, 0);
        } else {
            drone = make_unique<HeavyDrone>(
                "AR-H1-CO4-" + to_string(index + 1), 0, 0);
        }
        drone->drainBattery(100.0 - delivery.startingBattery);
        if (drone->needsCharging()) {
            if (!pad.startCharging()) {
                throw runtime_error("PSOOP CO4 could not reserve the charging pad.");
            }
            drone->chargeBattery(30.0f);
            pad.finishCharging();
        }

        CustomerSite customer(
            0, delivery.routeCells, "CS-CO4-" + to_string(index + 1),
            delivery.recipient, "AeroRoute delivery zone", true);
        Package package(
            delivery.packageWeight, 10.0, 10.0, 10.0, 0, 0,
            customer.getRow(), customer.getCol(), delivery.recipient);
        if (!warehouse.addStock(1) || !drone->loadPackage(&package)) {
            throw runtime_error("PSOOP CO4 could not prepare an eligible delivery.");
        }
        customer.addPendingDelivery();
        drone->fly(customer.getRow(), customer.getCol());
        if (!drone->unloadPackage()) {
            throw runtime_error("PSOOP CO4 could not complete the delivery.");
        }
        customer.receiveDelivery();
        warehouse.removeStock(1);
        ++deliveredCount;
        cout << drone->getModelName() << " selected; delivery completed";
        if (delivery.startingBattery < 20.0) cout << " after charging";
        cout << ".\n";
    }

    if (deliveredCount != 2 || rejectedCount != 1) {
        throw runtime_error("PSOOP CO4 decision tests returned unexpected results.");
    }
    cout << "PSOOP CO4 checks passed for 3 inputs: two safe dispatches and "
            "one payload/range rejection.\n";
}
