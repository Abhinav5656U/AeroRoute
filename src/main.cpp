#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "CG/GraphicsRenderer.h"
#include "CG/AeroRouteMenu.h"
#include "COA/AssemblyEngine.h"
#include "COA/DroneRegisters.h"
#include "PL/CityGrid.h"
#include "PL/DeliveryScheduler.h"
#include "PL/RouteTracker.h"
#include "PSOOP/ChargingPad.h"
#include "PSOOP/CustomerSite.h"
#include "PSOOP/Drone.h"
#include "PSOOP/Package.h"
#include "PSOOP/Warehouse.h"
#include "shared/Constants.h"
#include "shared/JsonWriter.h"

using namespace std;

namespace {

vector<string> graphicalMissionInputs;
size_t graphicalInputIndex = 0;
bool graphicalInputMode = false;

bool readInputLine(const string& prompt, string& line) {
    if (graphicalInputMode) {
        if (graphicalInputIndex >= graphicalMissionInputs.size()) {
            throw runtime_error(
                "The graphical form did not provide all mission values.");
        }
        line = graphicalMissionInputs[graphicalInputIndex++];
        return true;
    }
    cout << prompt;
    return static_cast<bool>(getline(cin, line));
}

template <typename T>
T readNumber(const string& prompt, T minimum, T maximum) {
    static_assert(is_arithmetic<T>::value, "T must be numeric");

    for (;;) {
        string line;
        if (!readInputLine(prompt, line)) {
            throw runtime_error("Input ended before the mission was configured.");
        }
        istringstream input(line);
        T value{};
        string extra;
        if ((input >> value) && !(input >> extra)) {
            bool valid = value >= minimum && value <= maximum;
            if constexpr (is_floating_point<T>::value) {
                valid = valid && isfinite(value);
            }
            if (valid) return value;
            cout << "Enter a value from " << minimum
                 << " to " << maximum << ".\n";
        } else {
            cout << "Invalid number. Enter one numeric value per line.\n";
        }
    }
}

string readText(const string& prompt) {
    for (;;) {
        string value;
        if (!readInputLine(prompt, value)) {
            throw runtime_error("Input ended before the mission was configured.");
        }
        const size_t first = value.find_first_not_of(" \t\r\n");
        if (first != string::npos) {
            const size_t last = value.find_last_not_of(" \t\r\n");
            return value.substr(first, last - first + 1);
        }
        cout << "This field cannot be empty.\n";
    }
}

pair<int, int> readCoordinate(const string& label, int rows, int cols) {
    const int row = readNumber<int>(
        label + " row (0-" + to_string(rows - 1) + "): ", 0, rows - 1);
    const int col = readNumber<int>(
        label + " column (0-" + to_string(cols - 1) + "): ", 0, cols - 1);
    return {row, col};
}

vector<pair<int, int>> findGridPath(
    const CityGrid& city, int rows, int cols,
    int startRow, int startCol, int targetRow, int targetCol) {
    const int rowOffsets[] = {0, 1, 0, -1};
    const int colOffsets[] = {1, 0, -1, 0};
    bool visited[GRID_ROWS][GRID_COLS] = {};
    pair<int, int> previous[GRID_ROWS][GRID_COLS] = {};
    queue<pair<int, int>> pending;

    if (city.getCellType(startRow, startCol) == CELL_OBSTACLE ||
        city.getCellType(targetRow, targetCol) == CELL_OBSTACLE) {
        return {};
    }

    visited[startRow][startCol] = true;
    pending.push({startRow, startCol});
    while (!pending.empty()) {
        const pair<int, int> current = pending.front();
        pending.pop();
        if (current.first == targetRow && current.second == targetCol) {
            break;
        }

        for (int direction = 0; direction < 4; ++direction) {
            const int nextRow = current.first + rowOffsets[direction];
            const int nextCol = current.second + colOffsets[direction];
            if (nextRow < 0 || nextRow >= rows ||
                nextCol < 0 || nextCol >= cols ||
                visited[nextRow][nextCol] ||
                city.getCellType(nextRow, nextCol) == CELL_OBSTACLE) {
                continue;
            }

            visited[nextRow][nextCol] = true;
            previous[nextRow][nextCol] = current;
            pending.push({nextRow, nextCol});
        }
    }

    if (!visited[targetRow][targetCol]) return {};

    vector<pair<int, int>> path;
    for (pair<int, int> cell = {targetRow, targetCol};
         cell != pair<int, int>{startRow, startCol};
         cell = previous[cell.first][cell.second]) {
        path.push_back(cell);
    }
    path.push_back({startRow, startCol});
    reverse(path.begin(), path.end());
    return path;
}

void displayActiveGrid(const CityGrid& city, int rows, int cols) {
    cout << "\nConfigured city grid (" << rows << "x" << cols << ")\n"
         << "    ";
    for (int col = 0; col < cols; ++col) {
        cout << setw(3) << col;
    }
    cout << '\n';
    for (int row = 0; row < rows; ++row) {
        cout << setw(3) << row << ' ';
        for (int col = 0; col < cols; ++col) {
            const int type = city.getCellType(row, col);
            const char symbol = type == CELL_WAREHOUSE ? 'W'
                              : type == CELL_CUSTOMER ? 'C'
                              : type == CELL_CHARGING ? 'P'
                              : type == CELL_OBSTACLE ? '#'
                                                      : '.';
            cout << setw(3) << symbol;
        }
        cout << '\n';
    }
}

long long getRegisterValue(const DroneRegisters& registers,
                           const string& name) {
    if (name == "RAX") return registers.getRAX();
    if (name == "RBX") return registers.getRBX();
    if (name == "RCX") return registers.getRCX();
    return registers.getRDX();
}

void setRegisterValue(DroneRegisters& registers, const string& name,
                      long long value) {
    if (name == "RAX") registers.MOV_RAX(value);
    else if (name == "RBX") registers.MOV_RBX(value);
    else if (name == "RCX") registers.MOV_RCX(value);
    else registers.MOV_RDX(value);
}

void runMission(int argc, char** argv) {
    cout << "\n=== New delivery setup ===\n"
         << "The simulator supports an active city area up to "
         << GRID_ROWS << " rows by " << GRID_COLS << " columns.\n"
         << "\nData entry guide:\n"
         << "  1. City map          - grid size and city/delivery area name\n"
         << "  2. Map locations     - warehouse, customer sites, pads, obstacles\n"
         << "  3. Drone and route   - model, serial number, route structure\n"
         << "  4. Delivery          - destination site, package, recipient\n"
         << "  5. Flight computer   - path, telemetry, instruction and register\n"
         << "  6. Display and launch - movement transform, then simulate\n"
         << "\nEnter each value when prompted. Grid coordinates start at 0.\n\n";

    // PL CO1 - Task 1: CityGrid stores the city in fixed-size arrays.
    const int rows = readNumber<int>("City grid row count: ", 1, GRID_ROWS);
    const int cols = readNumber<int>("City grid column count: ", 1, GRID_COLS);
    if (rows * cols < 2) {
        throw runtime_error(
            "The city needs at least two cells for a warehouse and a delivery site.");
    }

    CityGrid city;
    vector<unique_ptr<Location>> locations;
    vector<CustomerSite*> customerSites;
    int initialDroneIds[1] = {0};

    cout << "\n--- City map and dispatch ---\n";
    const string warehouseLabel = readText("Warehouse label: ");
    const pair<int, int> warehousePosition =
        readCoordinate("Warehouse", rows, cols);
    if (!city.insertNode(warehousePosition.first, warehousePosition.second,
                         CELL_WAREHOUSE, warehouseLabel)) {
        throw runtime_error("Could not place the warehouse on the city grid.");
    }
    auto warehouse = make_unique<Warehouse>(
        warehousePosition.first, warehousePosition.second, warehouseLabel,
        200, "AeroRoute", true);
    locations.push_back(move(warehouse));

    cout << "\nA customer site is required for a delivery mission.\n";
    const string firstCustomerLabel = readText("Customer site label: ");
    pair<int, int> firstCustomerPosition;
    for (;;) {
        firstCustomerPosition = readCoordinate("Customer site", rows, cols);
        if (city.insertNode(firstCustomerPosition.first,
                            firstCustomerPosition.second,
                            CELL_CUSTOMER, firstCustomerLabel)) {
            break;
        }
        cout << "That cell is occupied. Choose another location.\n";
    }
    auto firstCustomer = make_unique<CustomerSite>(
        firstCustomerPosition.first, firstCustomerPosition.second,
        firstCustomerLabel, firstCustomerLabel, "User-entered city", false);
    customerSites.push_back(firstCustomer.get());
    locations.push_back(move(firstCustomer));

    const int maxAdditionalNodes = rows * cols - 2;
    const int additionalNodeCount = readNumber<int>(
        "\nAdditional city nodes (customer, charging pad, or obstacle; 0-" +
            to_string(maxAdditionalNodes) + "): ",
        0, maxAdditionalNodes);
    for (int index = 0; index < additionalNodeCount; ++index) {
        cout << "\nAdditional node " << index + 1 << " of "
             << additionalNodeCount << ":\n"
             << "  [1] Customer site\n"
             << "  [2] Charging pad\n"
             << "  [3] Obstacle\n";
        const int nodeType = readNumber<int>("Node type: ", 1, 3);
        const string label = readText("Node label: ");
        const pair<int, int> position = readCoordinate("Node", rows, cols);
        const int cellType = nodeType == 1 ? CELL_CUSTOMER
                           : nodeType == 2 ? CELL_CHARGING
                                           : CELL_OBSTACLE;
        if (!city.insertNode(position.first, position.second,
                             cellType, label)) {
            cout << "Cell occupied; re-enter this node.\n";
            --index;
            continue;
        }

        if (nodeType == 1) {
            auto site = make_unique<CustomerSite>(
                position.first, position.second, label, label,
                "User-entered city", false);
            customerSites.push_back(site.get());
            locations.push_back(move(site));
        } else if (nodeType == 2) {
            locations.push_back(make_unique<ChargingPad>(
                position.first, position.second, label));
        }
    }

    cout << "\nAvailable delivery sites:\n";
    for (size_t index = 0; index < customerSites.size(); ++index) {
        cout << "  [" << index + 1 << "] "
             << customerSites[index]->getLabel() << " at ("
             << customerSites[index]->getRow() << ","
             << customerSites[index]->getCol() << ")\n";
    }
    cout << "\nEnter the city, district, or delivery area for this delivery.\n";
    const string dispatchDestination = readText("City / delivery area name: ");

    cout << "\nRoute structure:\n"
         << "  [1] Singly linked list (SLL)\n"
         << "  [2] Doubly linked list (DLL)\n"
         << "  [3] Circular linked list (CLL)\n";
    const int routeChoice = readNumber<int>("Select route structure: ", 1, 3);
    const string routeType = routeChoice == 1 ? ROUTE_SLL
                           : routeChoice == 2 ? ROUTE_DLL
                                              : ROUTE_CLL;
    const string droneSerial = readText("Drone serial (e.g. AR-D1-004): ");

    cout << "\n--- Drone and delivery package ---\n";
    const int siteChoice = readNumber<int>(
        "Select delivery site by number: ", 1,
        static_cast<int>(customerSites.size()));
    CustomerSite* customer = customerSites[siteChoice - 1];

    vector<pair<int, int>> deliveryPath;
    for (;;) {
        cout << "\n--- Flight route (must join the warehouse and customer) ---\n";
        const pair<int, int> from = readCoordinate("Flight path From", rows, cols);
        const pair<int, int> to = readCoordinate("Flight path To", rows, cols);
        const pair<int, int> customerPosition = {
            customer->getRow(), customer->getCol()};
        if (from != warehousePosition || to != customerPosition) {
            cout << "For an integrated mission, From must match the warehouse ("
                 << warehousePosition.first << "," << warehousePosition.second
                 << ") and To must match the selected delivery site ("
                 << customerPosition.first << "," << customerPosition.second
                 << "). Please enter the flight path again.\n";
            continue;
        }
        deliveryPath = findGridPath(
            city, rows, cols, from.first, from.second, to.first, to.second);
        if (deliveryPath.empty()) {
            cout << "No route is available because obstacles block the path. "
                    "Reconfigure the city to continue.\n";
            throw runtime_error("No collision-free route reaches the delivery site.");
        }
        break;
    }

    cout << "\nDrone subclass:\n"
         << "  [1] DeliveryDrone\n"
         << "  [2] ScoutDrone (cannot carry a package)\n"
         << "  [3] HeavyDrone\n";
    unique_ptr<Drone> drone;
    for (;;) {
        const int droneChoice = readNumber<int>("Select drone subclass: ", 1, 3);
        if (droneChoice == 1) {
            drone = make_unique<DeliveryDrone>(
                droneSerial, warehousePosition.first, warehousePosition.second);
        } else if (droneChoice == 2) {
            drone = make_unique<ScoutDrone>(
                droneSerial, warehousePosition.first, warehousePosition.second);
        } else {
            drone = make_unique<HeavyDrone>(
                droneSerial, warehousePosition.first, warehousePosition.second);
        }

        if (drone->getMaxPayloadKg() <= 0.0) {
            cout << "ScoutDrone has no package payload capacity. "
                    "Choose DeliveryDrone or HeavyDrone for this mission.\n";
            drone.reset();
            continue;
        }
        break;
    }
    if (deliveryPath.size() - 1 >
        static_cast<size_t>(drone->getMaxRangeUnits())) {
        cout << "Warning: planned route exceeds this model's nominal range ("
             << drone->getMaxRangeUnits() << " cells). The simulator will "
                "continue the route using the project's per-waypoint flight model.\n";
    }

    const double packageWeight = readNumber<double>(
        "Package weight (kg; max " +
            to_string(drone->getMaxPayloadKg()) + "): ",
        0.01, drone->getMaxPayloadKg());
    const string recipientName = readText("Recipient name: ");
    Package package(packageWeight, 10.0, 10.0, 10.0,
                    warehousePosition.first, warehousePosition.second,
                    customer->getRow(), customer->getCol(), recipientName);
    customer->addPendingDelivery();
    if (!drone->loadPackage(&package)) {
        throw runtime_error("The package could not be loaded onto the selected drone.");
    }
    package.markInTransit();
    initialDroneIds[0] = drone->getId();
    drone->display();
    package.display();

    cout << "\n--- Flight telemetry ---\n";
    const int altitudeM = readNumber<int>("Altitude (m): ", 0, 10000);
    const double speedMs = readNumber<double>("Speed (m/s): ", 0.0, 300.0);
    const double headingDeg = readNumber<double>("Heading (degrees): ", 0.0, 359.999);
    const double batteryPct = readNumber<double>("Battery (%): ", 0.1, 100.0);
    DroneRegisters registers;
    registers.loadTelemetry(altitudeM, speedMs, headingDeg, batteryPct);
    drone->setAltitude(altitudeM);
    drone->setHeading(headingDeg);
    drone->drainBattery(100.0 - batteryPct);
    registers.displayRegisters();

    cout << "\n--- Flight computer instruction ---\n"
         << "  [1] MOV\n"
         << "  [2] ADD\n"
         << "  [3] SUB\n";
    const int instructionChoice = readNumber<int>("Instruction: ", 1, 3);
    const string instruction = instructionChoice == 1 ? "MOV"
                            : instructionChoice == 2 ? "ADD" : "SUB";
    cout << "Register choices: [1] RAX  [2] RBX  [3] RCX  [4] RDX\n";
    const int registerChoice = readNumber<int>("Register: ", 1, 4);
    const string registerName = registerChoice == 1 ? "RAX"
                              : registerChoice == 2 ? "RBX"
                              : registerChoice == 3 ? "RCX" : "RDX";
    const long long operand = readNumber<long long>(
        "Operand (integer, -1000000000 to 1000000000): ",
        -1000000000LL, 1000000000LL);

    AssemblyEngine assembly(registers);
    assembly.clearLog();
    const DisplacementData displacement = assembly.computeDisplacement(
        warehousePosition.first, warehousePosition.second,
        customer->getRow(), customer->getCol());
    long long registerValue = getRegisterValue(registers, registerName);
    if (instruction == "MOV") {
        assembly.SIM_MOV(registerValue, operand, registerName,
                         to_string(operand));
    } else if (instruction == "ADD") {
        registerValue = assembly.SIM_ADD(
            registerValue, operand, registerName, to_string(operand));
    } else {
        registerValue = assembly.SIM_SUB(
            registerValue, operand, registerName, to_string(operand));
    }
    setRegisterValue(registers, registerName, registerValue);
    assembly.displayDisplacement();
    registers.exportToJson(assembly.getInstructionLog(), displacement);

    cout << "\n--- Flight display settings ---\n";
    const float translateXCells = readNumber<float>(
        "Translate X (cells, -5 to 5): ", -5.0f, 5.0f);
    const float translateYCells = readNumber<float>(
        "Translate Y (cells, -5 to 5): ", -5.0f, 5.0f);
    const float rotationDegrees = readNumber<float>(
        "Rotate (degrees, -360 to 360): ", -360.0f, 360.0f);
    const float scale = readNumber<float>(
        "Scale multiplier (0.2 to 3.0): ", 0.2f, 3.0f);

    // PL CO1 - Task 1: Copy the fixed-size grid arrays for the renderer.
    int cells[GRID_ROWS][GRID_COLS] = {};
    string labels[GRID_ROWS][GRID_COLS];
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            cells[row][col] = city.getCellType(row, col);
            labels[row][col] = city.getLabel(row, col);
        }
    }
    displayActiveGrid(city, rows, cols);
    city.exportToJson(rows, cols);

    // PL CO2 - Task 2: Store every route waypoint in the selected linked list.
    RouteTracker route;
    route.setRouteType(routeType);
    for (size_t index = 0; index < deliveryPath.size(); ++index) {
        const int row = deliveryPath[index].first;
        const int col = deliveryPath[index].second;
        const string waypointLabel = labels[row][col].empty()
            ? "WP-" + to_string(index) : labels[row][col];
        route.addWaypoint(row, col, waypointLabel);
    }
    route.displayRoute();
    route.exportToJson();

    // PSOOP CO1 - Task 1: The grid, package, warehouse, and drone are
    // encapsulated objects; initialDroneIds is the PL Task 1 ID array.
    // PSOOP CO2 - Task 2: Calls through Drone* dispatch to the chosen subclass.
    // CG CO1 - Task 1 and CG CO3 - Task 2: render primitives and transforms.
    GraphicsRenderer renderer;
    renderer.loadGrid(cells, labels, rows, cols);
    renderer.loadRoute({}, routeType);
    renderer.setSimulationPaths(deliveryPath, {});
    renderer.setViewport(-250.0f, 250.0f, -250.0f, 250.0f);
    renderer.setTask3EffectsEnabled(true);
    renderer.applyTransform(
        translateXCells * 50.0f, translateYCells * 50.0f,
        rotationDegrees, scale);
    int waypointCurrent = 0;
    renderer.setSimulationCallback(
        [&](int row, int col, bool isTraffic, bool delivered) {
            if (isTraffic) return;

            drone->fly(row, col);
            route.advanceDrone();
            ++waypointCurrent;
            if (delivered) {
                drone->unloadPackage();
                customer->receiveDelivery();
            }
            const int reportedWaypoint = routeType == ROUTE_CLL
                ? waypointCurrent % static_cast<int>(deliveryPath.size())
                : min(waypointCurrent,
                      static_cast<int>(deliveryPath.size()) - 1);
            writeTelemetryJson(
                drone->getCurrentRow(), drone->getCurrentCol(),
                registers.getSpeedMs(), registers.getAltitudeM(),
                drone->getBatteryPct(),
                static_cast<int>(drone->getHeadingDeg()),
                reportedWaypoint,
                static_cast<int>(deliveryPath.size()),
                drone->getModelName(), route.getActiveType(),
                delivered ? "DELIVERED" : "ACTIVE");
        });
    renderer.exportToJson();

    vector<EntityData> entities;
    for (const auto& location : locations) {
        string type;
        switch (location->getTypeCode()) {
            case CELL_WAREHOUSE: type = "warehouse"; break;
            case CELL_CUSTOMER:  type = "customer"; break;
            case CELL_CHARGING:  type = "charging"; break;
            default:             type = "location"; break;
        }
        entities.push_back({type, location->getLabel(),
                            location->getRow(), location->getCol(), 0.0});
    }
    entities.push_back({"drone", drone->getModelName(),
                        drone->getCurrentRow(), drone->getCurrentCol(), 0.0});
    entities.push_back({"package", package.getTrackingCode(),
                        customer->getRow(), customer->getCol(),
                        package.getWeightKg()});
    const GraphicsData graphics = {
        renderer.getTranslateX(), renderer.getTranslateY(),
        renderer.getRotationAngle(), renderer.getScaleFactor(),
        renderer.getClippedCount()
    };
    writeMissionJson(entities, graphics);
    writeTelemetryJson(
        warehousePosition.first, warehousePosition.second,
        registers.getSpeedMs(), registers.getAltitudeM(),
        drone->getBatteryPct(), static_cast<int>(drone->getHeadingDeg()),
        0, static_cast<int>(deliveryPath.size()), drone->getModelName(),
        routeType, "READY");

    cout << "\n=== Mission configured ===\n"
         << "Dispatch destination: " << dispatchDestination << "\n"
         << "Delivery site: " << customer->getLabel() << "\n"
         << "Route: " << routeType << " (" << deliveryPath.size()
         << " waypoints)\n"
         << "Drone ID stored in PL Task 1 array: " << initialDroneIds[0] << "\n"
         << "Assembly operation: " << instruction << " "
         << registerName << ", " << operand << "\n"
         << "OpenGL window will start the mission. Close it to exit.\n";

    renderer.launchWindow(
        argc, argv, "AeroRoute — " + dispatchDestination + " Delivery Mission");
}

