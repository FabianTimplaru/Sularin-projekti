import type { Measurement } from './../types/measurement';
import { pool } from './db';

export async function addMeasurement(measurement: Measurement): Promise<void> {
    await pool.query(`INSERT INTO measurements(
        temp,
        air_humidity,
        soil_moisture_adc,
        soil_moisture_percent,
        timestamp)
        VALUES($1, $2, $3, $4, $5)`,
        [
            measurement.temp,
            measurement.airHumidity,
            measurement.soilMoisture_adc,
            measurement.soilMoisture_percent,
            measurement.timestamp
        ]
    );
}

export async function getAllMeasurements(): Promise<Measurement[]> {
    const result = await pool.query<Measurement>(`
        SELECT 
            temp,
            air_humidity AS "airHumidity",
            soil_moisture_adc AS "soilMoisture_adc",
            soil_moisture_percent AS "soilMoisture_percent",
            timestamp
        FROM measurements
        ORDER BY timestamp ASC, id ASC
        `);
    return result.rows;
}

export async function getLatestMeasurement(): Promise<Measurement | undefined> {
    const result = await pool.query<Measurement>(`
        SELECT 
            temp,
            air_humidity AS "airHumidity",
            soil_moisture_adc AS "soilMoisture_adc",
            soil_moisture_percent AS "soilMoisture_percent",
            timestamp
        FROM measurements
        ORDER BY timestamp DESC, id DESC LIMIT 1
        `);
    return result.rows[0];
}