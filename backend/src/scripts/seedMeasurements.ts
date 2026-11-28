
import type { Measurement } from "../types/measurement";
import { pool } from "../storage/db";

// Simulation configuration
const DAYS = 90;
const INTERVAL_MINUTES = 5;
const INTERVAL_MS = INTERVAL_MINUTES * 60 * 1000;
const TOTAL_MEASUREMENTS = (DAYS * 24 * 60) / INTERVAL_MINUTES;
const END_TIME = Date.now();
const START_TIME = END_TIME - DAYS * 24 * 60 * 60 * 1000;

// Temperature configuration
const BASE_TEMP = 21.5;
const TEMP_AMPLITUDE = 1.5;
const PEAK_HOUR = 15;
const TEMP_NOISE = 0.3;

// Air humidity configuration
const BASE_HUMIDITY = 45;
const HUMIDITY_TEMP_FACTOR = 2.5;
const HUMIDITY_NOISE = 1.5;

// Soil moisture configuration
const INITIAL_MOISTURE = 80;
const BASE_DAILY_DRYING_RATE = 0.30;
const TEMP_DRYING_FACTOR = 0.04;
const HUMIDITY_DRYING_FACTOR = 0.01;
const MIN_MOISTURE = 5;
const WATERING_COOLDOWN_MS = 6 * 60 * 60 * 1000;
const SOIL_MOISTURE_NOISE = 0.3;

// Default firmware calibration
const DRY_ADC = 1342;
const WET_ADC = 3200;

// Database configuration
const BATCH_SIZE = 1000;

// Simulation state
let currentMoisture = INITIAL_MOISTURE;
let wateringThreshold = generateWateringThreshold();
let lastWateringTime = -Infinity;
let wateringEventsToday = 0;
let currentDay = "";
let nextFollowUpWatering: number | null = null;

// Statistics
let totalWateringEvents = 0;
let totalFollowUpEvents = 0;

const measurements: Measurement[] = [];

// Generate a random number within a specified range
function randomBetween(min: number, max: number): number {
    return Math.random() * (max - min) + min;
}

// Limit a number to a specified range
function clamp(value: number, min: number, max: number): number {
    return Math.max(min, Math.min(max, value));
}

// Round to one decimal place
function round(value: number): number {
    return Math.round(value * 10) / 10;
}

// Generate temperature based on the time of day
function generateTemperature(timestamp: Date): number {
    const hour =
        timestamp.getHours() + timestamp.getMinutes() / 60;

    const noise = randomBetween(-TEMP_NOISE, TEMP_NOISE);

    const generatedTemp =
        BASE_TEMP +
        TEMP_AMPLITUDE *
        Math.cos(2 * Math.PI * (hour - PEAK_HOUR) / 24) +
        noise;

    return round(generatedTemp);
}

// Generate relative humidity based on temperature
function generateAirHumidity(temperature: number): number {
    const noise = randomBetween(
        -HUMIDITY_NOISE,
        HUMIDITY_NOISE
    );

    const generatedHumidity =
        BASE_HUMIDITY -
        (temperature - BASE_TEMP) * HUMIDITY_TEMP_FACTOR +
        noise;

    return clamp(round(generatedHumidity), 0, 100);
}

// Generate a threshold representing different watering habits
function generateWateringThreshold(): number {
    const chance = Math.random();

    switch (true) {
        case chance < 0.20:
            return randomBetween(35, 45);

        case chance < 0.65:
            return randomBetween(25, 35);

        case chance < 0.85:
            return randomBetween(15, 25);

        default:
            return randomBetween(10, 15);
    }
}

// Simulate different amounts of water
function waterPlant(currentMoisture: number): number {
    const chance = Math.random();

    switch (true) {
        case chance < 0.25:
            return Math.min(
                100,
                currentMoisture + randomBetween(5, 15)
            );

        case chance < 0.70:
            return Math.min(
                100,
                currentMoisture + randomBetween(25, 45)
            );

        default:
            return Math.max(
                currentMoisture,
                randomBetween(85, 99)
            );
    }
}

