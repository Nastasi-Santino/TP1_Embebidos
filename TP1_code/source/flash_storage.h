#ifndef FLASH_STORAGE_H_
#define FLASH_STORAGE_H_

#include <stdint.h>
#include <stdbool.h>

#define USERS_IN_SYSTEM_MAX   10 /**< Total registered user capacity limit */

/**
 * @brief Represents a single user profile stored in system memory.
 */
typedef struct
{
    uint8_t id[8];          /**< Array storing user ID digits */
    uint8_t password[5];    /**< Array storing user password digits */
    uint8_t password_length;/**< Actual length of user's password (4 or 5)*/
} user_t;


bool loadUsersFromFlash(user_t * users, uint8_t * user_count);
bool saveUsersToFlash(const user_t *users, uint8_t users_cant);

#endif /* FLASH_STORAGE_H_ */
