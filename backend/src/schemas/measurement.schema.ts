import { z } from 'zod';

export const measurementSchema = z.object({
    token: z.string().min(1),
    temp: z.number().min(0).max(50),
    airHumidity: z.number().min(0).max(100),
    soilMoisture_percent: z.number().min(0).max(100),
    soilMoisture_adc: z.number().int().min(0).max(4095)
});