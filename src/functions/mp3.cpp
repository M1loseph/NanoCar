#include "mp3.h"

void MP3command(Stream &stream, int8_t command, int8_t data_fst, int8_t data_snd)
{
    int8_t Send_buf[8];
    Send_buf[0] = 0x7e;             //starting byte
    Send_buf[1] = 0xff;             //version
    Send_buf[2] = 0x06;             //the number of bytes of the command without starting byte and ending byte
    Send_buf[3] = command;          //
    Send_buf[4] = 0x00;             //0x00 = no feedback, 0x01 = feedback
    Send_buf[5] = data_fst;         //datah
    Send_buf[6] = data_snd;         //datal
    Send_buf[7] = 0xef;             //ending byte
    for (uint8_t i = 0; i < 8; i++) //
        stream.write(Send_buf[i]);
}