//#include <app.h>
#include<stdint.h>
#include<stdbool.h>
#include<stdio.h>

//#include<typeinfo.h>
#include"cur_filter.h"
#include <math.h>
#include "simplegluco.h"
#define  S    			    1.0f

#if defined(DX_I_SAMPLE_NUM_18)
  #define  ARRAY_MAX     	18
#else
  #define ARRAY_MAX  20
#endif


#define I_ERROR_MIN cur_error_min_value
#define I_ERROR_MAX cur_error_max_value

// #define I_ERROR_MIN 1.1f
// #define I_ERROR_MAX 30.0f



const t_Sensor_k_I_Data Sensor_K_I_Rang_Data[]=
{
 ///0.3 ----0.4 sensork
{	.Sensor_K_Low_Lmt=0.30f,
	.Sensor_K_Hi_Lmt=0.40f,
	.I_Error_Min_Value=0.10f,
	.I_Error_Max_Value=26.00f	},
	 
{
.Sensor_K_Low_Lmt=0.40f,
	.Sensor_K_Hi_Lmt=0.50f,
	.I_Error_Min_Value=0.18f,
	.I_Error_Max_Value=30.00f
				},
{
.Sensor_K_Low_Lmt=0.50f,
	.Sensor_K_Hi_Lmt=0.60f,
	.I_Error_Min_Value=0.40f,
	.I_Error_Max_Value=34.10f
				},
{
.Sensor_K_Low_Lmt=0.60f,
	.Sensor_K_Hi_Lmt=0.70f,
	.I_Error_Min_Value=0.6f,
	.I_Error_Max_Value=38.00f
				},
{
.Sensor_K_Low_Lmt=0.70f,
	.Sensor_K_Hi_Lmt=0.80f,
	.I_Error_Min_Value=0.80f,
	//.I_Error_Min_Value=1.1f,
	//.I_Error_Max_Value=30.00f
	.I_Error_Max_Value=43.00f
				},
{.Sensor_K_Low_Lmt=0.80f,
	.Sensor_K_Hi_Lmt=0.90f,
	.I_Error_Min_Value=1.0f,
	.I_Error_Max_Value=47.00f

				},
{
.Sensor_K_Low_Lmt=0.90f,
	.Sensor_K_Hi_Lmt=1.0f,
	.I_Error_Min_Value=1.20f,
	.I_Error_Max_Value=50.10f
				},
{
.Sensor_K_Low_Lmt=1.0f,
	.Sensor_K_Hi_Lmt=1.1f,
	.I_Error_Min_Value=1.5f,
	.I_Error_Max_Value=55.00f
				},
{
	.Sensor_K_Low_Lmt= 1.1f,
	.Sensor_K_Hi_Lmt=  1.2f,
	.I_Error_Min_Value=1.7f,
	.I_Error_Max_Value=60.00f
	},

	{
	.Sensor_K_Low_Lmt= 1.2f,
	.Sensor_K_Hi_Lmt=  1.3f,
	.I_Error_Min_Value=1.9f,
	.I_Error_Max_Value=64.00f
	},
	{
	.Sensor_K_Low_Lmt= 1.3f,
	.Sensor_K_Hi_Lmt=  1.4f,
	.I_Error_Min_Value=2.1f,
	.I_Error_Max_Value=68.0f
	},
	{
	.Sensor_K_Low_Lmt= 1.4f,
	.Sensor_K_Hi_Lmt=  1.5f,
	.I_Error_Min_Value=2.3f,
	.I_Error_Max_Value=72.0f
	},
	{
	.Sensor_K_Low_Lmt= 1.5f,
	.Sensor_K_Hi_Lmt=  1.6f,
	.I_Error_Min_Value=2.5f,
	.I_Error_Max_Value=76.0f
	},
	{
	.Sensor_K_Low_Lmt= 1.6f,
	.Sensor_K_Hi_Lmt=  1.7f,
	.I_Error_Min_Value=2.7f,
	.I_Error_Max_Value=81.0f
	},
	{
	.Sensor_K_Low_Lmt= 1.7f,
	.Sensor_K_Hi_Lmt=  1.8f,
	.I_Error_Min_Value=2.9f,
	.I_Error_Max_Value=85.0f
	},
};

static bool b_cur_filter=false;
float avgxt_1=0;
//float avgxt =0;
float cur_mean =0;
float xt_diff[ARRAY_MAX];

float cur_error_min_value=0.1f;
float cur_error_max_value=50.0f;

//local functions
static float Filtering_4(float *it_1, float *v, uint32_t size_n);

/*******************/

