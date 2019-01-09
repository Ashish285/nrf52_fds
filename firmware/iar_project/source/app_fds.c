#include "fds.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/**************************Private Function Definitions**************************/

uint32_t file_id = 0x01000;
uint32_t rec_id = 0x1111;

/* Data to write in flash. */
struct 
{
  char string[100];
}fds_data;

/* A record containing dummy configuration data. */
fds_record_t record =
{
  .file_id           = 0x00,
  .key               = 0x00,
  .data.p_data       = &fds_data,
  /* The length of a record is always expressed in 4-byte units (words). */
  .data.length_words = 25,
};

/* Flag to check fds initialization. */
static bool volatile m_fds_initialized = false;;
static bool volatile m_fds_write = false;
static bool volatile m_gc_complete = false;


/**
* @brief             Event handler for FDS events
* @param[in]         p_evt              Structure with FDS data
* @retval            NONE
*/
static void fds_evt_handler(fds_evt_t const * p_evt)
{
  /*NRF_LOG_INFO("Event: %s received (%s)",
               fds_evt_str[p_evt->id],
               fds_err_str[p_evt->result]);
  */
  switch (p_evt->id)
  {
  case FDS_EVT_INIT:
    if (p_evt->result == FDS_SUCCESS)
    {
      m_fds_initialized = true;
    }
    break;
    
  case FDS_EVT_WRITE:
    {
      if (p_evt->result == FDS_SUCCESS)
      {
        m_fds_write = true;
      }
    } break;
    
  case FDS_EVT_DEL_RECORD:
    {
      if (p_evt->result == FDS_SUCCESS)
      {
        NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->del.record_id);
        NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->del.file_id);
        NRF_LOG_INFO("Record key:\t0x%04x", p_evt->del.record_key);
      }
      ///m_delete_all.pending = false;
    } break;
    
  case FDS_EVT_GC:
    {
      NRF_LOG_INFO("GC Done");
      m_gc_complete = true;
    }break;
    
  default:
    break;
  }
}
/**
* @brief             Function to go to low power when waiting for FDS
* @param[in]         NONE
* @retval            NONE
*/
static void power_manage(void)
{
#ifdef SOFTDEVICE_PRESENT
  (void) sd_app_evt_wait();
#else
  __WFE();
#endif
}

/**
* @brief             The FDS takes some time for init. Waiting till it gets initialised properly.
* @param[in]         NONE
* @retval            NONE
*/
static void wait_for_fds_ready(void)
{
  while (!m_fds_initialized)
  {
    power_manage();
  }
}

static void wait_for_write(void)
{
  while (!m_fds_write)
  {
    power_manage();
  }
  m_fds_write = false;
}


void fds_write()
{
  ret_code_t rc;
  fds_record_desc_t desc = {0};
  record.file_id = file_id;

  for(int i=0;i<10;i++)
  {  
    memset(&desc, 0x00, sizeof(fds_record_desc_t)); 
    record.key = rec_id;
    rc = fds_record_write(&desc, &record);
    wait_for_write();
    if(rc == FDS_SUCCESS) {
      NRF_LOG_INFO("Data written with id %d",rec_id);
    }
    else {
      NRF_LOG_INFO("Write Failed, id %d",rc);
    }
    rec_id++;
  }  
  rec_id = 0x1111;
}

void fds_read()
{
  NRF_LOG_INFO("Starting Read");
  ret_code_t rc;
  
  fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};
  
  for(int i=0;i<10;i++)
  {
    memset(&desc, 0x00, sizeof(fds_record_desc_t)); 
    memset(&tok, 0x00, sizeof(fds_find_token_t)); 
    rc = fds_record_find(file_id, rec_id, &desc, &tok);
  
    if (rc == FDS_SUCCESS)
    {
      fds_flash_record_t config = {0};
      
      /* Open the record and read its contents. */
      rc = fds_record_open(&desc, &config);
      APP_ERROR_CHECK(rc);

      NRF_LOG_INFO("File Found with id %d",rec_id);
      
      /* Close the record when done reading. */
      rc = fds_record_close(&desc);
      APP_ERROR_CHECK(rc);
    }
    rec_id++;
  }
}

/**
* @brief             Initialise FDS
* @param[in]         NONE
* @retval            NONE
*/
void init_fds()
{
  ret_code_t rc;
  
  /* Register first to receive an event when initialization is complete. */
  (void) fds_register(fds_evt_handler);
  
  NRF_LOG_INFO("Initializing fds...");
  
  rc = fds_init();
  NRF_LOG_INFO("fds init %d",rc);
  APP_ERROR_CHECK(rc);
  
  /* Wait for fds to initialize. */
  wait_for_fds_ready();
}
