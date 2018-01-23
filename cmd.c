#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "com.h"

#include "cmd.h"

float pwm_kp, pwm_ki, pwm_kd, pwm_v;

typedef struct param_s param;

struct param_s
{
    char *name;
    float *val;
};

param cmd_params[] =
{
    {"k_p", &pwm_kp},
    {"k_i", &pwm_ki},
    {"k_d", &pwm_kd},
    {"v",   &pwm_v}
};

char cmd_text[CMD_LEN];
size_t cmd_ind = 0;

static void cmd_char(char c);
static void cmd_run(char *cmd);
static char *cmd_word_split(char *s);
static void cmd_run_get(char *s);
static void cmd_run_set(char *s);

static void cmd_char(char c)
{
    if      (c == '\n') return;
    else if (c == '\r')
    {
        cmd_text[cmd_ind] = '\0';
        if (cmd_ind)
            cmd_run(cmd_text);
        cmd_ind = 0;
    }
    else
    {
        if (cmd_ind == CMD_LEN - 1)
            return;
        else
            cmd_text[cmd_ind++] = c;
    }
}

static void cmd_run(char *s)
{
    char *args;
    args = cmd_word_split(s);
    if (strcmp(s, "set")==0)
        cmd_run_set(args);
    else if (strcmp(s, "get") ==0)
        cmd_run_get(args);
    else
        uart_str("err cmd\r\n");
}

static char *cmd_word_split(char *s)
{
    while (*s)
    {
        if (*s == ' ')
        {
            *s = '\0';
            return s + 1;
        }

        s++;
    }
    return NULL;
}

param *cmd_get_param(char *name)
{
    size_t ind;

    for (ind = 0;
         ind < sizeof(cmd_params)/sizeof(param);
         ind++)
    {
        param *p;
        p = &cmd_params[ind];
        if (strcmp(p->name, name) == 0)
            return p;
    }

    cli();
    uart_str("err prm\r\n");
    sei();

    return NULL;
}

static void cmd_run_get(char *s)
{
    param *p;
    char val[20];

    p = cmd_get_param(s);

    if (!p) return;

    cli();

    dtostre(*(p->val), val, 4, 0);

    uart_str("ok ");
    uart_str(val);
    uart_str("\r\n");

    sei();
}

static void cmd_run_set(char *s)
{
    param *p;
    char *valstr;
    float val;

    valstr = cmd_word_split(s);

    if (valstr == NULL)
    {
        cli();
        uart_str("err val\r\n");
        sei();
        return;
    }

    val = atof(valstr);

    p = cmd_get_param(s);

    if (!p) return;

    cli();
    uart_str("ok");
    *(p->val) = val;
    uart_str("\r\n");
    sei();
}

void cmd_update(void)
{
    int8_t c;

    for (;;)
    {
        cli();
        c = buf_pop(&uart_rx_buf);
        sei();
        if (c == -1) break;
        cmd_char((char)c);
    }
}

int main(void)
{
    uart_init();
    sei();

    DDRB |= 1<<7;
    while (1)
    {
        _delay_ms(10);
        cmd_update();
    }

    return 0;
}
