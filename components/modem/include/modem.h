#ifndef MODEM_H
#define MODEM_H

void modem_task(void *p1, void *p2, void *p3);

int modem_init(void);

void modem_send_at(const char *command);

void modem_read_response(void);
#endif
