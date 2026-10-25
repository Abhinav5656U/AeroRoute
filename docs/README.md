# AeroRoute

Autonomous drone delivery simulator with four C++ lab modules and a JSON-backed dashboard.

## Project layout

- `src/shared/`: shared constants and JSON writer
- `src/PL/`: grid and route tracking
- `src/COA/`: register simulation and assembly engine
- `src/CG/`: OpenGL renderer
- Subject-specific `Co4.cpp` / `Co4.h` files keep each CO4 implementation
  with its subject: `src/PL/`, `src/COA/`, `src/CG/`, and `src/PSOOP/`
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
g++ Subject2_Main.cpp DroneRegisters.cpp AssemblyEngine.cpp DisplacementAsm.S -o ..\..\build\subject2.exe -std=c++17
..\..\build\subject2.exe
```

`DisplacementAsm.S` is the separate x86-64 GNU assembly implementation used
to calculate the coordinate deltas, squared distance, and square root. It uses
the Windows x64 calling convention expected by this MinGW-w64 build.

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
g++ ..\main.cpp ..\PL\CityGrid.cpp ..\PL\Co4.cpp ..\PL\RouteTracker.cpp ..\PL\DeliveryScheduler.cpp ..\COA\DroneRegisters.cpp ..\COA\Co4.cpp ..\COA\AssemblyEngine.cpp ..\COA\DisplacementAsm.S ..\CG\AeroRouteMenu.cpp ..\CG\Co4.cpp ..\CG\GraphicsRenderer.cpp ..\PSOOP\Location.cpp ..\PSOOP\Warehouse.cpp ..\PSOOP\CustomerSite.cpp ..\PSOOP\ChargingPad.cpp ..\PSOOP\Co4.cpp ..\PSOOP\Package.cpp ..\PSOOP\Drone.cpp -o ..\..\build\aeroroute.exe -std=c++17 -lfreeglut -lopengl32 -lglu32
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

The integrated C++ application also includes subject-specific CO4 work:

- **PL CO4:** route-node sorting, linear/binary/chained-hash searches, explicit
  collision handling, and comparison counts across increasing dataset sizes.
- **COA CO4:** a mission workflow mapping telemetry to registers, arithmetic to
  the ALU, conditional branching to dispatch decisions, and mission text to the
  64-bit assembly string routines.
- **CG CO4:** final texture/route alignment with a numeric affine-transform
  precision check; the graphics state is exported for the dashboard.
- **PSOOP CO4:** a C++ object-oriented delivery decision that selects a drone,
  charges when needed, completes eligible deliveries, and rejects an unsafe
  payload/range combination across three test cases.

To run the CO4 checks from `src/PL` without opening a graphics window:

```bat
..\..\build\aeroroute.exe --co4-check
```

To run the same checks and then launch the integrated CG rendering:

```bat
..\..\build\aeroroute.exe --co4
```

To execute all PL, COA, OOP, and CG CO4 demonstrations, then open the
customizable setup form for the integrated delivery mission:

```bat
..\..\build\aeroroute.exe --all-demo
```

The first setup screen offers ready-made Delhi, Mumbai, Pune, and Patna maps,
or **Custom city**. Each ready-made 10 x 10 city has customer sites, charging
pads, and a repeatable randomized arrangement of building/obstacle cells.
For a ready-made city, select any reachable clear cell on its grid as the
delivery destination; the app finds the shortest obstacle-avoiding route.
Press **R** or click the trip toggle to choose a one-way flight or a
warehouse-return flight. The custom setup has the same choice in its
scrollable settings. The route planner accounts for the selected drone's
battery and plans charging-pad stops as needed; a mission is rejected if no
battery-safe route to the destination (and back, when selected) exists.
Charging is performed automatically when the drone reaches a planned pad.
Custom city first asks for the grid and primary
locations, then puts the remaining map, route, drone, package, telemetry,
flight-computer, and display settings into one scrollable form. These choices
configure the integrated PL, COA, OOP, and CG mission.

The dashed Cohen-Sutherland clipping window follows the drone as it flies.
Only the nearby portion of its shortest route is drawn; yellow markers show
route/window intersections, and the HUD
reports clipped versus visible segments. Use **C/V** to shrink or grow the
window. Only route portions inside the clipping window are drawn; accepted
segments, clipped endpoint coordinates, and clipping flags are saved to
`website/data/clipped_route.json`. Let the animation finish before closing
the window to verify delivery.

## Dashboard

Serve the `website/` directory locally, for example with `python -m http.server 8080`, then open `http://localhost:8080`. C++ executables write their JSON output to `website/data/`.