// Simulate gradual soil drying
function calculateSoilMoisture(
    currentMoisture: number,
    temperature: number,
    airHumidity: number
): number {
    const environmentalMultiplier = Math.max(
        0,
        1 +
        (temperature - BASE_TEMP) * TEMP_DRYING_FACTOR +
        (BASE_HUMIDITY - airHumidity) * HUMIDITY_DRYING_FACTOR
    );

    const moistureLoss =
        currentMoisture *
        BASE_DAILY_DRYING_RATE *
        environmentalMultiplier *
        (INTERVAL_MINUTES / 1440);

    return clamp(
        currentMoisture - moistureLoss,
        MIN_MOISTURE,
        100
    );
}

// Polynomial regression from Pico firmware
function regression(adc: number): number {
    return (
        0.0000909081 * adc * adc -
        0.244061 * adc +
        177.237
    );
}

// Convert ADC to moisture using the default firmware calibration
function calculateMoistureFromAdc(adc: number): number {
    if (adc <= DRY_ADC) {
        return 0;
    }

    if (adc >= WET_ADC) {
        return 100;
    }

    const moisture =
        (regression(adc) - regression(DRY_ADC)) /
        (regression(WET_ADC) - regression(DRY_ADC)) *
        100;

    return clamp(moisture, 0, 100);
}

