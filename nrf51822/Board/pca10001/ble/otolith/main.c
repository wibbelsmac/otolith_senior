/*
 *
 * Based on the Heart Rate Service Sample Application for nRF51822 evaluation board
 *
 */

#include "adc.h"
#include "app_button.h"
#include "app_error.h"
#include "app_gpiote.h"
#include "app_timer.h"
#include "ble.h"
#include "ble_advdata.h"
#include "ble_as.h"
#include "ble_bondmngr.h"
#include "ble_conn_params.h"
#include "ble_debug_assert_handler.h"
#include "ble_eval_board_pins.h"
#include "ble_flash.h"
#include "ble_oto.h"
#include "ble_radio_notification.h"
#include "ble_srv_common.h"
#include "ble_stack_handler.h"
#include "dac_driver.h"
#include "led.h"
#include "main.h"
#include "motor.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf51_bitfields.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "pulse.h"
#include "step_counter.h"
#include "user_alarm.h"
#include "util.h"
#include <stdint.h>
#include <string.h>


#define BONDMNGR_DELETE_BUTTON_PIN_NO        EVAL_BOARD_BUTTON_1                      /**< Button used for deleting all bonded masters during startup. */

#define DEVICE_NAME                          "Otolith"                                 /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                    "NordicSemiconductor"                     /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                     40                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS           180                                       /**< The advertising timeout in units of seconds. */

#define APP_TIMER_PRESCALER                  0                                         /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS                 4                                         /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              5                                         /**< Size of timer operation queues. */

#define APP_GPIOTE_MAX_USERS                 2                                         /**< Maximum number of users of the GPIOTE handler. */

#define BUTTON_DETECTION_DELAY               APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)  /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define SECOND_1_25_MS_UNITS                 800                                       /**< Definition of 1 second, when 1 unit is 1.25 ms. */
#define SECOND_10_MS_UNITS                   100                                       /**< Definition of 1 second, when 1 unit is 10 ms. */
#define MIN_CONN_INTERVAL                    (SECOND_1_25_MS_UNITS / 2)                /**< Minimum acceptable connection interval (0.5 seconds), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL                    (SECOND_1_25_MS_UNITS)                    /**< Maximum acceptable connection interval (1 second), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                        0                                         /**< Slave latency. */
#define CONN_SUP_TIMEOUT                     (4 * SECOND_10_MS_UNITS)                  /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)/**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY        APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)/**< Time between each call to sd_ble_gap_conn_param_update after the first (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT         3                                         /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_TIMEOUT                    30                                        /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                       1                                         /**< Perform bonding. */
#define SEC_PARAM_MITM                       0                                         /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES            BLE_GAP_IO_CAPS_NONE                      /**< No I/O capabilities. */
#define SEC_PARAM_OOB                        0                                         /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE               7                                         /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE               16                                        /**< Maximum encryption key size. */

#define FLASH_PAGE_SYS_ATTR                  253                                       /**< Flash page used for bond manager system attribute information. */
#define FLASH_PAGE_BOND                      255                                       /**< Flash page used for bond manager bonding information. */

#define DEAD_BEEF                            0xDEADBEEF                                /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

static ble_gap_sec_params_t                  m_sec_params;                             /**< Security requirements for this application. */
static ble_gap_adv_params_t                  m_adv_params;                             /**< Parameters to be passed to the stack when starting advertising. */

static ble_oto_t                             m_oto;       
static ble_as_t                              m_as;

static bool                                  connected;

static void ble_evt_dispatch(ble_evt_t * p_ble_evt);



/*****************************************************************************
 * Error Handling Functions
 *****************************************************************************/


/**@brief Error handler function, which is called when an error has occurred. 
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
  // On assert, the system can only recover with a reset.
  NVIC_SystemReset();
}


/**@brief Assert macro callback function.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Connection Parameters module error handler.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}


/**@brief Bond Manager module error handler.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void bond_manager_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}


// forward declaration
static void advertising_start(void);


/**@brief Button event handler.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
static void button_event_handler(uint8_t pin_no)
{
  switch (pin_no)
  {
    case EVAL_BOARD_BUTTON_0:
      mlog_str("button 0 pressed\r\n");

//      if (connected) {
//               ble_oto_send_step_count(&m_oto);
//      }

      motor_off();
      led_stop();
      break;

    case EVAL_BOARD_BUTTON_1:
      mlog_str("button 1 pressed\r\n");

      if (!connected)
        advertising_start();
      break;

    default:
      APP_ERROR_HANDLER(pin_no);
  }
}


/* 
 * Alarm Service update handler.
 *
 * Called when the user has set an alarm remotely.
 */
void on_ble_as_update(uint16_t updated_alarm_time)
{
  user_alarm_set(updated_alarm_time);
}

