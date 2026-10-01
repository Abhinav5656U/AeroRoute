const CELL_COLORS = {
    1: '#e5a251',
    2: '#5aaab1',
    3: '#4f9b76',
    4: '#ce6657'
};

const DRONES = {
    delivery: { name: 'AR-D1 Delivery', model: 'AR-D1', capacity: 5, speed: 18.5 },
    scout: { name: 'AR-S1 Scout', model: 'AR-S1', capacity: 0, speed: 24 },
    heavy: { name: 'AR-H1 Heavy Lift', model: 'AR-H1', capacity: 20, speed: 12 }
};

class DeliveryQueueNode {
    constructor(droneSerial, destination) {
        this.droneSerial = droneSerial;
        this.destination = destination;
        this.next = null;
    }
}

class LinkedListDeliveryQueue {
    constructor() {
        this.front = null;
        this.rear = null;
        this.count = 0;
    }

    enqueue(droneSerial, destination) {
        const node = new DeliveryQueueNode(droneSerial, destination);
        if (this.rear) this.rear.next = node;
        else this.front = node;
        this.rear = node;
        this.count += 1;
    }

    dequeue() {
        if (!this.front) return null;
        const node = this.front;
        this.front = node.next;
        if (!this.front) this.rear = null;
        this.count -= 1;
        return { droneSerial: node.droneSerial, destination: node.destination };
    }

    toArray() {
        const deliveries = [];
        for (let node = this.front; node; node = node.next) {
            deliveries.push({ droneSerial: node.droneSerial, destination: node.destination });
        }
        return deliveries;
    }
}

class CompletedDroneNode {
    constructor(droneSerial) {
        this.droneSerial = droneSerial;
        this.next = null;
    }
}

class LinkedListCompletedStack {
    constructor() {
        this.top = null;
        this.count = 0;
    }

    push(droneSerial) {
        const node = new CompletedDroneNode(droneSerial);
        node.next = this.top;
        this.top = node;
        this.count += 1;
    }

    toArray() {
        const drones = [];
        for (let node = this.top; node; node = node.next) drones.push(node.droneSerial);
        return drones;
    }
}

const defaultNodes = [
    { row: 0, col: 0, type: 1, label: 'WH-Main' },
    { row: 0, col: 9, type: 1, label: 'WH-East' },
    { row: 2, col: 4, type: 4, label: 'OB-1' },
    { row: 3, col: 3, type: 3, label: 'CP-1' },
    { row: 9, col: 9, type: 2, label: 'CS-Alpha' }
];

function initialState() {
    return {
        nodes: defaultNodes.map(node => ({ ...node })),
        route: [
            { row: 0, col: 0, label: 'WH-Main' },
            { row: 9, col: 9, label: 'CS-Alpha' }
        ],
        routeType: 'SLL',
        routeIndex: 0,
        drone: { row: 0, col: 0, model: 'AR-D1', name: 'AR-D1 Delivery' },
        telemetry: { altitude: 120, speed: 18.5, heading: 45, battery: 87 },
        registers: { RAX: 120, RBX: 1850, RCX: 4500, RDX: 8700 },
        transform: { x: 0, y: 0, rotation: 0, scale: 1 },
        deliveryQueue: new LinkedListDeliveryQueue(),
        completedDrones: new LinkedListCompletedStack(),
        entities: ['WH-Main · Warehouse', 'CS-Alpha · CustomerSite', 'CP-1 · ChargingPad']
    };
}

let state = initialState();

const byId = id => document.getElementById(id);
const numberValue = id => Number(byId(id).value);
byId('beLabel').textContent = 'Browser demo';
byId('beDot').classList.add('online');

function logMission(message) {
    const line = document.createElement('div');
    line.textContent = `[${new Date().toLocaleTimeString()}] ${message}`;
    const log = byId('missionLog');
    log.append(line);
    while (log.children.length > 28) log.firstElementChild.remove();
    log.scrollTop = log.scrollHeight;
}

function markSubject(subject, message) {
    const stateLabel = byId(`state-${subject}`);
    if (stateLabel) stateLabel.textContent = 'RAN';
    const statusIcon = byId(`status-${subject === 'oop' ? 's4' : subject === 'cg' ? 's3' : subject === 'coa' ? 's2' : 's1'}`);
    if (statusIcon) statusIcon.className = 'fas fa-check-circle status-icon';
    byId('beLabel').textContent = 'Demo active';
    byId('beDot').classList.add('online');
    if (message) logMission(message);
}

