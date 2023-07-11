

#include "DS18x20.h"
#include "mcc_generated_files/mcc.h"



//
// Perform a search. If this function returns a '1' then it has
// enumerated the next device and you may retrieve the ROM from the
// OneWire::address variable. If there are no devices, no further
// devices, or something horrible happens in the middle of the
// enumeration then a 0 is returned.  If a new device is found then
// its address is copied to newAddr.  Use OneWire::reset_search() to
// start over.
//
// --- Replaced by the one from the Dallas Semiconductor web site ---
//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return true  : device found, ROM number in ROM_NO buffer
//        false : device not found, end of search
//

uint8_t search(uint8_t * newAddr) {
    uint8_t id_bit_number;
    uint8_t last_zero, rom_byte_number, search_result;
    uint8_t id_bit, cmp_id_bit;

    unsigned char rom_byte_mask, search_direction;

    // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;
    search_result = 0;

    // if the last call was not the last one
    if (!LastDeviceFlag) {
        // 1-Wire reset
        if (!reset()) {
            // reset the search
            LastDiscrepancy = 0;
            LastDeviceFlag = false;
            LastFamilyDiscrepancy = 0;
            return false;
        }

        // issue the search command
        write(0xF0);

        // loop to do the search
        do {
            // read a bit and its complement
            id_bit = read_bit();
            cmp_id_bit = read_bit();

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
                break;
            else {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                    search_direction = id_bit; // bit write value for search
                else {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy)
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == LastDiscrepancy);

                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0) {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if (last_zero < 9)
                            LastFamilyDiscrepancy = last_zero;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if (search_direction == 1)
                    ROM_NO[rom_byte_number] |= rom_byte_mask;
                else
                    ROM_NO[rom_byte_number] &= ~rom_byte_mask;

                // serial number search direction write bit
                write_bit(search_direction);

                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if (rom_byte_mask == 0) {
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        } while (rom_byte_number < 8); // loop until through all ROM bytes 0-7

        // if the search was successful then
        if (!(id_bit_number < 65)) {
            // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
            LastDiscrepancy = last_zero;

            // check for last device
            if (LastDiscrepancy == 0)
                LastDeviceFlag = true;

            search_result = true;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !ROM_NO[0]) {
        LastDiscrepancy = 0;
        LastDeviceFlag = false;
        LastFamilyDiscrepancy = 0;
        search_result = false;
    }
    for (int i = 0; i < 8; i++) newAddr[i] = ROM_NO[i];
    return search_result;
}



// Perform the onewire reset function.  We will wait up to 250uS for
// the bus to come high, if it doesn't then it is broken or shorted
// and we return a 0;
//
// Returns 1 if a device asserted a presence pulse, 0 otherwise.
//

uint8_t reset(void) {
    uint8_t r;
    uint8_t retries = 125;
    do {
        if (--retries == 0) return 0;
        __delay_us(2);
    } while (!OW_GetValue());

    OW_SetLow();
    OW_SetDigitalOutput(); // drive output low
    __delay_us(480);
    OW_SetDigitalInput(); // allow it to float
    __delay_us(70);
    r = !OW_GetValue();
    __delay_us(410);
    return r;
}


//
// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set 'power' to 1, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//*/

void write(uint8_t v) {
    uint8_t bitMask;

    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
        write_bit((bitMask & v) ? 1 : 0);
    }
    OW_SetDigitalInput();
    OW_SetLow();
}

//
// Write a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//

void write_bit(uint8_t v) {

    if (v & 1) {

        OW_SetLow();
        OW_SetDigitalOutput(); // drive output low
        __delay_us(10);
        OW_SetHigh(); // drive output high
        __delay_us(55);
    } else {
        OW_SetLow();
        OW_SetDigitalOutput(); // drive output low
        __delay_us(65);
        OW_SetHigh(); // drive output high
        __delay_us(5);
    }
}


//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//

uint8_t read_bit(void) {
    uint8_t r;
    OW_SetDigitalOutput();
    OW_SetLow();
    __delay_us(3);

    OW_SetDigitalInput(); // let pin float, pull up will raise
    __delay_us(10);
    r = OW_GetValue();
    __delay_us(53);
    return r;
}

//
// You need to use this function to start a search again from the beginning.
// You do not need to do it for the first search, though you could.
//

void reset_search() {
    // reset the search state
    LastDiscrepancy = 0;
    LastDeviceFlag = false;
    LastFamilyDiscrepancy = 0;
    for (int i = 7;; i--) {
        ROM_NO[i] = 0;
        if (i == 0) break;
    }
}

uint8_t dscrc_table[] = {
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};


//
// Compute a Dallas Semiconductor 8 bit CRC. These show up in the ROM
// and the registers.  (note: this might better be done without to
// table, it would probably be smaller and certainly fast enough
// compared to all those delayMicrosecond() calls.  But I got
// confused, so I use this table from the examples.)
//

//
// Do a ROM select
//

void select(const uint8_t rom[8]) {
    uint8_t i;

    write(0x55); // Choose ROM

    for (i = 0; i < 8; i++) write(rom[i]);
}



//
// Read a byte
//

uint8_t read() {
    uint8_t bitMask;
    uint8_t r = 0;

    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
        if (read_bit()) r |= bitMask;
    }
    return r;
}