#ifndef CARD_DECODER_H_
#define CARD_DECODER_H_

/*******************************************************************************
* INCLUDE HEADER FILES 
*******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "card_reader.h"


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Decodifica y valida una trama Track 2.
 * @param raw_data: Buffer de data cruda.
 * @param total_bits: Cantidad real de bits recibidos.
 * @param card: Estructura donde se almacenan los datos decodificados.
 * @return Validacion de protocolo.
 */
bool card_decode_track2(
        const volatile uint8_t *raw_data,
        uint16_t total_bits,
        track2_card_t *card
);

#endif /* CARD_DECODER_H */
