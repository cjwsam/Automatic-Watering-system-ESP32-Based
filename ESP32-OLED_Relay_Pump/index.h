/*
 * Main Web Dashboard for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Defines the HTML and JavaScript for the main web dashboard, including water level
 * monitoring, relay control, and calibration interface.
 */

#ifndef INDEX_H
#define INDEX_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Watering Dashboard - Zone 3</title>
    <style>
        body {
            background: linear-gradient(135deg, #1e3a8a, #4c1d95);
            color: white;
            font-family: 'Arial', sans-serif;
            text-align: center;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        .card {
            background: rgba(255, 255, 255, 0.1);
            border-radius: 10px;
            padding: 20px;
            margin: 10px 0;
        }
        .card h2 {
            margin-top: 0;
            font-size: 24px;
        }
        button {
            background: #3b82f6;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
            margin: 5px;
        }
        button:hover {
            background: #2563eb;
        }
        button:disabled {
            background: #6b7280;
            cursor: not-allowed;
        }
        .status {
            font-weight: bold;
        }
        a {
            color: #3b82f6;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
        .calibrate-form {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 10px;
        }
        .calibrate-form input {
            padding: 5px;
            border-radius: 5px;
            border: none;
            width: 100px;
        }
        #calibrateMessage {
            margin-top: 10px;
        }
        #calibrateInstructions {
            max-width: 600px;
            margin: 0 auto;
            text-align: left;
        }
        #countdown {
            margin-top: 10px;
            color: yellow;
        }
        .nav-links {
            margin-top: 20px;
        }
        .nav-links a {
            margin: 0 10px;
        }
    </style>
