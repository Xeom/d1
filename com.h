#if !defined(COM_H)
# define COM_H
# include <stddef.h>
# include <stdint.h>

#define UART_BUF_SIZE 64
#define UART_BAUD 9600

/* Circular buffer */
typedef struct buf_s buf;

struct buf_s
{
    volatile char data[UART_BUF_SIZE];
    volatile size_t ind_in, ind_out;
    volatile unsigned int full  : 1;
};

extern buf uart_tx_buf;
extern buf uart_rx_buf;

/* Send a character or string over UART */
void uart_char(char c);
void uart_str(char* s);

/* Initialize, pop and push to circular buffers */
void   buf_init (buf *b);
int8_t buf_pop  (buf *b);
int8_t buf_push (buf *b, char c);

/* Initialize the UART system */
void uart_init(void);

#endif
