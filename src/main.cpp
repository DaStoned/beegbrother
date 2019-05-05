/**
@file main.c
@brief Entry point for beegbrother application
*/

#include "common.hpp"
#include "DriverGpio.hpp"
#include "Timers.hpp"
#include "DriverAm2302.hpp"
#include "DriverHx811.hpp"

extern "C" {
    // Include the C-only SDK headers
    #include "osapi.h"
    #include "user_interface.h"

    // Declare these as C functions to allow SDK to call them
    void ICACHE_FLASH_ATTR user_pre_init(void);
    void ICACHE_FLASH_ATTR user_init();
}

#define APP_VERSION_MAJOR       0
#define APP_VERSION_MINOR       1
#define APP_VERSION_STR         (TOSTRING(APP_VERSION_MAJOR) "." TOSTRING(APP_VERSION_MINOR))

//----------------------- Set up partition table ----------------------

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
    {SYSTEM_PARTITION_BOOTLOADER,       0x0,                                    0x1000},
    {SYSTEM_PARTITION_OTA_1,            0x1000,                                 SYSTEM_PARTITION_OTA_SIZE},
    {SYSTEM_PARTITION_OTA_2,            SYSTEM_PARTITION_OTA_2_ADDR,            SYSTEM_PARTITION_OTA_SIZE},
    {SYSTEM_PARTITION_RF_CAL,           SYSTEM_PARTITION_RF_CAL_ADDR,           0x1000},
    {SYSTEM_PARTITION_PHY_DATA,         SYSTEM_PARTITION_PHY_DATA_ADDR,         0x1000},
    {SYSTEM_PARTITION_SYSTEM_PARAMETER, SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000},
};

void ICACHE_FLASH_ATTR user_pre_init(void) {
    // Partition table mandated by SDK 3.0.0
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
        os_printf("Failed to register system partition table!\n");
        while(1);
    }
}

//--------------------- Invoke global constructors --------------------

// The ESP8266 toolchain doesn't automatically call C++ constructors for
// global objects, so we have to call them manually with the help of a
// modified linker script.
// https://www.esp8266.com/viewtopic.php?f=9&t=478&start=8
extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

static void do_global_ctors(void)
{
        void (**p)(void);
        for (p = &__init_array_start; p != &__init_array_end; ++p)
                (*p)();
}
//------------------------- Set up user tasks -------------------------

const unsigned int mainTaskPrio = 0;
const unsigned int mainTaskQueueLen = 1;
const unsigned int tracePeriodUs = 1000000;
os_event_t mainTaskQueue[mainTaskQueueLen];

static void mainTask(os_event_t *events);

// ESP-12 modules have LED on GPIO2. Change to another GPIO
// for other boards.
static const int ledPin = 2;

static void ICACHE_FLASH_ATTR mainTask(os_event_t *events)
{
    //static int i = 0;
    static uint32_t tsPrev = 0;
    uint32_t tsNow = system_get_time();
    if (tsPrev == 0) {
        tsPrev = system_get_time();
    }
    if (tsPrev + tracePeriodUs <= tsNow || tsPrev > tsNow) {
        //os_printf("Hello %d prev: %u now: %u\n", i++, tsPrev, tsNow);
        tsPrev = tsNow;
          //Do blinky stuff
        if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << ledPin)) {
            // set gpio low
            gpio_output_set(0, (1 << ledPin), (1 << ledPin), 0);
        } else {
            // set gpio high
            gpio_output_set((1 << ledPin), 0, (1 << ledPin), 0);
        }
    } 
    system_os_post(mainTaskPrio, 0, 0 );
}

// Global objects
DriverGpio gpio;
Timers timers;
DriverAm2302 tempSens(gpio, timers);
DriverHx811 loadSens(gpio, timers);

static volatile os_timer_t timerReadTemp;
static volatile os_timer_t timerReadLoad;

static void ICACHE_FLASH_ATTR readTempCb(os_event_t *events) {
    if (tempSens.update()) {
        os_printf("Temp: %d dC, humidity: %u d%%\n", tempSens.getTemperature(), tempSens.getHumidity());
    } else {
        os_printf("Temp/Hum sensor failed to update!\n");
    }
    IfSensorTempHumidity::DiagInfo diag = {0};
    tempSens.getDiagInfo(&diag);
    if (diag.readFailures || diag.readGlitches) {
        os_printf("Temp/Hum sensor read failures %u, glitches %u\n", diag.readFailures, diag.readGlitches);
    }
}

static void ICACHE_FLASH_ATTR readLoadCb(os_event_t *events) {
    if (loadSens.update()) {
        os_printf("Load: %d \n", loadSens.getLoad());
    } else {
        os_printf("Load sensor failed to update!\n");
    }
}

void ICACHE_FLASH_ATTR user_init()
{
    // Run C++ global constructors
    do_global_ctors();

    os_printf("App version: %s, SDK version:%s\n", APP_VERSION_STR, system_get_sdk_version());

    // Prep WiFi
    char ssid[32] = WIFI_SSID;
    char password[64] = WIFI_PASSWORD;
    struct station_config stationConf;
    //Set station mode
    wifi_set_opmode(STATION_MODE);
    //Set AP settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    if (!wifi_station_set_config(&stationConf)) {
        os_printf("Failed to set WiFi config!\n");
    }

    if (!gpio.init()) {
        os_printf("Failed to init GPIO!\n");
    }

    // Start blinky loop
    system_os_task(mainTask, mainTaskPrio, mainTaskQueue, mainTaskQueueLen);
    system_os_post(mainTaskPrio, 0, 0 );

    // Start temperature/humidity sensor loop
    if (!tempSens.init(IfGpio::PIN4)) {
        os_printf("Failed to init temperature/humidity sensor\n");
    }
    os_timer_setfn((os_timer_t*)&timerReadTemp, (os_timer_func_t *)readTempCb, NULL);
    os_timer_arm((os_timer_t*)&timerReadTemp, 20000, 1);

    // Start load sensor loop
    if (!loadSens.init(IfGpio::PIN14, IfGpio::PIN12)) {
        os_printf("Failed to init load sensor\n");
    } else {
        os_printf("Inited load sensor\n");
    }

    os_timer_setfn((os_timer_t*)&timerReadLoad, (os_timer_func_t *)readLoadCb, NULL);
    os_timer_arm((os_timer_t*)&timerReadLoad, 2000, 1);
}