</head>
<body>
    <div class="container" id="dashboard">
        <h1>Smart Watering Dashboard - Zone 3</h1>
        <div class="card">
            <h2>Water Level</h2>
            <p><span id="waterLevel">0.0%</span></p>
            <p>Status: <span id="waterStatus" class="status">Unknown</span></p>
        </div>
        <div class="card">
            <h2>Relay Status</h2>
            <p>Zone 3 Relay: <span id="relayStatus" class="status">OFF</span></p>
            <button id="toggleButton" onclick="toggleRelay()">Toggle Relay</button>
        </div>
        <div class="card">
            <h2>System Status</h2>
            <p>WiFi: <span id="wifiStatus" class="status">Disconnected</span></p>
            <p>Soil Data: <span id="soilDataStatus" class="status">No Data</span></p>
        </div>
        <div class="nav-links">
            <a href="/calibrate">Calibrate Water Level</a>
            <a href="/logs">View Logs</a>
        </div>
    </div>
    <div class="container" id="calibrate" style="display: none;">
        <h1>Calibrate Water Level</h1>
        <div class="card">
            <h2>Easy Calibration</h2>
            <div id="calibrateInstructions">
                <p>Follow these steps to calibrate your water level:</p>
                <ol>
                    <li>Fill the bucket to a known level (e.g., halfway for 50%).</li>
                    <li>Enter the percentage below and click "Save Point".</li>
                    <li>Change the water level (e.g., fill to full for 100%) and wait 10 seconds.</li>
                    <li>Enter the new percentage and save again. Calibration will complete!</li>
                </ol>
                <p>Current status: <span id="calibrateStatus">Need 2 points</span></p>
            </div>
            <div class="calibrate-form">
                <label>Current Water Level (%):</label>
                <input type="number" id="waterLevelPercent" step="0.1" min="0" max="100" required>
                <button id="savePointButton" onclick="saveCalibration()">Save Point</button>
                <button onclick="resetCalibration()">Reset Calibration</button>
            </div>
            <p id="calibrateMessage"></p>
            <p id="countdown"></p>
        </div>
        <div class="nav-links">
            <a href="/" onclick="showDashboard()">Back to Dashboard</a>
            <a href="/logs">View Logs</a>
        </div>
    </div>

    <script>
        let isToggling = false;
        let isSaving = false;
        let countdown = 0;

        function showDashboard() {
            document.getElementById('dashboard').style.display = 'block';
            document.getElementById('calibrate').style.display = 'none';
        }

        function updateDashboard() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    console.log('Received data:', data);
                    document.getElementById('waterLevel').textContent = data.waterLevel >= 0 ? `${data.waterLevel.toFixed(1)}%` : 'Error';
                    document.getElementById('waterStatus').textContent = data.waterLevel >= 0 ? (data.waterLevel >= 50 ? 'Good' : data.waterLevel >= 20 ? 'Low' : 'Critical') : 'Sensor Error';
                    document.getElementById('waterStatus').style.color = data.waterLevel >= 0 ? (data.waterLevel >= 50 ? 'green' : data.waterLevel >= 20 ? 'yellow' : 'red') : 'red';
                    document.getElementById('relayStatus').textContent = data.relayZ3;
                    document.getElementById('relayStatus').style.color = data.relayZ3 === 'ON' ? 'green' : 'red';
                    document.getElementById('wifiStatus').textContent = data.wifiConnected ? 'Connected' : 'Disconnected';
                    document.getElementById('wifiStatus').style.color = data.wifiConnected ? 'green' : 'red';
                    document.getElementById('soilDataStatus').textContent = data.soilDataReceived ? 'OK' : 'No Data';
                    document.getElementById('soilDataStatus').style.color = data.soilDataReceived ? 'green' : 'red';
                })
                .catch(error => console.error('Error fetching data:', error));
        }

        function toggleRelay() {
            if (isToggling) return;
            isToggling = true;
            document.getElementById('toggleButton').disabled = true;
            const currentStatus = document.getElementById('relayStatus').textContent;
            const newStatus = currentStatus === 'ON' ? 'OFF' : 'ON';
            const payload = JSON.stringify({ relayZ3: newStatus });
            console.log('Sending toggle request to /toggleRelay with payload:', payload);
            fetch('/toggleRelay', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Accept': 'application/json'
                },
                body: payload
            })
                .then(response => {
                    console.log('Response status:', response.status);
                    if (!response.ok) {
                        throw new Error('Network response was not ok: ' + response.statusText);
                    }
                    return response.json();
                })
                .then(data => {
                    console.log('Toggle response:', data);
                    document.getElementById('relayStatus').textContent = data.relayZ3;
                    document.getElementById('relayStatus').style.color = data.relayZ3 === 'ON' ? 'green' : 'red';
                })
                .catch(error => {
                    console.error('Error toggling relay:', error);
                    alert('Failed to toggle relay: ' + error.message);
                })
                .finally(() => {
                    isToggling = false;
                    document.getElementById('toggleButton').disabled = false;
                });
        }

        function saveCalibration() {
            if (isSaving) return;
            const waterLevelPercent = parseFloat(document.getElementById('waterLevelPercent').value);
            if (isNaN(waterLevelPercent) || waterLevelPercent < 0 || waterLevelPercent > 100) {
                document.getElementById('calibrateMessage').textContent = 'Error: Enter a percentage between 0 and 100';
                document.getElementById('calibrateMessage').style.color = 'red';
                return;
            }
            isSaving = true;
            document.getElementById('savePointButton').disabled = true;
            document.getElementById('calibrateMessage').textContent = 'Saving point... Please wait.';
            document.getElementById('calibrateMessage').style.color = 'yellow';
            fetch('/saveCalibration', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ waterLevelPercent })
            })
                .then(response => response.json())
                .then(data => {
                    console.log('Calibration response:', data);
                    document.getElementById('calibrateMessage').textContent = data.message;
                    document.getElementById('calibrateMessage').style.color = data.status === 'success' ? 'green' : 'red';
                    document.getElementById('calibrateStatus').textContent = data.calibrationStatus;
                    if (data.status === 'success') {
                        startCountdown();
                    } else {
                        isSaving = false;
                        document.getElementById('savePointButton').disabled = false;
                    }
                })
                .catch(error => {
                    console.error('Error saving calibration:', error);
                    document.getElementById('calibrateMessage').textContent = 'Error saving calibration';
                    document.getElementById('calibrateMessage').style.color = 'red';
                    isSaving = false;
                    document.getElementById('savePointButton').disabled = false;
                });
        }

        function startCountdown() {
            countdown = 10;
            document.getElementById('countdown').textContent = `Please change the water level and wait ${countdown} seconds...`;
            const interval = setInterval(() => {
                countdown--;
                if (countdown <= 0) {
                    clearInterval(interval);
                    document.getElementById('countdown').textContent = '';
                    document.getElementById('savePointButton').disabled = false;
                    isSaving = false;
                    document.getElementById('calibrateMessage').textContent += ' Now set the next water level and save.';
                } else {
                    document.getElementById('countdown').textContent = `Please change the water level and wait ${countdown} seconds...`;
                }
            }, 1000);
        }

        function resetCalibration() {
            fetch('/resetCalibration', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: '{}'
            })
                .then(response => response.json())
                .then(data => {
                    console.log('Reset response:', data);
                    document.getElementById('calibrateMessage').textContent = 'Calibration reset successfully!';
                    document.getElementById('calibrateMessage').style.color = 'green';
                    document.getElementById('calibrateStatus').textContent = 'Need 2 points';
                    countdown = 0;
                    document.getElementById('countdown').textContent = '';
                    document.getElementById('savePointButton').disabled = false;
                    isSaving = false;
                })
                .catch(error => {
                    console.error('Error resetting calibration:', error);
                    document.getElementById('calibrateMessage').textContent = 'Error resetting calibration';
                    document.getElementById('calibrateMessage').style.color = 'red';
                });
        }

        // Update dashboard every 5 seconds
        setInterval(updateDashboard, 5000);
        // Initial update
        updateDashboard();

        // Show calibrate page if URL is /calibrate
        if (window.location.pathname === '/calibrate') {
            document.getElementById('dashboard').style.display = 'none';
            document.getElementById('calibrate').style.display = 'block';
            // Fetch initial calibration status
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('calibrateStatus').textContent = data.calibrationStatus;
                });
        }
    </script>
</body>
</html>
)rawliteral";

#endif
