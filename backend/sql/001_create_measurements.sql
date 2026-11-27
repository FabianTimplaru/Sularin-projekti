CREATE TABLE measurements (
id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
temp DOUBLE PRECISION NOT NULL CHECK (temp BETWEEN 0 AND 50),
air_humidity DOUBLE PRECISION NOT NULL CHECK(air_humidity BETWEEN 0 AND 100),
soil_moisture_adc INTEGER NOT NULL CHECK(soil_moisture_adc BETWEEN 0 AND 4095),
soil_moisture_percent DOUBLE PRECISION NOT NULL CHECK(soil_moisture_percent BETWEEN 0 AND 100),
timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
ALTER TABLE measurements ENABLE ROW LEVEL SECURITY;
CREATE INDEX idx_measurements_timestamp
ON measurements (timestamp);