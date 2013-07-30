#ifndef LAYER_USB_H
#define LAYER_USB_H

#include<stdbool.h>

int usb_wake(int dev, unsigned char **answer);
int usb_idle(int dev);
int usb_command(int dev, unsigned char *raw_packet, unsigned char **answer);

#endif //LAYER_USB_H