/*
 * Called when the user alarm has expired
 */
void on_user_alarm_expire()
{
  motor_on();
  led_start();
}


/*****************************************************************************
 * Static Initialization Functions
 *****************************************************************************/

/**@brief Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
  uint32_t err_code;

  // Initialize timer module
  APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);

  // Init User Alarm with callback
  err_code = user_alarm_init(on_user_alarm_expire);

  APP_ERROR_CHECK(err_code);
}


/**@brief GAP initialization.
 *
 * @details This function shall be used to setup all the necessary GAP (Generic Access Profile)
 *          parameters of the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
  uint32_t                err_code;
  ble_gap_conn_params_t   gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  err_code = sd_ble_gap_device_name_set(&sec_mode, DEVICE_NAME, strlen(DEVICE_NAME));
  APP_ERROR_CHECK(err_code);

  err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT);
  APP_ERROR_CHECK(err_code);

  memset(&gap_conn_params, 0, sizeof(gap_conn_params));

  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency     = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
  APP_ERROR_CHECK(err_code);
}


/**@brief Advertising functionality initialization.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
  uint32_t      err_code;
  ble_advdata_t advdata;
  uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

  ble_uuid_t adv_uuids[] =
  {
    {BLE_UUID_OTOLITH_SERVICE,            BLE_UUID_TYPE_BLE},
    {BLE_UUID_ALARM_SERVICE,              BLE_UUID_TYPE_BLE}
  };

  // Build and set advertising data
  memset(&advdata, 0, sizeof(advdata));

  advdata.name_type               = BLE_ADVDATA_FULL_NAME;
  advdata.include_appearance      = true;
  advdata.flags.size              = sizeof(flags);
  advdata.flags.p_data            = &flags;
  advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
  advdata.uuids_complete.p_uuids  = adv_uuids;

  err_code = ble_advdata_set(&advdata, NULL);
  APP_ERROR_CHECK(err_code);

  // Initialise advertising parameters (used when starting advertising)
  memset(&m_adv_params, 0, sizeof(m_adv_params));

  m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
  m_adv_params.p_peer_addr = NULL;                           // Undirected advertisement
  m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
  m_adv_params.interval    = APP_ADV_INTERVAL;
  m_adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;
}

/**@brief Initialize services that will be used by the application.
*/
static void services_init(void)
{
  uint32_t       err_code;
  ble_oto_init_t oto_init;
  ble_as_init_t  as_init;

  // Initialize Otolith Service
  memset(&oto_init, 0, sizeof(oto_init));

  // Here the sec level for the Otolith Service can be changed/increased.
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&oto_init.step_count_char_attr_md.cccd_write_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&oto_init.step_count_char_attr_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&oto_init.step_count_char_attr_md.write_perm);

  oto_init.evt_handler          = NULL;
  oto_init.initial_step_count   = 42;

  err_code = ble_oto_init(&m_oto, &oto_init);
  APP_ERROR_CHECK(err_code);

  // Initialize Alarm Service
  memset(&as_init, 0, sizeof(as_init));
  as_init.evt_handler = on_ble_as_update;

  err_code = ble_as_init(&m_as, &as_init);
  APP_ERROR_CHECK(err_code);
}


/**@brief Initialize security parameters.
*/
static void sec_params_init(void)
{
  m_sec_params.timeout      = SEC_PARAM_TIMEOUT;
  m_sec_params.bond         = SEC_PARAM_BOND;
  m_sec_params.mitm         = SEC_PARAM_MITM;
  m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
  m_sec_params.oob          = SEC_PARAM_OOB;  
  m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
  m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}


