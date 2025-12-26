/*
 * cgms_timer.c
 *
 *  Created on: 2024年5月11日
 *      Author: SZZ0030
 */

#include "sl_bt_api.h"
#include "sl_sleeptimer.h"
#include "app_log.h"
#include "cgms_timer.h"
#include "app_log.h"
#include "em_gpio.h"
#include "simplegluco.h"
#include "cur_filter.h"
#include "cgms_error_fault.h"



static void GPIO_OutPutTest(GPIO_Port_TypeDef port,unsigned int pin)
{
      static int cnt =0;

      cnt++;
      if(cnt%2)
        {
          GPIO_PinOutClear(port, pin);
          app_log_info("LED OFF\r\n");

        }
      else
        {
          GPIO_PinOutSet(port, pin);
          app_log_info("LED On\r\n");
        }
}


static unsigned int GPIO_InputTest(GPIO_Port_TypeDef port,unsigned int pin)
{

    unsigned int input_value=GPIO_PinInGet(port,pin);

    app_log_info("port[%d],pin[%d] ;inputvalue = %d\r\n",port,pin,input_value);

    return input_value;
}




static uint32_t i_cnt =0;

const float sensor_cur[]=
    {
        9.76,
        8.54,
        8.2,
        7.88,
        7.97,
        8.16,
        8.54,
        9.26,
        9.73,
        9.58,
        9.93,
        10.18,
        10.29,
        9.98,
        9.55,
        9.18,
        8.65,
        8.51,
        8.38,
        8.25,
        8.29,
        7.98,
        7.89,
        7.86,
        7.94,
        7.83,
        7.68,
        7.61,
        7.63,
        7.76,
        7.64,
        7.72,
        7.62,
        7.78,
        7.71,
        7.57,
        7.49,
        7.3,
        8.02,
        7.85,
        7.56,
        7.32,
        7.14,
        6.98,
        6.91,
        6.75,
        6.67,
        6.76,
        6.93,
        6.78,
        6.87,
        6.58,
        6.31,
        5.75,
        5.92,
        6.17,
        6.3,
        6.37,
        6.64,
        6.82,
        6.86,
        7.04,
};


static uint32_t times =0,m_current_offset;

 float  gluco_calc_val =0.0f ;

float  *p_current = sensor_cur;

uint8_t sensor_status;

//static uint32_t b_sample_flag=false;

static void my_timer_callback(sl_sleeptimer_timer_handle_t *handle __attribute__((unused)),
                              void *data __attribute__((unused)))
{
  //Code executed when the timer expire.

//  (void *)data;
//  (void *)handle;

  GPIO_OutPutTest(BSP_GPIO_LED1_PORT,BSP_GPIO_LED1_PIN);

  GPIO_InputTest(BSP_GPIO_KEY1_PORT,BSP_GPIO_KEY1_PIN);

  sl_bt_external_signal(SLEEP_TIMER_WAKEUP_EVT);
  times ++;
  app_log_info("timer out callback times:%lu\r\n", times);

  app_log_info("cur_error_min_value =%d\r\n",(uint32_t)(cur_error_min_value *100));
  app_log_info("cur_error_max_value =%d\r\n",(uint32_t)(cur_error_max_value *100));
//

  float glucose;

  if(i_cnt < ( sizeof(sensor_cur)/sizeof(float) -1))

    {

     // float avg=  cur_filter(p_current,m_current_offset);  //计算电流平均值,20条， 每个3秒


      sfCurrI0=sensor_cur[i_cnt];
     ///
    //  sfCurrI0 = avg ;
     // simpleGlucoCalc(&glucose,i_cnt++);

      simpleGlucoCalc(&gluco_calc_val,m_current_offset);//calc the concentration here

      glucose =gluco_calc_val;

      /*******异常和趋势计算*********************/
     float i_cv = cgms_i_cv(sfCurrI0,m_current_offset);  //计算cv

     cgms_error_fault_cal(m_current_offset,gluco_calc_val,sfCurrI0,&sensor_status,i_cv); // 计算异常逻辑



     uint8_t trend = cgms_cal_trend(gluco_calc_val,m_current_offset);// 计算趋势

     m_current_offset++;
     i_cnt++;


    }

   app_log_info("i_cnt = %u, sfCurrI0 = %u, glucose=%u\r\n",i_cnt,(unsigned int )(sfCurrI0*100),(unsigned int)(glucose * 100));

}

static sl_sleeptimer_timer_handle_t my_timer;


int cgms_timer_start(void)
{
  sl_status_t status;

  uint32_t timer_timeout = SENSOR_SAMPLE_INTERVAL * 1 ;


  GPIO_PinModeSet(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN, gpioModePushPull, 1);

  GPIO_PinModeSet(BSP_GPIO_KEY1_PORT, BSP_GPIO_KEY1_PIN, gpioModeInputPull, 1);


  GPIO_PinModeSet(BSP_GPIO_HALL_LOCK_PORT,BSP_GPIO_HALL_LOCK_PIN,gpioModePushPull,0);

  GPIO_PinOutSet(BSP_GPIO_HALL_LOCK_PORT,BSP_GPIO_HALL_LOCK_PIN);
//  GPIO_PinOutClear(BSP_GPIO_HALL_LOCK_PORT,BSP_GPIO_HALL_LOCK_PIN);


  // We assume the sleeptimer is initialized properly

  status = sl_sleeptimer_start_periodic_timer(&my_timer,
                                     timer_timeout,
                                     my_timer_callback,
                                     (void *)NULL,
                                     0,
                                     0);
  app_log_info("start_timer_errocode =%ld\r\n",status);

  if(status != SL_STATUS_OK)
  {
    app_log_info("start timer fail\n");
    return -1;
  }
  return 1;
}
