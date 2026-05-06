#!/usr/bin/env bash
# Usage: ./scripts/import_geo.sh /path/to/IP2LOCATION-LITE-DB5.CSV
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 /path/to/IP2LOCATION-LITE-DB5.CSV" >&2
    exit 1
fi

CSV="$1"
DB="$(dirname "$0")/../data/visits.db"

sqlite3 "$DB" <<SQL
CREATE TABLE IF NOT EXISTS ip_ranges (
  start_ip INTEGER NOT NULL,
  end_ip   INTEGER NOT NULL,
  country  TEXT NOT NULL,
  city     TEXT NOT NULL
);
DELETE FROM ip_ranges;
CREATE TEMP TABLE _import (
  ip_from TEXT, ip_to TEXT, cc TEXT, country TEXT, region TEXT, city TEXT, latitude TEXT, longitude TEXT
);
.separator ","
.import "$CSV" _import
INSERT INTO ip_ranges(start_ip, end_ip, country, city, latitude, longitude)
  SELECT CAST(REPLACE(ip_from,    '"', '') AS INTEGER),
         CAST(REPLACE(ip_to,      '"', '') AS INTEGER),
         REPLACE(country,  '"', ''),
         REPLACE(city,     '"', ''),
         CAST(REPLACE(latitude,  '"', '') AS REAL),
         CAST(REPLACE(longitude, '"', '') AS REAL)
  FROM _import;
DROP TABLE _import;
CREATE INDEX IF NOT EXISTS idx_ip_ranges_start ON ip_ranges(start_ip);
SQL

echo "Imported $(sqlite3 "$DB" 'SELECT COUNT(*) FROM ip_ranges;') IP ranges into $DB"
