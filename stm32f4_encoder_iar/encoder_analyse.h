#ifndef _ENC_ANALYSE
#define _ENC_ANALYSE
#include <stdint.h>

uint8_t analyse_point(uint16_t val_a, uint16_t val_b);
int16_t calc_phase_delta(uint16_t old_val, uint16_t new_val);
int8_t get_interpol_value(void);

#endif
