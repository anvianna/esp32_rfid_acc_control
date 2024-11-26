#include "dev_sg90.hpp"

SG90::SG90(int servoPin)
{
    m_duty = 0;
    m_servo_pin = servoPin;
    m_angle = 0;
}

SG90::~SG90()
{
}


void SG90::init()
{
  /* Timer Configuration */
    ledc_timer_config_t ledc_timer =
    {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 50,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = 
    {
        .gpio_num = m_servo_pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty  = 0,
        .hpoint = 0,
        .flags = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, m_duty);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}

int32_t SG90::duty_calculate(int angle)
{
    m_angle = angle;
    if (m_angle <= -90) m_angle = -90;
    if (m_angle >= 90) m_angle = 90;

    m_duty = DUTY_ANGLE_ZERO + ((m_angle * (MAX_DUTY - MIN_DUTY)) / MAX_ROTATION);
    
    // Limite o m_duty entre 200 e 1000
    if (m_duty <= MIN_DUTY) m_duty = MIN_DUTY;
    if (m_duty >= MAX_DUTY) m_duty = MAX_DUTY;

    return m_duty;
}

void SG90::clockWise(int32_t start_angle, int32_t end_angle, int32_t step, int delay_ms)
{
  for(m_angle = start_angle; m_angle<=end_angle; m_angle+=step)
  {
    m_duty = duty_calculate(m_angle);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (uint32_t) m_duty);
    printf("angle -> %d, m_duty -> %ld\n", m_angle, m_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(delay_ms/portTICK_PERIOD_MS);
  }
  ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, m_duty);
}

void SG90::anticlockWise(int32_t start_angle, int32_t end_angle, int32_t step, int delay_ms)
{
  for(m_angle = start_angle; m_angle>=end_angle; m_angle-=step)
  {
    m_duty = duty_calculate(m_angle);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (uint32_t)m_duty);
    printf("angle -> %d, m_duty -> %ld\n", m_angle, m_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(delay_ms/portTICK_PERIOD_MS);
  }
  if(m_angle == 0)
  {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1);
    // ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, m_duty+1);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
  }
}