void runPlTask1() {
    cout << "\n--- PL Task 1: City grid arrays and CRUD ---\n";
    CityGrid city;
    city.insertNode(0, 0, CELL_WAREHOUSE, "WH-Main");
    city.insertNode(4, 4, CELL_CUSTOMER, "CS-Central");
    city.insertNode(2, 2, CELL_CHARGING, "CP-1");
    city.insertNode(1, 3, CELL_OBSTACLE, "OB-1");
    city.displayGrid();
    cout << "\nSearching for CS-Central:\n";
    int foundRow = -1;
    int foundCol = -1;
    if (city.searchByLabel("CS-Central", foundRow, foundCol)) {
        cout << "  Found at (" << foundRow << ", " << foundCol << ")\n";
    }
    cout << "\nUpdating CP-1 to CP-2, then removing OB-1:\n";
    city.updateNode(2, 2, CELL_CHARGING, "CP-2");
    city.deleteNode(1, 3);
    city.displayGrid();
    cout << "\nAdjacency matrix:\n";
    city.displayAdjacency();
    city.exportToJson(5, 5);
}

void runPlTask2() {
    cout << "\n--- PL Task 2: Linked-list route tracking ---\n";
    const pair<int, int> waypoints[] = {
        {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}
    };
    const string labels[] = {
        "WH-Main", "WP-1", "CP-1", "WP-2", "CS-Central"
    };
    const string routeTypes[] = {ROUTE_SLL, ROUTE_DLL, ROUTE_CLL};
    for (const string& type : routeTypes) {
        RouteTracker route;
        route.setRouteType(type);
        for (size_t index = 0; index < 5; ++index) {
            route.addWaypoint(waypoints[index].first,
                              waypoints[index].second, labels[index]);
        }
        cout << "\nRoute type: " << type << '\n';
        route.displayRoute();
        route.advanceDrone();
        route.displayRoute();
    }
}

