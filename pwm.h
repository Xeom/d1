#if !defined(PWM_H)
# define PWM_H

extern float pwm_slope_lowpass_weight;
extern float pwm_kp;
extern float pwm_ki;
extern float pwm_kd;
extern float pwm_v;
extern float pwm_duty;
extern float pwm_adc_ref;
extern float pwm_voltage_div;

void pwm_init(void);

#endif
