/*
 * cgms_error_fault.c
 *
 *  Created on: 2023��4��17��
 *  Author: SZZ0030
 */
#include <app.h>
#include "cgms_error_fault.h"
//#include "ble_cgms.h"
#include "simplegluco.h"
#include "cur_filter.h"
//#include "elog.h"

//static float cgms_cur_buff[CUR_BUFF_MAX]={0.00f};
static float cgms_glu_buff[GLU_BUFF_MAX]={0.00f};
//static uint16_t cur_low_count=0,fault_recovey_cnt=0;
static uint16_t warm_cur_error_cnt_down=0;
static uint16_t cur_abnormal_cnt=0;

uint8_t user_cal_cnt_down_trend=0;


static uint16_t warm_cur_error_cnt=0;

CGMS_TREND_STRUCT_LMT cgms_trend_lmt_t[4]=
{

	{CGMS_TREND_STABLE_LOW_LMT,CMGS_TREND_STABLE_HI_LMT,
			CGMS_TREND_STABLE,
			CGMS_TREND_STABLE},
	{CGMS_TREND_SLOW_LOW_LMT,CMGS_TREND_SLOW_HI_LMT,
			CGMS_TREND_SLOW_DOWN,
			CGMS_TREND_SLOW_UP},
	{CGMS_TREND_DOWW_UU_LOW_LMT,CGMS_TREND_DOWW_UU_HI_LMT,
			CGMS_TREND_DOWN_DOWN,
			CGMS_TREND_UP_UP},
	{CGMS_TREND_FAST_LOW_LMT,CGMS_TREND_FAST_HI_LMT,
			CGMS_TREND_FAST_DOWN,
			CGMS_TREND_FAST_UP},

};

static uint8_t status_backup=0;

#if 0
uint8_t cgms_error_fault_cal(uint16_t cgms_offset,float glucose,float cur ,uint8_t *status)
{
	uint8_t cgms_trend=0;
	float glucos_diff;

	cgms_glu_buff[0]=cgms_glu_buff[1];
	cgms_glu_buff[1]= glucose;
   // Check sensor implanted
	if(cgms_offset < CUR_LOW_COUT_MAX)
	{
		if(cur< CUR_NORMAL_LOW_LMT)
		{
			cur_low_count ++;
		}
		else
		{
			cur_low_count=0;
		}

		if(cur_low_count>=CUR_LOW_COUT_LMT)
		{
			status_backup=CGMS_STATUS_ABNORMAL_INVALID_IMPLANTED;
			//cur_abnormal_cnt=0;
		}

	}
	else
	{
		//after check implanted,check low current



				//	// hi current
//				if(cur>CUR_NORMAL_HI_LMT)
//				{
//					cur_hi_count++;
//					if(cur_hi_count==CUR_HI_COUT_LMT)
//					{
//						p_cgms->sensor_status.status.warning=CGMS_STATUS_ABNORMAL_SENSOR_FAUL;
//						fault_recovey_cnt=0;
//					}
//
//				}
//				else
//				{
//
//					cur_hi_count=0;
//				}

				//******recovery step***********************/
				if((status_backup==CGMS_STATUS_ABNORMAL_SENSOR_ABNOMAL)||
						(status_backup==CGMS_STATUS_ABNORMAL_SENSOR_FAUL)||
						(status_backup==CGMS_STATUS_ABNORMAL_INVALID_IMPLANTED))
				{
					if((cur<CUR_NORMAL_HI_LMT)&&(cur>CUR_NORMAL_LOW_LMT))
					{
						fault_recovey_cnt++;
						if(fault_recovey_cnt>=CGMS_ABNORMAL_RECOVERY_COUNT_MAX)
						{

							cur_hi_count=0;
							cur_low_count=0;
							fault_recovey_cnt=0;
							cur_abnormal_cnt=0;
							status_backup= CGMS_STATUS_ABNORMAL_NONE;
						}
					}
					else
					{
						cur_abnormal_cnt++;
						fault_recovey_cnt=0;
						if(cur_abnormal_cnt>60)
						{

							status_backup= CGMS_STATUS_ABNORMAL_SENSOR_FAUL;
						}

					}
				}
				else
				{

					if((cur< CUR_NORMAL_LOW_LMT)||(cur>CUR_NORMAL_HI_LMT))
					{
						cur_low_count ++;
					}
					else
					{
						cur_low_count=0;
					}

					if(cur_low_count>=CUR_LOW_COUT_LMT)
					{
						status_backup=CGMS_STATUS_ABNORMAL_SENSOR_ABNOMAL;
						fault_recovey_cnt=0;
						cur_abnormal_cnt=0;
					}

				}

	}


	if((cgms_offset<D_X7)&&
	 ((status_backup==CGMS_STATUS_ABNORMAL_NONE)||(status_backup == CGMS_STATUS_ABNORMAL_SENSOR_ABNOMAL)))
	{

	 *status=CGMS_STATUS_ABNOARNAL_WARMUP;

	}
	else
	{
		*status =status_backup;
	}

	if(user_cal_cnt_down_trend==0)
	{
		if(cgms_offset>D_X7)  // warm up 120 minutues, offset=41 is second glucose
		{
			//if((cgms_glu_buff[0])&&(cgms_glu_buff[1]<=GLUCOSE_MIN_LMT))
			{

				if(cgms_glu_buff[0]<cgms_glu_buff[1]) //up
				{
					glucos_diff=(cgms_glu_buff[1]-cgms_glu_buff[0])/3;

					for(uint8_t loop=0;loop<4;loop++)
					{
						if((glucos_diff>cgms_trend_lmt_t[loop].low_lmt)&&
								(glucos_diff<=cgms_trend_lmt_t[loop].hi_lmt))
								cgms_trend=cgms_trend_lmt_t[loop].up_trend_val;

					}

				}
				else
					if(cgms_glu_buff[0]>cgms_glu_buff[1])//down
					{

						glucos_diff=(cgms_glu_buff[0]-cgms_glu_buff[1])/3;
						for(uint8_t loop=0;loop<4;loop++)
						{
							if((glucos_diff>cgms_trend_lmt_t[loop].low_lmt)&&
									(glucos_diff<=cgms_trend_lmt_t[loop].hi_lmt))
								cgms_trend=cgms_trend_lmt_t[loop].down_trend_val;

						}
					}
					else //stable
					{
						cgms_trend=CGMS_TREND_STABLE;

					}
			}
		}
		else //offset<dx
			{cgms_trend=CGMS_TREND_INVILD;}
	}
	else  //trend  !=0
	{
		user_cal_cnt_down_trend--;
		cgms_trend=CGMS_TREND_INVILD;
	}
	return cgms_trend;
}
#endif




