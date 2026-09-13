/***************************************************************************//**
  @file     card_decoder.c
  @brief    Magnetic stripe Track 2 card decoder implementation
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "card_decoder.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define TRACK2_START_SENTINEL     0x0BU /**< Start Sentinel delimiter pattern (';' / 11) */
#define TRACK2_FIELD_SEPARATOR    0x0DU /**< Field Separator delimiter pattern ('=' / 13) */
#define TRACK2_END_SENTINEL       0x0FU /**< End Sentinel delimiter pattern ('?' / 15) */

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Reads 5 sequential bits from raw buffer and packs them LSB-first into a 5-bit character word.
 * @param raw_data Pointer to raw bitstream array.
 * @param start_bit Bit offset position to start reading.
 * @return 5-bit character value containing data bits and parity bit.
 */
static uint8_t read_character(const volatile uint8_t *raw_data, uint16_t start_bit);

/**
 * @brief Validates odd parity of a 5-bit card character word.
 * @param character 5-bit word (4 data bits + 1 parity bit).
 * @return True if odd parity condition is met, false otherwise.
 */
static bool check_odd_parity(uint8_t character);

/**
 * @brief Scans bitstream for the initial Start Sentinel pattern.
 * @param raw_data Pointer to raw bitstream array.
 * @param total_bits Total length of raw bitstream in bits.
 * @param start_bit Pointer to store starting bit index of found Start Sentinel.
 * @return True if Start Sentinel was located, false otherwise.
 */
static bool find_start_sentinel(const volatile uint8_t *raw_data, uint16_t total_bits, uint16_t *start_bit);

/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

/**
 * @brief Decodes raw bitstream into Track 2 magnetic card structured fields.
 * @param raw_data Pointer to raw bitstream array.
 * @param total_bits Total number of bits available in raw buffer.
 * @param card Pointer to target card structure to populate.
 * @return True if Track 2 data was successfully parsed and verified (Parity & LRC), false otherwise.
 */
bool card_decode_track2(const volatile uint8_t *raw_data, uint16_t total_bits, track2_card_t *card)
{
    if ((raw_data == (uint8_t *)0) || (card == (track2_card_t *)0))
    {
        return false;
    }

    *card = (track2_card_t){0};

    uint16_t bit_index;

    /* Search bitstream for valid Start Sentinel */
    if (!find_start_sentinel(raw_data, total_bits, &bit_index))
    {
        return false;
    }

    uint8_t calculated_lrc = 0;          /* Longitudinal Redundancy Check (LRC) accumulator */
    bool field_separator_found = false;
    uint8_t post_separator_count = 0;
    uint8_t character_count = 0;

    /* Parse sequential 5-bit character frames */
    while (bit_index + 5U <= total_bits)
    {
        uint8_t character = read_character(raw_data, bit_index);

        if (!check_odd_parity(character))
        {
            return false;
        }

        /* Extract 4-bit data payload excluding parity bit */
        uint8_t value = character & 0x0FU;

        /* First word verification (Start Sentinel) */
        if (character_count == 0U)
        {
            if (value != TRACK2_START_SENTINEL)
            {
                return false;
            }

            calculated_lrc ^= value; /* Start Sentinel included in LRC calculation */

            character_count++;
            bit_index += 5U;

            continue;
        }

        /* End Sentinel handling */
        if (value == TRACK2_END_SENTINEL)
        {
            if (!field_separator_found) /* Cannot terminate prior to Field Separator */
            {
                return false;
            }

            if (post_separator_count < 4U) /* Expiration date requires minimum 4 digits */
            {
                return false;
            }

            calculated_lrc ^= value;    /* End Sentinel included in LRC calculation */

            character_count++;

            if ((character_count + 1U) > TRACK2_MAX_CHARACTERS) /* Ensure space remains for LRC character */
            {
                return false;
            }

            bit_index += 5U;

            if (bit_index + 5U > total_bits)
            {
                return false;
            }

            /* Read and validate LRC character frame */
            uint8_t lrc_character = read_character(raw_data, bit_index);

            if (!check_odd_parity(lrc_character))
            {
                return false;
            }

            uint8_t received_lrc = lrc_character & 0x0FU;

            if (received_lrc != calculated_lrc) /* Final validation: LRC match */
            {
                return false;
            }

            return true; /* All checks passed successfully */
        }

        /* Processing data payload before Field Separator (PAN) */
        if (!field_separator_found)
        {
            if (value == TRACK2_FIELD_SEPARATOR)
            {
                if (card->pan_length == 0U) /* Empty PAN field not allowed */
                {
                    return false;
                }

                field_separator_found = true;

                calculated_lrc ^= value; /* Field Separator included in LRC calculation */

                character_count++;
                bit_index += 5U;

                continue;
            }

            if (value > 9U) /* PAN characters must be strictly numeric (0-9) */
            {
                return false;
            }

            if (card->pan_length >= TRACK2_MAX_PAN_LENGTH)
            {
                return false;
            }

            card->pan[card->pan_length] = value;
            card->pan_length++;
        } else  /* Processing data payload after Field Separator */
        {
            if (value > 9U) /* Expiration date, service code, and additional data must be numeric */
            {
                return false;
            }

            if (post_separator_count < TRACK2_DATE_LENGTH)
            {
                card->expiration_date[post_separator_count] = value;
            } else if(post_separator_count < (TRACK2_DATE_LENGTH + TRACK2_SERVICE_CODE_LENGTH))
            {
                card->service_code[post_separator_count - TRACK2_DATE_LENGTH] = value;
            } else
            {
                if (card->additional_length >= TRACK2_MAX_ADDITIONAL_LENGTH)
                {
                    return false;
                }

                card->additional_data[card->additional_length] = value;

                card->additional_length++;
            }

            post_separator_count++;
        }

        calculated_lrc ^= value;

        character_count++;

        if (character_count > (TRACK2_MAX_CHARACTERS - 2U)) /* Boundary guard: space for ES and LRC */
        {
            return false;
        }

        bit_index += 5U;
    }

    return false; /* Buffer depleted before encountering End Sentinel */
}

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINES & LOCAL FUNCTIONS
 ******************************************************************************/

/**
 * @brief Reads 5 sequential bits from raw buffer and packs them LSB-first into a 5-bit character word.
 */
static uint8_t read_character(const volatile uint8_t *raw_data, uint16_t start_bit)
{
    uint8_t value = 0;

    for (uint8_t i = 0; i < 5U; i++)
    {
        if (raw_data[start_bit + i])
        {
            value |= (1U << i);
        }
    }

    return value;
}

/**
 * @brief Validates odd parity of a 5-bit card character word.
 */
static bool check_odd_parity(uint8_t character)
{
    uint8_t ones = 0;

    character &= 0x1FU;

    for (uint8_t i = 0; i < 5U; i++)
    {
        if ((character >> i) & 0x01U)
        {
            ones++;
        }
    }

    return ((ones & 0x01U) != 0U);
}

/**
 * @brief Scans bitstream for the initial Start Sentinel pattern.
 */
static bool find_start_sentinel(const volatile uint8_t *raw_data, uint16_t total_bits,
        uint16_t *start_bit)
{
    for (uint16_t i = 0; i + 5U <= total_bits; i++)
    {
        uint8_t character = read_character(raw_data, i);

        if (check_odd_parity(character) &&
            ((character & 0x0FU) == TRACK2_START_SENTINEL))
        {
            *start_bit = i;
            return true;
        }
    }

    return false;
}