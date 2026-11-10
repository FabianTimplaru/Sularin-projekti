// Define the Measurement interface to represent the structure of a measurement
export interface Measurement {
    temp: number;
    airHumidity: number;
    soilMoisture_adc: number;
    soilMoisture_percent: number;
    timestamp: Date;
}