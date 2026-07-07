/**
 * @file sensor_task.cpp
 * @brief FreeRTOS sensor task: reads MPU6050 (tilt/elevation) and QMC5883L (heading/azimuth)
 *        with rolling average smoothing. Updates sysState under mutex.
 * 
 * Hardware: MPU6050 @ I2C 0x68, QMC5883L @ I2C 0x0D
 */

#include "sensor_task.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <esp_task_wdt.h>
#include <math.h>

// ============================================================
// QMC5883L Simple I2C Driver (no external library needed)
// ============================================================
#define QMC5883L_ADDR           0x0D
#define QMC5883L_REG_X_LSB      0x00
#define QMC5883L_REG_STATUS     0x06
#define QMC5883L_REG_CTRL1      0x09
#define QMC5883L_REG_CTRL2      0x0A
#define QMC5883L_REG_SET_RESET  0x0B

// Control Register 1: OSR=512, RNG=2G, ODR=200Hz, Mode=Continuous
#define QMC5883L_CTRL1_CONFIG   0xDD  // 0b11011101

static bool qmc5883l_initialized = false;

/**
 * @brief Initialize QMC5883L magnetometer in continuous mode.
 */
static bool qmc5883l_init() {
    Wire.beginTransmission(QMC5883L_ADDR);
    Wire.write(QMC5883L_REG_CTRL1);
    Wire.write(QMC5883L_CTRL1_CONFIG);
    if (Wire.endTransmission() != 0) {
        Serial.println("[QMC5883L] Init failed - device not responding");
        return false;
    }

    // Set reset period (recommended)
    Wire.beginTransmission(QMC5883L_ADDR);
    Wire.write(QMC5883L_REG_SET_RESET);
    Wire.write(0x01);
    Wire.endTransmission();

    qmc5883l_initialized = true;
    Serial.println("[QMC5883L] Initialized OK");
    return true;
}

/**
 * @brief Read raw 16-bit magnetometer data from QMC5883L.
 * @param x Output: raw X-axis value
 * @param y Output: raw Y-axis value
 * @param z Output: raw Z-axis value
 * @return true if data read successfully
 */
static bool qmc5883l_read(int16_t &x, int16_t &y, int16_t &z) {
    if (!qmc5883l_initialized) return false;

    Wire.beginTransmission(QMC5883L_ADDR);
    Wire.write(QMC5883L_REG_X_LSB);
    if (Wire.endTransmission(false) != 0) return false;

    uint8_t bytes = Wire.requestFrom(QMC5883L_ADDR, (uint8_t)6);
    if (bytes != 6) return false;

    uint8_t buf[6];
    for (int i = 0; i < 6; i++) {
        buf[i] = Wire.read();
    }

    // QMC5883L stores data in little-endian, 2's complement
    x = (int16_t)(buf[0] | (buf[1] << 8));
    y = (int16_t)(buf[2] | (buf[3] << 8));
    z = (int16_t)(buf[4] | (buf[5] << 8));

    return true;
}

/**
 * @brief Compute compass heading (azimuth) from raw magnetometer values.
 * @param x Raw X
 * @param y Raw Y
 * @return Heading in degrees (0-360), 0 = North
 */
static float qmc5883l_heading(int16_t x, int16_t y) {
    // Calculate angle in radians
    float heading = atan2f((float)y, (float)x);

    // Convert to degrees
    float heading_deg = heading * (180.0f / PI);

    // Magnetic declination from User_config.h (site-specific)
    heading_deg += MAGNETIC_DECLINATION;

    // Normalize to 0-360
    if (heading_deg < 0) heading_deg += 360.0f;
    if (heading_deg >= 360.0f) heading_deg -= 360.0f;

    return heading_deg;
}


// ============================================================
// MPU6050 Accelerometer (for tilt/elevation)
// ============================================================
static Adafruit_MPU6050 mpu;
static bool mpu_initialized = false;

/**
 * @brief Initialize the MPU6050 sensor.
 */
static bool mpu6050_init() {
    if (!mpu.begin()) {
        Serial.println("[MPU6050] Init failed - check wiring");
        return false;
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    mpu_initialized = true;
    Serial.println("[MPU6050] Initialized OK");
    return true;
}

/**
 * @brief Compute tilt/elevation angle from accelerometer data.
 *        Elevation = angle of the panel from horizontal (0 = flat, 90 = vertical).
 *        Uses the pitch angle derived from the gravity vector.
 * @return Elevation angle in degrees (0-90)
 */
static float mpu6050_elevation() {
    if (!mpu_initialized) return 0.0f;

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Pitch from accelerometer: angle between the Z-axis and the gravity vector projected on XZ plane
    // pitch = atan2(-ax, sqrt(ay^2 + az^2)) in radians
    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = a.acceleration.z;

    float pitch_rad = atan2f(-ax, sqrtf(ay * ay + az * az));
    float pitch_deg = pitch_rad * (180.0f / PI);

    // Elevation from horizontal: if panel is flat (pitch=0), elevation=0
    // If panel is tilted up (pitch positive), elevation = pitch
    // Clamp to sensible range
    if (pitch_deg < 0.0f) pitch_deg = 0.0f;
    if (pitch_deg > 90.0f) pitch_deg = 90.0f;

    return pitch_deg;
}


// ============================================================
// Rolling Average Filter
// ============================================================
template <typename T, size_t N>
class RollingAverage {
private:
    T _buffer[N];
    size_t _index;
    size_t _count;
    T _sum;

public:
    RollingAverage() : _index(0), _count(0), _sum(0) {
        memset(_buffer, 0, sizeof(_buffer));
    }

    void add(T value) {
        _sum -= _buffer[_index];
        _buffer[_index] = value;
        _sum += value;
        _index = (_index + 1) % N;
        if (_count < N) _count++;
    }

    T average() const {
        if (_count == 0) return 0;
        return _sum / (T)_count;
    }

    bool is_full() const {
        return _count >= N;
    }
};

static RollingAverage<float, SMOOTHING_WINDOW_SIZE> elevation_filter;
static RollingAverage<float, SMOOTHING_WINDOW_SIZE> azimuth_filter;


// ============================================================
// Public: Sensor Initialization
// ============================================================
bool init_sensors() {
    bool ok = true;

    // Initialize MPU6050
    if (!mpu6050_init()) {
        ok = false;
    }
    
    // QMC5883L is often missing in Wokwi, so we bypass strict init failure
    qmc5883l_init();

    return ok;
}


// ============================================================
// FreeRTOS Sensor Task
// ============================================================
void sensor_task(void *pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 20 Hz sensor read rate

    Serial.println("[SensorTask] Running...");

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        esp_task_wdt_reset();

        // --- Read MPU6050 Elevation ---
        float raw_elevation = mpu6050_elevation();
        elevation_filter.add(raw_elevation);
        float smooth_elevation = elevation_filter.average();

        // --- Read QMC5883L Azimuth ---
        int16_t mx, my, mz;
        float heading = 0.0f;
        if (qmc5883l_read(mx, my, mz)) {
            heading = qmc5883l_heading(mx, my);
        }
        azimuth_filter.add(heading);
        float smooth_azimuth = azimuth_filter.average();

        // --- Update Global State under Mutex ---
        if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
            sysState.current_elevation = smooth_elevation;
            sysState.current_azimuth   = smooth_azimuth;
            xSemaphoreGive(stateMutex);
        }
    }
}
