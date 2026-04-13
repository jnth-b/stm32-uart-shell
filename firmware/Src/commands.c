#include "stm32f4xx.h"
#include "commands.h"
#include "uart.h"
#include "adc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern volatile uint32_t ms_ticks;

/* command handler prototypes */
static void cmd_help(int argc, char **argv);
static void cmd_led(int argc, char **argv);
static void cmd_adc(int argc, char **argv);
static void cmd_pwm(int argc, char **argv);
static void cmd_uptime(int argc, char **argv);
static void cmd_status(int argc, char **argv);

typedef void (*cmd_func)(int argc, char **argv);

typedef struct {
    const char *name;
    cmd_func    fn;
} command;

static const command cmd_table[] = {
    { "help",   cmd_help },
    { "led",    cmd_led },
    { "adc",    cmd_adc },
    { "pwm",    cmd_pwm },
    { "uptime", cmd_uptime },
    { "status", cmd_status },
};
#define N_CMDS (sizeof(cmd_table) / sizeof(cmd_table[0]))

static void send(const char *s)
{
    dma_uart_send(s, strlen(s));
}

/* linear search through command table */
static void dispatch(int argc, char **argv)
{
    for (int i = 0; i < (int)N_CMDS; i++) {
        if (strcmp(argv[0], cmd_table[i].name) == 0) {
            cmd_table[i].fn(argc, argv);
            return;
        }
    }
    send("Unknown command. Type 'help'\r\n");
}

void parse_and_dispatch(char *line)
{
    char *argv[8];
    int   argc = 0;

    char *tok = strtok(line, " ");
    while (tok && argc < 8) {
        argv[argc++] = tok;
        tok = strtok(NULL, " ");
    }
    if (argc == 0) return;
    dispatch(argc, argv);
}

/* ---- command implementations ---- */

static void cmd_help(int argc, char **argv)
{
    (void)argc; (void)argv;
    send("Commands:\r\n");
    send("  help          - show this message\r\n");
    send("  led [on|off]  - toggle onboard LED\r\n");
    send("  adc           - read voltage on PA0\r\n");
    send("  pwm [0-100]   - set LED brightness\r\n");
    send("  uptime        - time since boot\r\n");
    send("  status        - dump peripheral state\r\n");
}

static void cmd_led(int argc, char **argv)
{
    if (argc < 2) {
        send("Usage: led [on|off]\r\n");
        return;
    }
    if (strcmp(argv[1], "on") == 0)
        TIM2->CCR1 = 100;
    else if (strcmp(argv[1], "off") == 0)
        TIM2->CCR1 = 0;
    else
        send("Usage: led [on|off]\r\n");
}

static void cmd_adc(int argc, char **argv)
{
    (void)argc; (void)argv;
    uint16_t val = adc_read();
    uint32_t mv  = (uint32_t)val * 3300 / 4095;
    char buf[48];
    snprintf(buf, sizeof(buf), "ADC PA0: raw=%u  voltage=%lumV\r\n", val, mv);
    send(buf);
}

static void cmd_pwm(int argc, char **argv)
{
    if (argc < 2) {
        send("Usage: pwm [0-100]\r\n");
        return;
    }
    int duty = atoi(argv[1]);
    if (duty < 0)   duty = 0;
    if (duty > 100) duty = 100;
    TIM2->CCR1 = (uint32_t)duty;

    char buf[32];
    snprintf(buf, sizeof(buf), "PWM duty set to %d%%\r\n", duty);
    send(buf);
}

static void cmd_uptime(int argc, char **argv)
{
    (void)argc; (void)argv;
    uint32_t t = ms_ticks;
    uint32_t s = t / 1000;
    uint32_t m = s / 60;
    uint32_t h = m / 60;
    char buf[32];
    snprintf(buf, sizeof(buf), "Uptime: %02lu:%02lu:%02lu\r\n", h, m % 60, s % 60);
    send(buf);
}

static void cmd_status(int argc, char **argv)
{
    (void)argc; (void)argv;
    uint32_t t    = ms_ticks;
    uint16_t aval = adc_read();
    uint32_t duty = TIM2->CCR1;

    char buf[160];
    snprintf(buf, sizeof(buf),
        "=== System Status ===\r\n"
        "Uptime : %lums\r\n"
        "ADC PA0: %u raw  (%lumV)\r\n"
        "PWM LED: %lu%%\r\n"
        "====================\r\n",
        t, aval, (uint32_t)aval * 3300 / 4095, duty);
    send(buf);
}