void runPlTask3() {
    cout << "\n--- PL Task 3: FIFO queue and LIFO completed stack ---\n";
    DeliveryScheduler scheduler;
    if (scheduler.enqueueDelivery("", "CS-Invalid")) {
        throw runtime_error("PL scheduler accepted a delivery without a drone.");
    }
    scheduler.enqueueDelivery("AR-D1-101", "CS-North");
    scheduler.enqueueDelivery("AR-D1-102", "CS-East");
    scheduler.enqueueDelivery("AR-D1-103", "CS-West");
    scheduler.displayPendingDeliveries();
    scheduler.completeNextDelivery();
    scheduler.displayPendingDeliveries();
    scheduler.completeNextDelivery();
    scheduler.completeNextDelivery();
    scheduler.displayCompletedDrones();
    if (scheduler.completeNextDelivery() ||
        scheduler.getPendingCount() != 0 ||
        scheduler.getCompletedCount() != 3) {
        throw runtime_error("PL queue/stack boundary check failed.");
    }
    cout << "PL Task 3 checks passed, including empty-queue boundary handling.\n";
}

void runCoaTask1() {
    cout << "\n--- COA Task 1: Drone telemetry registers ---\n";
    DroneRegisters registers;
    registers.loadTelemetry(120, 18.5, 45.0, 87.0);
    registers.displayRegisters();
    const long long newAltitude =
        registers.ADD(registers.getRAX(), 15, "RAX");
    registers.MOV_RAX(newAltitude);
    cout << "\nAfter a 15 m climb:\n";
    registers.displayRegisterState("Task 1 altitude update");
    registers.CMP(registers.getRDX(), 1000LL, "RDX", "1000");
    registers.displayRegisters();
}