function updateDashboard() {
    byId('heroAlt').textContent = `${state.telemetry.altitude} m`;
    byId('heroSpd').textContent = `${Number(state.telemetry.speed).toFixed(1)} m/s`;
    byId('heroBat').textContent = `${Math.round(state.telemetry.battery)}%`;
    byId('heroStatus').textContent = state.telemetry.battery <= 20 ? 'LOW BATTERY' : 'IN FLIGHT';
    byId('tpPos').textContent = `${state.drone.row}, ${state.drone.col}`;
    byId('tpHead').textContent = `${Math.round(state.telemetry.heading).toString().padStart(3, '0')}°`;
    byId('tpModel').textContent = state.drone.model;
    byId('tpRoute').textContent = state.routeType;
    byId('entityList').textContent = state.entities.join(' · ');
    byId('gridStatus').textContent = `${state.nodes.length} NODES / DEMO`;
    byId('llVisual').textContent = routeDescription();

    for (const [register, value] of Object.entries(state.registers)) {
        byId(`hex-${register}`).textContent = `0x${Math.max(0, Math.trunc(value)).toString(16).toUpperCase().padStart(16, '0')}`;
        byId(`dec-${register}`).textContent = String(Math.trunc(value));
    }
    drawGrid();
}

function routeDescription() {
    const names = state.route.map(point => point.label);
    if (state.routeType === 'CLL' && names.length > 1) names.push(names[0]);
    const separator = state.routeType === 'DLL' ? ' ⇄ ' : state.routeType === 'CLL' ? ' ↻ ' : ' → ';
    return names.join(separator);
}

function transformedPoint(row, col, board) {
    const transform = state.transform;
    let x = board.left + (col + 0.5) * board.cell - board.centerX;
    let y = board.top + (row + 0.5) * board.cell - board.centerY;
    const radians = transform.rotation * Math.PI / 180;
    const rotatedX = x * Math.cos(radians) - y * Math.sin(radians);
    const rotatedY = x * Math.sin(radians) + y * Math.cos(radians);
    return {
        x: board.centerX + rotatedX * transform.scale + transform.x * board.cell,
        y: board.centerY + rotatedY * transform.scale - transform.y * board.cell
    };
}

function drawGrid() {
    const canvas = byId('dashCanvas');
    const rect = canvas.getBoundingClientRect();
    const width = rect.width || 560;
    const height = rect.height || width;
    const pixelRatio = window.devicePixelRatio || 1;
    canvas.width = Math.round(width * pixelRatio);
    canvas.height = Math.round(height * pixelRatio);
    const context = canvas.getContext('2d');
    context.setTransform(pixelRatio, 0, 0, pixelRatio, 0, 0);
    context.clearRect(0, 0, width, height);
    context.fillStyle = '#fbfcfb';
    context.fillRect(0, 0, width, height);

    const padding = Math.max(22, width * 0.06);
    const boardSize = Math.min(width, height) - padding * 2;
    const cell = boardSize / 10;
    const board = {
        left: (width - boardSize) / 2,
        top: (height - boardSize) / 2,
        cell,
        centerX: width / 2,
        centerY: height / 2
    };

    context.strokeStyle = '#dfe5e2';
    context.lineWidth = 1;
    for (let index = 0; index <= 10; index += 1) {
        const offset = board.left + index * cell;
        const vertical = board.top + index * cell;
        context.beginPath();
        context.moveTo(offset, board.top);
        context.lineTo(offset, board.top + boardSize);
        context.stroke();
        context.beginPath();
        context.moveTo(board.left, vertical);
        context.lineTo(board.left + boardSize, vertical);
        context.stroke();
    }

    if (state.route.length > 1) {
        const points = state.route.map(point => transformedPoint(point.row, point.col, board));
        if (state.routeType === 'DLL') points.push(...points.slice(0, -1).reverse());
        if (state.routeType === 'CLL') points.push(points[0]);
        context.beginPath();
        points.forEach((point, index) => index === 0 ? context.moveTo(point.x, point.y) : context.lineTo(point.x, point.y));
        context.strokeStyle = '#167a72';
        context.lineWidth = Math.max(2, cell * 0.07);
        context.setLineDash([cell * 0.18, cell * 0.12]);
        context.stroke();
        context.setLineDash([]);
    }

    for (const node of state.nodes) {
        const point = transformedPoint(node.row, node.col, board);
        context.fillStyle = CELL_COLORS[node.type];
        context.beginPath();
        context.roundRect(point.x - cell * 0.3, point.y - cell * 0.3, cell * 0.6, cell * 0.6, Math.max(3, cell * 0.12));
        context.fill();
        if (cell >= 42) {
            context.fillStyle = '#384541';
            context.font = `600 ${Math.max(8, Math.min(11, cell * 0.2))}px Inter, sans-serif`;
            context.textAlign = 'center';
            context.fillText(node.label, point.x, point.y + cell * 0.47, cell * 1.8);
        }
    }

    const dronePoint = transformedPoint(state.drone.row, state.drone.col, board);
    context.fillStyle = '#173f63';
    context.beginPath();
    context.arc(dronePoint.x, dronePoint.y, Math.max(5, cell * 0.18), 0, Math.PI * 2);
    context.fill();
    context.strokeStyle = '#fff';
    context.lineWidth = 2;
    context.stroke();
}

