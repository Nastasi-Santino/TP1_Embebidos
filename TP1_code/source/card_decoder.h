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
#define TRACK2_MAX_PAN_LENGTH          19U
#define TRACK2_DATE_LENGTH              4U
#define TRACK2_SERVICE_CODE_LENGTH      3U

#define TRACK2_MAX_ADDITIONAL_LENGTH   28U

#define TRACK2_MAX_CHARACTERS          40U

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct
{
    uint8_t pan[TRACK2_MAX_PAN_LENGTH];
    uint8_t pan_length;
    uint8_t expiration_date[TRACK2_DATE_LENGTH];
    uint8_t service_code[TRACK2_SERVICE_CODE_LENGTH];
    uint8_t additional_data[TRACK2_MAX_ADDITIONAL_LENGTH];
    uint8_t additional_length;
} track2_card_t;

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
