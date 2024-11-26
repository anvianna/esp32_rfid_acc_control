#include "stdio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#define pinServo 4
#define MAX_DUTY 1000
#define MIN_DUTY 200
#define MAX_CLOCKWISE_ANGLE 90
#define MAX_ANTICLOCKWISE_ANGLE -90
#define MAX_ROTATION 180
#define DUTY_ANGLE_ZERO 600

typedef enum
{
    CLOSE = 0,
    OPEN
}servo_movement_t;

class SG90
{

    public:
    SG90(int servoPin);
    ~SG90();
    void init();     
    int32_t duty_calculate(int angle);
    void clockWise(int32_t start_angle, int32_t end_angle, int32_t step, int delay_ms);
    void anticlockWise(int32_t start_angle, int32_t end_angle, int32_t step, int delay_ms);

    private:
    int m_servo_pin;
    int m_angle;
    uint32_t m_duty;

};