function selectLab(subject) {
    document.querySelectorAll('.lab-tab').forEach(tab => {
        const selected = tab.dataset.tab === subject;
        tab.classList.toggle('active', selected);
        tab.setAttribute('aria-selected', String(selected));
        tab.tabIndex = selected ? 0 : -1;
    });
    document.querySelectorAll('.lab-panel').forEach(panel => {
        const selected = panel.dataset.panel === subject;
        panel.hidden = !selected;
        panel.classList.toggle('active', selected);
    });
}

function readCoordinate(value, fieldName) {
    const match = value.trim().match(/^(\d)\s*,\s*(\d)$/);
    if (!match) throw new Error(`${fieldName} must use row,col from 0 to 9.`);
    return { row: Number(match[1]), col: Number(match[2]) };
}

function addGridNode() {
    const row = numberValue('plRow');
    const col = numberValue('plCol');
    const type = numberValue('plNodeType');
    const label = byId('plNodeName').value.trim();
    if (!Number.isInteger(row) || !Number.isInteger(col) || row < 0 || row > 9 || col < 0 || col > 9) {
        logMission('PL rejected node: row and column must be between 0 and 9.');
        return;
    }
    if (!label) {
        logMission('PL rejected node: enter a label first.');
        return;
    }
    const existing = state.nodes.find(node => node.row === row && node.col === col);
    if (existing && existing.label === 'WH-Main') {
        logMission('PL cannot replace the dispatch warehouse.');
        return;
    }
    if (existing) Object.assign(existing, { type, label });
    else state.nodes.push({ row, col, type, label });
    markSubject('pl', `${existing ? 'Updated' : 'Added'} ${label} at ${row},${col}.`);
    updateDashboard();
}

function buildRoute() {
    const destination = readCoordinate(byId('plDestination').value, 'Destination');
    const destinationNode = state.nodes.find(node => node.row === destination.row && node.col === destination.col);
    const destinationLabel = destinationNode?.label || ({ '9,9': 'CS-Alpha', '7,7': 'CP-1', '0,9': 'WH-East' }[`${destination.row},${destination.col}`] || 'Destination');
    const middleNodes = state.nodes
        .filter(node => node.type !== 4 && !(node.row === 0 && node.col === 0) && !(node.row === destination.row && node.col === destination.col))
        .map(node => ({ row: node.row, col: node.col, label: node.label }));
    state.route = [{ row: 0, col: 0, label: 'WH-Main' }, ...middleNodes, { ...destination, label: destinationLabel }];
    state.routeType = byId('plRouteType').value;
    state.routeIndex = 0;
    state.drone.row = 0;
    state.drone.col = 0;
    markSubject('pl', `Built ${state.routeType} route with ${state.route.length} waypoints.`);
    updateDashboard();
}

function advanceDrone() {
    if (state.routeType === 'CLL') state.routeIndex = (state.routeIndex + 1) % state.route.length;
    else if (state.routeIndex < state.route.length - 1) state.routeIndex += 1;
    else {
        logMission('PL route is complete. Build a new route to dispatch again.');
        return;
    }
    const waypoint = state.route[state.routeIndex];
    state.drone.row = waypoint.row;
    state.drone.col = waypoint.col;
    state.telemetry.battery = Math.max(0, state.telemetry.battery - 2);
    markSubject('pl', `Drone advanced to ${waypoint.label} (${waypoint.row},${waypoint.col}).`);
    updateDashboard();
}

