#include "card_decoder.h"

#define TRACK2_START_SENTINEL     0x0BU
#define TRACK2_FIELD_SEPARATOR    0x0DU
#define TRACK2_END_SENTINEL       0x0FU

// Lee 5 bits seguidos y guarda la palabra de 5 bits en un uint8_t.
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

// Verifica que se cumpla la paridad impar: cantidad de 1's en la palabra de 5 (imcluye paridad) impar.
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

// Avanza de a 1 bit y lee palabras de 5 hasta hallar el inicializador.
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

bool card_decode_track2(const volatile uint8_t *raw_data, uint16_t total_bits, track2_card_t *card)
{
    if ((raw_data == (uint8_t *)0) || (card == (track2_card_t *)0))
    {
        return false;
    }

    *card = (track2_card_t){0};

    uint16_t bit_index;

    if (!find_start_sentinel(raw_data, total_bits, &bit_index))
    {
        return false;
    }

    uint8_t calculated_lrc = 0;				// Vamos calculando el LRC, debe coincidir con el recibido.
    bool field_separator_found = false;
    uint8_t post_separator_count = 0;
    uint8_t character_count = 0;

    while (bit_index + 5U <= total_bits)
    {
    	uint8_t character = read_character(raw_data, bit_index);

        if (!check_odd_parity(character))
        {
            return false;
        }

        // Nos quedamos con la data sin paridad.
        uint8_t value = character & 0x0FU;

        // Primer palabra.
        if (character_count == 0U)
        {
            if (value != TRACK2_START_SENTINEL)
            {
                return false;
            }

            calculated_lrc ^= value; // El inicializador tambien se cuenta en la paridad LRC.

            character_count++;
            bit_index += 5U;

            continue;
        }

        // Finalizador
        if (value == TRACK2_END_SENTINEL)
        {
            if (!field_separator_found) // No puede terminar si no encontro el separador.
            {
                return false;
            }

            if (post_separator_count < 4U) // Minimo tiene 4 de fecha.
            {
                return false;
            }

            calculated_lrc ^= value;	// el finalizador tambien cuenta para el LRC.

            character_count++;

            if ((character_count + 1U) > TRACK2_MAX_CHARACTERS) // Falta el caracter LRC.
            {
                return false;
            }

            bit_index += 5U;

            if (bit_index + 5U > total_bits)
            {
                return false;
            }

            uint8_t lrc_character = read_character(raw_data, bit_index);

            if (!check_odd_parity(lrc_character))
            {
                return false;
            }

            uint8_t received_lrc = lrc_character & 0x0FU;

            if (received_lrc != calculated_lrc) // ultima verificacion: LRC.
            {
                return false;
            }

            return true; // si llego aca cumplio toda verificacion.
        }

        if (!field_separator_found)	// Antes del separador (PAN)
        {
            if (value == TRACK2_FIELD_SEPARATOR)
            {
                if (card->pan_length == 0U) // No se permite PAN vacia.
                {
                    return false;
                }

                field_separator_found = true;

                calculated_lrc ^= value; // separador tambien cuenta para el LRC.

                character_count++;
                bit_index += 5U;

                continue;
            }

            if (value > 9U) // Todos los valores del PAN son numericos.
            {
                return false;
            }

            if (card->pan_length >= TRACK2_MAX_PAN_LENGTH)
            {
                return false;
            }

            card->pan[card->pan_length] = value;
            card->pan_length++;
        } else	// Despues del separador
        {
            if (value > 9U) // Todos numericos tambien.
            {
                return false;
            }

            if (post_separator_count < TRACK2_DATE_LENGTH)
            {
                card->expiration_date[post_separator_count] = value;
            } else if(post_separator_count <(TRACK2_DATE_LENGTH + TRACK2_SERVICE_CODE_LENGTH))
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

        if (character_count > (TRACK2_MAX_CHARACTERS - 2U)) // Si llega aca falta el ES y LRC.
        {
            return false;
        }

        bit_index += 5U;
    }

    return false; // el buffer se quedo sin data antes del ES.
}