void cur_get_cur_error_value(float sensor_K)
{
	uint8_t i=0;

	for(i=0;i<sizeof(Sensor_K_I_Rang_Data)/sizeof(t_Sensor_k_I_Data);i++)
	{
		if((sensor_K >= Sensor_K_I_Rang_Data[i].Sensor_K_Low_Lmt)&&
			(sensor_K <= Sensor_K_I_Rang_Data[i].Sensor_K_Hi_Lmt))
		{
			cur_error_max_value=Sensor_K_I_Rang_Data[i].I_Error_Max_Value;
			cur_error_min_value=Sensor_K_I_Rang_Data[i].I_Error_Min_Value;
			//printf("cur_limt index=%d\r\n",i);
			return;
		}

	}

	cur_error_min_value=0.10f;
	cur_error_max_value=85.00f;
}

/*******************************************/
float average_f(float *v, uint32_t size_n)
{
	float sum=0.00f;

	for(uint32_t i=0; i<size_n;i++)
	{
		sum+=v[i];
	}
	return (sum/size_n);
}

/*********************remove max, min , the average***********************/
float average_minmax(float *v, uint32_t size_n)
{
	float cur_max=0,cur_min=0,sum=0;

	cur_max=v[0];
	cur_min=v[0];
	sum=v[0];

	for(uint8_t i=1;i<size_n;i++)
	{
		if(cur_max<v[i])cur_max=v[i];
		if(cur_min>v[i])cur_min=v[i];

		sum+=v[i];
	}

			//remove max and min
	return ((sum-cur_max-cur_min)/(size_n-2));

}

/**************************************************/


#define CV_I_MAX_NUM   4

static float i_buff_cv[CV_I_MAX_NUM];

uint8_t cgms_i_cv(float i, uint32_t offset)
{
	uint8_t x=0;

	float cv_v=0;


	for( x=0;x<(CV_I_MAX_NUM-1);x++)
	{
		i_buff_cv[x]=i_buff_cv[x+1];
	}

	i_buff_cv[x]=i;


	if(offset>=CV_I_MAX_NUM)
	{
		cv_v=Cal_CV(i_buff_cv,CV_I_MAX_NUM);
	}

	if(cv_v>2)cv_v=2;

	return (uint8_t)(cv_v*100);

}


/****************************** V01*/

#if (FILTER_VER ==FILTER_VER01)

float cur_filter(float *cur_buff, uint32_t count_offset)
{
	uint8_t err_cnt=0,ok_cnt=0;
	float cur_ok_sum=0,cur_err_sum=0;
	float cur_max=0,cur_min=0;

	if(b_cur_filter==false)
	{
		if(count_offset>=9)
		{
			//printf(" b_cur_filter=false; cnt=%d\r\n",count_offset);
			for(int loop=0;loop<18;loop++)
			{
				if((cur_buff[loop]<I_ERROR_MAX)&&(cur_buff[loop]>I_ERROR_MIN))
				{
					cur_ok_sum+=cur_buff[loop];
					ok_cnt++;
				}
				else
				{
					cur_err_sum+=cur_buff[loop];
					err_cnt++;
				}
			}

			if(err_cnt==18)//first cur error
			{
					return (cur_err_sum/err_cnt);
			}
			else
			{
				//printf(" b_cur_filter false to true; cnt=%d\r\n",count_offset);
				b_cur_filter=true;
				avgxt_1 =(cur_ok_sum/ok_cnt);
				return(avgxt_1);
			}
		}
		else
		{
			return cur_buff[17];
		}

	}
	else//cur filter progress
	{

		//Filtering_4(&avgxt_1,cur_buff,ARRAY_MAX);
		for(uint8_t i=0; i<18; i++)
		{
			xt_diff[i]=avgxt_1-cur_buff[i];

			if((xt_diff[i]<S)&&(xt_diff[i]>(0-S)))
			{
				cur_ok_sum+=cur_buff[i];
				ok_cnt++;
			}
			else
			{
				cur_err_sum+=cur_buff[i];
				err_cnt++;
			}
		}

		if(err_cnt==18)		
		{
			//printf("  flag =true,error cnt=18; cnt=%d\r\n",count_offset);
			//find max and min, average

			cur_mean=average_minmax(cur_buff,ARRAY_MAX);

			if((cur_mean<I_ERROR_MAX)&&(cur_mean>I_ERROR_MIN))
			{
				//printf("  flag =true (cur_mean<50)&&(cur_mean>0.1\r\n");
				if((cur_mean-avgxt_1)>S)
				{
					cur_mean=avgxt_1+S;
				}
				else
					if((cur_mean-avgxt_1)<(0-S))
					{
						cur_mean=avgxt_1-S;
					}				
			}
			else
			{
				//printf(" b_cur_filter true to false; cnt=%d\r\n",count_offset);
				b_cur_filter=false;
			}

			avgxt_1=cur_mean;
			return avgxt_1;
		}
		else
		{
			//printf("  flag =true;offset=%d, error cnt=%d-->",count_offset,err_cnt);
			avgxt_1=(cur_ok_sum/ok_cnt);
			return avgxt_1;
		}

	}

	return avgxt_1;
}



