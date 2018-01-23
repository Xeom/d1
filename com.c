#include <avr/interrupt.h>

#include "com.h"

#define buf_empty(b) ((b)->ind_in == (b)->ind_out && !((b)->full))
#define buf_full(b)  ((b)->full)

buf uart_tx_buf;
buf uart_rx_buf;

#define uart_tx_ready (UCSR0A & _BV(UDRE0))
#define uart_rx_ready (UCSR0A & _BV(RXC0))

/* Functions to recieve or transmit to or from UART buffers */
static        void   uart_tx  (void);
static inline void   uart_rx  (void);

void buf_init(buf *b)
{
    b->ind_in  = 0;
    b->ind_out = 0;
    b->full    = 0;
}

int8_t buf_pop(buf *b)
{
    int8_t rtn;

    if (buf_empty(b)) return -1;

    rtn = (int8_t)b->data[b->ind_out];
    b->ind_out = (b->ind_out + 1) % UART_BUF_SIZE;

    /* A buffer popped from can never be full */
    b->full = 0;

    return rtn;
}

int8_t buf_push(buf *b, char c)
{
    if (buf_full(b)) return -1;

    b->data[b->ind_in] = c;
    b->ind_in = (b->ind_in + 1) % UART_BUF_SIZE;

    /* Set if the buffer is full */
    if (b->ind_in == b->ind_out)
        b->full = 1;

    return 0;
}

void uart_tx(void)
{
    int8_t c;

    if (!uart_tx_ready) return;

    /* Get the next character */
    c = buf_pop(&uart_tx_buf);

    /* Check it is valid */
    if (c == -1) return;

    /* Transmit */
    UDR0 = c;
}

static inline void uart_rx(void)
{
    char c;

    if (!uart_rx_ready) return;

    c = UDR0;

    /* Push the character to the buffer. If the buffer *
     * is full, this character is ignored.             */
    buf_push(&uart_rx_buf, c);
}

void uart_char(char c)
{
    /* If the buffer is full, wait for the UART to be *
     * ready to transmit another character, and get   *
     * one character out of the buffer to make way    */
    if (buf_full(&uart_tx_buf))
    {
        while (!uart_tx_ready);
        uart_tx();
    }

    /* Insert the new character into the buffer. *
     * Possible optimization: if (uart_tx_ready) *
     * then transmit directly. This can cause    *
     * bugs though.                              */
    buf_push(&uart_tx_buf, c);

    uart_tx();
}

void uart_str(char *s)
{
    while (*s)
    {
        uart_char(*s);
        s++;
    }
}

void uart_init(void)
{
    buf_init(&uart_tx_buf);
    buf_init(&uart_rx_buf);

    /* Set the baudrate */
    UBRR0H = (F_CPU / UART_BAUD / 16 - 1) >> 8;
    UBRR0L = (F_CPU / UART_BAUD / 16 - 1)  & 0xff;

    /* Set up other things */
    UCSR0B = _BV(RXEN0)  | _BV(TXEN0)  | _BV(RXCIE0) | _BV(TXCIE0);
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);

    uart_rx();
}

/* Interrupt routines - run the rx and tx functions *
 * when characters are ready to transmit or read.   */
ISR(USART0_RX_vect)
{
    uart_rx();
}

ISR(USART0_TX_vect)
{
    uart_tx();
}
