#include "flash_storage.h"
#include "hardware.h"
#include "MK64F12.h"
#include <string.h>

#define FLASH_DB_MAGIC 0x55444231U
#define FLASH_DB_VERSION 1U


#define FLASH_STORAGE_ADDRESS     0x000FF000U
#define FLASH_SECTOR_SIZE         4096U
#define FLASH_PHRASE_SIZE         8U
#define FTFE_CMD_PROGRAM_PHRASE   0x07U
#define FTFE_CMD_ERASE_SECTOR     0x09U

typedef struct
{
    uint32_t magic;
    uint8_t version;
    uint8_t users_cant;
    uint8_t reserved[2];

    user_t users[USERS_IN_SYSTEM_MAX];

} flash_database_t;

#define FLASH_DB_PROGRAM_SIZE \
    ((sizeof(flash_database_t) + FLASH_PHRASE_SIZE - 1u) & \
    ~(FLASH_PHRASE_SIZE - 1u))

static bool flashLaunchCommand(void);
static bool flashEraseSector(uint32_t address);
static bool flashProgramPhrase(uint32_t address, const uint8_t data[FLASH_PHRASE_SIZE]);

bool loadUsersFromFlash(user_t * users, uint8_t * users_cant)
{
    const flash_database_t * database =
        (const flash_database_t *) FLASH_STORAGE_ADDRESS;

    /* Check whether Flash contains our database */
    if(database->magic != FLASH_DB_MAGIC)
    {
        return false;
    }

    /* Check database format version */
    if(database->version != FLASH_DB_VERSION)
    {
        return false;
    }

    /* Sanity check */
    if(database->users_cant >= USERS_IN_SYSTEM_MAX)
    {
        return false;
    }

    memcpy(users,
           database->users,
           sizeof(database->users));

    *users_cant = database->users_cant;

    return true;
}

bool saveUsersToFlash(const user_t *users, uint8_t users_cant)
{
    flash_database_t database;
    uint8_t buffer[FLASH_DB_PROGRAM_SIZE];

    uint8_t total_users = users_cant + 1u;


    /* Check arguments */
    if(users == NULL)
    {
        return false;
    }

    /*
     * users_cant does not include administrator.
     * Maximum value is therefore USERS_IN_SYSTEM_MAX - 1.
     */
    if(users_cant >= USERS_IN_SYSTEM_MAX)
    {
        return false;
    }


    /*
     * Initialize the whole database to erased Flash value.
     * This also leaves unused user slots as 0xFF.
     */
    memset(&database, 0xFF, sizeof(database));
    database.magic      = FLASH_DB_MAGIC;
    database.version    = FLASH_DB_VERSION;
    database.users_cant = users_cant;


    /*
     * Copy administrator + all currently registered users.
     *
     * users[0]              -> administrator
     * users[1..users_cant] -> regular users
     */
    memcpy(database.users,
           users,
           total_users * sizeof(user_t));


    /*
     * Create an 8-byte aligned programming image.
     * The padding at the end remains 0xFF.
     */
    memset(buffer, 0xFF, sizeof(buffer));
    memcpy(buffer, &database, sizeof(database));


    /* Erase the complete 4 KB storage sector */
    if(!flashEraseSector(FLASH_STORAGE_ADDRESS))
    {
        return false;
    }


    /*
     * Program all phrases EXCEPT the first one.
     *
     * The first phrase contains:
     *   magic
     *   version
     *   users_cant
     *   reserved
     *
     * We program it last so that magic acts as a commit marker.
     */
    for(uint32_t offset = FLASH_PHRASE_SIZE;
        offset < FLASH_DB_PROGRAM_SIZE;
        offset += FLASH_PHRASE_SIZE)
    {
        if(!flashProgramPhrase(FLASH_STORAGE_ADDRESS + offset,
                               &buffer[offset]))
        {
            return false;
        }
    }


    /*
     * Commit database.
     * Once this phrase is successfully programmed,
     * loadUsersFromFlash() will recognize the database as valid.
     */
    if(!flashProgramPhrase(FLASH_STORAGE_ADDRESS, buffer))
    {
        return false;
    }


    return true;
}

