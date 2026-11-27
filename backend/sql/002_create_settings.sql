CREATE TABLE settings(
id INTEGER PRIMARY KEY CHECK(id = 1),
min_moisture_threshold DOUBLE PRECISION NOT NULL DEFAULT 30 CHECK(min_moisture_threshold BETWEEN 10 AND 50)
);
ALTER TABLE settings ENABLE ROW LEVEL SECURITY;
INSERT INTO settings (id, min_moisture_threshold)
VALUES(1, DEFAULT);