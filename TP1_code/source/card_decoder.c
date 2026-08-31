#include "card_decoder.h"

#define START_SENTINEL 0x0B  // 1011 en binario
#define FIELD_SEPARATOR 0x0D // 1101 en binario

// Verifica la paridad impar de un nibble (4 bits) más su bit de paridad
static bool check_odd_parity(uint8_t val, bool parity_bit) 
{
    uint8_t ones = 0;

    for (int i = 0; i < 4; i++) 
    {
        if ((val >> i) & 1) ones++;
    }
    
    if (parity_bit) ones++;
    
    return ((ones & 0x01) == 0x01);
}

bool decode_card_id(const uint8_t *raw_data, uint8_t total_bits, uint8_t *id_out) 
{
    uint8_t current_bit_index = 0;
    uint8_t character_value = 0;
    uint8_t digit_count = 0;
    uint8_t id_index = 0;
    bool found_ss = false; // Flag de estado

    for (current_bit_index = 0; current_bit_index + 5 <= total_bits; current_bit_index++) 
    {
        // 1. Leemos 5 posiciones consecutivas
        for (int i = 0; i < 5; i++) 
        {
            if (raw_data[current_bit_index + i] == 1) {
                character_value |= (1 << i); 
            }
        }

        // 2. Verificamos la paridad impar
        bool parity_bit = (character_value >> 4) & 0x01;
        if (!check_odd_parity(character_value & 0x0F, parity_bit)) 
        {
            character_value = 0; 
            continue;
        }

        // 3. Nos quedamos solo con los datos (aislamos el bit de paridad)
        character_value &= 0x0F; 

        // 4. MÁQUINA DE ESTADOS
        if (!found_ss) 
        {
            // ESTADO A: Todavía no encontramos el inicio
            if (character_value == START_SENTINEL) 
            {
                found_ss = true; // Levantamos el flag de estado
                digit_count = 0;
                id_index = 0;
                current_bit_index += 4; // Salto de 5 (4 acá + 1 del for)
            }
            // Si no es el Start Sentinel, no hacemos nada y el for avanza 1 bit
        } 
        else 
        {
            // ESTADO B: Ya detectamos el SS y estamos leyendo el ID
            if (character_value == FIELD_SEPARATOR) 
            {
                id_out[id_index] = '\0'; // Cerramos el string en C
                return true; // Lectura exitosa
            } 
            else 
            {
                id_out[id_index++] = character_value + '0';
                digit_count++;
                
                if (digit_count >= ID_LENGTH)    
                {
                    id_out[id_index] = '\0'; // Cerramos el string
                    return true; // Leímos los 8 justos
                }
                
                current_bit_index += 4; // Salto de 5 para el próximo dígito
            }
        }
        
        character_value = 0; // Reiniciamos para el próximo carácter
    }

    return false; // Si el bucle termina sin retornar true, la lectura falló
}
