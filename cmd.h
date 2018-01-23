#if !defined(CMD_H)
# define CMD_H

# define CMD_LEN 30

/* Call to read from the rx buffer and run commands if needed */
void cmd_update(void);

#endif
