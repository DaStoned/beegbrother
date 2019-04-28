/**
@file main.c
@brief Entry point for beegbrother application
*/

#include "common.hpp"

extern "C" {
    // Include the C-only SDK headers
    #include "osapi.h"
    #include "user_interface.h"

    // Declare these as C functions to allow SDK to call them
    void ICACHE_FLASH_ATTR user_pre_init(void);
    void ICACHE_FLASH_ATTR user_init();
}

#define user_procTaskPrio       0
#define user_procTaskQueueLen   1
#define PRINT_DELAY_US          1000000LL

#define APP_VERSION_MAJOR       0
#define APP_VERSION_MINOR       1
#define APP_VERSION_STR         (TOSTRING(APP_VERSION_MAJOR) "." TOSTRING(APP_VERSION_MINOR))

os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop(os_event_t *events);

// ESP-12 modules have LED on GPIO2. Change to another GPIO
// for other boards.
static const int pin = 2;

// Wemos D1 mini uses flash map 4
#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE               0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR             0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR            0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR          0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR  0xfd000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE               0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR             0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR            0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR          0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR  0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE               0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR             0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR            0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR          0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR  0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE               0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR             0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR            0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR          0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR  0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE               0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR             0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR            0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR          0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR  0x3fd000
#else
#error "The flash map is not supported"
#endif

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER,          0x0,                                    0x1000},
    { SYSTEM_PARTITION_OTA_1,               0x1000,                                 SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,               SYSTEM_PARTITION_OTA_2_ADDR,            SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,              SYSTEM_PARTITION_RF_CAL_ADDR,           0x1000},
    { SYSTEM_PARTITION_PHY_DATA,            SYSTEM_PARTITION_PHY_DATA_ADDR,         0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER,    SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000},
};

void ICACHE_FLASH_ATTR user_pre_init(void) {
    // Partition table mandated by SDK 3.0.0
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
        os_printf("Failed to register system partition table!\n");
        while(1);
    }
}

static void ICACHE_FLASH_ATTR loop(os_event_t *events)
{
    static int i = 0;
    static uint32_t time_prev = 0;
    uint32_t time_now = system_get_time();
    if (time_prev == 0) {
        time_prev = system_get_time();
    }
    if (time_prev + PRINT_DELAY_US <= time_now || time_prev > time_now) {
        os_printf("Hello %ld prev: %lu now: %lu\n", i++, time_prev, time_now);
        time_prev = time_now;

          //Do blinky stuff
        if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << pin)) {
            // set gpio low
            gpio_output_set(0, (1 << pin), (1 << pin), 0);
        } else {
            // set gpio high
            gpio_output_set((1 << pin), 0, (1 << pin), 0);
        }
    } 

    system_os_post(user_procTaskPrio, 0, 0 );
}

void ICACHE_FLASH_ATTR user_init()
{
    char ssid[32] = WIFI_SSID;
    char password[64] = WIFI_PASSWORD;
    struct station_config stationConf;

    os_printf("App version: %s, SDK version:%s\n", APP_VERSION_STR, system_get_sdk_version());
    //Set station mode
    wifi_set_opmode(STATION_MODE);

    //Set ap settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);

    // init gpio subsytem
    gpio_init();

    //Start os task
    system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
    system_os_post(user_procTaskPrio, 0, 0 );
}