void runCoaTask2() {
    cout << "\n--- COA Task 2: Displacement and assembly operations ---\n";
    DroneRegisters registers;
    AssemblyEngine assembly(registers);
    assembly.clearLog();
    const DisplacementData displacement =
        assembly.computeDisplacement(0, 0, 4, 4);
    cout << "Warehouse (0,0) to customer (4,4):\n";
    assembly.displayDisplacement();
    const long long sum = assembly.asmAddCoordinates(4, 4);
    const long long delta = assembly.asmSubCoordinates(4, 0);
    cout << "Real assembly checks: 4 + 4 = " << sum
         << ", 4 - 0 = " << delta << '\n';
    assembly.displayInstructionLog();
    registers.exportToJson(assembly.getInstructionLog(), displacement);
}

void runCoaTask3() {
    cout << "\n--- COA Task 3: Arithmetic and string processing ---\n";
    DroneRegisters registers;
    AssemblyEngine assembly(registers);
    assembly.clearLog();
    const long long product = assembly.SIM_MUL_SUCCESSIVE(7, 8);
    string text = "AR-DRONE";
    const size_t originalLength = assembly.asmStringLength(text);
    assembly.asmStringAppend(text, "-DELIVERED");
    if (product != 56 || originalLength != 8 ||
        text != "AR-DRONE-DELIVERED") {
        throw runtime_error("COA Task 3 arithmetic/string check failed.");
    }
    cout << "  String result: " << text
         << " | original length: " << originalLength
         << " | product: " << product << "\n";
    assembly.displayInstructionLog();
    registers.exportToJson(assembly.getInstructionLog(),
                           assembly.getDisplacement());
}

