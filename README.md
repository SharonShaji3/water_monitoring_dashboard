# Water Quality Monitor

An ESP32-based water quality monitoring system that measures pH, TDS, turbidity and temperature, shows the status of 5 filters, and displays everything on a live web dashboard.

Live dashboard: https://sharonshaji3.github.io/water_monitoring_dashboard/

## About

The ESP32 reads the water quality sensors and sends the data to a cloud database every 15 seconds. The web dashboard reads the data from the database and shows it live, so the water can be monitored from any phone or computer.

## Features

- Live pH, TDS, turbidity and temperature readings
- Status of 5 filters (Normal, Clogged, Low flow, etc.)
- Alerts when a value is out of range or the ESP32 goes offline
- History charts and CSV download
- Login required to view the data

## How it works

ESP32 -> Supabase database -> Web dashboard

## Built with

ESP32, Supabase, HTML/CSS/JavaScript and GitHub Pages.
