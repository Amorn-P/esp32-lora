/**
 * @file astro_task.cpp
 * @brief FreeRTOS task that reads DS3231 RTC time and computes solar position
 *        (target elevation and azimuth) using astronomical algorithms.
 *        Algorithm based on NOAA Solar Position Calculator / PSA equations.
 *        Updates sysState.target_elevation and sysState.target_azimuth under mutex.
 */

#include "astro_task.h"
#include "config.h"
#include "rtc_functions.h"
#include <RTClib.h>
#include <esp_task_wdt.h>
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

// ============================================================
// Solar Position Algorithm (Simplified PSA / NOAA Hybrid)
// Accuracy: ~0.5° for solar elevation and azimuth
// ============================================================

/**
 * @brief Convert degrees to radians.
 */
static inline float deg2rad(float deg) {
    return deg * (PI / 180.0f);
}

/**
 * @brief Convert radians to degrees.
 */
static inline float rad2deg(float rad) {
    return rad * (180.0f / PI);
}

/**
 * @brief Calculate the Julian Day from a DateTime object.
 *        Uses the standard astronomical formula.
 */
static double julian_day(const DateTime &dt) {
    int year  = dt.year();
    int month = dt.month();
    int day   = dt.day();
    int hour  = dt.hour();
    int minute = dt.minute();
    int second = dt.second();

    // If month is January or February, treat as 13th/14th month of previous year
    if (month <= 2) {
        year -= 1;
        month += 12;
    }

    int A = year / 100;
    int B = 2 - A + (A / 4);

    double jd = floor(365.25 * (year + 4716))
              + floor(30.6001 * (month + 1))
              + day
              + B
              - 1524.5;

    // Add fractional day
    double frac = (hour + minute / 60.0 + second / 3600.0) / 24.0;
    jd += frac;

    return jd;
}

/**
 * @brief Calculate the Julian Century from J2000.0 epoch.
 */
static double julian_century(double jd) {
    return (jd - 2451545.0) / 36525.0;
}

/**
 * @brief Calculate the Geometric Mean Longitude of the Sun (degrees).
 */
static double geom_mean_long_sun(double jc) {
    double L0 = 280.46646 + jc * (36000.76983 + jc * 0.0003032);
    L0 = fmod(L0, 360.0);
    if (L0 < 0) L0 += 360.0;
    return L0;
}

/**
 * @brief Calculate the Geometric Mean Anomaly of the Sun (degrees).
 */
static double geom_mean_anomaly_sun(double jc) {
    return 357.52911 + jc * (35999.05029 - 0.0001537 * jc);
}

/**
 * @brief Calculate the Earth's orbit eccentricity.
 */
static double earth_orbit_eccentricity(double jc) {
    return 0.016708634 - jc * (0.000042037 + 0.0000001267 * jc);
}

/**
 * @brief Calculate the Sun's Equation of Center (degrees).
 */
static double sun_eq_of_center(double jc) {
    double m_rad = deg2rad(geom_mean_anomaly_sun(jc));
    double sin_m  = sin(m_rad);
    double sin_2m = sin(2.0 * m_rad);
    double sin_3m = sin(3.0 * m_rad);
    return sin_m * (1.914602 - jc * (0.004817 + 0.000014 * jc))
         + sin_2m * (0.019993 - 0.000101 * jc)
         + sin_3m * 0.000289;
}

/**
 * @brief Calculate the Sun's True Longitude (degrees).
 */
static double sun_true_long(double jc) {
    return geom_mean_long_sun(jc) + sun_eq_of_center(jc);
}

/**
 * @brief Calculate the Sun's Apparent (True) Longitude (degrees).
 */
static double sun_apparent_long(double jc) {
    double omega = 125.04 - 1934.136 * jc;
    double lambda = sun_true_long(jc);
    return lambda - 0.00569 - 0.00478 * sin(deg2rad(omega));
}

/**
 * @brief Calculate the Mean Obliquity of the Ecliptic (degrees).
 */
static double mean_obliquity_ecliptic(double jc) {
    return 23.0 + (26.0 + (21.448 - jc * (46.815 + jc * (0.00059 - jc * 0.001813))) / 60.0) / 60.0;
}

/**
 * @brief Calculate the Obliquity corrected for nutation (degrees).
 */
static double obliquity_corrected(double jc) {
    double e0 = mean_obliquity_ecliptic(jc);
    double omega = 125.04 - 1934.136 * jc;
    return e0 + 0.00256 * cos(deg2rad(omega));
}

/**
 * @brief Calculate the Solar Declination (degrees).
 */
static double solar_declination(double jc) {
    double epsilon = deg2rad(obliquity_corrected(jc));
    double lambda  = deg2rad(sun_apparent_long(jc));
    return rad2deg(asin(sin(epsilon) * sin(lambda)));
}

/**
 * @brief Calculate the Equation of Time (minutes).
 */
static double equation_of_time(double jc) {
    double epsilon = deg2rad(obliquity_corrected(jc));
    double L0      = deg2rad(geom_mean_long_sun(jc));
    double e       = earth_orbit_eccentricity(jc);
    double m       = deg2rad(geom_mean_anomaly_sun(jc));

    double y = tan(epsilon / 2.0);
    y *= y;

    double sin_2L0 = sin(2.0 * L0);
    double sin_m   = sin(m);
    double sin_2m  = sin(2.0 * m);
    double cos_2L0 = cos(2.0 * L0);
    double cos_2m  = cos(2.0 * m);

    double eot = y * sin_2L0
               - 2.0 * e * sin_m
               + 4.0 * e * y * sin_m * cos_2L0
               - 0.5 * y * y * sin(4.0 * L0)
               - 1.25 * e * e * sin_2m;

    return rad2deg(eot) * 4.0; // convert radians to minutes of time
}

