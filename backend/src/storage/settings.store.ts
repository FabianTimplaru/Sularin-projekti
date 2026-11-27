import type { Settings } from './../types/settings';
import { pool } from './db';

export async function getSettings(): Promise<Settings> {
    const result = await pool.query<Settings>(`
        SELECT min_moisture_threshold AS "minMoistureThreshold" FROM settings
        WHERE id = 1
        `);
    const settings = result.rows[0];

    if (settings === undefined) {
        throw new Error("Settings not found in database");
    }

    return settings;
}

export async function updateSettings(newSettings: Settings): Promise<void> {
    const result = await pool.query(`
        UPDATE settings
        SET min_moisture_threshold = $1
        WHERE id = 1
        `,
        [newSettings.minMoistureThreshold]
    );
    if (result.rowCount !== 1) {
        throw new Error("Failed to update settings");
    }
}