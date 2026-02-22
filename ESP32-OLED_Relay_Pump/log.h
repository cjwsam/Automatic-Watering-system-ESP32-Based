/*
 * Log Viewer Web Page for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Defines the HTML and JavaScript for the log viewer web page, allowing users to
 * view and clear system logs.
 */

#ifndef LOG_H
#define LOG_H

const char logs_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>System Logs - Smart Watering</title>
    <style>
        body {
            background: #0a0f1a;
            color: #00ffcc;
            font-family: 'Courier New', monospace;
            text-align: center;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        h1 {
            text-shadow: 0 0 10px #00ffcc, 0 0 20px #ff00ff;
        }
        .card {
            background: rgba(0, 255, 204, 0.1);
            border: 1px solid #00ffcc;
            border-radius: 5px;
            padding: 20px;
            margin: 10px 0;
            text-shadow: 0 0 5px #00ffcc;
        }
        .log-entry {
            text-align: left;
            font-size: 14px;
            margin: 3px 0;
            padding: 3px 5px;
            border-bottom: 1px solid rgba(0, 255, 204, 0.2);
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
        #noLogs {
            color: #888;
            font-style: italic;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>System Logs</h1>
        <div class="card">
            <div id="logContainer">
                <p id="noLogs">Loading logs...</p>
            </div>
        </div>
        <button onclick="refreshLogs()">Refresh Logs</button>
        <button onclick="clearLogs()">Clear Logs</button>
        <div class="nav-links">
            <a href="/">Dashboard</a>
            <a href="/diagnostics">Diagnostics</a>
            <a href="/schedule">Schedules</a>
        </div>
    </div>
    <script>
        function refreshLogs() {
            fetch('/getLogs')
                .then(response => response.json())
                .then(data => {
                    const container = document.getElementById('logContainer');
                    if (data.logs && data.logs.length > 0) {
                        container.innerHTML = '';
                        data.logs.forEach(function(log) {
                            const entry = document.createElement('div');
                            entry.className = 'log-entry';
                            entry.textContent = log;
                            container.appendChild(entry);
                        });
                    } else {
                        container.innerHTML = '<p id="noLogs">No logs available.</p>';
                    }
                })
                .catch(function(error) {
                    console.error('Error fetching logs:', error);
                    document.getElementById('logContainer').innerHTML =
                        '<p style="color: red;">Error loading logs.</p>';
                });
        }

        function clearLogs() {
            if (confirm('Are you sure you want to clear all logs?')) {
                fetch('/clearLogs', { method: 'POST' })
                    .then(response => response.json())
                    .then(data => {
                        if (data.status === 'success') {
                            refreshLogs();
                        }
                    })
                    .catch(function(error) {
                        console.error('Error clearing logs:', error);
                    });
            }
        }

        // Auto-refresh every 5 seconds
        setInterval(refreshLogs, 5000);
        // Initial load
        refreshLogs();
    </script>
</body>
</html>
)rawliteral";

#endif