/**
 * @brief Calculate the Hour Angle for the given longitude and time (degrees).
 * @param jd Julian Day
 * @param longitude Site longitude in degrees (positive East)
 * @param eq_time Equation of Time in minutes
 * @return Hour angle in degrees (morning = negative, afternoon = positive)
 */
static double hour_angle(double jd, double longitude, double eq_time) {
    // Time offset in minutes from the start of the UTC day
    double time_utc = (jd - floor(jd - 0.5) - 0.5) * 1440.0; // minutes since 0h UT
    double solar_time = time_utc + eq_time + (4.0 * longitude); // 4 min per degree
    double ha = (solar_time / 4.0) - 180.0; // convert to degrees, 0° at solar noon
    // Normalize to -180..180
    if (ha < -180.0) ha += 360.0;
    if (ha > 180.0)  ha -= 360.0;
    return ha;
}

/**
 * @brief Calculate solar elevation (altitude) angle in degrees.
 * @param lat Site latitude in degrees (positive North)
 * @param decl Solar declination in degrees
 * @param ha Hour angle in degrees
 * @return Elevation angle (0 = horizon, 90 = zenith), negative = below horizon
 */
static double solar_elevation(double lat, double decl, double ha) {
    double lat_rad = deg2rad(lat);
    double decl_rad = deg2rad(decl);
    double ha_rad = deg2rad(ha);

    double sin_elev = sin(lat_rad) * sin(decl_rad)
                    + cos(lat_rad) * cos(decl_rad) * cos(ha_rad);

    return rad2deg(asin(sin_elev));
}

/**
 * @brief Calculate solar azimuth angle in degrees (0 = North, clockwise).
 * @param lat Site latitude in degrees
 * @param decl Solar declination in degrees
 * @param ha Hour angle in degrees
 * @param elev Solar elevation in degrees
 * @return Azimuth in degrees (0-360, 0 = North, 90 = East)
 */
static double solar_azimuth(double lat, double decl, double ha, double elev) {
    double lat_rad = deg2rad(lat);
    double decl_rad = deg2rad(decl);
    double ha_rad = deg2rad(ha);
    double elev_rad = deg2rad(elev);

    double cos_az = (sin(decl_rad) - sin(lat_rad) * sin(elev_rad))
                  / (cos(lat_rad) * cos(elev_rad));

    // Clamp to valid range to avoid NaN from floating point rounding
    if (cos_az > 1.0)  cos_az = 1.0;
    if (cos_az < -1.0) cos_az = -1.0;

    double azimuth_rad = acos(cos_az);

    // Convert to degrees; azimuth is measured from North clockwise
    double azimuth_deg = rad2deg(azimuth_rad);

    // In the morning (ha < 0), azimuth is east of north; in afternoon, west
    if (ha > 0) {
        azimuth_deg = 360.0 - azimuth_deg;
    }

    return azimuth_deg;
}


// ============================================================
// Public: Astro Task
// ============================================================
void astro_task(void *pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(2000); // Every 2 seconds

    Serial.println("[AstroTask] Running...");

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        esp_task_wdt_reset();

        // --- Read RTC DateTime (mutex-protected) ---
        RtcDateTime rtcNow = getRtcDateTime();
        DateTime dt(rtcNow.Year(), rtcNow.Month(), rtcNow.Day(),
                    rtcNow.Hour(), rtcNow.Minute(), rtcNow.Second());

        // --- Snapshot RTC validity under mutex ---
        bool rtc_ok = false;
        if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
            rtc_ok = sysState.rtc_valid;
            xSemaphoreGive(stateMutex);
        }

        float target_el = 0.0f;
        float target_az = 0.0f;

        if (!rtc_ok) {
            // RTC failure — stow flat at current azimuth
            if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
                target_az = sysState.current_azimuth;
                xSemaphoreGive(stateMutex);
            }
            target_el = 0.0f;
        } else {
            // --- Calculate Solar Position ---
            double jd    = julian_day(dt);
            double jc    = julian_century(jd);
            double decl  = solar_declination(jc);
            double eq_t  = equation_of_time(jc);
            double ha    = hour_angle(jd, SITE_LONGITUDE, eq_t);
            double elev  = solar_elevation(SITE_LATITUDE, decl, ha);
            double azim  = solar_azimuth(SITE_LATITUDE, decl, ha, elev);

            target_el = (float)elev;
            target_az = (float)azim;

            // Night stow: below horizon → flat at current azimuth
            if (target_el <= 0.0f) {
                if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
                    target_az = sysState.current_azimuth;
                    xSemaphoreGive(stateMutex);
                }
                target_el = 0.0f;
            }
        }

        // Clamp
        if (target_el < 0.0f) target_el = 0.0f;
        if (target_el > 90.0f) target_el = 90.0f;
        if (target_az < 0.0f) target_az += 360.0f;
        if (target_az >= 360.0f) target_az -= 360.0f;

        // --- Update Global State under Mutex ---
        if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
            sysState.target_elevation = target_el;
            sysState.target_azimuth   = target_az;
            xSemaphoreGive(stateMutex);
        }
    }
}
