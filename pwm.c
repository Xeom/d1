#include <math.h>
#include <avr/interrupt.h>

#include "pwm.h"

void pwm_init(void)
{
    TCCR2A  = _BV(COM2A1) | _BV(COM2A0);
    TCCR2A |= _BV(WGM20)  | _BV(WGM21);
    TCCR2B  = _BV(CS20);
    DDRD   |= _BV(PD7);

    ADMUX = 0;
    ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    ADCSRA |= _BV(ADEN)  | _BV(ADATE) | _BV(ADIE);
    ADCSRB  = 0;
}

void pwm_set_duty(float d)
{
    if      (d > 1.0) d = 1.0;
    else if (d < 0.0) d = 0.0;

    OCR2A = d * 0xff;
}

float pwm_slope_lowpass_weight = 0.5;
float pwm_kp = 0.01;
float pwm_ki = 0;
float pwm_kd = 0;
float pwm_v = 5;
float pwm_duty = 0.5;
float pwm_adc_ref = 3.3;
float pwm_voltage_div = 0.5;

float pwm_get_slope(float v)
{
    static float prev_dv = NAN;
    static float prev_v  = NAN;
    float dv, rtn;

    if (prev_v == NAN)
    {
        prev_v = v;
        return 0;
    }

    dv = v - prev_v;

    if (prev_dv == NAN)
        prev_dv = dv;

    rtn = dv * (1 - pwm_slope_lowpass_weight) + prev_dv * pwm_slope_lowpass_weight;

    prev_dv = rtn;

    return rtn;
}

float get_voltage(void)
{
    return (float)ADC * pwm_adc_ref / 1024.0 / pwm_voltage_div;
}

float get_error(void)
{
    return get_voltage() - pwm_v;
}

ISR(ADC_vect)
{
    pwm_duty -= get_error() * pwm_kp;
    pwm_set_duty(pwm_duty);
}
