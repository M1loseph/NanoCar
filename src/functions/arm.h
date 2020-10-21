#ifndef __ARM_H__
#define __ARM_H__

#include <Arduino.h>
#include <buffer/buffer.h>
#include <Adafruit_PWMServoDriver.h>

namespace arm
{
    namespace commands
    {
        extern const char *SERVO_MINUS;
        extern const char *SERVO_PLUS;
        extern const char *SERVO_STOP;
        extern const char *SERVO_ANGLE;
    } // namespace commands

    typedef struct
    {
        const uint8_t MIN_ANGLE;
        const uint8_t MAX_ANGLE;
        uint8_t current_angle;
        uint8_t destination_angle;
        uint8_t extern_module_pin;
    } servo_data;

    extern Adafruit_PWMServoDriver pwm;

    void init_arm();

    void servo_minus(const CommandBuffer *b);
    void servo_plus(const CommandBuffer *b);
    void servo_stop(const CommandBuffer *b);
    void servo_angle(const CommandBuffer *b);

    void update_servos_movement();

} // namespace arm

#endif // __ARM_H__