/**@brief Initialize the Connection Parameters module.
*/
static void conn_params_init(void)
{
  uint32_t               err_code;
  ble_conn_params_init_t cp_init;

  memset(&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params                  = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.disconnect_on_fail             = true;
  cp_init.evt_handler                    = NULL;
  cp_init.error_handler                  = conn_params_error_handler;

  err_code = ble_conn_params_init(&cp_init);
  APP_ERROR_CHECK(err_code);
}


/**@brief Bond Manager initialization.
*/
static void bond_manager_init(void)
{
  uint32_t            err_code;
  ble_bondmngr_init_t bond_init_data;
  bool                bonds_delete;

  // Clear all bonded masters if the Bonds Delete button is pushed
  err_code = app_button_is_pushed(BONDMNGR_DELETE_BUTTON_PIN_NO, &bonds_delete);
  APP_ERROR_CHECK(err_code);

  // Initialize the Bond Manager
  bond_init_data.flash_page_num_bond     = FLASH_PAGE_BOND;
  bond_init_data.flash_page_num_sys_attr = FLASH_PAGE_SYS_ATTR;
  bond_init_data.evt_handler             = NULL;
  bond_init_data.error_handler           = bond_manager_error_handler;
  bond_init_data.bonds_delete            = bonds_delete;

  err_code = ble_bondmngr_init(&bond_init_data);
  APP_ERROR_CHECK(err_code);
}


/**@brief Initialize Radio Notification event handler.
*/
static void radio_notification_init(void)
{
  uint32_t err_code;

  err_code = ble_radio_notification_init(NRF_APP_PRIORITY_HIGH,
      NRF_RADIO_NOTIFICATION_DISTANCE_4560US,
      ble_flash_on_radio_active_evt);
  APP_ERROR_CHECK(err_code);
}


/**@brief BLE stack initialization.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
  BLE_STACK_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM,
      BLE_L2CAP_MTU_DEF,
      ble_evt_dispatch,
      false);
}


/**@brief Initialize GPIOTE handler module.
*/
static void gpiote_init(void)
{
  APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}


/**@brief Initialize button handler module.
*/
static void buttons_init(void)
{
  // Configure EVAL_BOARD_BUTTON_0 and EVAL_BOARD_BUTTON_1 as wake up buttons and also configure
  // for 'pull up' because the eval board does not have external pull up resistors connected to
  // the buttons.
  static app_button_cfg_t buttons[] =
  {
    {EVAL_BOARD_BUTTON_0, false, NRF_GPIO_PIN_PULLUP, button_event_handler},
    {EVAL_BOARD_BUTTON_1, false, NRF_GPIO_PIN_PULLUP, button_event_handler}  // Note: This pin is also BONDMNGR_DELETE_BUTTON_PIN_NO
  };

  APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, false);
}


/*****************************************************************************
 * Static Start Functions
 *****************************************************************************/

/**@brief Start advertising.
*/
static void advertising_start(void)
{
  uint32_t err_code;

  err_code = sd_ble_gap_adv_start(&m_adv_params);
  APP_ERROR_CHECK(err_code);

  led_start();
}


/*****************************************************************************
 * Static Event Handling Functions
 *****************************************************************************/

/**@brief Application's BLE Stack event handler.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
  uint32_t        err_code      = NRF_SUCCESS;
  static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
      led_stop();
      led1_on();
      connected = true;

      m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

      // Start handling button presses
      //err_code = app_button_enable();
      break;

    case BLE_GAP_EVT_DISCONNECTED:
      // Since we are not in a connection and have not started advertising, store bonds
      err_code = ble_bondmngr_bonded_masters_store();
      APP_ERROR_CHECK(err_code);

      led1_off();
      connected = false;

      // Go to system-off mode, should not return from this function, wakeup will trigger
      // a reset.
      //system_off_mode_enter();
      break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
      err_code = sd_ble_gap_sec_params_reply(m_conn_handle, 
          BLE_GAP_SEC_STATUS_SUCCESS, 
          &m_sec_params);
      break;

    case BLE_GAP_EVT_TIMEOUT:
      if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISEMENT)
      {
        led_stop();
        //system_off_mode_enter();
      }
      break;

    default:
      break;
  }

  APP_ERROR_CHECK(err_code);
}


/**@brief Dispatches a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
  ble_bondmngr_on_ble_evt(p_ble_evt);
  ble_oto_on_ble_evt(&m_oto, p_ble_evt);
  ble_as_on_ble_evt(&m_as, p_ble_evt);
  ble_conn_params_on_ble_evt(p_ble_evt);
  on_ble_evt(p_ble_evt);
}

/*****************************************************************************
 * Main Function
 *****************************************************************************/

/**@brief Application main function.
*/
int main(void)
{
  uint32_t err_code;
  connected = false;
  mlog_init();
  mlog_str("Started Main\r\n");
  volatile double* myDouble = malloc(sizeof(double)* 8); 
  mlog_num((int) myDouble);
  timers_init();
  gpiote_init();
  buttons_init();
  motor_init();
  led1_init();
  adc_config();
  dac_init();
  //pulse_init();
  mlog_str("Finished Config...\r\n");

  bond_manager_init();
  ble_stack_init();
  radio_notification_init();

  // Initialize Bluetooth Stack parameters
  gap_params_init();
  advertising_init();
  services_init();
  conn_params_init();
  sec_params_init();

  // Actually start advertising
  mlog_str("Finished Init...\r\n");
  app_button_enable();

  // initialize after bluetooth is enabled
  step_counter_init(&m_oto);
  // Enter main loop
  for (;;)
  {
    // Switch to a low power state until an event is available for the application
    err_code = sd_app_event_wait();
    APP_ERROR_CHECK(err_code);
  }
}

/**
 * @}
 */
