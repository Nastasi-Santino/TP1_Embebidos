/***************************************************************************//**
  @file     card_decoder.h
  @brief    Magnetic stripe Track 2 card decoder interface
 ******************************************************************************/

#ifndef CARD_DECODER_H_
#define CARD_DECODER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "card_reader.h"

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Decodes and validates a Track 2 raw bitstream payload.
 * @param raw_data Pointer to raw bits buffer.
 * @param total_bits Total count of bits recorded during card swipe.
 * @param card Pointer to output structure where parsed fields will be saved.
 * @return True if bitstream conforms to ISO 7813 Track 2 format and passes validation, false otherwise.
 */
bool card_decode_track2(
        const volatile uint8_t *raw_data,
        uint16_t total_bits,
        track2_card_t *card
);

#endif /* CARD_DECODER_H_ */