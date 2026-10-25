#include "AeroRouteMenu.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <queue>
#include <random>
#include <sstream>
#include <utility>

#include <windows.h>
#include <GL/gl.h>
#include <GL/freeglut.h>

class AeroRouteMenuState;

namespace {

struct MenuField {
    std::string id;
    std::string label;
    std::string value;
    std::string hint;
};

struct MenuPage {
    std::string title;
    std::string description;
    std::vector<MenuField> fields;
};

AeroRouteMenuState* activeMenu = nullptr;
const int windowWidth = 1040;
const int windowHeight = 760;
const size_t missionFormVisibleFields = 5;

struct PresetLocation {
    int type;
    std::string label;
    int row;
    int col;
};

struct PresetCity {
    std::string name;
    std::string warehouseLabel;
    std::vector<PresetLocation> customers;
    std::vector<PresetLocation> facilities;
};

PresetCity makePresetCity(int cityChoice) {
    const std::string cityNames[] = {"Delhi", "Mumbai", "Pune", "Patna"};
    const std::string siteNames[][4] = {
        {"CS-Connaught", "CS-Rohini", "CS-Saket", "CS-Dwarka"},
        {"CS-Colaba", "CS-Andheri", "CS-Powai", "CS-Bandra"},
        {"CS-Shivajinagar", "CS-Hadapsar", "CS-Kothrud", "CS-Wakad"},
        {"CS-GandhiMaidan", "CS-Kankarbagh", "CS-BoringRoad", "CS-Danapur"}
    };

    const int rows[] = {0, 9, 9, 5};
    const int cols[] = {9, 9, 0, 0};
    PresetCity city;
    const std::string& cityName = cityNames[cityChoice - 1];
    city.name = cityName;
    city.warehouseLabel = "WH-" + cityName;
    for (int index = 0; index < 4; ++index) {
        city.customers.push_back(
            {1, siteNames[cityChoice - 1][index], rows[index], cols[index]});
    }

    std::vector<std::pair<int, int>> candidates;
    for (int row = 1; row < 9; ++row) {
        for (int col = 1; col < 9; ++col) {
            candidates.push_back({row, col});
        }
    }
    std::mt19937 generator(0xA3E000u + static_cast<unsigned>(cityChoice));
    std::shuffle(candidates.begin(), candidates.end(), generator);
    for (int index = 0; index < 5; ++index) {
        const auto cell = candidates[static_cast<size_t>(index)];
        city.facilities.push_back(
            {2, "CP-" + cityName + "-" + std::to_string(index + 1),
             cell.first, cell.second});
    }
    for (int index = 0; index < 18; ++index) {
        const auto cell = candidates[static_cast<size_t>(index + 5)];
        city.facilities.push_back(
            {3, "BLD-" + cityName + "-" + std::to_string(index + 1),
             cell.first, cell.second});
    }
    return city;
}

void drawText(float x, float y, const std::string& text,
              void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (unsigned char ch : text) {
        glutBitmapCharacter(font, ch);
    }
}

void drawRect(float x, float y, float width, float height,
              float red, float green, float blue) {
    glColor3f(red, green, blue);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

bool parseNumber(const std::string& text, double& value) {
    if (text.empty()) return false;
    char* end = nullptr;
    value = std::strtod(text.c_str(), &end);
    return end != text.c_str() && *end == '\0' && std::isfinite(value);
}

bool parseInteger(const std::string& text, int& value) {
    double parsed = 0.0;
    if (!parseNumber(text, parsed) || std::floor(parsed) != parsed ||
        parsed < -2147483648.0 || parsed > 2147483647.0) {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

} // namespace

class AeroRouteMenuState {
public:
    std::vector<MenuPage> pages;
    size_t pageIndex = 0;
    size_t activeField = 0;
    bool replaceFieldOnType = true;
    bool suffixBuilt = false;
    bool cancelled = false;
    bool choosingCity = true;
    bool choosingDestination = false;
    bool selectedRoundTrip = true;
    int selectedCity = 0;
    int selectedTargetRow = 0;
    int selectedTargetCol = 0;
    size_t scrollOffset = 0;
    std::string message;
    std::vector<std::string> result;

    AeroRouteMenuState() {
        pages.push_back({"City map", "Choose the working city grid size.",
            {{"rows", "Grid rows", "5", "1-10"},
             {"cols", "Grid columns", "5", "1-10"}}});
        pages.push_back({"Warehouse", "Set the drone starting location.",
            {{"warehouse_label", "Warehouse name", "WH-Mumbai", "Example: WH-Main"},
             {"warehouse_row", "Warehouse row", "0", "Grid coordinates start at 0"},
             {"warehouse_col", "Warehouse column", "0", "Grid coordinates start at 0"}}});
        pages.push_back({"First delivery site", "Add the required customer destination.",
            {{"customer_label", "Customer site name", "CS-Powai", "Example: CS-Central"},
             {"customer_row", "Customer row", "4", "Must be inside the grid"},
             {"customer_col", "Customer column", "4", "Must be inside the grid"}}});
        pages.push_back({"Additional map locations",
                         "Add optional customer sites, charging pads, or obstacles.",
            {{"extra_count", "Number of additional locations", "0",
              "Each location uses its own screen"}}});
    }

    MenuField* findField(const std::string& id) {
        for (MenuPage& page : pages) {
            for (MenuField& field : page.fields) {
                if (field.id == id) return &field;
            }
        }
        return nullptr;
    }

    const MenuField* findField(const std::string& id) const {
        for (const MenuPage& page : pages) {
            for (const MenuField& field : page.fields) {
                if (field.id == id) return &field;
            }
        }
        return nullptr;
    }

    bool integerValue(const std::string& id, int& value) const {
        const MenuField* field = findField(id);
        return field != nullptr && parseInteger(field->value, value);
    }

    bool coordinateWithinGrid(const std::string& rowId,
                              const std::string& colId) const {
        int rows = 0, cols = 0, row = 0, col = 0;
        return integerValue("rows", rows) && integerValue("cols", cols) &&
               integerValue(rowId, row) && integerValue(colId, col) &&
               row >= 0 && row < rows && col >= 0 && col < cols;
    }

    std::vector<std::pair<int, int>> configuredCoordinates() const {
        std::vector<std::pair<int, int>> coordinates;
        int row = 0, col = 0;
        if (integerValue("warehouse_row", row) &&
            integerValue("warehouse_col", col)) {
            coordinates.push_back({row, col});
        }
        if (integerValue("customer_row", row) &&
            integerValue("customer_col", col)) {
            coordinates.push_back({row, col});
        }
        for (const MenuPage& page : pages) {
            for (const MenuField& field : page.fields) {
                if (field.id.find("node_") != 0 ||
                    field.id.find("_row") == std::string::npos) {
                    continue;
                }
                const std::string nodeId =
                    field.id.substr(0, field.id.size() - 4);
                int nodeRow = 0, nodeCol = 0;
                if (integerValue(field.id, nodeRow) &&
                    integerValue(nodeId + "_col", nodeCol)) {
                    coordinates.push_back({nodeRow, nodeCol});
                }
            }
        }
        return coordinates;
    }

    bool validateCurrentPage() {
        const MenuPage& page = pages[pageIndex];
        for (const MenuField& field : page.fields) {
            if (field.value.find_first_not_of(" \t\r\n") == std::string::npos) {
                message = field.label + " is required.";
                return false;
            }
        }

        int value = 0;
        double number = 0.0;
        const auto requireIntegerRange = [&](const std::string& id,
                                             int minimum, int maximum,
                                             const std::string& label) {
            if (!integerValue(id, value) || value < minimum ||
                value > maximum) {
                message = label + " must be between " +
                          std::to_string(minimum) + " and " +
                          std::to_string(maximum) + ".";
                return false;
            }
            return true;
        };
        const auto requireNumberRange = [&](const std::string& id,
                                            double minimum, double maximum,
                                            const std::string& label) {
            const MenuField* field = findField(id);
            if (field == nullptr || !parseNumber(field->value, number) ||
                number < minimum || number > maximum) {
                std::ostringstream range;
                range << label << " must be between " << minimum
                      << " and " << maximum << ".";
                message = range.str();
                return false;
            }
            return true;
        };

        if (pageIndex == 0) {
            if (!requireIntegerRange("rows", 1, 10, "Grid rows") ||
                !requireIntegerRange("cols", 1, 10, "Grid columns")) {
                return false;
            }
            int rows = 0, cols = 0;
            integerValue("rows", rows);
            integerValue("cols", cols);
            if (rows * cols < 2) {
                message = "The map needs room for a warehouse and customer.";
                return false;
            }
        } else if (pageIndex == 1) {
            if (!coordinateWithinGrid("warehouse_row", "warehouse_col")) {
                message = "Warehouse coordinates must be inside the grid.";
                return false;
            }
        } else if (pageIndex == 2) {
            if (!coordinateWithinGrid("customer_row", "customer_col")) {
                message = "Customer coordinates must be inside the grid.";
                return false;
            }
            int wr = 0, wc = 0, cr = 0, cc = 0;
            integerValue("warehouse_row", wr);
            integerValue("warehouse_col", wc);
            integerValue("customer_row", cr);
            integerValue("customer_col", cc);
            if (wr == cr && wc == cc) {
                message = "Warehouse and customer need different grid cells.";
                return false;
            }
        } else if (pageIndex == 3) {
            int rows = 0, cols = 0;
            integerValue("rows", rows);
            integerValue("cols", cols);
            if (!requireIntegerRange("extra_count", 0, rows * cols - 2,
                                     "Additional locations")) {
                return false;
            }
        } else if (page.title.find("Map location ") == 0) {
            if (!requireIntegerRange(page.fields[0].id, 1, 3, "Location type")) {
                return false;
            }
            const std::string base = page.fields[0].id.substr(
                0, page.fields[0].id.size() - 5);
            const std::string rowId = base + "_row";
            const std::string colId = base + "_col";
            if (!coordinateWithinGrid(rowId, colId)) {
                message = "Location coordinates must be inside the grid.";
                return false;
            }
            int row = 0, col = 0;
            integerValue(rowId, row);
            integerValue(colId, col);
            const std::vector<std::pair<int, int>> coordinates =
                configuredCoordinates();
            if (std::count(coordinates.begin(), coordinates.end(),
                           std::make_pair(row, col)) > 1) {
                message = "That grid cell is already occupied.";
                return false;
            }
        } else if (page.title == "City and route") {
            if (!requireIntegerRange("route_type", 1, 3, "Route type")) {
                return false;
            }
        } else if (page.title == "Drone and delivery site") {
            if (!requireIntegerRange("site_choice", 1,
                                     static_cast<int>(customerCount()),
                                     "Delivery site number")) {
                return false;
            }
        } else if (page.title == "Flight path") {
            int expectedRow = 0, expectedCol = 0;
            if (!selectedCustomer(expectedRow, expectedCol)) {
                message = "Select a valid customer site on the previous screen.";
                return false;
            }
            int fromRow = 0, fromCol = 0, toRow = 0, toCol = 0;
            if (!integerValue("from_row", fromRow) ||
                !integerValue("from_col", fromCol) ||
                !integerValue("to_row", toRow) ||
                !integerValue("to_col", toCol)) {
                message = "Flight path coordinates must be whole numbers.";
                return false;
            }
            int warehouseRow = 0, warehouseCol = 0;
            integerValue("warehouse_row", warehouseRow);
            integerValue("warehouse_col", warehouseCol);
            if (fromRow != warehouseRow || fromCol != warehouseCol ||
                toRow != expectedRow || toCol != expectedCol) {
                message = "From must match the warehouse and To the selected customer.";
                return false;
            }
        } else if (page.title == "Drone and package") {
            if (!requireIntegerRange("drone_type", 1, 3, "Drone model")) {
                return false;
            }
            double packageWeight = 0.0;
            const MenuField* weight = findField("package_weight");
            int droneType = 0;
            integerValue("drone_type", droneType);
            const double maximum = droneType == 1 ? 15.0
                                 : droneType == 2 ? 0.5 : 50.0;
            if (weight == nullptr || !parseNumber(weight->value, packageWeight) ||
                packageWeight < 0.01 || packageWeight > maximum) {
                std::ostringstream range;
                range << "Package weight must be 0.01-" << maximum
                      << " kg for this drone.";
                message = range.str();
                return false;
            }
        } else if (page.title == "Flight telemetry") {
            if (!requireIntegerRange("altitude", 0, 10000, "Altitude") ||
                !requireNumberRange("speed", 0.0, 300.0, "Speed") ||
                !requireNumberRange("heading", 0.0, 359.999, "Heading") ||
                !requireNumberRange("battery", 0.1, 100.0, "Battery")) {
                return false;
            }
        } else if (page.title == "Flight computer") {
            if (!requireIntegerRange("instruction", 1, 3, "Instruction") ||
                !requireIntegerRange("register", 1, 4, "Register") ||
                !requireIntegerRange("operand", -1000000000, 1000000000,
                                     "Operand")) {
                return false;
            }
        } else if (page.title == "Display and launch") {
            if (!requireNumberRange("translate_x", -5.0, 5.0, "Translate X") ||
                !requireNumberRange("translate_y", -5.0, 5.0, "Translate Y") ||
                !requireNumberRange("rotation", -360.0, 360.0, "Rotation") ||
                !requireNumberRange("scale", 0.2, 3.0, "Scale")) {
                return false;
            }
        } else if (page.title == "Flight and delivery settings") {
            int rows = 0, cols = 0;
            integerValue("rows", rows);
            integerValue("cols", cols);
            for (const MenuPage& configuredPage : pages) {
                for (const MenuField& field : configuredPage.fields) {
                    if (field.id.size() <= 5 ||
                        field.id.compare(field.id.size() - 5, 5, "_type") != 0 ||
                        field.id.find("node_") != 0) {
                        continue;
                    }
                    if (!requireIntegerRange(field.id, 1, 3, "Location type")) {
                        return false;
                    }
                    const std::string base =
                        field.id.substr(0, field.id.size() - 5);
                    const std::string rowId = base + "_row";
                    const std::string colId = base + "_col";
                    int row = 0, col = 0;
                    if (!integerValue(rowId, row) ||
                        !integerValue(colId, col) ||
                        row < 0 || row >= rows || col < 0 || col >= cols) {
                        message = field.label +
                                  " must use coordinates inside the grid.";
                        return false;
                    }
                    const std::vector<std::pair<int, int>> coordinates =
                        configuredCoordinates();
                    if (std::count(coordinates.begin(), coordinates.end(),
                                   std::make_pair(row, col)) > 1) {
                        message = "Map locations cannot share a grid cell.";
                        return false;
                    }
                }
            }
            if (!requireIntegerRange("route_type", 1, 3, "Route type") ||
                !requireIntegerRange("round_trip", 0, 1,
                                     "Return to warehouse") ||
                !requireIntegerRange("site_choice", 1,
                    static_cast<int>(customerCount()), "Delivery site") ||
                !requireIntegerRange("drone_type", 1, 3, "Drone model")) {
                return false;
            }
            double packageWeight = 0.0;
            int droneType = 0;
            integerValue("drone_type", droneType);
            const double maximum = droneType == 1 ? 15.0
                                 : droneType == 2 ? 0.5 : 50.0;
            const MenuField* weight = findField("package_weight");
            if (weight == nullptr || !parseNumber(weight->value, packageWeight) ||
                packageWeight < 0.01 || packageWeight > maximum) {
                message = "Package weight is over the selected drone's capacity.";
                return false;
            }
            int expectedRow = 0, expectedCol = 0;
            int warehouseRow = 0, warehouseCol = 0;
            int fromRow = 0, fromCol = 0, toRow = 0, toCol = 0;
            selectedCustomer(expectedRow, expectedCol);
            integerValue("warehouse_row", warehouseRow);
            integerValue("warehouse_col", warehouseCol);
            if (!integerValue("from_row", fromRow) ||
                !integerValue("from_col", fromCol) ||
                !integerValue("to_row", toRow) ||
                !integerValue("to_col", toCol) ||
                fromRow != warehouseRow || fromCol != warehouseCol ||
                toRow != expectedRow || toCol != expectedCol) {
                message = "Flight path endpoints must match the warehouse and selected customer.";
                return false;
            }
            if (!requireIntegerRange("altitude", 0, 10000, "Altitude") ||
                !requireNumberRange("speed", 0.0, 300.0, "Speed") ||
                !requireNumberRange("heading", 0.0, 359.999, "Heading") ||
                !requireNumberRange("battery", 0.1, 100.0, "Battery") ||
                !requireIntegerRange("instruction", 1, 3, "Instruction") ||
                !requireIntegerRange("register", 1, 4, "Register") ||
                !requireIntegerRange("operand", -1000000000, 1000000000,
                                     "Operand") ||
                !requireNumberRange("translate_x", -5.0, 5.0, "Translate X") ||
                !requireNumberRange("translate_y", -5.0, 5.0, "Translate Y") ||
                !requireNumberRange("rotation", -360.0, 360.0, "Rotation") ||
                !requireNumberRange("scale", 0.2, 3.0, "Scale")) {
                return false;
            }
        }
        message.clear();
        return true;
    }

    size_t customerCount() const {
        size_t count = 1;
        for (const MenuPage& page : pages) {
            for (const MenuField& field : page.fields) {
                if (field.id.size() <= 5 ||
                    field.id.compare(field.id.size() - 5, 5, "_type") != 0 ||
                    field.id.find("node_") != 0) {
                    continue;
                }
                int type = 0;
                if (parseInteger(field.value, type) && type == 1) ++count;
            }
        }
        return count;
    }

    bool selectedCustomer(int& row, int& col) const {
        int choice = 0;
        if (!integerValue("site_choice", choice) || choice < 1) return false;
        if (choice == 1) {
            return integerValue("customer_row", row) &&
                   integerValue("customer_col", col);
        }
        int found = 1;
        for (const MenuPage& page : pages) {
            for (const MenuField& field : page.fields) {
                if (field.id.size() <= 5 ||
                    field.id.compare(field.id.size() - 5, 5, "_type") != 0 ||
                    field.id.find("node_") != 0) {
                    continue;
                }
                int type = 0;
                if (!parseInteger(field.value, type) || type != 1) continue;
                ++found;
                if (found == choice) {
                    const std::string base =
                        field.id.substr(0, field.id.size() - 5);
                    return integerValue(base + "_row", row) &&
                           integerValue(base + "_col", col);
                }
            }
        }
        return false;
    }

    void buildRemainingPages() {
        std::map<std::string, std::string> previousValues;
        if (suffixBuilt) {
            for (size_t index = 4; index < pages.size(); ++index) {
                for (const MenuField& field : pages[index].fields) {
                    previousValues[field.id] = field.value;
                }
            }
            pages.erase(pages.begin() + 4, pages.end());
        }
        suffixBuilt = true;
        scrollOffset = 0;
        const auto savedOr = [&](const std::string& id,
                                 const std::string& fallback) {
            const auto saved = previousValues.find(id);
            return saved == previousValues.end() ? fallback : saved->second;
        };

        int count = 0;
        integerValue("extra_count", count);
        const std::vector<std::pair<int, int>> occupied =
            configuredCoordinates();
        std::vector<std::pair<int, int>> assigned = occupied;
        int rows = 5, cols = 5;
        integerValue("rows", rows);
        integerValue("cols", cols);
        MenuPage details{
            "Flight and delivery settings",
            "Scroll to review/edit the remaining map, delivery, flight and display fields.",
            {}
        };
        for (int index = 0; index < count; ++index) {
            const std::string base = "node_" + std::to_string(index + 1);
            std::pair<int, int> position{0, 0};
            bool foundFreeCell = false;
            for (int row = 0; row < rows && !foundFreeCell; ++row) {
                for (int col = 0; col < cols; ++col) {
                    if (std::find(assigned.begin(), assigned.end(),
                                  std::make_pair(row, col)) ==
                        assigned.end()) {
                        position = {row, col};
                        foundFreeCell = true;
                        break;
                    }
                }
            }
            if (!foundFreeCell) {
                message = "There are no unoccupied grid cells for the added locations.";
                return;
            }
            assigned.push_back(position);
            details.fields.push_back({base + "_type", "Location type",
                savedOr(base + "_type", "2"),
                "1 customer | 2 charging pad | 3 building/obstacle"});
            details.fields.push_back({base + "_label", "Location name",
                savedOr(base + "_label", "CP-" + std::to_string(index + 1)),
                "Use a short, unique label"});
            details.fields.push_back({base + "_row", "Grid row",
                savedOr(base + "_row", std::to_string(position.first)),
                "Coordinate from 0 to grid rows - 1"});
            details.fields.push_back({base + "_col", "Grid column",
                savedOr(base + "_col", std::to_string(position.second)),
                "Coordinate from 0 to grid columns - 1"});
        }
        details.fields.insert(details.fields.end(), {
            {"destination", "City / delivery area",
             savedOr("destination", "Mumbai"), "City or delivery zone"},
            {"route_type", "Route structure",
             savedOr("route_type", "1"), "1 singly | 2 doubly | 3 circular"},
            {"round_trip", "Return to warehouse after delivery",
             savedOr("round_trip", "1"), "1 round trip | 0 one-way"},
            {"drone_serial", "Drone serial number",
             savedOr("drone_serial", "AR-D1-004"), "Example: AR-D1-004"},
            {"site_choice", "Delivery site number",
             savedOr("site_choice", "1"), "1=first customer; added customers follow"},
            {"from_row", "Path start row",
             savedOr("from_row", valueOf("warehouse_row")), "Must match warehouse"},
            {"from_col", "Path start column",
             savedOr("from_col", valueOf("warehouse_col")), "Must match warehouse"},
            {"to_row", "Path destination row",
             savedOr("to_row", valueOf("customer_row")), "Must match selected customer"},
            {"to_col", "Path destination column",
             savedOr("to_col", valueOf("customer_col")), "Must match selected customer"},
            {"drone_type", "Drone model",
             savedOr("drone_type", "1"), "1 Delivery | 2 Scout | 3 Heavy"},
            {"package_weight", "Package weight (kg)",
             savedOr("package_weight", "1.0"), "Delivery <=15 | Scout <=0.5 | Heavy <=50"},
            {"recipient", "Recipient name",
             savedOr("recipient", "Asha Sharma"), "Person receiving the parcel"},
            {"altitude", "Altitude (m)",
             savedOr("altitude", "100"), "0-10000"},
            {"speed", "Speed (m/s)",
             savedOr("speed", "15"), "0-300"},
            {"heading", "Heading (degrees)",
             savedOr("heading", "45"), "0-359.999"},
            {"battery", "Battery (%)",
             savedOr("battery", "90"), "0.1-100"},
            {"instruction", "Flight instruction",
             savedOr("instruction", "2"), "1 MOV | 2 ADD | 3 SUB"},
            {"register", "Register",
             savedOr("register", "1"), "1 RAX | 2 RBX | 3 RCX | 4 RDX"},
            {"operand", "Instruction operand",
             savedOr("operand", "5"), "-1000000000 to 1000000000"},
            {"translate_x", "Display translate X (cells)",
             savedOr("translate_x", "0"), "-5 to 5"},
            {"translate_y", "Display translate Y (cells)",
             savedOr("translate_y", "0"), "-5 to 5"},
            {"rotation", "Display rotation (degrees)",
             savedOr("rotation", "0"), "-360 to 360"},
            {"scale", "Display scale",
             savedOr("scale", "1"), "0.2 to 3"}
        });
        pages.push_back(std::move(details));
    }

    void chooseCity(int choice) {
        if (choice < 1 || choice > 5) return;
        choosingCity = false;
        if (choice == 5) {
            pageIndex = 0;
            message.clear();
            glutPostRedisplay();
            return;
        }
        selectedCity = choice;
        choosingDestination = true;
        const PresetCity city = makePresetCity(selectedCity);
        selectedTargetRow = city.customers.front().row;
        selectedTargetCol = city.customers.front().col;
        glutPostRedisplay();
    }

    int presetCellType(const PresetCity& city, int row, int col) const {
        if (row == 0 && col == 0) return 1;
        for (const PresetLocation& customer : city.customers) {
            if (customer.row == row && customer.col == col) return 1;
        }
        for (const PresetLocation& facility : city.facilities) {
            if (facility.row == row && facility.col == col) {
                return facility.type == 3 ? 3 : 2;
            }
        }
        return 0;
    }

    bool presetPathExists(const PresetCity& city, int targetRow,
                          int targetCol) const {
        if (targetRow < 0 || targetRow >= 10 ||
            targetCol < 0 || targetCol >= 10 ||
            (targetRow == 0 && targetCol == 0) ||
            presetCellType(city, targetRow, targetCol) == 3) {
            return false;
        }
        bool visited[10][10] = {};
        std::queue<std::pair<int, int>> pending;
        pending.push({0, 0});
        visited[0][0] = true;
        const int rowOffsets[] = {0, 1, 0, -1};
        const int colOffsets[] = {1, 0, -1, 0};
        while (!pending.empty()) {
            const auto current = pending.front();
            pending.pop();
            if (current.first == targetRow && current.second == targetCol) {
                return true;
            }
            for (int direction = 0; direction < 4; ++direction) {
                const int row = current.first + rowOffsets[direction];
                const int col = current.second + colOffsets[direction];
                if (row < 0 || row >= 10 || col < 0 || col >= 10 ||
                    visited[row][col] || presetCellType(city, row, col) == 3) {
                    continue;
                }
                visited[row][col] = true;
                pending.push({row, col});
            }
        }
        return false;
    }

    bool choosePresetTarget(int row, int col) {
        const PresetCity city = makePresetCity(selectedCity);
        const int type = presetCellType(city, row, col);
        if (row == 0 && col == 0) {
            message = "The warehouse cell cannot be the delivery destination.";
            return false;
        }
        if (type == 3) {
            message = "That cell is a building/obstacle. Choose a clear cell.";
            return false;
        }
        if (type == 2) {
            message = "Choose an empty cell or a customer cell, not a charging pad.";
            return false;
        }
        selectedTargetRow = row;
        selectedTargetCol = col;
        if (!presetPathExists(city, row, col)) {
            message = "Delivery is not possible: no route avoids the buildings.";
            glutPostRedisplay();
            return true;
        }
        message.clear();
        glutPostRedisplay();
        return true;
    }

    void buildPresetResult() {
        const PresetCity city = makePresetCity(selectedCity);
        const PresetLocation& warehouse{1, city.warehouseLabel, 0, 0};
        std::vector<PresetLocation> extraLocations;
        for (size_t index = 1; index < city.customers.size(); ++index) {
            if (city.customers[index].row == selectedTargetRow &&
                city.customers[index].col == selectedTargetCol) {
                continue;
            }
            extraLocations.push_back(city.customers[index]);
        }
        extraLocations.insert(extraLocations.end(),
                              city.facilities.begin(), city.facilities.end());

        result = {
            "10", "10",
            warehouse.label, std::to_string(warehouse.row),
            std::to_string(warehouse.col),
            "CS-" + city.name + "-Delivery",
            std::to_string(selectedTargetRow),
            std::to_string(selectedTargetCol),
            std::to_string(extraLocations.size())
        };
        for (const PresetLocation& location : extraLocations) {
            result.push_back(std::to_string(location.type));
            result.push_back(location.label);
            result.push_back(std::to_string(location.row));
            result.push_back(std::to_string(location.col));
        }
        const std::string serial = "AR-" + city.name + "-01";
        result.insert(result.end(), {
            city.name, "1", selectedRoundTrip ? "1" : "0", serial,
            "1", "0", "0", std::to_string(selectedTargetRow),
            std::to_string(selectedTargetCol),
            "1", "1.0", "AeroRoute Recipient",
            "100", "15", "45", "90",
            "2", "1", "5",
            "0", "0", "0", "1"
        });
    }

    void scrollFields(int direction) {
        if (pages[pageIndex].title != "Flight and delivery settings") return;
        const size_t lastOffset = pages[pageIndex].fields.size() >
                missionFormVisibleFields
            ? pages[pageIndex].fields.size() - missionFormVisibleFields : 0;
        if (direction > 0) {
            scrollOffset = std::min(lastOffset, scrollOffset + 1);
        } else if (scrollOffset > 0) {
            --scrollOffset;
        }
        glutPostRedisplay();
    }

    void drawCityChooser() const {
        drawRect(0, 0, windowWidth, 110, 0.035f, 0.08f, 0.15f);
        glColor3f(0.18f, 0.82f, 0.95f);
        drawText(56, 48, "AEROROUTE  /  SELECT A CITY");
        glColor3f(0.76f, 0.84f, 0.92f);
        drawText(56, 80,
                 "Choose a ready-made city or build a custom delivery map.",
                 GLUT_BITMAP_9_BY_15);
        const std::string names[] = {
            "Delhi", "Mumbai", "Pune", "Patna", "Custom city"
        };
        const std::string details[] = {
            "10 x 10 map | 4 delivery sites | chargers and buildings",
            "10 x 10 map | 4 delivery sites | chargers and buildings",
            "10 x 10 map | 4 delivery sites | chargers and buildings",
            "10 x 10 map | 4 delivery sites | chargers and buildings",
            "Set grid, locations, route, drone, package and telemetry"
        };
        const float positions[][2] = {
            {70.0f, 160.0f}, {545.0f, 160.0f},
            {70.0f, 305.0f}, {545.0f, 305.0f},
            {70.0f, 450.0f}
        };
        const float widths[] = {425.0f, 425.0f, 425.0f, 425.0f, 900.0f};
        const float colors[][3] = {
            {0.13f, 0.30f, 0.47f}, {0.10f, 0.35f, 0.42f},
            {0.24f, 0.30f, 0.43f}, {0.28f, 0.27f, 0.40f},
            {0.04f, 0.43f, 0.54f}
        };
        for (int index = 0; index < 5; ++index) {
            const float x = positions[index][0];
            const float y = positions[index][1];
            drawRect(x, y, widths[index], 112,
                     colors[index][0], colors[index][1], colors[index][2]);
            glColor3f(0.96f, 0.98f, 1.0f);
            drawText(x + 22, y + 38,
                     std::to_string(index + 1) + "  " + names[index]);
            glColor3f(0.76f, 0.85f, 0.93f);
            drawText(x + 22, y + 70, details[index], GLUT_BITMAP_9_BY_15);
            drawText(x + 22, y + 94,
                     index == 4
                         ? "Click to start custom setup"
                         : "Click to choose this city",
                     GLUT_BITMAP_9_BY_15);
        }
        glColor3f(0.62f, 0.72f, 0.82f);
        drawText(72, 625, "Tip: press 1-5 to select a city. ESC cancels.",
                 GLUT_BITMAP_9_BY_15);
    }

    void drawDestinationChooser() const {
        const PresetCity city = makePresetCity(selectedCity);
        drawRect(0, 0, windowWidth, 110, 0.035f, 0.08f, 0.15f);
        glColor3f(0.18f, 0.82f, 0.95f);
        drawText(56, 48, "AEROROUTE  /  " + city.name + " DESTINATION");
        glColor3f(0.76f, 0.84f, 0.92f);
        drawText(56, 80, "Click any clear grid cell to deliver there.",
                 GLUT_BITMAP_9_BY_15);

        const float gridX = 90.0f;
        const float gridY = 165.0f;
        const float cell = 48.0f;
        for (int col = 0; col < 10; ++col) {
            glColor3f(0.72f, 0.82f, 0.9f);
            drawText(gridX + col * cell + 17.0f, gridY - 8.0f,
                     std::to_string(col), GLUT_BITMAP_9_BY_15);
        }
        for (int row = 0; row < 10; ++row) {
            glColor3f(0.72f, 0.82f, 0.9f);
            drawText(gridX - 20.0f, gridY + row * cell + 29.0f,
                     std::to_string(row), GLUT_BITMAP_9_BY_15);
            for (int col = 0; col < 10; ++col) {
                const int type = presetCellType(city, row, col);
                const float x = gridX + col * cell;
                const float y = gridY + row * cell;
                const float red = type == 3 ? 0.58f
                                : type == 2 ? 0.12f
                                : type == 1 ? 0.10f : 0.075f;
                const float green = type == 3 ? 0.27f
                                  : type == 2 ? 0.48f
                                  : type == 1 ? 0.36f : 0.11f;
                const float blue = type == 3 ? 0.16f
                                 : type == 2 ? 0.32f
                                 : type == 1 ? 0.58f : 0.18f;
                drawRect(x + 1, y + 1, cell - 2, cell - 2,
                         red, green, blue);
                const bool selected = row == selectedTargetRow &&
                                      col == selectedTargetCol;
                glColor3f(selected ? 1.0f : 0.20f,
                          selected ? 0.92f : 0.28f,
                          selected ? 0.20f : 0.37f);
                glLineWidth(selected ? 3.0f : 1.0f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(x + 1, y + 1);
                glVertex2f(x + cell - 1, y + 1);
                glVertex2f(x + cell - 1, y + cell - 1);
                glVertex2f(x + 1, y + cell - 1);
                glEnd();
                if (row == 0 && col == 0) {
                    glColor3f(1.0f, 1.0f, 1.0f);
                    drawText(x + 16, y + 29, "W", GLUT_BITMAP_HELVETICA_18);
                } else if (type == 1) {
                    glColor3f(1.0f, 1.0f, 1.0f);
                    drawText(x + 17, y + 29, "C", GLUT_BITMAP_HELVETICA_18);
                } else if (type == 2) {
                    glColor3f(1.0f, 1.0f, 1.0f);
                    drawText(x + 17, y + 29, "P", GLUT_BITMAP_HELVETICA_18);
                } else if (type == 3) {
                    glColor3f(1.0f, 1.0f, 1.0f);
                    drawText(x + 17, y + 29, "X", GLUT_BITMAP_HELVETICA_18);
                }
            }
        }

        const float infoX = 610.0f;
        glColor3f(0.95f, 0.97f, 1.0f);
        drawText(infoX, 190, "CITY FACILITIES");
        glColor3f(0.68f, 0.80f, 0.9f);
        drawText(infoX, 218, "W  Warehouse   C  Customer",
                 GLUT_BITMAP_9_BY_15);
        drawText(infoX, 240, "P  Charging pad   X  Building",
                 GLUT_BITMAP_9_BY_15);
        drawText(infoX, 280, "DELIVERY POINT");
        drawText(infoX, 308,
                 "(" + std::to_string(selectedTargetRow) + ", " +
                 std::to_string(selectedTargetCol) + ")");
        const bool reachable = presetPathExists(
            city, selectedTargetRow, selectedTargetCol);
        glColor3f(reachable ? 0.35f : 1.0f,
                  reachable ? 0.9f : 0.35f,
                  reachable ? 0.55f : 0.25f);
        drawText(infoX, 346, reachable
            ? "Shortest safe route available"
            : "DELIVERY NOT POSSIBLE",
            GLUT_BITMAP_9_BY_15);
        glColor3f(0.72f, 0.80f, 0.89f);
        drawText(infoX, 385, "Buildings block the drone.",
                 GLUT_BITMAP_9_BY_15);
        drawText(infoX, 408, "Choose another clear cell.",
                 GLUT_BITMAP_9_BY_15);
        drawText(infoX, 456, "W/A/S/D moves the target.",
                 GLUT_BITMAP_9_BY_15);
        drawText(infoX, 479, "Mouse click selects a cell.",
                 GLUT_BITMAP_9_BY_15);
        drawText(infoX, 502, "Enter confirms destination.",
                 GLUT_BITMAP_9_BY_15);
        drawRect(760, 535, 210, 48, 0.08f, 0.16f, 0.24f);
        glColor3f(0.88f, 0.94f, 1.0f);
        drawText(775, 564, selectedRoundTrip
            ? "TRIP: ROUND-TRIP" : "TRIP: ONE-WAY",
            GLUT_BITMAP_9_BY_15);
        glColor3f(0.62f, 0.72f, 0.82f);
        drawText(infoX, 604, "Click or press R to switch.",
                 GLUT_BITMAP_9_BY_15);

        drawRect(760, 625, 210, 54, 0.04f, 0.48f, 0.63f);
        glColor3f(0.96f, 0.98f, 1.0f);
        drawText(798, 659, reachable ? "CONFIRM DELIVERY" : "NO SAFE ROUTE");
        glColor3f(0.62f, 0.72f, 0.82f);
        drawText(72, 665,
                 message.empty()
                     ? "Choose a reachable empty/customer cell. ESC cancels."
                     : message,
                 GLUT_BITMAP_9_BY_15);
    }

    std::string valueOf(const std::string& id) const {
        const MenuField* field = findField(id);
        return field == nullptr ? "0" : field->value;
    }

    void updateFlightDefaults() {
        int row = 0, col = 0;
        if (!selectedCustomer(row, col)) return;
        if (MenuField* field = findField("to_row")) {
            field->value = std::to_string(row);
        }
        if (MenuField* field = findField("to_col")) {
            field->value = std::to_string(col);
        }
    }

    void advance() {
        if (pages[pageIndex].title == "Flight and delivery settings") {
            updateFlightDefaults();
        }
        if (!validateCurrentPage()) return;
        if (pageIndex == 3) {
            buildRemainingPages();
        }
        if (pages[pageIndex].title == "Drone and delivery site") {
            updateFlightDefaults();
        }
        if (pageIndex + 1 < pages.size()) {
            ++pageIndex;
            activeField = 0;
            replaceFieldOnType = true;
            message.clear();
        } else {
            for (const MenuPage& page : pages) {
                for (const MenuField& field : page.fields) {
                    result.push_back(field.value);
                }
            }
            glutLeaveMainLoop();
        }
    }

    void goBack() {
        if (pageIndex == 0) return;
        --pageIndex;
        activeField = 0;
        replaceFieldOnType = true;
        message.clear();
    }

    void draw() const {
        glClearColor(0.025f, 0.045f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (choosingCity) {
            drawCityChooser();
            glutSwapBuffers();
            return;
        }
        if (choosingDestination) {
            drawDestinationChooser();
            glutSwapBuffers();
            return;
        }

        drawRect(0, 0, windowWidth, 110, 0.045f, 0.09f, 0.16f);
        glColor3f(0.17f, 0.78f, 0.92f);
        drawText(48, 48, "AEROROUTE  /  CUSTOM MISSION SETUP",
                 GLUT_BITMAP_HELVETICA_18);
        glColor3f(0.75f, 0.83f, 0.90f);
        drawText(48, 80, "Configure the map and flight, then launch the live OpenGL mission.",
                 GLUT_BITMAP_9_BY_15);

        drawRect(36, 132, 968, 540, 0.055f, 0.085f, 0.14f);
        const MenuPage& page = pages[pageIndex];
        glColor3f(0.96f, 0.98f, 1.0f);
        drawText(72, 176, page.title);
        glColor3f(0.67f, 0.75f, 0.84f);
        drawText(72, 205, page.description, GLUT_BITMAP_9_BY_15);
        const bool scrollable =
            page.title == "Flight and delivery settings";
        drawText(815, 176, scrollable
            ? "CUSTOM SETTINGS" : "STEP " + std::to_string(pageIndex + 1) +
                 " / 4", GLUT_BITMAP_9_BY_15);

        const float rowTop = scrollable ? 250.0f : 232.0f;
        const float rowSpacing = scrollable ? 70.0f : 82.0f;
        const size_t firstField = scrollable ? scrollOffset : 0;
        const size_t visibleFields = scrollable
            ? missionFormVisibleFields : page.fields.size();
        const size_t endField = std::min(
            page.fields.size(), firstField + visibleFields);
        for (size_t index = firstField; index < endField; ++index) {
            const MenuField& field = page.fields[index];
            const float y = rowTop + rowSpacing *
                static_cast<float>(index - firstField);
            glColor3f(0.88f, 0.92f, 0.97f);
            drawText(76, y, field.label, GLUT_BITMAP_HELVETICA_18);
            glColor3f(0.52f, 0.63f, 0.74f);
            drawText(76, y + 19, field.hint, GLUT_BITMAP_9_BY_15);
            const bool selected = index == activeField;
            drawRect(570, y - 23, 360, 42,
                     selected ? 0.10f : 0.075f,
                     selected ? 0.17f : 0.12f,
                     selected ? 0.24f : 0.19f);
            glColor3f(selected ? 0.28f : 0.16f,
                      selected ? 0.78f : 0.24f,
                      selected ? 0.92f : 0.34f);
            glLineWidth(1.5f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(570, y - 23);
            glVertex2f(930, y - 23);
            glVertex2f(930, y + 19);
            glVertex2f(570, y + 19);
            glEnd();
            glColor3f(0.96f, 0.98f, 1.0f);
            drawText(584, y + 5, field.value.empty() ? "_" : field.value);
        }

        glColor3f(0.95f, 0.45f, 0.38f);
        if (!message.empty()) drawText(76, 612, message, GLUT_BITMAP_9_BY_15);
        glColor3f(0.55f, 0.68f, 0.79f);
        drawText(76, 642, scrollable
            ? "Mouse wheel scrolls. Click a value and type; TAB changes field; ENTER launches; ESC cancels."
            : "Click a box and type to replace its value. BACKSPACE edits. TAB changes field. ENTER continues. ESC cancels.",
                 GLUT_BITMAP_9_BY_15);
        if (scrollable) {
            const size_t total = page.fields.size();
            drawRect(962, rowTop, 5, rowSpacing *
                     static_cast<float>(missionFormVisibleFields),
                     0.13f, 0.18f, 0.25f);
            const float thumbHeight = 5.0f * rowSpacing *
                static_cast<float>(missionFormVisibleFields) /
                static_cast<float>(std::max(total, missionFormVisibleFields));
            const float maxOffset = static_cast<float>(
                std::max(total, missionFormVisibleFields) -
                missionFormVisibleFields);
            const float thumbY = rowTop + (maxOffset == 0.0f ? 0.0f :
                (rowSpacing * missionFormVisibleFields - thumbHeight) *
                static_cast<float>(scrollOffset) / maxOffset);
            drawRect(958, thumbY, 13, thumbHeight, 0.16f, 0.72f, 0.86f);
            drawText(850, 617,
                     std::to_string(scrollOffset + 1) + "-" +
                     std::to_string(endField) + "/" + std::to_string(total),
                     GLUT_BITMAP_9_BY_15);
        }

        drawRect(680, 690, 138, 44, 0.12f, 0.18f, 0.25f);
        drawRect(830, 690, 174, 44, 0.04f, 0.48f, 0.63f);
        glColor3f(0.92f, 0.96f, 1.0f);
        drawText(720, 718, "BACK");
        drawText(864, 718, pageIndex + 1 == pages.size()
                 ? "LAUNCH FLIGHT" : "NEXT");

        glutSwapBuffers();
    }

    void key(unsigned char key) {
        if (choosingCity) {
            if (key >= '1' && key <= '5') chooseCity(key - '0');
            else if (key == 27) {
                cancelled = true;
                glutLeaveMainLoop();
            }
            return;
        }
        if (choosingDestination) {
            if (key == 'r' || key == 'R') {
                selectedRoundTrip = !selectedRoundTrip;
                message.clear();
                glutPostRedisplay();
                return;
            }
            int row = selectedTargetRow;
            int col = selectedTargetCol;
            if (key == 'w' || key == 'W') --row;
            else if (key == 's' || key == 'S') ++row;
            else if (key == 'a' || key == 'A') --col;
            else if (key == 'd' || key == 'D') ++col;
            if (row != selectedTargetRow || col != selectedTargetCol) {
                if (row >= 0 && row < 10 && col >= 0 && col < 10) {
                    choosePresetTarget(row, col);
                }
            } else if (key == 13) {
                const PresetCity city = makePresetCity(selectedCity);
                if (presetPathExists(city, selectedTargetRow,
                                     selectedTargetCol)) {
                    buildPresetResult();
                    glutLeaveMainLoop();
                } else {
                    message =
                        "Delivery is not possible there. Choose a reachable cell.";
                    glutPostRedisplay();
                }
                return;
            } else if (key == 27) {
                cancelled = true;
                glutLeaveMainLoop();
                return;
            }
            glutPostRedisplay();
            return;
        }
        MenuPage& page = pages[pageIndex];
        if (key == 27) {
            cancelled = true;
            glutLeaveMainLoop();
        } else if (key == 13) {
            advance();
        } else if (key == 9) {
            if (!page.fields.empty()) {
                activeField = (activeField + 1) % page.fields.size();
                replaceFieldOnType = true;
                if (page.title == "Flight and delivery settings") {
                    if (activeField < scrollOffset) {
                        scrollOffset = activeField;
                    } else if (activeField >=
                               scrollOffset + missionFormVisibleFields) {
                        scrollOffset =
                            activeField - missionFormVisibleFields + 1;
                    }
                }
            }
        } else if (key == 1) {
            page.fields[activeField].value.clear();
            replaceFieldOnType = false;
        } else if (key == 8 || key == 127) {
            if (!page.fields[activeField].value.empty()) {
                page.fields[activeField].value.pop_back();
            }
            replaceFieldOnType = false;
        } else if (key >= 32 && key <= 126 &&
                   page.fields[activeField].value.size() < 64) {
            if (replaceFieldOnType) {
                page.fields[activeField].value.clear();
                replaceFieldOnType = false;
            }
            page.fields[activeField].value.push_back(
                static_cast<char>(key));
        }
        glutPostRedisplay();
    }

    void mouse(int button, int state, int x, int y) {
        if (state != GLUT_DOWN) return;
        if (choosingCity && button == GLUT_LEFT_BUTTON) {
            const float positions[][2] = {
                {70.0f, 160.0f}, {545.0f, 160.0f},
                {70.0f, 305.0f}, {545.0f, 305.0f},
                {70.0f, 450.0f}
            };
            const float widths[] = {425.0f, 425.0f, 425.0f, 425.0f, 900.0f};
            for (int index = 0; index < 5; ++index) {
                if (x >= positions[index][0] &&
                    x <= positions[index][0] + widths[index] &&
                    y >= positions[index][1] &&
                    y <= positions[index][1] + 112.0f) {
                    chooseCity(index + 1);
                    return;
                }
            }
            return;
        }
        if (choosingDestination && button == GLUT_LEFT_BUTTON) {
            const float gridX = 90.0f;
            const float gridY = 165.0f;
            const float cell = 48.0f;
            if (x >= gridX && x < gridX + 10.0f * cell &&
                y >= gridY && y < gridY + 10.0f * cell) {
                const int col = static_cast<int>((x - gridX) / cell);
                const int row = static_cast<int>((y - gridY) / cell);
                choosePresetTarget(row, col);
                return;
            }
            if (x >= 760 && x <= 970 && y >= 625 && y <= 679) {
                const PresetCity city = makePresetCity(selectedCity);
                if (presetPathExists(city, selectedTargetRow,
                                     selectedTargetCol)) {
                    buildPresetResult();
                    glutLeaveMainLoop();
                } else {
                    message =
                        "Delivery is not possible there. Choose a reachable cell.";
                    glutPostRedisplay();
                }
            } else if (x >= 760 && x <= 970 && y >= 535 && y <= 583) {
                selectedRoundTrip = !selectedRoundTrip;
                message.clear();
                glutPostRedisplay();
            }
            return;
        }
        if ((button == 3 || button == 4) &&
            pages[pageIndex].title == "Flight and delivery settings") {
            scrollFields(button == 3 ? -1 : 1);
            return;
        }
        if (button != GLUT_LEFT_BUTTON) return;
        MenuPage& page = pages[pageIndex];
        const bool scrollable =
            page.title == "Flight and delivery settings";
        const float rowTop = scrollable ? 250.0f : 232.0f;
        const float rowSpacing = scrollable ? 70.0f : 82.0f;
        const size_t firstField = scrollable ? scrollOffset : 0;
        const size_t endField = std::min(page.fields.size(),
            firstField + (scrollable ? missionFormVisibleFields :
                                       page.fields.size()));
        for (size_t index = firstField; index < endField; ++index) {
            const float rowY = rowTop + rowSpacing *
                static_cast<float>(index - firstField);
            if (x >= 570 && x <= 930 && y >= rowY - 23 && y <= rowY + 19) {
                activeField = index;
                replaceFieldOnType = true;
                message.clear();
                glutPostRedisplay();
                return;
            }
        }
        if (y >= 690 && y <= 734) {
            if (x >= 680 && x <= 818) goBack();
            else if (x >= 830 && x <= 1004) advance();
        }
        glutPostRedisplay();
    }
};

namespace {

void displayCallback() {
    if (activeMenu) activeMenu->draw();
}

void keyboardCallback(unsigned char key, int, int) {
    if (activeMenu) activeMenu->key(key);
}

void mouseCallback(int button, int state, int x, int y) {
    if (!activeMenu) return;
    if ((button == 3 || button == 4) && state == GLUT_DOWN) {
        activeMenu->mouse(button, state, x, y);
        return;
    }
    activeMenu->mouse(button, state, x, y);
}

} // namespace

std::vector<std::string> AeroRouteMenu::run(int argc, char** argv) {
    AeroRouteMenuState state;
    activeMenu = &state;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
                  GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(80, 50);
    const int windowId = glutCreateWindow("AeroRoute - Delivery Setup");
    glutDisplayFunc(displayCallback);
    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouseCallback);
    glutMainLoop();
    if (glutGetWindow() != 0) glutDestroyWindow(windowId);
    activeMenu = nullptr;
    if (state.cancelled) return {};
    return state.result;
}
