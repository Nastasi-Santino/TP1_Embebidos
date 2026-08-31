#ifndef CARD_DECODER_H_
#define CARD_DECODER_H_

/*******************************************************************************
* INCLUDE HEADER FILES 
*******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define ID_LENGTH 8

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// Recibe el buffer crudo (como uint8_t), la cantidad de bits y el arreglo de salida.
// Usamos "const" para asegurarnos de que el decodificador no modifique el buffer original.
bool decode_card_id(const uint8_t *raw_data, uint8_t total_bits, uint8_t *id_out);

#endif /* CARD_DECODER_H */