#if 0
void cgms_error_fault_cal(uint16_t cgms_offset,float glucose,float cur ,uint8_t *status,uint8_t cv)
{

	// Sensor Planted check
	if(cgms_offset < SENSOR_PLANTED_CHECK_CNT)
	{
		if(cur <= CUR_NORMAL_LOW_LMT){
			cur_low_count ++;
		}
		else
		{
			cur_low_count=0;
		}

		if(cur_low_count>=CUR_LOW_COUT_LMT)
		{
			status_backup=CGMS_STATUS_ABNORMAL_INVALID_IMPLANTED;
			//cur_abnormal_cnt=0;
		}
		else
		{
			status_backup = CGMS_STATUS_ABNOARNAL_WARMUP;
		}

		warm_cur_error_cnt =0;
	}
	else
	{
		//senor fault and unplanted, error can't turn to back normal
      if(( status_backup != CGMS_STATUS_ABNORMAL_INVALID_IMPLANTED)&&(status_backup != CGMS_STATUS_ABNORMAL_SENSOR_FAUL))
		{
		// Warm up check Current
			if(cgms_offset < D_X7 )
			{
				if((cur<CUR_NORMAL_HI_LMT)&&(cur>CUR_NORMAL_LOW_LMT))
				{
					status_backup = CGMS_STATUS_ABNOARNAL_WARMUP;
					warm_cur_error_cnt=0;
				}
				else
				{
					warm_cur_error_cnt++;
					if(warm_cur_error_cnt>=WARM_UP_CUR_ERROR_MAX)
					{
						status_backup= CGMS_STATUS_FAIL_WARMUP;
					}
				}
				cur_abnormal_cnt=0;
			}  //cgms_offset < D_X7,warmup
			else
			{
				 // after warm up Check sensor current
				if((( cv > 20 )||(cur <= cur_error_min_value)||(cur >= cur_error_max_value))&&((glucose <3.9f)||(glucose>10.0f)))
				{
					cur_abnormal_cnt++;
					status_backup= CGMS_STATUS_ABNORMAL_SENSOR_ABNOMAL;

					if(cur_abnormal_cnt >=60)status_backup = CGMS_STATUS_ABNORMAL_SENSOR_FAUL;
				}
				else
				{
					cur_abnormal_cnt=0;
					status_backup= CGMS_STATUS_ABNORMAL_NONE;
				}

			}////cgms_offset < D_X7
		}//senor fault and unplanted, error can't turn to back normal

	}//cgms_offset < SENSOR_PLANTED_CHECK_CNT

	*status =status_backup;
}
#endif