void runPsoopTask1() {
    cout << "\n--- PSOOP Task 1: Encapsulated delivery objects ---\n";
    Warehouse warehouse(0, 0, "WH-Main", 200, "AeroRoute", true);
    CustomerSite customer(4, 4, "CS-Central",
                          "Recipient", "Central District", true);
    ChargingPad pad(2, 2, "CP-1");
    Package package(2.0, 10.0, 10.0, 10.0,
                    0, 0, 4, 4, "Recipient");
    warehouse.display();
    customer.display();
    pad.display();
    package.display();
}

void runPsoopTask2() {
    cout << "\n--- PSOOP Task 2: Inheritance and runtime polymorphism ---\n";
    vector<unique_ptr<Drone>> fleet;
    fleet.push_back(make_unique<DeliveryDrone>("AR-D1-101", 0, 0));
    fleet.push_back(make_unique<ScoutDrone>("AR-S1-101", 0, 0));
    fleet.push_back(make_unique<HeavyDrone>("AR-H1-101", 0, 0));
    for (const auto& drone : fleet) {
        drone->display();
        drone->fly(2, 2);
        drone->displayFlightLog();
    }
}

void runPsoopTask3() {
    cout << "\n--- PSOOP Task 3: Exceptions and payload conflicts ---\n";
    try {
        Package invalidPackage(0.0, 10.0, 10.0, 10.0,
                              0, 0, 4, 4, "Test Recipient");
        (void)invalidPackage;
        throw runtime_error("PSOOP accepted an invalid package weight.");
    } catch (const invalid_argument& error) {
        cout << "  Invalid package rejected: " << error.what() << "\n";
    }
    Package package(2.0, 10.0, 10.0, 10.0,
                    0, 0, 4, 4, "Test Recipient");
    Package conflictingPackage(1.0, 10.0, 10.0, 10.0,
                               0, 0, 3, 3, "Second Recipient");
    DeliveryDrone drone("AR-D1-T3", 0, 0);
    if (!drone.loadPackage(&package) ||
        drone.loadPackage(&conflictingPackage)) {
        throw runtime_error("PSOOP package conflict check failed.");
    }
    try {
        drone.loadPackage(nullptr);
        throw runtime_error("PSOOP accepted a null package.");
    } catch (const invalid_argument& error) {
        cout << "  Null package rejected: " << error.what() << "\n";
    }
    drone.unloadPackage();
    cout << "PSOOP Task 3 checks completed.\n";
}