// Invert the firmware calculation using binary search
function calculateAdcFromMoisture(moisture: number): number {
    const target = clamp(moisture, 0, 100);

    let low = DRY_ADC;
    let high = WET_ADC;

    while (low < high) {
        const mid = Math.floor((low + high) / 2);

        if (calculateMoistureFromAdc(mid) < target) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    // Choose the integer ADC closest to the target moisture
    const previous = Math.max(DRY_ADC, low - 1);

    const currentDifference =
        Math.abs(calculateMoistureFromAdc(low) - target);

    const previousDifference =
        Math.abs(calculateMoistureFromAdc(previous) - target);

    return previousDifference < currentDifference
        ? previous
        : low;
}

// Schedule additional watering after insufficient watering
function scheduleFollowUp(timestamp: Date): number | null {
    if (
        currentMoisture >= 35 ||
        wateringEventsToday !== 1 ||
        Math.random() >= 0.70
    ) {
        return null;
    }

    const delay = randomBetween(1, 6) * 60 * 60 * 1000;

    // Align the delay with the measurement interval
    const alignedDelay =
        Math.ceil(delay / INTERVAL_MS) * INTERVAL_MS;

    const scheduledTime =
        timestamp.getTime() + alignedDelay;

    const scheduledDate = new Date(scheduledTime);

    // Follow-up must occur on the same day before 21:00
    if (
        scheduledDate.toDateString() !== timestamp.toDateString() ||
        scheduledDate.getHours() >= 21
    ) {
        return null;
    }

    return scheduledTime;
}

// Generate historical measurements
function generateMeasurements(): void {
    for (
        let i = START_TIME;
        i < END_TIME;
        i += INTERVAL_MS
    ) {
        const timestamp = new Date(i);
        const day = timestamp.toDateString();
        const hour = timestamp.getHours();

        // Reset daily watering state
        if (day !== currentDay) {
            currentDay = day;
            wateringEventsToday = 0;

            if (nextFollowUpWatering !== null) {
                nextFollowUpWatering = null;
                wateringThreshold = generateWateringThreshold();
            }
        }

        const temp = generateTemperature(timestamp);
        const airHumidity = generateAirHumidity(temp);

        // Update the actual simulated soil moisture
        currentMoisture = calculateSoilMoisture(
            currentMoisture,
            temp,
            airHumidity
        );

        // Execute scheduled follow-up watering
        if (
            nextFollowUpWatering !== null &&
            i >= nextFollowUpWatering &&
            wateringEventsToday < 2 &&
            hour >= 8 &&
            hour < 21
        ) {
            currentMoisture = waterPlant(currentMoisture);

            nextFollowUpWatering = null;
            lastWateringTime = i;
            wateringEventsToday++;
            totalWateringEvents++;
            totalFollowUpEvents++;

            wateringThreshold = generateWateringThreshold();
        }

        // Execute ordinary watering
        else if (
            nextFollowUpWatering === null &&
            currentMoisture <= wateringThreshold &&
            hour >= 8 &&
            hour < 21 &&
            i - lastWateringTime >= WATERING_COOLDOWN_MS &&
            wateringEventsToday < 2
        ) {
            currentMoisture = waterPlant(currentMoisture);

            lastWateringTime = i;
            wateringEventsToday++;
            totalWateringEvents++;

            nextFollowUpWatering = scheduleFollowUp(timestamp);

            // Start a new watering cycle if no follow-up is planned
            if (nextFollowUpWatering === null) {
                wateringThreshold = generateWateringThreshold();
            }
        }

        // Add sensor noise without changing the underlying soil state
        const measuredMoisture = clamp(
            currentMoisture +
            randomBetween(
                -SOIL_MOISTURE_NOISE,
                SOIL_MOISTURE_NOISE
            ),
            0,
            100
        );

        // Calculate ADC from the simulated moisture
        const soilMoisture_adc =
            calculateAdcFromMoisture(measuredMoisture);

        // Recalculate the percentage from the integer ADC
        const soilMoisture_percent = round(
            calculateMoistureFromAdc(soilMoisture_adc)
        );

        measurements.push({
            temp,
            airHumidity,
            soilMoisture_adc,
            soilMoisture_percent,
            timestamp
        });
    }
}

// Insert generated measurements in batches
async function insertMeasurements(): Promise<void> {
    const client = await pool.connect();

    try {
        await client.query("BEGIN");

        // Prevent concurrent seed scripts from inserting duplicates
        await client.query(
            "LOCK TABLE measurements IN SHARE ROW EXCLUSIVE MODE"
        );

        // Delete all simulated measurements before generating new history
        const deleted = await client.query(`
            DELETE FROM measurements
            WHERE is_simulated = TRUE
        `);

        console.log(`Removed ${deleted.rowCount} simulated measurements`);

        for (
            let i = 0;
            i < measurements.length;
            i += BATCH_SIZE
        ) {
            const batch = measurements.slice(i, i + BATCH_SIZE);

            const values: (number | Date)[] = [];

            const placeholders = batch.map((measurement, index) => {
                const offset = index * 5;

                values.push(
                    measurement.temp,
                    measurement.airHumidity,
                    measurement.soilMoisture_adc,
                    measurement.soilMoisture_percent,
                    measurement.timestamp
                );

                return `(
                    $${offset + 1},
                    $${offset + 2},
                    $${offset + 3},
                    $${offset + 4},
                    $${offset + 5},
                    TRUE
                )`;
            });

            await client.query(`
                INSERT INTO measurements (
                    temp,
                    air_humidity,
                    soil_moisture_adc,
                    soil_moisture_percent,
                    timestamp,
                    is_simulated
                )
                VALUES ${placeholders.join(",")}
            `, values);

            console.log(
                `Inserted ${i + batch.length}/${measurements.length}`
            );
        }

        await client.query("COMMIT");

        console.log("All simulated measurements saved successfully");

    } catch (error) {
        await client.query("ROLLBACK");
        throw error;

    } finally {
        client.release();
    }
}

// Print simulation statistics
function printStatistics(): void {
    const moistureValues = measurements.map(
        measurement => measurement.soilMoisture_percent
    );

    console.table({
        expectedMeasurements: TOTAL_MEASUREMENTS,
        generatedMeasurements: measurements.length,
        simulationStart: new Date(START_TIME).toISOString(),
        simulationEnd: new Date(END_TIME).toISOString(),
        wateringEvents: totalWateringEvents,
        followUpEvents: totalFollowUpEvents,
        minimumMoisture: Math.min(...moistureValues),
        maximumMoisture: Math.max(...moistureValues),
        measurementsBelow30: moistureValues.filter(
            value => value < 30
        ).length,
        measurementsBelow15: moistureValues.filter(
            value => value < 15
        ).length
    });

    console.log("Example measurements:");
    console.table(measurements.slice(0, 5));
}

// Main script
async function main(): Promise<void> {
    try {
        generateMeasurements();
        printStatistics();

        if (process.argv.includes("--write")) {
            await insertMeasurements();
        } else {
            console.log(
                "Preview only. Use --write to save measurements."
            );
        }

    } catch (error) {
        console.error("Failed to generate measurements:", error);
        process.exitCode = 1;

    } finally {
        await pool.end();
    }
}

main();