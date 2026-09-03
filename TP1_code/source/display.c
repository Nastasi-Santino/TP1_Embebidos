#include "display.h"
#include "pisr.h"

#define REFRESH_PERIOD_MS 	1000U/REFRESH_RATE_HZ
#define COLUMN_PERIOD_MS REFRESH_PERIOD_MS/DISPLAY_COUNT

#define COLUMN_PERIOD_TICKS	PISR_MS_TO_TICKS(COLUMN_PERIOD_MS)


void refreshColumns(void);
static uint8_t numberToSegments(uint8_t num);

static uint8_t current_column;

bool display_INIT(void)
{
	if(!pisrRegister(refreshColumns, COLUMN_PERIOD_TICKS))
	{
		return false;
	}

	return true;
}

display_t print(uint8_t * data, uint8_t data_length,
		uint8_t selection, uint8_t mode, bool private, uint8_t row)
{
	display_t output = 0;
	uint8_t selection_column = 0;
	uint8_t index = 0;
	uint8_t current_column_temp = current_column;
	output.sel = current_column_temp & 0x03;

	if(mode == EDITING)
	{
		if(data_length >= DISPLAY_COUNT - 1)
		{
			selection_column = 3;
			index = data_length - DISPLAY_COUNT + 1 + current_column_temp;
		} else
		{
			selection_column = data_length;
			index = current_column_temp;
		}

		if(current_column_temp == selection_column)
		{
			output.seg = numberToSegments(selection, true);
		} else
		{
			if(private)
			{
				output.seg = numberToSegments(15, false);
			} else
			{
				output.seg = numberToSegments(data[index], false);
			}
		}
	} else if(mode == COMPLETE)
	{
		index = current_column_temp + row * DISPLAY_COUNT;
		output.seg = numberToSegments(data[index], false);
	}

	return output;
}

void refreshColumns(void)
{
	current_column = (current_column + 1) & 0x03;
}

static uint8_t numberToSegments(uint8_t num, bool decimalPoint)
{
    static const uint8_t segments[] = {
        0x3F,  // 0: a b c d e f
        0x06,  // 1: b c
        0x5B,  // 2: a b d e g
        0x4F,  // 3: a b c d g
        0x66,  // 4: b c f g
        0x6D,  // 5: a c d f g
        0x7D,  // 6: a c d e f g
        0x07,  // 7: a b c
        0x7F,  // 8: a b c d e f g
        0x6F,  // 9: a b c d f g

        0x70,  // 10: e f g
        0x76,  // 11: b c e f g
        0x7C,  // 12: c d e f g
        0x39,  // 13: a d e f
        0x50,  // 14: e g

		0x40,  // 15: g
    };

    if (num > 15)
        return 0x00;

    return decimalPoint ? segments[num] | 0x80 : segments[num];
}

