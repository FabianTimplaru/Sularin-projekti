import { pool } from "./db";

async function testConnection() {
    try {
        const result = await pool.query('SELECT * FROM settings;');
        console.log(result.rows);
    } catch (error) {
        console.error('Error testing database connection:', error);
    } finally {
        await pool.end();
    }
}
testConnection();