void runCgTask(int task, int argc, char** argv) {
    const string taskName = task == 1 ? "Task 1: OpenGL primitives and city grid"
                          : task == 2 ? "Task 2: Transformations and clipping"
                                      : "Task 3: Procedural texture and lighting";
    cout << "\n--- CG " << taskName << " ---\n"
         << "Close the graphics window to return to the main menu.\n";
    GraphicsRenderer renderer;
    int cells[GRID_ROWS][GRID_COLS] = {};
    string labels[GRID_ROWS][GRID_COLS];
    const int warehouseRow = task == 2 ? 2 : 0;
    const int warehouseCol = task == 2 ? 2 : 0;
    cells[warehouseRow][warehouseCol] = CELL_WAREHOUSE;
    labels[warehouseRow][warehouseCol] = "WH-LAB";
    const int chargingRow = task == 2 ? 3 : 2;
    const int chargingCol = task == 2 ? 3 : 2;
    cells[chargingRow][chargingCol] = CELL_CHARGING;
    labels[chargingRow][chargingCol] = "CP-LAB";
    cells[4][4] = CELL_CUSTOMER;
    labels[4][4] = "CS-Central";
    renderer.loadGrid(cells, labels, 5, 5);
    const vector<pair<int, int>> task1FlightPath = {
        {0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2},
        {2, 3}, {3, 3}, {3, 4}, {4, 4}
    };
    const vector<pair<int, int>> task2FlightPath = {
        {2, 2}, {2, 3}, {3, 3}, {3, 4}, {4, 4}
    };
    const vector<pair<int, int>>& flightPath =
        task == 2 ? task2FlightPath : task1FlightPath;
    renderer.setSimulationPaths(flightPath, {});
    renderer.setViewport(-250.0f, 250.0f, -250.0f, 250.0f);
    if (task == 2) {
        renderer.makeTranslation(20.0f, -10.0f);
        renderer.makeRotation(15.0f);
        renderer.makeScaling(1.1f, 1.1f);
        renderer.applyTransform(20.0f, -10.0f, 15.0f, 1.1f);
        renderer.setViewport(-200.0f, 200.0f, -180.0f, 180.0f);
        renderer.displayTransformInfo();
    } else if (task == 3) {
        renderer.setTask3EffectsEnabled(true);
        cout << "Texture and landmark lighting are enabled; press T to toggle.\n";
    }
    renderer.exportToJson();
    renderer.launchWindow(argc, argv, "AeroRoute CG — " + taskName);
}

