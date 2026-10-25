#include "Co4.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "GraphicsRenderer.h"
#include "../COA/AssemblyEngine.h"
#include "../COA/DroneRegisters.h"
#include "../PL/CityGrid.h"
#include "../PL/DeliveryScheduler.h"
#include "../PL/RouteTracker.h"
#include "../PSOOP/ChargingPad.h"
#include "../PSOOP/CustomerSite.h"
#include "../PSOOP/Drone.h"
#include "../PSOOP/Package.h"
#include "../PSOOP/Warehouse.h"
#include "../shared/Constants.h"
#include "../shared/JsonWriter.h"

using namespace std;

void verifyCgCo4Transform() {
    GraphicsRenderer renderer;
    const TransformMatrix transform =
        renderer.makeTranslation(35.0f, -15.0f) *
        renderer.makeRotation(25.0f) *
        renderer.makeScaling(1.25f, 1.25f);
    const Point2D sourcePoint(40.0f, 20.0f);
    const Point2D actual = transform.apply(sourcePoint);
    const float radians = 25.0f * 3.14159265f / 180.0f;
    const float scaledX = sourcePoint.x * 1.25f;
    const float scaledY = sourcePoint.y * 1.25f;
    const Point2D expected(
        35.0f + scaledX * cosf(radians) - scaledY * sinf(radians),
        -15.0f + scaledX * sinf(radians) + scaledY * cosf(radians));
    if (fabs(actual.x - expected.x) > 0.01f ||
        fabs(actual.y - expected.y) > 0.01f) {
        throw runtime_error("CG CO4 transform precision check failed.");
    }
    cout << "CG CO4 transform precision check passed for point ("
         << sourcePoint.x << ", " << sourcePoint.y << ") -> ("
         << actual.x << ", " << actual.y << ").\n";
}

