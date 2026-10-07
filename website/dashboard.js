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
        traffic: { row: 9, col: 0, path: [], active: false },
        mission: {
            active: false,
            paused: false,
            status: 'READY',
            path: [],
            distance: 0,
            lastFrame: 0,
            trafficDistance: 0,
            trafficDirection: 1,
            subject: 'pl',
            completionMessage: '',
            onComplete: null
        },
        coaRun: { active: false, paused: false, lastFrame: 0, elapsed: 0, step: 0, steps: [] },
        cgRun: { active: false, paused: false, lastFrame: 0, elapsed: 0, duration: 1500, from: null, to: null },
        coaPc: 4096,
        flags: { ZF: 0, CF: 0, SF: 0, OF: 0, PF: 0 },
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
    byId('heroStatus').textContent = state.telemetry.battery <= 20 ? 'LOW BATTERY' : state.mission.status;
    byId('tpPos').textContent = `${state.drone.row.toFixed(1)}, ${state.drone.col.toFixed(1)}`;
    byId('tpTraffic').textContent = `${state.traffic.row.toFixed(1)}, ${state.traffic.col.toFixed(1)}`;
    byId('tpHead').textContent = `${Math.round(state.telemetry.heading).toString().padStart(3, '0')}°`;
    byId('tpModel').textContent = state.drone.model;
    byId('tpRoute').textContent = state.routeType;
    byId('tpMission').textContent = state.mission.status;
    byId('entityList').textContent = state.entities.join(' · ');
    byId('gridStatus').textContent = `${state.nodes.length} NODES / DEMO`;
    byId('llVisual').textContent = routeDescription();
    byId('llVisual').title = 'Flight path: ' +
        state.route.map(point => `(${point.row},${point.col})`).join(' -> ');

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

    const flightPath = state.mission.path;
    if (flightPath.length > 1) {
        const distance = Math.min(state.mission.distance, flightPath.length - 1);
        const lastWaypoint = Math.floor(distance);
        const visiblePath = flightPath.slice(0, lastWaypoint + 1);
        if (distance > lastWaypoint && lastWaypoint < flightPath.length - 1) {
            visiblePath.push(positionAlongPath(flightPath, distance));
        }
        const points = visiblePath.map(point => transformedPoint(point.row, point.col, board));
        context.beginPath();
        points.forEach((point, index) => index === 0 ? context.moveTo(point.x, point.y) : context.lineTo(point.x, point.y));
        context.strokeStyle = '#167a72';
        context.lineWidth = Math.max(2, cell * 0.07);
        context.setLineDash([cell * 0.18, cell * 0.12]);
        context.stroke();
        context.setLineDash([]);
    }

    if (state.traffic.path.length > 1) {
        const points = state.traffic.path.map(point => transformedPoint(point.row, point.col, board));
        context.beginPath();
        points.forEach((point, index) => index === 0 ? context.moveTo(point.x, point.y) : context.lineTo(point.x, point.y));
        context.strokeStyle = '#cf6a36';
        context.lineWidth = Math.max(1.5, cell * 0.045);
        context.setLineDash([cell * 0.12, cell * 0.12]);
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

    const trafficPoint = transformedPoint(state.traffic.row, state.traffic.col, board);
    context.strokeStyle = '#bd5c2c';
    context.lineWidth = Math.max(2, cell * 0.08);
    context.beginPath();
    context.moveTo(trafficPoint.x - cell * 0.18, trafficPoint.y);
    context.lineTo(trafficPoint.x + cell * 0.18, trafficPoint.y);
    context.moveTo(trafficPoint.x, trafficPoint.y - cell * 0.18);
    context.lineTo(trafficPoint.x, trafficPoint.y + cell * 0.18);
    context.stroke();
    context.fillStyle = '#df7139';
    context.beginPath();
    context.arc(trafficPoint.x, trafficPoint.y, Math.max(4, cell * 0.12), 0, Math.PI * 2);
    context.fill();
    context.strokeStyle = '#fff';
    context.lineWidth = 2;
    context.stroke();

    const dronePoint = transformedPoint(state.drone.row, state.drone.col, board);
    context.save();
    context.translate(dronePoint.x, dronePoint.y);
    context.rotate((state.drone.heading || 0) * Math.PI / 180);
    context.fillStyle = '#173f63';
    context.beginPath();
    context.moveTo(0, -Math.max(7, cell * 0.2));
    context.lineTo(Math.max(5, cell * 0.14), Math.max(5, cell * 0.14));
    context.lineTo(0, Math.max(2, cell * 0.06));
    context.lineTo(-Math.max(5, cell * 0.14), Math.max(5, cell * 0.14));
    context.closePath();
    context.fill();
    context.restore();
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

function findGridPath(start, destination) {
    const cellKey = (row, col) => row * 10 + col;
    const startKey = cellKey(start.row, start.col);
    const destinationKey = cellKey(destination.row, destination.col);
    const previous = Array(100).fill(-1);
    const visited = Array(100).fill(false);
    const pending = [startKey];
    const directions = [[0, 1], [1, 0], [0, -1], [-1, 0]];
    visited[startKey] = true;

    for (let head = 0; head < pending.length; head += 1) {
        const key = pending[head];
        if (key === destinationKey) break;
        const row = Math.floor(key / 10);
        const col = key % 10;
        for (const [rowStep, colStep] of directions) {
            const nextRow = row + rowStep;
            const nextCol = col + colStep;
            if (nextRow < 0 || nextRow > 9 || nextCol < 0 || nextCol > 9) continue;
            const nextKey = cellKey(nextRow, nextCol);
            if (visited[nextKey] || state.nodes.some(node => node.type === 4 && node.row === nextRow && node.col === nextCol)) continue;
            visited[nextKey] = true;
            previous[nextKey] = key;
            pending.push(nextKey);
        }
    }

    if (!visited[destinationKey]) return [];
    const path = [];
    for (let key = destinationKey; key !== -1; key = previous[key]) {
        path.push({ row: Math.floor(key / 10), col: key % 10 });
        if (key === startKey) break;
    }
    return path.reverse();
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
    const path = findGridPath({ row: 0, col: 0 }, destination);
    if (path.length === 0) {
        state.mission.status = 'NO SAFE ROUTE';
        byId('missionStatus').textContent = state.mission.status;
        logMission(`PL could not find an obstacle-free route to ${destinationLabel}.`);
        return false;
    }
    state.route = path.map((point, index) => {
        const node = state.nodes.find(item => item.row === point.row && item.col === point.col);
        const isStart = index === 0;
        const isDestination = index === path.length - 1;
        return {
            ...point,
            label: isStart ? 'WH-Main' : isDestination ? destinationLabel : node?.label || `WP-${index}`
        };
    });
    state.routeType = byId('plRouteType').value;
    state.routeIndex = 0;
    state.drone.row = 0;
    state.drone.col = 0;
    state.traffic.path = findGridPath({ row: 9, col: 0 }, { row: 9, col: 9 });
    state.traffic.row = 9;
    state.traffic.col = 0;
    state.mission = { active: false, paused: false, status: 'READY', path: state.route, distance: 0, lastFrame: 0, trafficDistance: 0, trafficDirection: 1 };
    renderMissionControls();
    markSubject('pl', `Built ${state.routeType} route with ${state.route.length - 1} flight steps around ${state.nodes.filter(node => node.type === 4).length} obstacles.`);
    updateDashboard();
    return true;
}

function advanceDrone() {
    if (state.mission.active) {
        logMission('Pause the delivery before using manual waypoint advance.');
        return;
    }
    if (state.routeType === 'CLL') state.routeIndex = (state.routeIndex + 1) % state.route.length;
    else if (state.routeIndex < state.route.length - 1) state.routeIndex += 1;
    else {
        logMission('PL route is complete. Build a new route to dispatch again.');
        return;
    }
    const waypoint = state.route[state.routeIndex];
    state.drone.row = waypoint.row;
    state.drone.col = waypoint.col;
    state.mission.distance = state.routeIndex;
    state.telemetry.battery = Math.max(0, state.telemetry.battery - 2);
    markSubject('pl', `Drone advanced to ${waypoint.label} (${waypoint.row},${waypoint.col}).`);
    updateDashboard();
}

function positionAlongPath(path, distance) {
    if (path.length === 0) return { row: 0, col: 0 };
    if (path.length === 1) return { ...path[0] };
    const boundedDistance = Math.max(0, Math.min(path.length - 1, distance));
    const segment = Math.min(path.length - 2, Math.floor(boundedDistance));
    const fraction = boundedDistance - segment;
    const start = path[segment];
    const end = path[segment + 1];
    return {
        row: start.row + (end.row - start.row) * fraction,
        col: start.col + (end.col - start.col) * fraction
    };
}

function renderMissionControls() {
    const mission = state.mission;
    const percent = mission.path.length > 1
        ? Math.round(mission.distance / (mission.path.length - 1) * 100) : 0;
    byId('missionStatus').textContent = mission.paused ? 'PAUSED' : mission.status;
    byId('missionProgress').value = percent;
    byId('missionProgressText').textContent = `${percent}%`;
    byId('startDelivery').disabled = mission.active;
    byId('pauseDelivery').disabled = !mission.active || mission.subject !== 'pl';
    byId('pauseDelivery').innerHTML = mission.paused
        ? '<i class="fas fa-play" aria-hidden="true"></i> Resume'
        : '<i class="fas fa-pause" aria-hidden="true"></i> Pause';
    byId('advanceDrone').disabled = mission.active;
    byId('runOop').disabled = mission.active;
    byId('pauseOop').disabled = !mission.active || mission.subject !== 'oop';
    byId('oopMissionStatus').textContent = mission.subject === 'oop' && mission.paused ? 'PAUSED' : mission.subject === 'oop' ? mission.status : 'READY';
    byId('oopProgress').value = mission.subject === 'oop' ? percent : 0;
    byId('oopProgressText').textContent = `${mission.subject === 'oop' ? percent : 0}%`;
}

function startFlightSimulation(subject, path, model, name, completionMessage, onComplete) {
    if (!path || path.length < 2) return false;
    state.route = path;
    state.drone = { ...state.drone, row: path[0].row, col: path[0].col, model, name, heading: 0 };
    state.traffic.path = findGridPath({ row: 9, col: 0 }, { row: 9, col: 9 });
    state.traffic.row = 9;
    state.traffic.col = 0;
    state.traffic.active = state.traffic.path.length > 1;
    state.mission = {
        active: true,
        paused: false,
        status: 'FLYING',
        path,
        distance: 0,
        lastFrame: 0,
        trafficDistance: 0,
        trafficDirection: 1,
        subject,
        completionMessage,
        onComplete
    };
    state.telemetry.battery = 100;
    renderMissionControls();
    updateDashboard();
    requestAnimationFrame(animateDelivery);
    return true;
}

function startDelivery() {
    if (state.mission.active || !buildRoute()) return;
    state.mission.active = true;
    const path = state.route;
    const model = state.drone.model;
    const name = state.drone.name;
    startFlightSimulation('pl', path, model, name,
        `Delivery reached ${path.at(-1).label}.`, null);
    markSubject('pl', `Delivery drone launched on a ${path.length - 1}-cell route. Traffic drone is patrolling.`);
}

function animateDelivery(timestamp) {
    const mission = state.mission;
    if (!mission.active || mission.paused) return;
    if (mission.lastFrame === 0) mission.lastFrame = timestamp;
    const deltaSeconds = Math.min((timestamp - mission.lastFrame) / 1000, 0.05);
    mission.lastFrame = timestamp;

    const trafficLength = Math.max(0, mission.path.length - 1);
    const patrolLength = Math.max(0, state.traffic.path.length - 1);
    const trafficStep = Number(byId('flightSpeed').value) * 0.65 * deltaSeconds;
    let nextTrafficDirection = mission.trafficDirection;
    let nextTrafficDistance = mission.trafficDistance + mission.trafficDirection * trafficStep;
    if (nextTrafficDistance >= patrolLength) {
        nextTrafficDistance = patrolLength;
        nextTrafficDirection = -1;
    } else if (nextTrafficDistance <= 0) {
        nextTrafficDistance = 0;
        nextTrafficDirection = 1;
    }
    let nextTraffic = positionAlongPath(state.traffic.path, nextTrafficDistance);

    const flightStep = Number(byId('flightSpeed').value) * 1.2 * deltaSeconds;
    const nextDistance = Math.min(trafficLength, mission.distance + flightStep);
    const nextDrone = positionAlongPath(mission.path, nextDistance);
    const trafficCanMove = !state.traffic.active ||
        Math.hypot(state.drone.row - nextTraffic.row, state.drone.col - nextTraffic.col) >= 0.7;
    if (!trafficCanMove) {
        nextTrafficDistance = mission.trafficDistance;
        nextTrafficDirection = mission.trafficDirection;
        nextTraffic = { row: state.traffic.row, col: state.traffic.col };
    }
    const deliveryCanMove = !state.traffic.active ||
        Math.hypot(nextDrone.row - nextTraffic.row, nextDrone.col - nextTraffic.col) >= 0.7;
    if (trafficCanMove) {
        mission.trafficDistance = nextTrafficDistance;
        mission.trafficDirection = nextTrafficDirection;
        Object.assign(state.traffic, nextTraffic);
    }

    if (!deliveryCanMove) {
        mission.status = 'YIELDING TO TRAFFIC';
    } else {
        const previous = state.drone;
        mission.distance = nextDistance;
        Object.assign(state.drone, nextDrone);
        state.drone.heading = Math.atan2(nextDrone.row - previous.row, nextDrone.col - previous.col) * 180 / Math.PI;
        mission.status = 'FLYING';
        state.telemetry.battery = Math.max(0, 100 - mission.distance * 0.6);
    }

    if (mission.distance >= trafficLength) {
        mission.active = false;
        mission.status = 'DELIVERED';
        state.traffic.active = false;
        state.telemetry.battery = Math.max(0, state.telemetry.battery - 2);
        if (mission.onComplete) mission.onComplete();
        markSubject(mission.subject, mission.completionMessage || `Package delivered to ${mission.path.at(-1).label}.`);
    }
    renderMissionControls();
    updateDashboard();
    if (mission.active) requestAnimationFrame(animateDelivery);
}

function toggleDeliveryPause(subject) {
    if (!state.mission.active || state.mission.subject !== subject) return;
    state.mission.paused = !state.mission.paused;
    if (!state.mission.paused) {
        state.mission.lastFrame = 0;
        requestAnimationFrame(animateDelivery);
    }
    renderMissionControls();
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

function renderCoaControls() {
    const run = state.coaRun;
    const percent = run.steps.length ? Math.round(run.step / run.steps.length * 100) : 0;
    byId('coaRunStatus').textContent = run.paused ? 'PAUSED' : run.active ? 'RUNNING' : percent === 100 ? 'COMPLETE' : 'READY';
    byId('coaProgress').value = percent;
    byId('coaProgressText').textContent = `${percent}%`;
    byId('runCoa').disabled = run.active;
    byId('pauseCoa').disabled = !run.active;
    byId('executeCoaInstruction').disabled = run.active;
}

function runCoa() {
    if (state.coaRun.active) return;
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
        const deltaRow = to.row - from.row;
        const deltaCol = to.col - from.col;
        const distance = Math.hypot(deltaRow, deltaCol);
        const steps = [
            { register: 'RAX', value: Math.round(altitude), text: `MOV RAX, ${Math.round(altitude)}` },
            { register: 'RBX', value: Math.round(speed * 100), text: `MOV RBX, ${Math.round(speed * 100)}` },
            { register: 'RCX', value: Math.round(heading * 100), text: `MOV RCX, ${Math.round(heading * 100)}` },
            { register: 'RDX', value: Math.round(battery * 100), text: `MOV RDX, ${Math.round(battery * 100)}` },
            { text: `SUB / ADD / SQRT displacement = ${distance.toFixed(2)} cells` }
        ];
        state.coaRun = { active: true, paused: false, lastFrame: 0, elapsed: 0, step: 0, steps, telemetry: { altitude, speed, heading, battery }, result: `Δrow ${deltaRow} · Δcol ${deltaCol} · displacement ${distance.toFixed(2)} cells` };
        state.registers = { RAX: 0, RBX: 0, RCX: 0, RDX: 0 };
        state.telemetry = { altitude: 0, speed: 0, heading: 0, battery: 0 };
        byId('coaResult').textContent = 'Simulation ready · registers will load in instruction order.';
        renderCoaControls();
        requestAnimationFrame(animateCoa);
        markSubject('coa', 'COA instruction simulation started.');
        updateDashboard();
    } catch (error) {
        byId('coaResult').textContent = error.message;
    }
}

function animateCoa(timestamp) {
    const run = state.coaRun;
    if (!run.active || run.paused) return;
    if (run.lastFrame === 0) run.lastFrame = timestamp;
    run.elapsed += Math.min((timestamp - run.lastFrame) / 1000, 0.1);
    run.lastFrame = timestamp;
    if (run.elapsed < 0.55) {
        requestAnimationFrame(animateCoa);
        return;
    }
    run.elapsed = 0;
    const step = run.steps[run.step];
    if (step.register) {
        state.registers[step.register] = step.value;
        if (step.register === 'RAX') state.telemetry.altitude = run.telemetry.altitude;
        if (step.register === 'RBX') state.telemetry.speed = run.telemetry.speed;
        if (step.register === 'RCX') state.telemetry.heading = run.telemetry.heading;
        if (step.register === 'RDX') state.telemetry.battery = run.telemetry.battery;
        state.coaPc += 4;
    }
    byId('coaResult').textContent = step.register
        ? `${step.text} · ${step.register}=${step.value}`
        : run.result;
    byId('coaInstructionResult').textContent = `PC 0x${state.coaPc.toString(16).toUpperCase()} · ${step.text} · ZF=${state.flags.ZF} CF=${state.flags.CF} SF=${state.flags.SF} OF=${state.flags.OF} PF=${state.flags.PF}`;
    run.step += 1;
    if (run.step >= run.steps.length) {
        run.active = false;
        byId('coaResult').textContent = run.result;
        markSubject('coa', 'COA telemetry registers loaded and displacement calculated.');
    }
    renderCoaControls();
    updateDashboard();
    if (run.active) requestAnimationFrame(animateCoa);
}

function toggleCoaPause() {
    const run = state.coaRun;
    if (!run.active) return;
    run.paused = !run.paused;
    byId('pauseCoa').innerHTML = run.paused
        ? '<i class="fas fa-play" aria-hidden="true"></i> Resume'
        : '<i class="fas fa-pause" aria-hidden="true"></i> Pause';
    if (!run.paused) {
        run.lastFrame = 0;
        requestAnimationFrame(animateCoa);
    }
    renderCoaControls();
}

function executeCoaInstruction() {
    if (state.coaRun.active) return;
    const opcode = byId('coaOpcode').value;
    const register = byId('coaRegister').value;
    const operand = Number(byId('coaOperand').value);
    if (!Number.isFinite(operand)) {
        byId('coaInstructionResult').textContent = 'Enter a numeric instruction operand.';
        return;
    }
    const current = state.registers[register];
    let result = operand;
    if (opcode === 'ADD') result = current + operand;
    if (opcode === 'SUB' || opcode === 'CMP') result = current - operand;
    state.flags.ZF = Number(result === 0);
    state.flags.SF = Number(result < 0);
    state.flags.CF = Number((opcode === 'SUB' || opcode === 'CMP') && current < operand);
    state.flags.OF = Number((opcode === 'ADD' && current >= 0 && operand >= 0 && result < 0) || ((opcode === 'SUB' || opcode === 'CMP') && current >= 0 && operand < 0 && result < 0));
    if (opcode !== 'CMP') state.registers[register] = result;
    state.coaPc += 4;
    byId('coaInstructionResult').textContent = `PC 0x${state.coaPc.toString(16).toUpperCase()} · ${opcode} ${register}, ${operand} · ${opcode === 'CMP' ? 'unchanged' : `${register}=${result}`} · ZF=${state.flags.ZF} CF=${state.flags.CF} SF=${state.flags.SF} OF=${state.flags.OF} PF=${state.flags.PF}`;
    markSubject('coa', `Executed ${opcode} ${register}, ${operand}.`);
    updateDashboard();
}

function readTransformTarget() {
    const target = {
        x: numberValue('cgTranslateX'),
        y: numberValue('cgTranslateY'),
        rotation: numberValue('cgRotation'),
        scale: numberValue('cgScale')
    };
    if (!Object.values(target).every(Number.isFinite) || target.scale < 0.4 || target.scale > 2.5) {
        throw new Error('Scale must be between 0.4 and 2.5.');
    }
    return target;
}

function applyTransform() {
    try {
        state.transform = readTransformTarget();
        byId('cgResult').textContent = `T(${state.transform.x}, ${state.transform.y}) · R${state.transform.rotation}° · S${state.transform.scale.toFixed(1)}`;
        markSubject('cg', 'CG transform applied immediately.');
        drawGrid();
    } catch (error) {
        byId('cgResult').textContent = error.message;
    }
}

function playGraphics() {
    if (state.cgRun.active) return;
    try {
        state.cgRun = { active: true, paused: false, lastFrame: 0, elapsed: 0, duration: 1500, from: { ...state.transform }, to: readTransformTarget() };
        renderCgControls();
        requestAnimationFrame(animateGraphics);
        markSubject('cg', 'CG translation, rotation, and scaling animation started.');
    } catch (error) {
        byId('cgResult').textContent = error.message;
    }
}

function renderCgControls() {
    const run = state.cgRun;
    const percent = run.active || run.elapsed > 0 ? Math.min(100, Math.round(run.elapsed / run.duration * 100)) : 0;
    byId('cgRunStatus').textContent = run.paused ? 'PAUSED' : run.active ? 'ANIMATING' : percent === 100 ? 'COMPLETE' : 'READY';
    byId('cgProgress').value = percent;
    byId('cgProgressText').textContent = `${percent}%`;
    byId('playCg').disabled = run.active;
    byId('pauseCg').disabled = !run.active;
}

function animateGraphics(timestamp) {
    const run = state.cgRun;
    if (!run.active || run.paused) return;
    if (run.lastFrame === 0) run.lastFrame = timestamp;
    run.elapsed = Math.min(run.duration, run.elapsed + Math.min(timestamp - run.lastFrame, 50));
    run.lastFrame = timestamp;
    const progress = run.elapsed / run.duration;
    const eased = progress * progress * (3 - 2 * progress);
    state.transform = {
        x: run.from.x + (run.to.x - run.from.x) * eased,
        y: run.from.y + (run.to.y - run.from.y) * eased,
        rotation: run.from.rotation + (run.to.rotation - run.from.rotation) * eased,
        scale: run.from.scale + (run.to.scale - run.from.scale) * eased
    };
    byId('cgResult').textContent = `T(${state.transform.x.toFixed(1)}, ${state.transform.y.toFixed(1)}) · R${state.transform.rotation.toFixed(0)}° · S${state.transform.scale.toFixed(2)}`;
    if (run.elapsed >= run.duration) {
        run.active = false;
        markSubject('cg', 'CG transform animation complete.');
    }
    renderCgControls();
    drawGrid();
    if (run.active) requestAnimationFrame(animateGraphics);
}

function toggleCgPause() {
    if (!state.cgRun.active) return;
    state.cgRun.paused = !state.cgRun.paused;
    byId('pauseCg').innerHTML = state.cgRun.paused
        ? '<i class="fas fa-play" aria-hidden="true"></i> Resume'
        : '<i class="fas fa-pause" aria-hidden="true"></i> Pause';
    if (!state.cgRun.paused) {
        state.cgRun.lastFrame = 0;
        requestAnimationFrame(animateGraphics);
    }
    renderCgControls();
}

function clipRoute() {
    const transformPoint = point => {
        const x = (point.col - 4.5) * state.transform.scale;
        const y = (point.row - 4.5) * state.transform.scale;
        const angle = state.transform.rotation * Math.PI / 180;
        return {
            x: 4.5 + x * Math.cos(angle) - y * Math.sin(angle) + state.transform.x,
            y: 4.5 + x * Math.sin(angle) + y * Math.cos(angle) - state.transform.y
        };
    };
    const code = point => (point.x < 0 ? 1 : point.x > 9 ? 2 : 0) | (point.y < 0 ? 4 : point.y > 9 ? 8 : 0);
    const clip = (first, second) => {
        let a = { ...first };
        let b = { ...second };
        for (let attempt = 0; attempt < 8; attempt += 1) {
            const codeA = code(a);
            const codeB = code(b);
            if (!(codeA | codeB)) return true;
            if (codeA & codeB) return false;
            const outside = codeA || codeB;
            let x;
            let y;
            if (outside & 8) { x = a.x + (b.x - a.x) * (9 - a.y) / (b.y - a.y); y = 9; }
            else if (outside & 4) { x = a.x + (b.x - a.x) * (0 - a.y) / (b.y - a.y); y = 0; }
            else if (outside & 2) { y = a.y + (b.y - a.y) * (9 - a.x) / (b.x - a.x); x = 9; }
            else { y = a.y + (b.y - a.y) * (0 - a.x) / (b.x - a.x); x = 0; }
            if (outside === codeA) a = { x, y };
            else b = { x, y };
        }
        return false;
    };
    let visible = 0;
    let rejected = 0;
    for (let index = 0; index < state.route.length - 1; index += 1) {
        if (clip(transformPoint(state.route[index]), transformPoint(state.route[index + 1]))) visible += 1;
        else rejected += 1;
    }
    byId('cgClipResult').textContent = `${visible} visible route segments · ${rejected} rejected outside viewport · bounds 0–9`;
    markSubject('cg', `Cohen-Sutherland clipped route: ${visible} visible, ${rejected} rejected segments.`);
}

function runOop() {
    if (state.mission.active) return;
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
    const path = findGridPath({ row: 0, col: 0 }, destination).map((point, index, points) => {
        const node = state.nodes.find(item => item.row === point.row && item.col === point.col);
        return { ...point, label: index === 0 ? 'WH-Main' : index === points.length - 1 ? destinationName : node?.label || `WP-${index}` };
    });
    if (path.length < 2) {
        byId('oopResult').textContent = 'No obstacle-free delivery path is available.';
        return;
    }
    state.telemetry.speed = selectedDrone.speed;
    state.telemetry.heading = 45;
    state.telemetry.battery = 100;
    state.entities = [
        'WH-Main · Warehouse',
        `${destinationName} · CustomerSite`,
        `${selectedDrone.model} · ${selectedDrone.name}`,
        `PKG-${String(Math.round(payload * 10)).padStart(3, '0')} · ${payload.toFixed(1)} kg · LOADED`
    ];
    byId('oopResult').textContent = `Package loaded · ${selectedDrone.name} en route to ${destinationName} for ${recipient}.`;
    state.mission.path = path;
    state.route = path;
    state.mission.distance = 0;
    startFlightSimulation('oop', path, selectedDrone.model, selectedDrone.name,
        `Delivered ${payload.toFixed(1)} kg to ${destinationName} for ${recipient}.`, () => {
            state.entities[3] = `PKG-${String(Math.round(payload * 10)).padStart(3, '0')} · ${payload.toFixed(1)} kg · DELIVERED`;
            byId('oopResult').textContent = `Delivered ${payload.toFixed(1)} kg to ${destinationName} for ${recipient}. ${selectedDrone.name}: DELIVERED.`;
        });
    markSubject('oop', `${selectedDrone.name} loaded ${payload.toFixed(1)} kg and launched toward ${destinationName}.`);
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
byId('startDelivery').addEventListener('click', startDelivery);
byId('pauseDelivery').addEventListener('click', () => toggleDeliveryPause('pl'));
byId('flightSpeed').addEventListener('input', () => {
    byId('flightSpeedValue').textContent = `${Number(byId('flightSpeed').value).toFixed(1)}x`;
});
byId('queueDelivery').addEventListener('click', queueDelivery);
byId('completeDelivery').addEventListener('click', completeNextDelivery);
byId('runCoa').addEventListener('click', runCoa);
byId('pauseCoa').addEventListener('click', toggleCoaPause);
byId('executeCoaInstruction').addEventListener('click', executeCoaInstruction);
byId('applyTransform').addEventListener('click', applyTransform);
byId('playCg').addEventListener('click', playGraphics);
byId('pauseCg').addEventListener('click', toggleCgPause);
byId('clipRoute').addEventListener('click', clipRoute);
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
byId('pauseOop').addEventListener('click', () => toggleDeliveryPause('oop'));
byId('resetDemo').addEventListener('click', () => window.location.reload());
window.addEventListener('resize', drawGrid);

document.querySelectorAll('.lab-tab').forEach((tab, index) => {
    tab.tabIndex = index === 0 ? 0 : -1;
});
updateDashboard();
renderDeliveryScheduler();
renderMissionControls();
renderCoaControls();
renderCgControls();