void runTask3Demos(int argc, char** argv) {
    cout << "\n=== Task 3 demonstrations from the syllabus ===\n";
    runPlTask3();
    runCoaTask3();
    runPsoopTask3();
    runCgTask(3, argc, argv);
}

void runFeatureMenu(int argc, char** argv) {
    for (;;) {
        cout << "\n--------------- System features ---------------\n"
             << "  [1] City map and grid locations\n"
             << "  [2] Flight routes and waypoints\n"
             << "  [3] Delivery queue and completion history\n"
             << "  [4] Drone telemetry and registers\n"
             << "  [5] Flight distance and assembly operations\n"
             << "  [6] Arithmetic and text processing\n"
             << "  [7] Warehouse, customer, charging pad and package\n"
             << "  [8] Drone models and flight behavior\n"
             << "  [9] Invalid data and package-loading conflicts\n"
             << " [10] City map display\n"
             << " [11] Drone movement, transforms and clipping\n"
             << " [12] City textures and landmark lighting\n"
             << " [13] All queue, string, validation and lighting examples\n"
             << "  [0] Back\n";
        const int choice = readNumber<int>("Select a feature: ", 0, 13);
        if (choice == 0) return;

        switch (choice) {
            case 1: runPlTask1(); break;
            case 2: runPlTask2(); break;
            case 3: runPlTask3(); break;
            case 4: runCoaTask1(); break;
            case 5: runCoaTask2(); break;
            case 6: runCoaTask3(); break;
            case 7: runPsoopTask1(); break;
            case 8: runPsoopTask2(); break;
            case 9: runPsoopTask3(); break;
            case 10: runCgTask(1, argc, argv); break;
            case 11: runCgTask(2, argc, argv); break;
            case 12: runCgTask(3, argc, argv); break;
            case 13: runTask3Demos(argc, argv); break;
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        AeroRouteMenu menu;
        graphicalMissionInputs = menu.run(argc, argv);
        if (graphicalMissionInputs.empty()) {
            cout << "Mission setup cancelled.\n";
            return 0;
        }
        graphicalInputMode = true;
        graphicalInputIndex = 0;
        runMission(argc, argv);
        if (graphicalInputIndex != graphicalMissionInputs.size()) {
            throw runtime_error(
                "The mission did not consume every value from the graphical form.");
        }
        graphicalInputMode = false;
    } catch (const exception& error) {
        cerr << "\nAeroRoute could not complete the mission: "
             << error.what() << '\n';
        return 1;
    }
    return 0;
}
