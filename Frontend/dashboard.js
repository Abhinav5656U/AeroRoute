// AeroRoute Simplified JS - Polls JSON files seamlessly
const POLL_INTERVAL = 1000;
let lastRegData = ""; 

// Utility to write to log
function logMission(msg) {
    const logDiv = document.getElementById('missionLog');
    const time = new Date().toLocaleTimeString();
    logDiv.innerHTML += `<div>[${time}] ${msg}</div>`;
    logDiv.scrollTop = logDiv.scrollHeight;
}

// Draw the grid on Canvas
function drawGrid(gridData) {
    const canvas = document.getElementById('dashCanvas');
    const ctx = canvas.getContext('2d');
    const cellW = canvas.width / 10;
    const cellH = canvas.height / 10;
    
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    // Draw Grid Lines (Clean Light mode style)
    ctx.strokeStyle = '#eaeaea';
    for(let i=0; i<=10; i++) {
        ctx.beginPath(); ctx.moveTo(i*cellW, 0); ctx.lineTo(i*cellW, canvas.height); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(0, i*cellH); ctx.lineTo(canvas.width, i*cellH); ctx.stroke();
    }

    if(!gridData || !gridData.cells) return;

    // Map types to colors
    const colors = { 1: '#f2994a', 2: '#56ccf2', 3: '#27ae60', 4: '#eb5757' };
    
    gridData.cells.forEach(cell => {
        if(cell.type > 0 && colors[cell.type]) {
            ctx.fillStyle = colors[cell.type];
            ctx.fillRect(cell.col * cellW + 2, cell.row * cellH + 2, cellW - 4, cellH - 4);
        }
    });
}

// Fetch loop
async function pollData() {
    try {
        const cacheBuster = `?t=${Date.now()}`;
        
        // 1. Grid Data
        const gridRes = await fetch(`data/grid.json${cacheBuster}`);
        if(gridRes.ok) {
            const gridData = await gridRes.json();
            drawGrid(gridData);
            document.getElementById('gridStatus').innerText = "Live";
            document.getElementById('status-s1').className = "fas fa-check-circle status-icon";
        }

        // 2. Telemetry Data
        const telemRes = await fetch(`data/telemetry.json${cacheBuster}`);
        if(telemRes.ok) {
            const telemData = await telemRes.json();
            document.getElementById('heroAlt').innerText = telemData.altitude || "— m";
            document.getElementById('heroSpd').innerText = telemData.speed || "— m/s";
            document.getElementById('heroBat').innerText = telemData.battery || "— %";
            document.getElementById('heroStatus').innerText = telemData.status || "ACTIVE";
            
            document.getElementById('tpPos').innerText = telemData.position || "—";
            document.getElementById('tpHead').innerText = telemData.heading || "—";
            document.getElementById('tpModel').innerText = telemData.model || "—";
        }

        // 3. Register Data (COA)
        const regRes = await fetch(`data/registers.json${cacheBuster}`);
        if(regRes.ok) {
            const regText = await regRes.text();
            if(regText !== lastRegData) { // Only update UI if registers changed
                const regData = JSON.parse(regText);
                ['RAX', 'RBX', 'RCX', 'RDX'].forEach(reg => {
                    if(regData[reg]) {
                        const tr = document.getElementById(`row-${reg}`);
                        document.getElementById(`hex-${reg}`).innerText = regData[reg].hex;
                        document.getElementById(`dec-${reg}`).innerText = regData[reg].dec;
                        
                        // Flash animation for updates
                        tr.classList.add('row-update');
                        setTimeout(() => tr.classList.remove('row-update'), 300);
                    }
                });
                lastRegData = regText;
                document.getElementById('status-s2').className = "fas fa-check-circle status-icon";
            }
        }

        // Update overall online status
        document.getElementById('beDot').className = "status-dot online";
        document.getElementById('beLabel').innerText = "System Online";

    } catch (e) {
        document.getElementById('beDot').className = "status-dot";
        document.getElementById('beLabel').innerText = "System Offline";
    }
}

// Form Submission (Simulates writing to input JSON)
document.getElementById('btnSend').addEventListener('click', () => {
    const payload = {
        model: document.getElementById('droneModel').value,
        route: document.getElementById('routeType').value,
        src: document.getElementById('srcNode').value,
        dst: document.getElementById('dstNode').value
    };
    
    // In a real browser context without a local server, JS cannot directly write files.
    // This logs what the C++ engine WOULD read if we used a blob download or local server post.
    logMission(`Transmitting to input/user_input.json: ${JSON.stringify(payload)}`);
});

// Start polling
setInterval(pollData, POLL_INTERVAL);