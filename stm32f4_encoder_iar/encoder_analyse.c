#include "encoder_analyse.h"
#include "math.h"

uint16_t ch_a_zero=500;
uint16_t ch_b_zero=500;
uint16_t ch_a_min=0;
uint16_t ch_b_min=0;
uint16_t ch_a_max=1000;
uint16_t ch_b_max=1000;

uint8_t ch_a_state=0;
uint8_t ch_b_state=0;
uint8_t ch_a_digi=0;
uint8_t ch_b_digi=0;
uint8_t ch_a_digi_prev=0;
uint8_t ch_b_digi_prev=0;
static uint16_t loc_ch_a_max =1000;
static uint16_t loc_ch_b_max =1000;
static uint16_t loc_ch_a_min =0;
static uint16_t loc_ch_b_min =0;
int32_t prev_direction = 0;

uint16_t prev_phase;
int32_t main_cnt = 0;

uint16_t last_val_a = 0;
uint16_t last_val_b = 0;

#define ABS(x)         (x < 0) ? (-x) : x

#define A_THRESHOLD 100

int8_t get_interpol_value(void)
{
  float singn_a_val = (float)(last_val_a-ch_a_zero);
  float singn_b_val = (float)(last_val_b-ch_a_zero);
  float angle = atan2(singn_a_val,singn_b_val);
  
  int8_t result =10-(int8_t)(angle*10/3.1415);
  if (result<0) result =0;
  if (result>19) result =19;
  return result;
}


uint8_t analyse_point(uint16_t val_a, uint16_t val_b)
{
	int16_t phase_delta = 0;
	uint16_t phase;
        volatile static int16_t tmp_delta = 0;
        
        last_val_a = val_a;
        last_val_b = val_b;

	//CH A
	if ((val_a > (ch_a_zero + A_THRESHOLD)) && (ch_a_state == 0))
	{
		//переход через 0+
		ch_a_state = 1;//запустить поиск максимума
		loc_ch_a_max = val_a;
		//минимум уже готов
		ch_a_min = loc_ch_a_min;
		ch_a_zero = (uint16_t)((ch_a_min + ch_a_max)/2);
	}

	else if ((val_a < (ch_a_zero - A_THRESHOLD)) && (ch_a_state == 1))
	{
		//переход через 0-
		ch_a_state = 0;//запустить поиск минимума
		loc_ch_a_min = val_a;
		//максимум уже готов
		ch_a_max = loc_ch_a_max;
		ch_a_zero = (uint16_t)((ch_a_min + ch_a_max)/2);
	}

	//CH B
	if ((val_b > (ch_b_zero + A_THRESHOLD)) && (ch_b_state == 0))
	{
		//переход через 0+
		ch_b_state = 1;//запустить поиск максимума
		loc_ch_b_max = val_b;
		//минимум уже готов
		ch_b_min = loc_ch_b_min;
		ch_b_zero = (uint16_t)((ch_b_min + ch_b_max)/2);

	}
	else if ((val_b < (ch_b_zero - A_THRESHOLD)) && (ch_b_state == 1))
	{
		//переход через 0-
		ch_b_state = 0;//запустить поиск минимума
		loc_ch_b_min = val_b;
		//максимум уже готов
		ch_b_max = loc_ch_b_max;
		ch_b_zero = (uint16_t)((ch_b_min + ch_b_max)/2);
	}

	if (ch_a_state == 0)//запущен поиск минимума
	{
		if (val_a < loc_ch_a_min) loc_ch_a_min = val_a;
	}
	else {if (val_a > loc_ch_a_max) loc_ch_a_max = val_a;}

	if (ch_b_state == 0)//запущен поиск минимума
	{
		if (val_b < loc_ch_b_min) loc_ch_b_min = val_b;
	}
	else { if (val_b > loc_ch_b_max) loc_ch_b_max = val_b;}

	if (val_a > ch_a_zero) {ch_a_digi = 1;} else {ch_a_digi = 0;}
	if (val_b > ch_b_zero) {ch_b_digi = 1;} else {ch_b_digi = 0;}

	if ((ch_a_digi_prev != ch_a_digi) || (ch_b_digi_prev != ch_b_digi))//фазы изменились
	{
		ch_a_digi_prev = ch_a_digi;
		ch_b_digi_prev = ch_b_digi;

		if ((ch_a_digi == 0) && (ch_b_digi == 0)) phase = 0;
		else if ((ch_a_digi == 1) && (ch_b_digi == 0)) phase = 90;
		else if ((ch_a_digi == 1) && (ch_b_digi == 1)) phase = 180;
		else if ((ch_a_digi == 0) && (ch_b_digi == 1)) phase = 270;

		phase_delta = calc_phase_delta(prev_phase,phase);
		prev_phase = phase;
                
                tmp_delta = ABS(phase_delta);

		if (tmp_delta > 90)//ошибка
		{
			main_cnt+=(prev_direction*2);
			return 1;
		}
		else
		{
			if (phase_delta > 0) {main_cnt++;prev_direction = 1;}
			if (phase_delta < 0) {
                          main_cnt--;prev_direction = -1;}
			return 0;
		}
	}
	else
	{
		//ch_a_digi_prev = ch_a_digi;
		//ch_b_digi_prev = ch_b_digi;
		return 0;//ничего не менялось
	}
}

int16_t calc_phase_delta(uint16_t old_val, uint16_t new_val)
{
	int16_t phase_delta = 0;

	if ((old_val == 270) && (new_val == 0)) {phase_delta = 90;}
	else if ((old_val == 0) && (new_val == 270)) {phase_delta = -90;}
	else {phase_delta = (int16_t)((int16_t)new_val - (int16_t)old_val);}
	return phase_delta;
}
