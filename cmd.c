#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "com.h"
#include "pwm.h"

#include "cmd.h"


typedef struct param_s param;

struct param_s
{
    char *name;
    float *val;
};

param cmd_params[] =
{
    {"k_p",   &pwm_kp},
    {"k_i",   &pwm_ki},
    {"k_d",   &pwm_kd},
    {"v",     &pwm_v},
    {"delp",  &pwm_slope_lowpass_weight},
    {"adcr",  &pwm_adc_ref},
    {"vdiv",  &pwm_voltage_div}
};

/* The current command */
char   cmd_text[CMD_LEN];
size_t cmd_ind = 0;

/* Buffer up a character for the next command */
static void  cmd_char       (char  c);
/* Runs the command buffered up */
static void  cmd_run        (char *cmd);

/* Replace the next space in a string with \0, and return the remainder *
 *                                 "abra\0cadabra"                      *
 * e.g. cmd_word_split("abra cadabra") ---^                             */
static char *cmd_word_split (char *s);

/* Functions for each command. Each takes the arguments of the command */
static void  cmd_run_get    (char *s); /* get [param]         -> ok [value] */
static void  cmd_run_set    (char *s); /* set [param] [value] -> ok         */
static void  cmd_run_all    (void);    /* all  -> [param]: [value]... ok */

static void cmd_char(char c)
{
    /* Ignore newlines, they may or may not be sent */
    if      (c == '\n') return;
    /* \r is a delimiter for lines and thus commands */
    else if (c == '\r')
    {
        cmd_text[cmd_ind] = '\0';
        if (cmd_ind)
            cmd_run(cmd_text);
        cmd_ind = 0;
    }
    else
    {
        /* No more characters if we run out of space */
        if (cmd_ind == CMD_LEN - 1)
            return;
        else
            cmd_text[cmd_ind++] = c;
    }
}

static void cmd_run(char *s)
{
    char *args;

    /* Split the arguments from the command in the first word */
    args = cmd_word_split(s);

    if      (strcmp(s, "set") == 0)
        cmd_run_set(args);
    else if (strcmp(s, "get") == 0)
        cmd_run_get(args);
    else if (strcmp(s, "all") == 0)
        cmd_run_all();
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

    /* Iterate across all params until one is found */
    for (ind = 0;
         ind < sizeof(cmd_params)/sizeof(param);
         ind++)
    {
        param *p;

        p = &cmd_params[ind];

        /* Return it if it is found */
        if (strcmp(p->name, name) == 0)
            return p;
    }

    /* If no param is found, send an error */
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

    /* Convert the float to a string */
    dtostre(*(p->val), val, 4, 0);

    uart_str(val);
    uart_str("\r\nok\r\n");

    sei();
}

static void cmd_run_set(char *s)
{
    param *p;
    char *valstr;
    float val;

    /* Try and get a second argument */
    valstr = cmd_word_split(s);

    /* Print an error if there is not one */
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
    /* Set the value and give an ok */
    *(p->val) = val;
    uart_str("ok\r\n");
    sei();
}

static void cmd_run_all(void)
{
    size_t ind;

    /* Iterate across all params until one is found */
    for (ind = 0;
         ind < sizeof(cmd_params)/sizeof(param);
         ind++)
    {
        param *p;
        char   val[20];

        p = &cmd_params[ind];
        dtostre(*(p->val), val, 4, 0);

        uart_str(p->name);
        uart_str(": ");
        uart_str(val);
        uart_str("\r\n");
    }

    uart_str("ok\r\n");
}

void cmd_update(void)
{
    int8_t c;

    /* Read from the rx buffer until it's empty */
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
    pwm_init();
    sei();

    DDRB |= 1<<7;
    while (1)
    {
        _delay_ms(10);
        cmd_update();
    }

    return 0;
}
