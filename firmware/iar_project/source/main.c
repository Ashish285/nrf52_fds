#include "app_fds.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/**************************Private Function Definitions**************************/

/**
* @brief                       Function to call the power management
* @param[in]                   NONE
* @retval                      NONE
*/
void idle_state_handle()
{
  nrf_pwr_mgmt_run();
}

/**
* @brief             Initialise the peripherals
* @param[in]         NONE
* @retval            NONE
*/
void init_peripherals()
{  
  NRF_LOG_INIT(NULL);
  NRF_LOG_DEFAULT_BACKENDS_INIT();                                                      //Use RTT as default backend
  NRF_LOG_INFO("Start\r\n");
  
  init_fds();
}

/**
* @brief             Main Function
* @param[in]         NONE
* @retval            NONE
*/
int main()
{
  init_peripherals();
  fds_write();
  fds_read();
  
  while(1)
  {
    NRF_LOG_PROCESS();
    idle_state_handle();
  }
}

