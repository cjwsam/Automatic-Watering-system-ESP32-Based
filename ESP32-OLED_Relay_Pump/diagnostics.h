/*
 * Diagnostics Web Page for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Defines the HTML and JavaScript for the diagnostics web page, displaying system
 * metrics like heap memory, uptime, and network strength.
 */

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

const char diagnostics_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>System Diagnostics - Smart Watering</title>
    <style>
        body {
            background: #0a0f1a;
            color: #00ffcc;
            font-family: 'Courier New', monospace;
            text-align: center;
            padding: 20px;
            overflow: hidden;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        h1 {
            text-shadow: 0 0 10px #00ffcc, 0 0 20px #ff00ff;
            animation: glitch 1s linear infinite;
        }
        .card {
            background: rgba(0, 255, 204, 0.1);
            border: 1px solid #00ffcc;
            border-radius: 5px;
            padding: 20px;
            margin: 10px 0;
            text-shadow: 0 0 5px #00ffcc;
        }
        .metric {
            font-size: 18px;
            margin: 5px 0;
        }
        button {
            background: #ff00ff;
            color: #0a0f1a;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
            margin: 5px;
            text-shadow: 0 0 5px #ff00ff;
            box-shadow: 0 0 10px #ff00ff;
        }
        button:hover {
            background: #00ffcc;
            text-shadow: 0 0 5px #00ffcc;
            box-shadow: 0 0 10px #00ffcc;
        }
        .nav-links {
            margin-top: 20px;
        }
        .nav-links a {
            color: #ff00ff;
            text-decoration: none;
            margin: 0 10px;
            text-shadow: 0 0 5px #ff00ff;
        }
        .nav-links a:hover {
            color: #00ffcc;
            text-shadow: 0 0 5px #00ffcc;
        }
        @keyframes glitch {
            2%, 64% {
                transform: translate(2px, 0) skew(0deg);
            }
            4%, 60% {
                transform: translate(-2px, 0) skew(0deg);
            }
            62% {
                transform: translate(0, 0) skew(5deg);
            }
        }
        .hack-overlay {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.9);
            color: #ff00ff;
            text-align: center;
            font-size: 24px;
            z-index: 1000;
            animation: hack 2s linear infinite;
        }
        @keyframes hack {
            0% { opacity: 0.5; transform: scale(1); }
            50% { opacity: 1; transform: scale(1.02); }
            100% { opacity: 0.5; transform: scale(1); }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>System Diagnostics</h1>
        <div class="card">
            <div class="metric">Heap Memory: <span id="heap">0</span> bytes</div>
            <div class="metric">Uptime: <span id="uptime">0</span> ms</div>
            <div class="metric">Network Strength: <span id="rssi">0</span> dBm</div>
        </div>
        <button onclick="reboot()">Reboot System</button>
        <button onclick="toggleLed()">Toggle Test LED</button>
        <button onclick="hackSystem()">Hack System</button>
        <div class="nav-links">
            <a href="/">Dashboard</a>
            <a href="/calibrate">Calibrate</a>
            <a href="/logs">Logs</a>
        </div>
    </div>
    <div id="hackOverlay" class="hack-overlay">
        <h2>System Hacked!</h2>
        <p>Access Granted: 0xFF...</p>
    </div>
    <script>
        function updateDiagnostics() {
            fetch('/diagnostics')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('heap').textContent = data.heap;
                    document.getElementById('uptime').textContent = data.uptime;
                    document.getElementById('rssi').textContent = data.rssi;
                })
                .catch(error => console.error('Error fetching diagnostics:', error));
        }

        function reboot() {
            if (confirm('Are you sure you want to reboot the system?')) {
                fetch('/reboot', { method: 'POST' })
                    .then(response => response.json())
                    .then(data => {
                        if (data.status === 'success') {
                            alert('Rebooting...');
                        }
                    })
                    .catch(error => console.error('Error rebooting:', error));
            }
        }

        function toggleLed() {
            fetch('/toggleLed', { method: 'POST' })
                .then(response => response.json())
                .then(data => {
                    console.log('LED toggled:', data);
                })
                .catch(error => console.error('Error toggling LED:', error));
        }

        function hackSystem() {
            const overlay = document.getElementById('hackOverlay');
            overlay.style.display = 'block';
            setTimeout(() => {
                overlay.style.display = 'none';
            }, 3000);
        }

        setInterval(updateDiagnostics, 5000);
        updateDiagnostics();
    </script>
</body>
</html>
)rawliteral";

#endif
