# AeroRoute

Autonomous drone delivery simulator with four C++ lab modules and a JSON-backed dashboard.

## Project layout

- `src/shared/`: shared constants and JSON writer
- `src/PL/`: grid and route tracking
- `src/COA/`: register simulation and assembly engine
- `src/CG/`: OpenGL renderer
- `src/PSOOP/`: delivery domain classes
- `src/main.cpp`: integrated mission entry point
- `website/`: dashboard, data, and browser input files

## Build

Use a MinGW-w64 `g++` installation. Run each command from the indicated module directory so the relative JSON paths resolve to `website/data/`.

Programming Lab, from `src/PL`:

```bat
g++ Subject1_Main.cpp CityGrid.cpp RouteTracker.cpp DeliveryScheduler.cpp -o ..\..\build\subject1.exe -std=c++17
..\..\build\subject1.exe
```

COA Lab, from `src/COA`:

```bat
g++ Subject2_Main.cpp DroneRegisters.cpp AssemblyEngine.cpp -o ..\..\build\subject2.exe -std=c++17
..\..\build\subject2.exe
```

Graphics Lab, from `src/CG`:

```bat
g++ Subject3_Main.cpp GraphicsRenderer.cpp -o ..\..\build\subject3.exe -std=c++17 -lfreeglut -lopengl32 -lglu32
..\..\build\subject3.exe
```

OOP Lab, from `src/PSOOP`:

```bat
g++ Subject4_Main.cpp Location.cpp Warehouse.cpp CustomerSite.cpp ChargingPad.cpp Package.cpp Drone.cpp -o ..\..\build\subject4.exe -std=c++17
..\..\build\subject4.exe
```

Integrated system, from `src/PL`:

```bat
g++ ..\main.cpp ..\PL\CityGrid.cpp ..\PL\RouteTracker.cpp ..\PL\DeliveryScheduler.cpp ..\COA\DroneRegisters.cpp ..\COA\AssemblyEngine.cpp ..\CG\AeroRouteMenu.cpp ..\CG\GraphicsRenderer.cpp ..\PSOOP\Location.cpp ..\PSOOP\Warehouse.cpp ..\PSOOP\CustomerSite.cpp ..\PSOOP\ChargingPad.cpp ..\PSOOP\Package.cpp ..\PSOOP\Drone.cpp -o ..\..\build\aeroroute.exe -std=c++17 -lfreeglut -lopengl32 -lglu32
..\..\build\aeroroute.exe
```

The integrated controller opens a graphical OpenGL setup window. Use the
mouse to select a field and type its value; TAB changes the active field,
ENTER or NEXT advances, BACK returns to the previous page, and ESC cancels.
On the final page click **LAUNCH FLIGHT**. The form gathers grid dimensions,
warehouse and customer locations, extra map nodes, city name, route type, drone
and package details, telemetry, flight-computer instruction, and graphics
transforms. After launch, a second OpenGL window animates the drone. Close that
window with ESC to exit the application.

The simulator uses the configured grid and route for collision-aware flight
planning, creates the selected drone and package objects, loads the telemetry
into registers, runs the selected assembly operation, and renders the animated
mission with the chosen affine transform. The separate subject programs
continue to provide their syllabus-focused demonstrations.

The Task 3 implementations are based on the Semester III exam reform guidelines:

- **PL CO3:** FIFO dispatch queue, LIFO completed-drone stack, invalid
  enqueue, and empty-queue boundary checks.
- **COA CO3:** multiplication by successive addition and 64-bit `SCASB` /
  `MOVSB` string length and append operations.
- **CG CO6:** procedural city-surface texture mapping and colored landmark
  lighting in the OpenGL renderer; press `T` to toggle effects.
- **PSOOP CO3:** exceptions for invalid packages and null package references,
  plus safe handling of a drone carrying a package when another is offered.

## Dashboard

Serve the `website/` directory locally, for example with `python -m http.server 8080`, then open `http://localhost:8080`. C++ executables write their JSON output to `website/data/`.
