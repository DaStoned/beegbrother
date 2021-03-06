/**
@file main.c
@brief Entry point for beegbrother application
*/

#include "common.hpp"
#include "DriverGpio.hpp"
#include "Timers.hpp"
#include "DriverAm2302.hpp"
#include "DriverHx711.hpp"
#include "Scale.hpp"
#include "espmissingincludes.h"

extern "C" {
    // Include the C-only SDK headers
    #include "osapi.h"
    #include "user_interface.h"
    #include "espconn.h"

    // Declare these as C functions to allow SDK to call them
    void ICACHE_FLASH_ATTR user_pre_init(void);
    void ICACHE_FLASH_ATTR user_init();
}

using namespace common;

#define APP_VERSION_MAJOR       0
#define APP_VERSION_MINOR       2
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

static void ICACHE_FLASH_ATTR do_global_ctors(void)
{
        void (**p)(void);
        for (p = &__init_array_start; p != &__init_array_end; ++p)
                (*p)();
}

//------------------------- Set up user tasks -------------------------

// Global objects
DriverGpio gpio;
Timers timers;
DriverAm2302 tempSens(gpio, timers);
DriverHx711 loadSens(gpio, timers);
Scale scale(loadSens, -24200, -418509);

static volatile os_timer_t timerReadTemp;
static volatile os_timer_t timerReadLoad;
static volatile os_timer_t timerBtnDebounce;

const unsigned int mainTaskPrio = 0;
const unsigned int mainTaskQueueLen = 1;
const unsigned int tracePeriodUs = 1000000;
os_event_t mainTaskQueue[mainTaskQueueLen];

// ESP-12 modules have LED on GPIO2. Change to another GPIO
// for other boards.
static const int ledPin = 2;
static volatile bool buttonPress = false;
static volatile bool wifiConnected = false;

static void ICACHE_FLASH_ATTR mainTask(os_event_t *events)
{
    //static int i = 0;
    static uint32_t tsPrev = 0;
    static bool mDnsDone = false;
    uint32_t tsNow = system_get_time();
    double weight = 0.0;
    char floatBuf[20];

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
    if (buttonPress) {
        weight = scale.getWeight();
        scale.tare();
        os_printf("Taring scale, before %s kg,", double_snprintf3(floatBuf, sizeof(floatBuf), weight));
        weight = scale.getWeight();
        os_printf(" after %s kg\n", double_snprintf3(floatBuf, sizeof(floatBuf), weight));
        buttonPress = false;
    }

    if (wifiConnected && !mDnsDone) {
        // Set up mDNS
        static struct mdns_info dnsInfo;
        struct ip_info ipInfo;
        if (!wifi_get_ip_info(STATION_IF, &ipInfo)) {
            os_printf("Failed to get IP info\n");
        }
        //static char hostName[] = "taru00";
        dnsInfo.host_name = (char*) "taru00";
        dnsInfo.ipAddr = ipInfo.ip.addr;
        dnsInfo.server_name = (char*) "taruserver";
        dnsInfo.server_port = 8080;
        espconn_mdns_init(&dnsInfo);
        mDnsDone = true;
    }
    system_os_post(mainTaskPrio, 0, 0 );
}

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
    double weight;
    char buf[20];
    weight = scale.getWeight();
    //c_sprintf(buf, "%.3f", weight);
    os_printf("Weight: %s kg\n", double_snprintf3(buf, sizeof(buf), weight));
}

static bool btnInDebounce = false;

static void ICACHE_FLASH_ATTR gpio_intr_handler(void *arg) {
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_DISABLE);
    btnInDebounce = true;
    os_timer_arm((os_timer_t*)&timerBtnDebounce, 50, 0);
    os_printf("B %0X\n", gpio_status);
}

static void ICACHE_FLASH_ATTR btnDebounceCb(os_event_t *events) {
    if (btnInDebounce) {
        if (gpio.getPin(IfGpio::PIN5)) {
            os_printf("Yup, B\n");
            buttonPress = true;
            // Post release delay
            os_timer_arm((os_timer_t*)&timerBtnDebounce, 50, 0);
        } else {
            os_printf("B glitch!\n");
            gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_POSEDGE);
        }
        btnInDebounce = false;
    } else {
        // Re-enable interrupt
        gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_POSEDGE);
    }
}

