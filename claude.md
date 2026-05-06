# CLAUDE.md — Visitor Tracker (C + Raspberry Pi)

## 📌 Project Overview

This project is a lightweight, self-hosted web analytics server written in C. It tracks visitor activity (IP-based), derives approximate geographic data, and exposes aggregated statistics via a simple HTTP API. A minimal frontend visualizes this data (e.g., maps, charts, live visitors).

The goal is to demonstrate:

* Systems-level programming in C
* Backend architecture (routing, services, storage separation)
* Data collection and aggregation
* Simple data visualization integration
* Self-hosting on low-resource hardware (Raspberry Pi)

---

## 🧱 Project Structure

```
visitor-tracker/
│
├── src/                    # Core C source code
│   ├── main.c             # Entry point (bootstraps server)
│   ├── server.c/h         # HTTP server setup + lifecycle
│   ├── router.c/h         # Route matching + dispatch
│   │
│   ├── handlers/          # HTTP route handlers
│   │   ├── visit.c/h      # Logs a visit
│   │   ├── stats.c/h      # Returns aggregated stats (JSON)
│   │   ├── health.c/h     # Simple health check endpoint
│   │
│   ├── services/          # Business logic layer
│   │   ├── logger.c/h     # Responsible for recording visits
│   │   ├── geo.c/h        # IP → geo lookup
│   │   ├── analytics.c/h  # Aggregation logic (counts, trends)
│   │
│   ├── storage/           # Data access layer
│   │   ├── db.c/h         # SQLite interface (preferred)
│   │   ├── file.c/h       # Optional file-based logging fallback
│   │   ├── schema.sql     # Database schema
│   │
│   └── utils/             # Shared helpers
│       ├── json.c/h       # JSON serialization helpers
│       ├── time.c/h       # Timestamp utilities
│       ├── net.c/h        # IP extraction, request parsing helpers
│
├── web/                   # Static frontend served by the server
│   ├── index.html
│   ├── js/
│   │   ├── app.js         # Fetches stats and updates UI
│   │   ├── globe.js       # Globe/map visualization
│   ├── css/
│   │   ├── styles.css
│
├── data/                  # Runtime data (ignored in git)
│   ├── visits.db          # SQLite database
│   ├── logs.txt           # Optional raw logs
│
├── scripts/               # Dev and deployment scripts
│   ├── build.sh
│   ├── run.sh
│
├── Makefile               # Build configuration
├── README.md              # Public project description
├── CLAUDE.md              # This file
└── .gitignore
```

---

## 🔁 High-Level Architecture

The system follows a simple layered architecture:

```
HTTP Request
    ↓
server.c (connection handling)
    ↓
router.c (route matching)
    ↓
handlers/* (endpoint logic)
    ↓
services/* (business logic)
    ↓
storage/* (data persistence)
```

### Example Flow

**Visitor hits `/`:**

1. `server.c` receives request
2. `router.c` routes to `visit.c`
3. `visit.c`:

   * Extracts IP
   * Calls `geo.c` for location
   * Calls `logger.c` to persist
4. Response returned (HTML or redirect)

**Frontend requests `/stats`:**

1. Routed to `stats.c`
2. `analytics.c` computes aggregates
3. JSON response returned

---

## 🌐 API Design

### Endpoints

```
GET /           → Serves frontend (index.html)
GET /visit      → Logs a visit (optional explicit endpoint)
GET /stats      → Returns aggregated statistics (JSON)
GET /health     → Health check endpoint
```

### Example `/stats` Response

```json
{
  "total_visits": 1234,
  "unique_countries": 12,
  "top_countries": [
    { "country": "Canada", "count": 500 },
    { "country": "USA", "count": 400 }
  ],
  "visits_over_time": [
    { "timestamp": "2026-05-01T12:00:00Z", "count": 20 }
  ]
}
```

---

## 🗄️ Data Storage

### Preferred: SQLite

**Schema:**

```sql
CREATE TABLE visits (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ip TEXT,
    country TEXT,
    city TEXT,
    timestamp DATETIME
);
```

---

## 🌍 Geo Location Strategy

Avoid per-request external API calls.

Recommended:

* Use a local IP-to-geo database (e.g., MaxMind GeoLite2)
* Perform lookups in `geo.c`

---

## 🧠 Design Principles

* **Separation of concerns**
  HTTP handling, business logic, and storage are isolated.

* **Stateless request handling**
  Each request is processed independently.

* **Append-first, aggregate-later**
  Raw visit data is stored, then aggregated on demand.

* **Minimal dependencies**
  Keep the project lightweight and portable.

---

## ⚙️ Build & Run

### Build

```

```
Bash file

### Run

```
./server
```

Server runs on:

```
http://localhost:8080
```

---

## 🧪 Future Enhancements

* Live visitor tracking (polling or WebSockets)
* Session tracking (group visits by user/IP over time)
* Rate limiting / bot detection
* Data export (CSV/JSON)
* Advanced analytics (retention, trends)
* Frontend improvements (interactive globe, filters)

---

## 🛡️ Privacy Considerations

* IP addresses are considered personal data in some jurisdictions
* Don't store raw IPs

---


## 🧭 Development Notes

* Implement `/stats` early for quick feedback loop
* Keep handlers thin; push logic into services
* Build incrementally — avoid premature complexity

---

## ✅ Definition of Done (MVP)

* [ ] HTTP server running in C
* [ ] Visits logged with timestamp + IP
* [ ] Geo lookup integrated
* [ ] `/stats` endpoint returns aggregated data
* [ ] Basic frontend displays stats

---

This file is intended to guide both human contributors and AI assistants working on the project.

