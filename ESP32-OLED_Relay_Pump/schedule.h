/*
 * Scheduling Web Page for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Defines the HTML and JavaScript for the scheduling web page, allowing users to
 * set automated watering schedules for different zones.
 */

#ifndef SCHEDULE_H
#define SCHEDULE_H

const char schedule_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Schedule - Smart Watering</title>
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
        .schedule-form {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 10px;
        }
        .schedule-form input, .schedule-form select {
            padding: 5px;
            border-radius: 5px;
            border: 1px solid #00ffcc;
            background: #1a2a44;
            color: #00ffcc;
            width: 150px;
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
        .timeline {
            margin: 20px 0;
            height: 50px;
            background: rgba(255, 255, 255, 0.05);
            border: 1px solid #00ffcc;
            position: relative;
        }
        .timeline-event {
            position: absolute;
            width: 5px;
            height: 50px;
            background: #ff00ff;
            box-shadow: 0 0 10px #ff00ff;
            animation: pulse 1s infinite;
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
        @keyframes pulse {
            0% { transform: scaleY(1); }
            50% { transform: scaleY(1.2); }
            100% { transform: scaleY(1); }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Smart Schedule</h1>
        <div class="card">
            <h2>Add Watering Schedule</h2>
            <div class="schedule-form">
                <select id="zone">
                    <option value="zone3">Zone 3</option>
                    <option value="zone4">Zone 4</option>
                </select>
                <input type="time" id="startTime" required>
                <input type="number" id="duration" placeholder="Duration (min)" min="1" max="60" required>
                <button onclick="addSchedule()">Add Schedule</button>
            </div>