/****************************** V02*/
#elif (FILTER_VER ==FILTER_VER02)

float cur_filter(float *cur_buff, uint32_t count_offset)
{
	uint8_t err_cnt=0,ok_cnt=0;
	float cur_ok_sum=0,cur_err_sum=0;
//	float cur_max=0,cur_min=0;
	float cv=0.00f;

#if FILTER_CUR_DEBUG_LOG
		printf("error_max =%.02f, error_min =%.02f\r\n",cur_error_max_value,cur_error_min_value);
#endif

	cv=Cal_CV(cur_buff,ARRAY_MAX);
	
#if FILTER_CUR_DEBUG_LOG
	printf("cv=%.03f\r\n",cv);
#endif

	if(cv<0.15)
	{
		avgxt_1=average_minmax(cur_buff,ARRAY_MAX);
#if FILTER_CUR_DEBUG_LOG
		printf("agg_minmax =%.04f\r\n",avgxt_1);
#endif
	
	}
	else//cur filter progress
	{
		
		if((avgxt_1<I_ERROR_MAX)&&(avgxt_1>I_ERROR_MIN))
			{
				Filtering_4(&avgxt_1,cur_buff,ARRAY_MAX);

		#if FILTER_CUR_DEBUG_LOG
				printf("Filtering_4 =%.02f\r\n",avgxt_1);
		#endif
			}
		else
		{

			for(int loop=0;loop<ARRAY_MAX;loop++)
			{
				if((cur_buff[loop]<I_ERROR_MAX)&&(cur_buff[loop]>I_ERROR_MIN))
				{
					cur_ok_sum+=cur_buff[loop];
					ok_cnt++;
				}
				else
				{
					cur_err_sum+=cur_buff[loop];
					err_cnt++;
				}
			}

			if(ok_cnt>=1)
				avgxt_1 = cur_ok_sum/ok_cnt;
			else
			{
				avgxt_1 = average_minmax(cur_buff,ARRAY_MAX);
				//avgxt_1=0.01f;

			}

		    #if FILTER_CUR_DEBUG_LOG
				  printf("ok_cnt=%d, agvxt_1=%.02f\r\n",ok_cnt,avgxt_1);
			  #endif

		}
	}

	return avgxt_1;
}

#endif

float Cal_CV(float *v, uint32_t size_n)
{

	float Iavg=0.00f;
	float sum_pow=0.00f;

	Iavg=average_f(v,size_n);

	if(Iavg<0.01)Iavg=0.01;

	for(uint32_t i=0;i<size_n;i++)
		sum_pow+=powf((v[i]-Iavg),2);

	return (sqrtf(sum_pow/size_n)/Iavg);

}




static float Filtering_4(float *it_1, float *v, uint32_t size_n)
{
	float cur_ok_sum=0,cur_err_sum=0;
	uint8_t err_cnt=0,ok_cnt=0;

		for(uint8_t i=0; i<size_n; i++)
		{
			xt_diff[i]= *it_1-v[i];

			if((xt_diff[i]<S)&&(xt_diff[i]>(0-S)))
			{
				cur_ok_sum+=v[i];
				ok_cnt++;
			}
			else
			{
				cur_err_sum+=v[i];
				err_cnt++;
			}
		}

		if(err_cnt==size_n)		
		{
			//printf("  flag =true,error cnt=18; cnt=%d\r\n",count_offset);
			//find max and min, average

			cur_mean=average_minmax(v,ARRAY_MAX);

			if((cur_mean<I_ERROR_MAX)&&(cur_mean>I_ERROR_MIN))
			{
				//printf("  flag =true (cur_mean<50)&&(cur_mean>0.1\r\n");
				if((cur_mean- *it_1)>S)
				{
					cur_mean= *it_1+S;
				}
				else
					if((cur_mean- *it_1)<(0-S))
					{
						cur_mean= *it_1-S;
					}				
			}
			else
			{
				//cur_mean=0.01f;
				//printf(" b_cur_filter true to false; cnt=%d\r\n",count_offset);
				b_cur_filter=false;
			}

			 *it_1=cur_mean;
			return  *it_1;
		}
		else
		{
			//printf("  flag =true;offset=%d, error cnt=%d-->",count_offset,err_cnt);
			 *it_1=(cur_ok_sum/ok_cnt);
			return  *it_1;
		}

}