function renderDeliveryScheduler() {
    const pending = state.deliveryQueue.toArray();
    const completed = state.completedDrones.toArray();
    const pendingList = byId('pendingDeliveries');
    const completedList = byId('completedDrones');

    pendingList.replaceChildren();
    completedList.replaceChildren();
    for (const delivery of pending) {
        const item = document.createElement('li');
        item.textContent = `${delivery.droneSerial} -> ${delivery.destination}`;
        pendingList.append(item);
    }
    for (const droneSerial of completed) {
        const item = document.createElement('li');
        item.textContent = droneSerial;
        completedList.append(item);
    }

    if (pending.length === 0) {
        const item = document.createElement('li');
        item.className = 'dispatch-empty';
        item.textContent = 'Queue empty';
        pendingList.append(item);
    }
    if (completed.length === 0) {
        const item = document.createElement('li');
        item.className = 'dispatch-empty';
        item.textContent = 'Stack empty';
        completedList.append(item);
    }

    byId('pendingCount').textContent = String(state.deliveryQueue.count);
    byId('completedCount').textContent = String(state.completedDrones.count);
    byId('dispatchCounts').textContent = `${state.deliveryQueue.count} pending / ${state.completedDrones.count} completed`;
    byId('completeDelivery').disabled = state.deliveryQueue.count === 0;
}

function queueDelivery() {
    const droneSerial = byId('dispatchDrone').value.trim();
    const destination = byId('dispatchDestination').value.trim();
    if (!droneSerial || !destination) {
        logMission('PL CO3 requires both a drone serial and destination.');
        return;
    }

    state.deliveryQueue.enqueue(droneSerial, destination);
    byId('dispatchDrone').value = '';
    byId('dispatchDestination').value = '';
    renderDeliveryScheduler();
    markSubject('pl', `CO3 queued ${droneSerial} for ${destination}.`);
}

function completeNextDelivery() {
    const delivery = state.deliveryQueue.dequeue();
    if (!delivery) {
        renderDeliveryScheduler();
        logMission('PL CO3 has no pending delivery to complete.');
        return;
    }

    state.completedDrones.push(delivery.droneSerial);
    renderDeliveryScheduler();
    markSubject('pl', `CO3 completed ${delivery.destination}; ${delivery.droneSerial} moved to the completed stack.`);
}

function runCoa() {
    try {
        const from = readCoordinate(byId('coaFrom').value, 'Start');
        const to = readCoordinate(byId('coaTo').value, 'Destination');
        const altitude = numberValue('coaAltitude');
        const speed = numberValue('coaSpeed');
        const heading = numberValue('coaHeading');
        const battery = numberValue('coaBattery');
        if (![altitude, speed, heading, battery].every(Number.isFinite) || altitude < 0 || speed < 0 || heading < 0 || heading > 359 || battery < 0 || battery > 100) {
            throw new Error('Enter valid non-negative telemetry; heading must be 0–359 and battery 0–100.');
        }
        state.registers = {
            RAX: Math.round(altitude),
            RBX: Math.round(speed * 100),
            RCX: Math.round(heading * 100),
            RDX: Math.round(battery * 100)
        };
        state.telemetry = { altitude, speed, heading, battery };
        const deltaRow = to.row - from.row;
        const deltaCol = to.col - from.col;
        const distance = Math.hypot(deltaRow, deltaCol);
        byId('coaResult').textContent = `Δrow ${deltaRow} · Δcol ${deltaCol} · displacement ${distance.toFixed(2)} cells`;
        markSubject('coa', `COA loaded telemetry. RAX=${state.registers.RAX}, RBX=${state.registers.RBX}, RCX=${state.registers.RCX}, RDX=${state.registers.RDX}.`);
        updateDashboard();
    } catch (error) {
        byId('coaResult').textContent = error.message;
    }
}

