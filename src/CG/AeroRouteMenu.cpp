#include "AeroRouteMenu.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
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
        }
        message.clear();
        return true;
    }

    size_t customerCount() const {
        size_t count = 1;
        for (const MenuPage& page : pages) {
            if (page.title.find("Map location ") != 0) continue;
            int type = 0;
            if (parseInteger(page.fields[0].value, type) && type == 1) {
                ++count;
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
            if (page.title.find("Map location ") != 0) continue;
            int type = 0;
            if (!parseInteger(page.fields[0].value, type) || type != 1) continue;
            ++found;
            if (found == choice) {
                const std::string base = page.fields[0].id.substr(
                    0, page.fields[0].id.size() - 5);
                return integerValue(base + "_row", row) &&
                       integerValue(base + "_col", col);
            }
        }
        return false;
    }

    std::pair<int, int> freeCell() const {
        int rows = 5, cols = 5;
        integerValue("rows", rows);
        integerValue("cols", cols);
        const auto occupied = configuredCoordinates();
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                if (std::find(occupied.begin(), occupied.end(),
                              std::make_pair(row, col)) != occupied.end()) {
                    continue;
                }
                return {row, col};
            }
        }
        return {0, 0};
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
        const auto savedOr = [&](const std::string& id,
                                 const std::string& fallback) {
            const auto saved = previousValues.find(id);
            return saved == previousValues.end() ? fallback : saved->second;
        };

        int count = 0;
        integerValue("extra_count", count);
        for (int index = 0; index < count; ++index) {
            const std::string base = "node_" + std::to_string(index + 1);
            const std::pair<int, int> position = freeCell();
            pages.push_back({
                "Map location " + std::to_string(index + 1),
                "Choose type: 1 customer, 2 charging pad, 3 obstacle.",
                {{base + "_type", "Location type", savedOr(base + "_type", "2"), "1 customer | 2 charging pad | 3 obstacle"},
                 {base + "_label", "Location name", savedOr(base + "_label", "CP-" + std::to_string(index + 1)), "Example: CP-1 or OB-1"},
                 {base + "_row", "Grid row", savedOr(base + "_row", std::to_string(position.first)), "Grid coordinates start at 0"},
                 {base + "_col", "Grid column", savedOr(base + "_col", std::to_string(position.second)), "Grid coordinates start at 0"}}
            });
        }

        pages.push_back({"City and route", "Name the city and choose a route style.",
            {{"destination", "City / delivery area", savedOr("destination", "Mumbai"), "Example: Mumbai, Powai"},
             {"route_type", "Route type", savedOr("route_type", "1"), "1 singly | 2 doubly | 3 circular"}}});
        pages.push_back({"Drone and delivery site",
                         "Name the drone and select the customer destination.",
            {{"drone_serial", "Drone serial number", savedOr("drone_serial", "AR-D1-004"), "Example: AR-D1-004"},
             {"site_choice", "Delivery site number", savedOr("site_choice", "1"),
              "1=first customer; next numbers follow added customer sites"}}});
        pages.push_back({"Flight path", "The path endpoints must match the selected locations.",
            {{"from_row", "Start row (warehouse)", savedOr("from_row", valueOf("warehouse_row")), "Must match warehouse"},
             {"from_col", "Start column (warehouse)", savedOr("from_col", valueOf("warehouse_col")), "Must match warehouse"},
             {"to_row", "Destination row (customer)", savedOr("to_row", valueOf("customer_row")), "Must match selected customer"},
             {"to_col", "Destination column (customer)", savedOr("to_col", valueOf("customer_col")), "Must match selected customer"}}});
        pages.push_back({"Drone and package", "Choose a drone that can carry this payload.",
            {{"drone_type", "Drone model", savedOr("drone_type", "1"), "1 DeliveryDrone | 2 ScoutDrone | 3 HeavyDrone"},
             {"package_weight", "Package weight (kg)", savedOr("package_weight", "1.0"), "Delivery 0.01-15 | Scout 0.01-0.5 | Heavy 0.01-50"},
             {"recipient", "Recipient name", savedOr("recipient", "Asha Sharma"), "Person receiving the parcel"}}});
        pages.push_back({"Flight telemetry", "Enter the drone's starting readings.",
            {{"altitude", "Altitude (m)", savedOr("altitude", "100"), "0-10000"},
             {"speed", "Speed (m/s)", savedOr("speed", "15"), "0-300"},
             {"heading", "Heading (degrees)", savedOr("heading", "45"), "0-359.999"},
             {"battery", "Battery (%)", savedOr("battery", "90"), "0.1-100"}}});
        pages.push_back({"Flight computer", "Select an operation for the chosen register.",
            {{"instruction", "Instruction", savedOr("instruction", "2"), "1 MOV | 2 ADD | 3 SUB"},
             {"register", "Register", savedOr("register", "1"), "1 RAX | 2 RBX | 3 RCX | 4 RDX"},
             {"operand", "Operand", savedOr("operand", "5"), "-1000000000 to 1000000000"}}});
        pages.push_back({"Display and launch",
                         "Set the drone marker transform, then launch the flight.",
            {{"translate_x", "Translate X (cells)", savedOr("translate_x", "0"), "-5 to 5"},
             {"translate_y", "Translate Y (cells)", savedOr("translate_y", "0"), "-5 to 5"},
             {"rotation", "Rotation (degrees)", savedOr("rotation", "0"), "-360 to 360"},
             {"scale", "Scale multiplier", savedOr("scale", "1"), "0.2 to 3"}}});
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

        drawRect(0, 0, windowWidth, 110, 0.045f, 0.09f, 0.16f);
        glColor3f(0.17f, 0.78f, 0.92f);
        drawText(48, 48, "AEROROUTE  /  DELIVERY SETUP",
                 GLUT_BITMAP_HELVETICA_18);
        glColor3f(0.75f, 0.83f, 0.90f);
        drawText(48, 80, "Enter the mission data, then launch the live OpenGL flight.",
                 GLUT_BITMAP_9_BY_15);

        drawRect(36, 132, 968, 540, 0.055f, 0.085f, 0.14f);
        const MenuPage& page = pages[pageIndex];
        glColor3f(0.96f, 0.98f, 1.0f);
        drawText(72, 176, page.title);
        glColor3f(0.67f, 0.75f, 0.84f);
        drawText(72, 205, page.description, GLUT_BITMAP_9_BY_15);
        drawText(815, 176, "STEP " + std::to_string(pageIndex + 1) +
                 " / " + std::to_string(pages.size()), GLUT_BITMAP_9_BY_15);

        const float rowTop = 232.0f;
        const float rowSpacing = 82.0f;
        for (size_t index = 0; index < page.fields.size(); ++index) {
            const MenuField& field = page.fields[index];
            const float y = rowTop + rowSpacing * static_cast<float>(index);
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
        if (!message.empty()) drawText(76, 620, message, GLUT_BITMAP_9_BY_15);
        glColor3f(0.55f, 0.68f, 0.79f);
        drawText(76, 645, "Click a box and type to replace its value. BACKSPACE edits. TAB changes field. ENTER continues. ESC cancels.",
                 GLUT_BITMAP_9_BY_15);

        drawRect(680, 690, 138, 44, 0.12f, 0.18f, 0.25f);
        drawRect(830, 690, 174, 44, 0.04f, 0.48f, 0.63f);
        glColor3f(0.92f, 0.96f, 1.0f);
        drawText(720, 718, "BACK");
        drawText(864, 718, pageIndex + 1 == pages.size()
                 ? "LAUNCH FLIGHT" : "NEXT");

        glutSwapBuffers();
    }

    void key(unsigned char key) {
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
        if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
        MenuPage& page = pages[pageIndex];
        const float rowTop = 232.0f;
        const float rowSpacing = 82.0f;
        for (size_t index = 0; index < page.fields.size(); ++index) {
            const float rowY = rowTop + rowSpacing * static_cast<float>(index);
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
    if (activeMenu) activeMenu->mouse(button, state, x, y);
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
