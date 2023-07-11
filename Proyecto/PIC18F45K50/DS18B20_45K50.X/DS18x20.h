#ifndef DS18X20_H
#define DS18X20_H

#include <stdint.h>
#include <stdbool.h>


// global search state
unsigned char ROM_NO[8];
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t LastDeviceFlag;

uint8_t reset(void);
void select(const uint8_t rom[8]);
void skip(void);
void write(uint8_t v);

void write_bytes(const uint8_t *buf, uint16_t count, bool power);

uint8_t read(void);
void read_bytes(uint8_t *buf, uint16_t count);

void write_bit(uint8_t v);
uint8_t read_bit(void);

void depower(void);
void reset_search();

void target_search(uint8_t family_code);

uint8_t search(uint8_t *newAddr);
static uint8_t crc8(const uint8_t *addr, uint8_t len);

static bool check_crc16(const uint8_t* input, uint16_t len, const uint8_t* inverted_crc, uint16_t crc);
static uint16_t crc16(const uint8_t* input, uint16_t len, uint16_t crc);


#endif	/* NEWFILE_H */


