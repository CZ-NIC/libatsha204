#ifndef ATSHA204CONSTS_H
#define ATSHA204CONSTS_H
//Status codes
static const unsigned char ATSHA204_STATUS_SUCCES = 0x00;
static const unsigned char ATSHA204_STATUS_CHECK_MAC_MISCOMPARE = 0x01;
static const unsigned char ATSHA204_STATUS_PARSE_ERROR = 0x03;
static const unsigned char ATSHA204_STATUS_EXEC_ERROR = 0x0F;
static const unsigned char ATSHA204_STATUS_WAKE_OK = 0x11;
static const unsigned char ATSHA204_STATUS_COMMUNICATION_ERROR = 0xFF;

//OpCodes
static const unsigned char ATSHA204_OPCODE_DEV_REV = 0x30;
static const unsigned char ATSHA204_OPCODE_RANDOM = 0x1B;
static const unsigned char ATSHA204_OPCODE_READ = 0x02;

#endif //ATSHA204CONSTS_H