void ICACHE_FLASH_ATTR wifi_handler_event_cb(System_Event_t *evt) {
    os_printf("WiFI: event %x\n", evt->event);
    switch (evt->event) {
    case EVENT_STAMODE_CONNECTED:
        os_printf("WiFI: connect to ssid %s, channel %d\n",
        evt->event_info.connected.ssid,
        evt->event_info.connected.channel);
        break;
    case EVENT_STAMODE_DISCONNECTED:
        os_printf("WiFI: disconnect from ssid %s, reason %d\n",
        evt->event_info.disconnected.ssid,
        evt->event_info.disconnected.reason);
        break;
    case EVENT_STAMODE_AUTHMODE_CHANGE:
        os_printf("WiFI: mode: %d -> %d\n",
        evt->event_info.auth_change.old_mode,
        evt->event_info.auth_change.new_mode);
        break;
    case EVENT_STAMODE_GOT_IP:
        os_printf("WiFI: ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
        IP2STR(&evt->event_info.got_ip.ip),
        IP2STR(&evt->event_info.got_ip.mask),
        IP2STR(&evt->event_info.got_ip.gw));
        os_printf("\n");
        wifiConnected = true;
        break;
    case EVENT_SOFTAPMODE_STACONNECTED:
        os_printf("WiFI: station: " MACSTR "join, AID = %d\n",
        MAC2STR(evt->event_info.sta_connected.mac),
        evt->event_info.sta_connected.aid);
        break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
        os_printf("WiFI: station: " MACSTR "leave, AID = %d\n",
        MAC2STR(evt->event_info.sta_disconnected.mac),
        evt->event_info.sta_disconnected.aid);
        break;
    default:
        break;
    }
}

void ICACHE_FLASH_ATTR user_init()
{
    // Run C++ global constructors
    do_global_ctors();

    os_printf("App version: %s, SDK version:%s\n", APP_VERSION_STR, system_get_sdk_version());

#ifdef WIFI_SSID
    // Prep WiFi
    const char ssid[sizeof(WIFI_SSID)] = WIFI_SSID;
    const char password[sizeof(WIFI_PASSWORD)] = WIFI_PASSWORD;
    struct station_config stationConf;
    //Set station mode
    wifi_set_opmode(STATION_MODE);
    //Set AP settings
    os_memcpy(&stationConf.ssid, ssid, sizeof(ssid));
    os_memcpy(&stationConf.password, password, sizeof(password));
    if (!wifi_station_set_config(&stationConf)) {
        os_printf("Failed to set WiFi config!\n");
    } else {
        os_printf("WiFi initialized, lengths %u/%u\n", sizeof(ssid) - 1, sizeof(password) - 1);
    }
#else
    // Disable wifi
    os_printf("Disconnecting WiFi\n");
    wifi_station_disconnect();
#endif
    if (!gpio.init()) {
        os_printf("Failed to init GPIO!\n");
    } else {
        ETS_GPIO_INTR_DISABLE();
        ETS_GPIO_INTR_ATTACH(gpio_intr_handler, NULL);
        gpio.setPinMode(IfGpio::PIN5, IfGpio::MODE_IN);
        GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(5));
        gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_POSEDGE);
        ETS_GPIO_INTR_ENABLE();
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
    if (!loadSens.init(IfGpio::PIN14, IfGpio::PIN12, DriverHx711::ChGain_A128)) {
        os_printf("Failed to init load sensor\n");
    } else {
        os_printf("Inited load sensor\n");
    }

    os_timer_setfn((os_timer_t*)&timerReadLoad, (os_timer_func_t *)readLoadCb, NULL);
    os_timer_arm((os_timer_t*)&timerReadLoad, 2000, 1);

    os_timer_setfn((os_timer_t*)&timerBtnDebounce, (os_timer_func_t *)btnDebounceCb, NULL);

    wifi_set_event_handler_cb(wifi_handler_event_cb);

}