void runCgCo4(int argc, char** argv) {
    cout << "\n--- CG integrated mission: PL + COA + OOP + CG ---\n"
         << "The final window animates the same route, drone and delivery "
            "configured by the other subjects.\n";

    const int activeRows = 5;
    const int activeCols = 5;
    CityGrid city;
    if (!city.insertNode(0, 0, CELL_WAREHOUSE, "WH-CO4") ||
        !city.insertNode(2, 2, CELL_CHARGING, "CP-CO4") ||
        !city.insertNode(1, 3, CELL_OBSTACLE, "OB-CO4") ||
        !city.insertNode(4, 4, CELL_CUSTOMER, "CS-CO4")) {
        throw runtime_error("Could not configure the integrated city grid.");
    }

    Warehouse warehouse(0, 0, "WH-CO4", 100, "AeroRoute", true);
    CustomerSite customer(
        4, 4, "CS-CO4", "AeroRoute Recipient", "Integrated City", true);
    ChargingPad chargingPad(2, 2, "CP-CO4");
    Package package(2.5, 10.0, 8.0, 6.0,
                    0, 0, 4, 4, "AeroRoute Recipient");
    DeliveryDrone drone("AR-D1-INTEGRATED", 0, 0);

    if (!warehouse.addStock(1) ||
        !drone.loadPackage(&package) ||
        !drone.hasPackage()) {
        throw runtime_error("Could not prepare the integrated OOP delivery.");
    }
    customer.addPendingDelivery();

    const vector<pair<int, int>> flightPath = {
        {0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2},
        {2, 3}, {3, 3}, {3, 4}, {4, 4}
    };
    RouteTracker route;
    route.setRouteType(ROUTE_SLL);
    for (size_t index = 0; index < flightPath.size(); ++index) {
        const int row = flightPath[index].first;
        const int col = flightPath[index].second;
        route.addWaypoint(row, col, city.getLabel(row, col).empty()
            ? "WP-" + to_string(index) : city.getLabel(row, col));
    }
    DeliveryScheduler scheduler;
    if (!scheduler.enqueueDelivery(
            drone.getSerialNumber(), customer.getLabel())) {
        throw runtime_error("Could not enqueue the integrated delivery.");
    }

    DroneRegisters registers;
    registers.loadTelemetry(120, 18.5, 45.0, drone.getBatteryPct());
    AssemblyEngine assembly(registers);
    assembly.clearLog();
    const DisplacementData displacement =
        assembly.computeDisplacement(
            0, 0, customer.getRow(), customer.getCol());
    const bool dispatchAllowed = assembly.SIM_CMP_JMP(
        registers.getRDX(), 2000, "JG", "RDX", "2000");
    if (!dispatchAllowed) {
        throw runtime_error("COA battery check blocked the integrated dispatch.");
    }
    cout << "  COA approves dispatch; computed route distance is "
         << displacement.total << " grid units.\n";
    assembly.displayInstructionLog();
    registers.exportToJson(assembly.getInstructionLog(), displacement);

    int cells[GRID_ROWS][GRID_COLS] = {};
    string labels[GRID_ROWS][GRID_COLS];
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            cells[row][col] = city.getCellType(row, col);
            labels[row][col] = city.getLabel(row, col);
        }
    }
    city.displayGrid();
    city.exportToJson(activeRows, activeCols);
    route.displayRoute();
    route.exportToJson();
    scheduler.displayPendingDeliveries();

    GraphicsRenderer renderer;
    renderer.loadGrid(cells, labels, activeRows, activeCols);
    renderer.setSimulationPaths(flightPath, {});
    renderer.followClipWindowToWaypoint(0);
    renderer.setTask3EffectsEnabled(true);
    verifyCgCo4Transform();
    renderer.applyTransform(0.0f, 0.0f, 0.0f, 1.0f);

    int waypointCurrent = 0;
    bool delivered = false;
    string simulationError;
    renderer.setSimulationCallback(
        [&](int row, int col, bool isTraffic, bool isDelivered) {
            if (isTraffic) return;

            drone.fly(row, col);
            route.advanceDrone();
            ++waypointCurrent;
            renderer.followClipWindowToWaypoint(
                static_cast<size_t>(waypointCurrent));
            if (isDelivered) {
                if (!drone.unloadPackage()) {
                    simulationError =
                        "The integrated drone could not unload its package.";
                    return;
                }
                customer.receiveDelivery();
                if (!scheduler.completeNextDelivery()) {
                    simulationError =
                        "The integrated scheduler had no delivery to complete.";
                    return;
                }
                if (!warehouse.removeStock(1)) {
                    simulationError =
                        "The integrated warehouse could not update its stock.";
                    return;
                }
                delivered = true;
            }
            writeTelemetryJson(
                drone.getCurrentRow(), drone.getCurrentCol(),
                registers.getSpeedMs(), registers.getAltitudeM(),
                drone.getBatteryPct(),
                static_cast<int>(drone.getHeadingDeg()),
                waypointCurrent, static_cast<int>(flightPath.size()),
                drone.getModelName(), route.getActiveType(),
                isDelivered ? "DELIVERED" : "ACTIVE");
        });

    vector<EntityData> entities = {
        {"warehouse", warehouse.getLabel(),
         warehouse.getRow(), warehouse.getCol(), 0.0},
        {"customer", customer.getLabel(),
         customer.getRow(), customer.getCol(), 0.0},
        {"charging", chargingPad.getLabel(),
         chargingPad.getRow(), chargingPad.getCol(), 0.0},
        {"drone", drone.getModelName(),
         drone.getCurrentRow(), drone.getCurrentCol(), 0.0},
        {"package", package.getTrackingCode(),
         customer.getRow(), customer.getCol(), package.getWeightKg()}
    };
    renderer.setMissionEntities(entities);
    const GraphicsData graphics = {
        renderer.getTranslateX(), renderer.getTranslateY(),
        renderer.getRotationAngle(), renderer.getScaleFactor(),
        renderer.getClippedCount()
    };
    renderer.exportToJson();
    writeMissionJson(entities, graphics);
    writeTelemetryJson(
        0, 0, registers.getSpeedMs(), registers.getAltitudeM(),
        drone.getBatteryPct(), static_cast<int>(drone.getHeadingDeg()),
        0, static_cast<int>(flightPath.size()),
        drone.getModelName(), route.getActiveType(), "READY");

    cout << "  CG now renders the PL grid and route, COA telemetry, "
            "and OOP drone/package objects together.\n"
         << "  Close the graphics window to finish the integrated mission.\n";
    renderer.launchWindow(argc, argv, "AeroRoute - All Subjects Integrated");
    renderer.displayClippingInfo();
    renderer.exportToJson();

    if (!simulationError.empty()) {
        throw runtime_error(simulationError);
    }
    if (!delivered || drone.hasPackage() ||
        customer.getCompletedDeliveries() != 1 ||
        scheduler.getPendingCount() != 0 ||
        scheduler.getCompletedCount() != 1) {
        throw runtime_error(
            "The CG mission closed before the integrated delivery completed.");
    }
    scheduler.displayCompletedDrones();
    cout << "Integrated CG mission completed: PL route/scheduler, COA "
            "registers, OOP delivery entities, and CG animation all ran.\n";
}
