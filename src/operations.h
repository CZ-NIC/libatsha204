#ifndef OPERATIONS_H
#define OPERATIONS_H

unsigned char *op_dev_rev();
uint32_t op_dev_rev_recv(unsigned char *packet);
unsigned char *op_random();
int op_random_recv(unsigned char *packet, unsigned char **data);
unsigned char *op_read(unsigned char slot_number);
int op_read_recv(unsigned char *packet, unsigned char **data);
unsigned char *op_write(unsigned char slot_number, size_t cnt, unsigned char *data);
int op_write_recv(unsigned char *packet);

#endif //OPERATIONS_H
