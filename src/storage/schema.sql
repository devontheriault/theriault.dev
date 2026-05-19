CREATE TABLE IF NOT EXISTS visits (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ip TEXT,
    country TEXT,
    city TEXT,
    user_agent TEXT,
    timestamp DATETIME,
    browser TEXT,
    device_type TEXT,
    os TEXT,
    entry_page TEXT,
    exit_page TEXT
);
