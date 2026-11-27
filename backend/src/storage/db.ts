import { Pool } from 'pg';
import 'dotenv/config';
import { readFileSync } from 'node:fs';
import { resolve } from 'node:path';

if (!process.env.DATABASE_URL) {
    throw new Error("DATABASE_URL is not configured");
}

export const pool = new Pool({
    connectionString: process.env.DATABASE_URL,
    ssl: {
        ca: readFileSync(resolve("certs/supabase-ca.crt"), "utf8"),
        rejectUnauthorized: true
    },
    max: 5
});