#if 0
void cgms_error_fault_cal(uint16_t cgms_offset,float glucose,float cur ,uint8_t *status,uint8_t cv)
{

  // Initialize
  if(cgms_offset == 0){

      status_backup = 0;
      warm_cur_error_cnt=0;
      fault_recovey_cnt=0;
      cur_abnormal_cnt =0;
  }


  // Sensor Planted check
  if(cgms_offset < (D_X7))
  {
      if( ((cur <=cur_error_min_value)||(cur >=cur_error_max_value))?1:0)
        {
              if(cur <=cur_error_min_value)
                {
                  status_backup = CGMS_STATUS_LOW_CUR;
                }
              else
                {
                  status_backup = CGMS_STATUS_HI_CUR;
                }

                fault_recovey_cnt = 0;
                cur_abnormal_cnt++;
        }
      else
        {
          fault_recovey_cnt++;
          cur_abnormal_cnt =0;
          if(status_backup!=CGMS_STATUS_ABNOARNAL_WARMUP)
          if(fault_recovey_cnt > 10)
          {
              status_backup =CGMS_STATUS_ABNOARNAL_WARMUP;
          }
        }

  }
  else
  {
      // after warm up Check sensor current
        if((( cv > 20 )||(cur <= cur_error_min_value)||(cur >= cur_error_max_value))&&((glucose <3.9f)||(glucose>10.0f)))
        {
          cur_abnormal_cnt++;
         // status_backup= CGMS_STATUS_ABNORMAL_SENSOR_ABNOMAL;

          if(cv > 20) status_backup = CGMS_STATUS_CV;
          if(cur <=cur_error_min_value) status_backup = CGMS_STATUS_LOW_CUR;
          if(cur >= cur_error_max_value) status_backup = CGMS_STATUS_HI_CUR;

          if(cur_abnormal_cnt >=60)status_backup = CGMS_STATUS_ABNORMAL_SENSOR_FAUL;
        }
        else
        {
          cur_abnormal_cnt=0;
          status_backup= CGMS_STATUS_ABNORMAL_NONE;
        }

  }//cgms_offset < SENSOR_PLANTED_CHECK_CNT

  *status =status_backup;
}
#endif