static bool flashLaunchCommand(void)
{
    uint32_t primask;
    uint8_t status;

    /* Save current interrupt state and disable interrupts */
    primask = __get_PRIMASK();
    __disable_irq();

    /* Launch previously loaded FTFE command */
    FTFE->FSTAT = FTFE_FSTAT_CCIF_MASK;

    /* Wait until command finishes */
    while((FTFE->FSTAT & FTFE_FSTAT_CCIF_MASK) == 0u)
    {
    }

    status = FTFE->FSTAT;

    /* Restore previous interrupt state */
    __set_PRIMASK(primask);

    /* Check command result */
    if(status & (FTFE_FSTAT_ACCERR_MASK  |
                 FTFE_FSTAT_FPVIOL_MASK |
                 FTFE_FSTAT_RDCOLERR_MASK |
                 FTFE_FSTAT_MGSTAT0_MASK))
    {
        return false;
    }

    return true;
}

static bool flashEraseSector(uint32_t address)
{
    /* We only allow erasing our reserved storage sector */
    if(address != FLASH_STORAGE_ADDRESS)
    {
        return false;
    }

    /* Wait until FTFE is ready */
    while((FTFE->FSTAT & FTFE_FSTAT_CCIF_MASK) == 0u)
    {
    }

    /*
     * Clear previous error flags.
     * These bits are cleared by writing a 1 to them.
     */
    FTFE->FSTAT = FTFE_FSTAT_RDCOLERR_MASK |
                  FTFE_FSTAT_ACCERR_MASK   |
                  FTFE_FSTAT_FPVIOL_MASK;

    /* Load Erase Flash Sector command */
    FTFE->FCCOB0 = FTFE_CMD_ERASE_SECTOR;

    /* Load 24-bit Flash address */
    FTFE->FCCOB1 = (uint8_t)(address >> 16);
    FTFE->FCCOB2 = (uint8_t)(address >> 8);
    FTFE->FCCOB3 = (uint8_t)(address);

    /* Launch and wait for completion */
    return flashLaunchCommand();
}

static bool flashProgramPhrase(uint32_t address,
                               const uint8_t data[FLASH_PHRASE_SIZE])
{
    /* Address must be aligned to an 8-byte phrase */
    if((address & 0x7u) != 0u)
    {
        return false;
    }

    /* Only allow writes inside our reserved sector */
    if((address < FLASH_STORAGE_ADDRESS) ||
       (address > (FLASH_STORAGE_ADDRESS +
                   FLASH_SECTOR_SIZE -
                   FLASH_PHRASE_SIZE)))
    {
        return false;
    }

    /* Wait until FTFE is ready */
    while((FTFE->FSTAT & FTFE_FSTAT_CCIF_MASK) == 0u)
    {
    }

    /* Clear previous error flags */
    FTFE->FSTAT = FTFE_FSTAT_RDCOLERR_MASK |
                  FTFE_FSTAT_ACCERR_MASK   |
                  FTFE_FSTAT_FPVIOL_MASK;

    /* Load Program Phrase command */
    FTFE->FCCOB0 = FTFE_CMD_PROGRAM_PHRASE;

    /* Load 24-bit Flash address */
    FTFE->FCCOB1 = (uint8_t)(address >> 16);
    FTFE->FCCOB2 = (uint8_t)(address >> 8);
    FTFE->FCCOB3 = (uint8_t)(address);

    /* Load the 8 bytes to be programmed */
    FTFE->FCCOB4 = data[3];
    FTFE->FCCOB5 = data[2];
    FTFE->FCCOB6 = data[1];
    FTFE->FCCOB7 = data[0];

    FTFE->FCCOB8 = data[7];
    FTFE->FCCOB9 = data[6];
    FTFE->FCCOBA = data[5];
    FTFE->FCCOBB = data[4];

    /* Launch and wait for completion */
    return flashLaunchCommand();
}
