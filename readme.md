# theriault.dev

Self-hosted real-time visitor analytics · Custom HTTP server in C · SQLite · Vanilla JS

A lightweight analytics platform built from scratch in C that tracks live visitors, geolocation data, and request statistics in real time.

This project started as a side project to better understand low-level networking, HTTP parsing, sockets, request handling, and backend architecture without relying on frameworks.

---

## Features

* Custom HTTP server written in C
* Real-time visitor tracking
* IP geolocation lookup
* SQLite-backed persistence layer
* Live frontend dashboard using Vanilla JS
* Country and city analytics
* Click-to-filter geographic statistics
* Self-hosted deployment

---

## Tech Stack

* **Language:** C
* **Database:** SQLite
* **Frontend:** Vanilla JavaScript, HTML, CSS
* **Networking:** POSIX sockets
* **Deployment:** Self-hosted Linux / Raspberry Pi

---

## What It Tracks

Each visit can include:

* IP address
* Country
* City
* Latitude / Longitude
* User-Agent
* Timestamp

The dashboard aggregates this into:

* Top countries
* Top cities
* Live visitor counts
* Historical traffic data
* Geographic activity visualizations

---

## Goals of the Project

This project exists mainly as:

* a systems programming exercise
* a networking/HTTP learning project
* an analytics sandbox
* a lightweight self-hosted service
* a fun excuse to write more C

---

## Disclaimer

This project is intentionally lightweight and experimental. 
It is mostly an excuse to build weird internet infrastructure for fun.