void cgms_error_fault_cal(uint16_t cgms_offset,float glucose,float cur ,uint8_t *status,uint8_t cv)
{
//Initialize variable
    if(cgms_offset <=1) {
      status_backup=CGMS_STATUS_ABNOARNAL_WARMUP;
      warm_cur_error_cnt_down=0;
      cur_abnormal_cnt=0;
    }
    else{
#if (D5_CUSTOMIZED_PROTOCOL)
    if(status_backup == CGMS_STATUS_ABNORMAL_SENSOR_FAUL)   ///sensor fault error, can't turn to back normal
#else
   if((status_backup != CGMS_STATUS_FAULT_CV)&&(status_backup != CGMS_STATUS_FAULT_HI)&&(status_backup != CGMS_STATUS_FAULT_LOW))   ///sensor fault error, can't turn to back normal
#endif
    {
    // Warm up check Current
      if(cgms_offset < D_X7 )
      {
        if((cur<cur_error_max_value)&&(cur>cur_error_min_value)){

          if(warm_cur_error_cnt_down) {
            warm_cur_error_cnt_down--;
          }
          else {
            status_backup = CGMS_STATUS_ABNOARNAL_WARMUP;
          }

          cur_abnormal_cnt=0; //clear Total error count;
        }
        else{
#if (D5_CUSTOMIZED_PROTOCOL)
            if((cur <=cur_error_min_value)||(cur >=cur_error_max_value)){
            status_backup=CGMS_STATUS_ABNORMAL_SENSOR_ABNOMAL;
            }       
#else
            if(cur >=cur_error_max_value) {
                status_backup=CGMS_STATUS_HI_CUR;
            }
            if(cur <=cur_error_min_value){
                status_backup=CGMS_STATUS_LOW_CUR;
            }
#endif
            warm_cur_error_cnt_down=WARM_UP_CUR_ERROR_MAX;
            cur_abnormal_cnt++;  //current error increasing
        }
      }  //cgms_offset < D_X7,warmup
      else {
        if((( cv >= 20 )||(cur <= cur_error_min_value)||(cur >= cur_error_max_value))&&
            ((glucose <3.9f)||(glucose>10.0f)))
        {
          cur_abnormal_cnt++;
#if (D5_CUSTOMIZED_PROTOCOL)
         status_backup= CGMS_STATUS_ABNORMAL_SENSOR_ABNOMAL;
#else
          if(cv >= 20){status_backup=CGMS_STATUS_CV;};
          if(cur <= cur_error_min_value){status_backup=CGMS_STATUS_LOW_CUR;};
          if(cur >= cur_error_max_value){status_backup=CGMS_STATUS_HI_CUR;};
#endif
          if(cur_abnormal_cnt >=60){
#if (D5_CUSTOMIZED_PROTOCOL)
              status_backup = CGMS_STATUS_ABNORMAL_SENSOR_FAUL;
#else
              if(status_backup==CGMS_STATUS_CV){status_backup=CGMS_STATUS_FAULT_CV;}
              if(status_backup==CGMS_STATUS_HI_CUR){status_backup=CGMS_STATUS_FAULT_HI;}
              if(status_backup==CGMS_STATUS_LOW_CUR){status_backup=CGMS_STATUS_FAULT_LOW;}
#endif
          }
        }
        else{
          cur_abnormal_cnt=0;
          status_backup= CGMS_STATUS_ABNORMAL_NONE;
        }

      }////cgms_offset < D_X7
    }//senor fault and unplanted, error can't turn to back normal
   }

    if(cgms_offset == 6719 ){
        status_backup=0x08;
    }
    *status =status_backup;
}






uint8_t cgms_cal_trend(float glucose, uint16_t cgms_offset)
{
	uint8_t cgms_trend;
	float glucos_diff=0.00f;

	cgms_glu_buff[0]=	cgms_glu_buff[1];
	cgms_glu_buff[1]=   glucose;


	if(user_cal_cnt_down_trend==0)
	{
			if(cgms_offset>D_X7)  // warm up 120 minutues, offset=41 is second glucose
			{
				//if((cgms_glu_buff[0])&&(cgms_glu_buff[1]<=GLUCOSE_MIN_LMT))
				{

					if(cgms_glu_buff[0]<cgms_glu_buff[1]) //up
					{
						glucos_diff=(cgms_glu_buff[1]-cgms_glu_buff[0])/3;

						for(uint8_t loop=0;loop<4;loop++)
						{
							if((glucos_diff>cgms_trend_lmt_t[loop].low_lmt)&&
									(glucos_diff<=cgms_trend_lmt_t[loop].hi_lmt))
									cgms_trend=cgms_trend_lmt_t[loop].up_trend_val;

						}

					}
					else
						if(cgms_glu_buff[0]>cgms_glu_buff[1])//down
						{

							glucos_diff=(cgms_glu_buff[0]-cgms_glu_buff[1])/3;

							for(uint8_t loop=0;loop<4;loop++)
							{
								if((glucos_diff>cgms_trend_lmt_t[loop].low_lmt)&&
										(glucos_diff<=cgms_trend_lmt_t[loop].hi_lmt))
									cgms_trend=cgms_trend_lmt_t[loop].down_trend_val;

							}
						}
						else //stable
						{
							cgms_trend=CGMS_TREND_STABLE;

						}
				}
			}
			else //offset<dx
			{
				cgms_trend=CGMS_TREND_INVILD;
			}
		}
		else  //trend  !=0
		{
			user_cal_cnt_down_trend--;
			cgms_trend=CGMS_TREND_INVILD;
		}

		return cgms_trend;

}





