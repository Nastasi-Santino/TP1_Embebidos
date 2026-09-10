#ifndef CARD_READER_H_
#define CARD_READER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

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
 * @brief Initializes GPIO pins and interrupt handlers for the magnetic card reader.
 * @return True if initialization and interrupt setup succeeded, false otherwise.
 */
bool card_reader_INIT(void);

/**
 * @brief Checks if a full card swipe dataset has been captured and is ready for reading.
 * @details Automatically clears the ready status flag upon execution.
 * @return True if new card data is pending processing, false otherwise.
 */
bool data_ready(void);

/**
 * @brief Decodifica y valida una trama.
 * @param card: Estructura donde se almacenan los datos decodificados.
 * @return Validacion de protocolo.
 */
bool card_decode(track2_card_t *card);

#endif /* CARD_READER_H_ */