function applyTransform() {
    const x = numberValue('cgTranslateX');
    const y = numberValue('cgTranslateY');
    const rotation = numberValue('cgRotation');
    const scale = numberValue('cgScale');
    if (![x, y, rotation, scale].every(Number.isFinite) || scale < 0.4 || scale > 2.5) {
        byId('cgResult').textContent = 'Scale must be between 0.4 and 2.5.';
        return;
    }
    state.transform = { x, y, rotation, scale };
    byId('cgResult').textContent = `T(${x}, ${y}) · R${rotation}° · S${scale.toFixed(1)}`;
    markSubject('cg', `CG applied translation (${x}, ${y}), rotation ${rotation}°, scale ${scale.toFixed(1)}.`);
    drawGrid();
}

function runOop() {
    const selectedDrone = DRONES[byId('oopDrone').value];
    const payload = numberValue('oopPayload');
    const recipient = byId('oopRecipient').value.trim() || 'Customer';
    const destination = readCoordinate(byId('oopDeliveryPoint').value, 'Delivery site');
    if (!Number.isFinite(payload) || payload <= 0) {
        byId('oopResult').textContent = 'Package weight must be greater than zero.';
        return;
    }
    if (payload > selectedDrone.capacity) {
        byId('oopResult').textContent = `${selectedDrone.name} payload limit is ${selectedDrone.capacity} kg. Delivery rejected.`;
        markSubject('oop', `${selectedDrone.name} rejected ${payload.toFixed(1)} kg package: payload limit exceeded.`);
        return;
    }
    const destinationNode = state.nodes.find(node => node.row === destination.row && node.col === destination.col);
    const destinationName = destinationNode?.label || 'CS-Beta';
    state.drone = { row: destination.row, col: destination.col, model: selectedDrone.model, name: selectedDrone.name };
    state.telemetry.speed = selectedDrone.speed;
    state.telemetry.heading = 45;
    state.telemetry.battery = Math.max(10, state.telemetry.battery - Math.ceil(payload / 2));
    state.entities = [
        'WH-Main · Warehouse',
        `${destinationName} · CustomerSite`,
        `${selectedDrone.model} · ${selectedDrone.name}`,
        `PKG-${String(Math.round(payload * 10)).padStart(3, '0')} · ${payload.toFixed(1)} kg`
    ];
    byId('oopResult').textContent = `Delivered ${payload.toFixed(1)} kg to ${destinationName} for ${recipient}. ${selectedDrone.name} returned status: DELIVERED.`;
    markSubject('oop', `${selectedDrone.name} accepted ${payload.toFixed(1)} kg for ${recipient}; delivery completed at ${destinationName}.`);
    updateDashboard();
}

document.querySelectorAll('.lab-tab').forEach(tab => {
    tab.addEventListener('click', () => selectLab(tab.dataset.tab));
    tab.addEventListener('keydown', event => {
        if (event.key !== 'ArrowLeft' && event.key !== 'ArrowRight') return;
        const tabs = [...document.querySelectorAll('.lab-tab')];
        const direction = event.key === 'ArrowRight' ? 1 : -1;
        const next = tabs[(tabs.indexOf(tab) + direction + tabs.length) % tabs.length];
        next.focus();
        selectLab(next.dataset.tab);
    });
});

byId('addNode').addEventListener('click', addGridNode);
byId('buildRoute').addEventListener('click', () => {
    try { buildRoute(); } catch (error) { logMission(error.message); }
});
byId('advanceDrone').addEventListener('click', advanceDrone);
byId('queueDelivery').addEventListener('click', queueDelivery);
byId('completeDelivery').addEventListener('click', completeNextDelivery);
byId('runCoa').addEventListener('click', runCoa);
byId('applyTransform').addEventListener('click', applyTransform);
byId('resetTransform').addEventListener('click', () => {
    state.transform = { x: 0, y: 0, rotation: 0, scale: 1 };
    byId('cgTranslateX').value = 0;
    byId('cgTranslateY').value = 0;
    byId('cgRotation').value = 0;
    byId('cgScale').value = 1;
    byId('cgResult').textContent = 'Transform reset to identity.';
    drawGrid();
});
byId('runOop').addEventListener('click', () => {
    try { runOop(); } catch (error) { byId('oopResult').textContent = error.message; }
});
byId('resetDemo').addEventListener('click', () => window.location.reload());
window.addEventListener('resize', drawGrid);

document.querySelectorAll('.lab-tab').forEach((tab, index) => {
    tab.tabIndex = index === 0 ? 0 : -1;
});
updateDashboard();
renderDeliveryScheduler();