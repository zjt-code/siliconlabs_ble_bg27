/**************************************************************************************************
  Filename:       simpleGluco.c
  Revised:        $Date: 2016-08-01 $
  Revision:       $Revision: 23333  $

  Description:    This file contains the SimpleGluco sample application
                  

  Copyright:	Bioshall. All rights reserved.

  IMPORTANT:    Algo version :GF20220313
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */


#include "simpleGluco.h"
#include "cgms_prm.h"
#include "cur_filter.h"
#include "matrix.h"
#include "memory.h"
#include <malloc.h>
#include <stdio.h>
#include "polyfit.h"

// #include "fault_handler.h"
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
signed char vD_X0;
float vD_X1;
unsigned char vD_X2;
float vD_X3;
float vD_X4;
float vD_X5;
unsigned char vD_X6;
unsigned char vD_X7;
float vD_X8;
float vD_X9;
float vD_XA;
unsigned char vD_XB;
float vD_XC;

float vP500;
float vP501;
float vP502;
float vP503;
float vP504;
float vP505;
float vP506;
float vP507;
float vP508;
float vP509;
float vP510;
float vP511;
float vP512;
float vP513;
float vP514;


#define G1  2.20f

#if (defined(D5_DX_PET) && (D5_ALGO == D5_ALGO_PET))
       #define G2  40.00f
#elif(defined(D5_DX_PET) && (D5_ALGO ==D5_ALGO_HUM))
      #define G2  30.00f
#elif(defined(D3_HUM))
      #define G2  30.00f
#else      
      #error " undefine algo type!!!!!!"
#endif


#define RSLFILTER_LAG_X14 	1.0f
#define POLYFIT_LAG_X14   	0.5f



/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
//==========calcGluco_D03闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氾拷===============
//缂侇垰顭烽弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛敓锟�
static float fD_lagCoeff;
static float fD_lagK[4];
static float fD_transientSmooth;
static float fD_temperCorrect;
static float fD_temperRef;
static float fD_currDeltaLimit;
static float fD_smoothCoeff;
static float fD_currBackground;
static float fD_SKfilt;
static unsigned short uiD_warmupNum;

struct corrK stCorrK;
//闁告垵妫涢柈鎾煥閻斿憡鐏柟椋庡厴閺佹捇寮妶鍡楊伓
float fD_temperReal;
float sfCurrI0Max;	//闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕柡浣哥墦閺佹捇寮妶鍡楊伓闁搞儻鎷�
float sfCurrI0Min;

//闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁瑰嚖鎷�
unsigned short simpleGlucoCalcCnt; //reset whenever sensor started
unsigned short usSampleCnt=0;
unsigned short usBgCnt;
unsigned short usBfFlg;
unsigned short usCurrDbg; // simpleGluco_B.h闂佽法鍠愰弸濠氬箯閿燂拷 extern闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氳绋夐敓浠嬫煥閻斿憡鐏柟鍑ゆ嫹

//float gluco_calc_val;
       float sfCurrI0;
static float sfCurrI1;
static float sfCurrI2;	//闂佽法鍠愰弸濠氬箯瀹勬澘顤呴梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氬綊宕愭總鍛婃櫢闁哄倶鍊栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲矗椤愩們锟藉骞忛悜鑺ユ櫢闁哄倶鍊栫�氾拷
       float sfCurrI3;
static float sfCurrI4;
       float sfCurrI5;

static float sfLastI2;	//闁告挸绉崇粩鎾煥閻斿憡鐏柟椋庡厴閺佹捇寮妶鍡楊伓闁稿﹤銈搁弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁告瑯鍋嗛妴瀣箯閻戣姤鏅搁柡鍌樺�栫�氾拷
static float sfLastI3; 
static float sfLastI0;

  
       float sfCurrSum;	//闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛敓锟�
       float sfCurrK = 0.0f;	//閻炴冻鎷烽梺璺ㄥ枑閺嬪骞忛悿顖涚ギ闂佽法鍠愰弸濠氬箯妞嬪酣鍏囬梺璺ㄥ枑閺嬪骞忛敓锟�(閻庡湱鍋炲鍌炴煥閻斿憡鐏柟椋庡厴閺佹捇寮妶鍡楊伓闂佽法鍠愰弸濠氬箯閿燂拷)
       float sfCurrBg;	//闂佽法鍠曟繛鍥箟鐎ｎ偄顏堕悶娑虫嫹闂佽法鍠愰弸濠氬箯閿燂拷
       float sensorK = 0.0f;  //CODE闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏�(0闁哄啫鐖奸弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氬綊鏌ㄩ悢鍛婄伄闁瑰嚖鎷�)
       
static float fMovFilterG[4];
static float fGluco,fGlucoOld;
static unsigned char ucMovFilterIni;
static unsigned char ucBgTransientCnt;

//static uint8_t I0_Error_Cnt=0;

static float cv_i0_buff[CV_IO_BUFF_MAX];


static float  gvp_g[GVP_SAMPLE_MAX+1];
static int    gvp_g_index=0;
static float  gvp=0;


static float gvp_cal(float g);
static void RLSFilter(float *data, float * filter_data,uint16_t data_len, uint16_t order);
static float Lag(float * G, float g1,float g2, float x14, float * delay_data, uint16_t len);

//#if( defined(BOARD) && (BOARD == BRD_N52832_SDK1530) )
//unsigned char CodeRecvFlag = 0;
//#endif
#if LOG_DEBUG
  static float fD_glucoNoDelay;
#endif
/*********************************************************************
 * LOCAL FUNCTIONS

 */

/*********************************************************************
 * 闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氳绋夌弧寮唘co_D03闂佽法鍠庨～鏇炩枖閿燂拷
*********************************************************************/
void simpleGlucoCoefRead(void)
{//闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鎭嶆帡鏌堥妸鈺傛櫢闁哄倶鍊栫�氬湱鎷犻姀銈嗘櫢闁哄倶鍊栫�氬綊宕ｉ弽顓熸櫢闁哄倶鍊栫�氬綊鏌ㄩ悢濂夋綗婵炲娲滈柈鎾煥閻斿憡鐏柟鍑ゆ嫹
#if( defined(BOARD) && (BOARD == BRD_N52832_SDK1530) )
    #if( defined(SENSOR_TYPE) && (SENSOR_TYPE == SENSOR_GN1) )
    vD_X0=(signed char)parm.group3.DX0;
    vD_X1=(float)parm.group3.DX1/100;
    vD_X2=parm.group3.DX2;
    vD_X3=(float)parm.group3.DX3/10;
    vD_X4=(float)parm.group3.DX4/10;
    vD_X5=(float)parm.group3.DX5/10;
  //vD_X6=parm.group3.DX6;            //Not use
    vD_X7=parm.group3.DX7;
  //vD_X8=(float)parm.group3.DX8/1000;//Not use
    vD_X9=(float)parm.group3.DX9/10;
  //vD_XA=(float)parm.group3.DXA/10;  //Not use
  //vD_XB=parm.group3.DXB;            //Not use
  //vD_XC=(float)parm.group3.DXC/1000;//Not use

    vP500 = 0.1; //闂佽法鍠栭崣娲偖鐎涙ê顏舵俊顖ょ節閺佹捇寮妶鍡楊伓缂侇垰顭烽弫鎾诲棘閵堝棗顏�
    vP501 = 0.2;
    vP502 = 0.325;
    vP503 = 0.325;
    vP504 = 3.5;
    vP505 = 0.5;

    vP506 = 0.3; //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忔搴ㄥ厙闂佽法鍠愰弸濠氬箯閿燂拷1~5
    vP507 = 0.3;
    vP508 = 1.2;
    vP509 = 40.0;
    vP510 = 1.0;
  #endif
#else
  #if( defined(SENSOR_TYPE) && (SENSOR_TYPE == SENSOR_P3) )
  //vD_X0=(signed char)m_prm_db.P3.prmDX0;//Not use
    vD_X1=(float)D_X1; //--tempCorrect
    vD_X2=D_X2;            //--temperRef
    vD_X3=(float)D_X3;  //--currdata limit
    vD_X4=(float)(D_X4);  //--smooth coef
    vD_X5=(float)(D_X5);  //--curr background
  //vD_X6=m_prm_db.P3.prmDX6;            //Not use 
   // vD_X7=m_prm_db.P3.prmDX7;            //--warmup number
    vD_X7=D_X7;
  //vD_X8=(float)m_prm_db.P3.prmDX8/1000;//Not use
    vD_X9=(float)D_X9;  //--SKfilt
  //vD_XA=(float)m_prm_db.P3.prmDXA/10;  //Not use
  //vD_XB=m_prm_db.P3.prmDXB;            //Not use
  //vD_XC=(float)m_prm_db.P3.prmDXC/1000;//Not use

    vP500 = 0.1; //闂佽法鍠栭崣娲偖鐎涙ê顏舵俊顖ょ節閺佹捇寮妶鍡楊伓缂侇垰顭烽弫鎾诲棘閵堝棗顏�
    vP501 = 0.2;
    vP502 = 0.325;
    vP503 = 0.325;
    /******101t2e_01********
    vP504 = 3.5
    		*****************/
    //******101t2e_02,101t2e_04
    vP504 = 2.0;

    //******101t2e_03
   /// vP504 = 3.5;
    vP505 = 0.5;
    vP506 = 0.4; //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忔搴ㄥ厙闂佽法鍠愰弸濠氬箯閿燂拷1~5
    vP507 = 0.3;
    vP508 = 1.0;
    // vP509 = 35.0
//    vP509 = 20.0;//P3_REV2   // v25E + D3_V23

#ifdef  D5_DX_PET
//    vP509 = 25.0;// 3 electo ,2025.01.02
      vP509 = 70.0f; // 2025.05.06
#else
       vP509 = 20.0;// 2 pol ,2024.10.22
#endif

    vP510 = 1.0;
  #elif( defined(SENSOR_TYPE) && (SENSOR_TYPE == SENSOR_GN1) )
    vD_X0=(signed char)m_prm_db.P3.prmDX0;
    vD_X1=(float)m_prm_db.P3.prmDX1/100;
    vD_X2=m_prm_db.P3.prmDX2;
    vD_X3=(float)m_prm_db.P3.prmDX3/10;
    vD_X4=(float)m_prm_db.P3.prmDX4/10;
    vD_X5=(float)m_prm_db.P3.prmDX5/10;
  //vD_X6=m_prm_db.P3.prmDX6;            //Not use
    vD_X7=m_prm_db.P3.prmDX7;
  //vD_X8=(float)m_prm_db.P3.prmDX8/1000;//Not use
    vD_X9=(float)m_prm_db.P3.prmDX9/10;
  //vD_XA=(float)m_prm_db.P3.prmDXA/10;  //Not use
  //vD_XB=m_prm_db.P3.prmDXB;            //Not use
  //vD_XC=(float)m_prm_db.P3.prmDXC/1000;//Not use

    vP500 = 0.1; //闂佽法鍠栭崣娲偖鐎涙ê顏舵俊顖ょ節閺佹捇寮妶鍡楊伓缂侇垰顭烽弫鎾诲棘閵堝棗顏�
    vP501 = 0.2;
    vP502 = 0.325;
    vP503 = 0.325;
    vP504 = 3.5;
    vP505 = 0.5;

    vP506 = 0.3; //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忔搴ㄥ厙闂佽法鍠愰弸濠氬箯閿燂拷1~5
    vP507 = 0.3;
    vP508 = 1.2;
    vP509 = 40.0;
    vP510 = 1.0;
#elif( defined(SENSOR_TYPE) && (SENSOR_TYPE == SENSOR_GN2) )
  //vD_X0=(signed char)m_prm_db.P3.prmDX0;//Not use
    vD_X1=(float)m_prm_db.P3.prmDX1/100;
    vD_X2=m_prm_db.P3.prmDX2;
    vD_X3=(float)m_prm_db.P3.prmDX3/10;
    vD_X4=(float)m_prm_db.P3.prmDX4/10;
    vD_X5=(float)m_prm_db.P3.prmDX5/10;
  //vD_X6=m_prm_db.P3.prmDX6;            //Not use
    vD_X7=m_prm_db.P3.prmDX7;
  //vD_X8=(float)m_prm_db.P3.prmDX8/1000;//Not use
    vD_X9=(float)m_prm_db.P3.prmDX9/10;
  //vD_XA=(float)m_prm_db.P3.prmDXA/10;  //Not use
  //vD_XB=m_prm_db.P3.prmDXB;            //Not use
  //vD_XC=(float)m_prm_db.P3.prmDXC/1000;//Not use

    vP500 = 0.1; //闂佽法鍠栭崣娲偖鐎涙ê顏舵俊顖ょ節閺佹捇寮妶鍡楊伓缂侇垰顭烽弫鎾诲棘閵堝棗顏�
    vP501 = 0.2;
    vP502 = 0.325;
    vP503 = 0.325;
    vP504 = 0.0;//vP504 = 3.5;
    vP505 = 0.5;

    vP506 = 0.25; //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忔搴ㄥ厙闂佽法鍠愰弸濠氬箯閿燂拷1~5
    vP507 = 0.3;
    vP508 = 1.0;
    vP509 = 30.0;
    vP510 = 1.0;
  #endif
  
#endif

}
void simpleGlucoInit(void)
{
	fD_temperReal = 25;//闂佽法鍠栭幗顏嗘媼鐟欏嫬顏�
	sfCurrI0Min=0.1; //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柟鍦焿閹活亪骞忛悜鑺ユ櫢闁哄倶鍊栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氬綊鏌ㄩ悢鍏兼噳閻犱浇顫夌�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁告瑯鍊ゅ鍡楊伓闂佽法鍠曟繛鍥晸閿燂拷
	sfCurrI0Max=800.0;
  
	calcGluco_StateInit();
	simpleGlucoCoefRead();
	calcGlucoInit_D03();
#if LOG_DEBUG
  LOGI("simple gluco init");
#endif
  return;
}


static float origal_data[ORIGNAL_DATA_LEN]={11,12,13,14,15,1,2,3,4,5,6,7,8,9,10};
static float rls_filter_data[RLS_FILTER_DATA_LEN]={0};
static float rsl_lag_data[RLS_FILTER_DATA_LEN]={0};
static float poly_fit_x[ORIGNAL_DATA_LEN]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};
static float poly_fit_value[ORIGNAL_DATA_LEN]={0};
static float poly_fit_coff[DEGREE+1]={0};
static float poly_lag_data[ORIGNAL_DATA_LEN]={0};
static bool  check_elements(float *elements, int len);


void simpleGlucoCalc(float* pResult,unsigned short usSampleCnt)
{
  unsigned short tCount;

  tCount = usSampleCnt + 1;//usSampleCnt++;//闂佽法鍠愰弸濠氬箯閻戣姤鏅搁悷娆欑秮閸庢挳鏌ㄩ悢鍛婄伄闁圭兘鏀卞鍌炴煥閻斿憡鐏柟椋庡厴閺佹捇寮妶鍡楊伓闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁活収鍋撶紞鍥箯閿燂拷

  {
    *pResult = 0.0f;
    
    calcGluco_D03(pResult, tCount);

#if (defined(D5_DX_PET)&& defined(D5_ALGO)&&(D5_ALGO==D5_ALGO_PET))

   if(tCount >=120)
   if((*pResult > 3.0f))
     *pResult = *pResult*1.1f-0.8f;
 #elif(defined(D3_HUM))
     if((*pResult > 3.0f)&&( *pResult <25.0f))
     *pResult = *pResult*1.1f-0.8f;
 #elif((D5_ALGO==D5_ALGO_HUM))
     if(tCount >=120)
     if((*pResult > 3.0f)&&( *pResult <25.0f)){
       *pResult = *pResult*1.1f-0.8f;
     }
 #endif



    for(uint8_t x=1;x<ORIGNAL_DATA_LEN;x++)
    origal_data[x-1]=origal_data[x];
    origal_data[ORIGNAL_DATA_LEN-1]= *pResult;   
    //  memset((uint8_t *)rls_filter_data,0,RLS_FILTER_DATA_LEN*sizeof(float));
    //  for(uint8_t i=0;i<RLS_FILTER_DATA_LEN;i++)
    //   printf("%8.4f",rls_filter_data[i]);
    //   printf("\r\n");
        
    if(tCount>=(uiD_warmupNum+ORIGNAL_DATA_LEN))
    {
     // printf("start rsl_filter\r\n");

			 if(check_elements(origal_data,ORIGNAL_DATA_LEN))
			 {
			  RLSFilter(origal_data+RSL_ORDER-1,rls_filter_data,RLS_FILTER_DATA_LEN,RSL_ORDER);
			  //Lag algorithm
			  Lag(rls_filter_data,G1,G2,RSLFILTER_LAG_X14,rsl_lag_data,RLS_FILTER_DATA_LEN);
			  //ployfit algorithm
			  polyfit(poly_fit_x,origal_data,ORIGNAL_DATA_LEN,DEGREE,poly_fit_coff,poly_fit_value);
			  Lag(poly_fit_value,G1,G2,POLYFIT_LAG_X14,poly_lag_data,ORIGNAL_DATA_LEN);
			  //float ave_g=(*pResult+rsl_lag_data[RLS_FILTER_DATA_LEN-1]+poly_lag_data[ORIGNAL_DATA_LEN-1])/3;
			  *pResult =(*pResult+rsl_lag_data[RLS_FILTER_DATA_LEN-1]+poly_lag_data[ORIGNAL_DATA_LEN-1])/3;
      }
    }
    

     if ((tCount <= uiD_warmupNum) || (sfCurrK == 0)) //婵ɑ鎸抽弫鎾诲棘閵堝棗顏堕梺璺ㄥ枎瑜般劍瀵煎▎鎰伓闁哄牜浜弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛摎鍌氭倧闂佽法鍣﹂幏锟�
     {
        *pResult = 0.0f;
     }
     else
     {
       if(*pResult > G2) *pResult = G2;
       if(*pResult < 0.1f)  *pResult = 0.1f;

       gvp = gvp_cal(*pResult);
     }
  }
  //calcGluco_StateBackup();

  //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氬綊鏌ㄩ悢鑽ょ獩闁跨噦鎷�
  #if LOG_DEBUG
  if(usSampleCnt & 0x1)
  {
    LOGI("Cnt = %d, I0 = %f, I1 = %f,I2 = %f,I2x = %f, I3 = %f, I3x = %f,I5 = %f, K = %f, G = %f, Gd = %f \r\n ",\
        usSampleCnt,sfCurrI0,sfCurrI1,sfCurrI2,sfLastI2,sfCurrI3,sfLastI3,sfCurrI5,sfCurrK,fD_glucoNoDelay,*pResult);
  }
  else
  {
    LOGW("Cnt = %d, I0 = %f, I1 = %f,I2 = %f,I2x = %f, I3 = %f, I3x = %f,I5 = %f, K = %f, G = %f, Gd = %f \r\n ",\
        usSampleCnt,sfCurrI0,sfCurrI1,sfCurrI2,sfLastI2,sfCurrI3,sfLastI3,sfCurrI5,sfCurrK,fD_glucoNoDelay,*pResult);
  }
  #endif
}

static bool  check_elements(float *elements, int len)
{
  for(int x=0;x<len;x++)
  {
      if( elements[x] <=G1 || elements[x] >= G2)
       return false;
  }
      return true;
}


void calcGlucoInit_D03(void)

{
  uint16_t fccode;
	//闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢璇参濋悹鍥囧嫬顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氳鎷呴崸妤佹櫢闁活収鍋嗛。顕�骞忛悜鑺ユ櫢濠殿喗鍔栫涵鍓佸寲婵犳碍鏅搁柡鍌樺�栫�氾拷
  fD_temperCorrect = vD_X1;
  fD_temperRef = vD_X2;
  fD_currDeltaLimit = vD_X3;
  fD_currBackground = vD_X5;
  
  uiD_warmupNum = vD_X7;
  fD_SKfilt = vD_X9;
//fD_glucoGain = sfCurrK;

  fD_lagK[0] = vP500;
  fD_lagK[1] = vP501;
  fD_lagK[2] = vP502;
  fD_lagK[3] = vP503;
  fD_lagCoeff = vP504;
  fD_transientSmooth = vP505; // 闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氳瀵奸鍡楀弮闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬湱鍖栨繝姘櫢闁哄倶鍊栫�氾拷



	stCorrK.corr1 = vP506;
	stCorrK.corr2 = vP507;
	stCorrK.corr3 = vP508;
	stCorrK.corr4 = vP509;
	stCorrK.corr5 = vP510;
	fD_smoothCoeff = vD_X4;



	// fccode = uint16_decode(g_factory_config.fc_buffer);

	// sensorK=(float)fccode/1000;

	// cur_get_cur_error_value(sensorK);

  return;
}

static float Abnormal_Fuc(float cv, float diff,float i0)
{
    //shift value

    float abs = (diff > 0) ? diff: -diff;

    if((cv > CV_THRESOLD && abs >DIFF_THRESOLD) ||((i0 <= cur_error_min_value)||(i0 >= cur_error_max_value)))
    {
      return 0.01;
    }
    else
      return i0;


}

static float g_i0=0;


void calcGluco_D03(float* pGluco, unsigned short tCnt)
{
  float fExpValue;
  float fDataTmp = 0.0f;

  //Step_0:  code闂佽法鍠曢鐘碉拷瑙勫哺閺佹捇寮妶鍡楊伓闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氬綊鏌ㄩ悢鍛婄伄闁瑰嚖鎷�
#ifndef D5_DX_PET //两电极算法,或者三电极转两电极
    fExpValue = stCorrK.corr1 * (1.0f-EXPF(-stCorrK.corr2*tCnt/480));
    fExpValue += stCorrK.corr3 * (1.0f+EXPF(-stCorrK.corr4*tCnt/480));
#else  //三电极算法
    // fExpValue = stCorrK.corr1 * (1.0f-EXPF(-stCorrK.corr2*tCnt/480));
    // fExpValue += stCorrK.corr3 * (1.0f-EXPF(-stCorrK.corr4*tCnt/480));
     // 2025.05.06
     if (tCnt<40){
      fExpValue = stCorrK.corr1 * (1.0f - 1.1f * expf(-stCorrK.corr2 * tCnt / 480 + 0.2f)) + stCorrK.corr3 * (1.0f - 1.3f * expf(-stCorrK.corr4 * tCnt / 480 + 0.2f)) +  0.1 * (log(3.5 * tCnt / 480));
      // sfCurrK = fExpValue * x12 * sensorK;
     }

     else{
      fExpValue = stCorrK.corr1 * (1.0f - 0.9 * expf(-stCorrK.corr2 * tCnt / 480 + 0.2));
      fExpValue += stCorrK.corr3 * (1.0f - 1.35 * expf(-25.0f * tCnt / 480 + 0.2));
      // sfCurrK = fExpValue * x12 * sensorK;
      }

 #endif


  /// Abnormal_Fuc**********************************/

  g_i0 = sfCurrI0;

  for(uint8_t x=0; x<(CV_IO_BUFF_MAX-1); x++)
  {
    cv_i0_buff[x] = cv_i0_buff[x+1];
  }
  cv_i0_buff[CV_IO_BUFF_MAX-1] = sfCurrI0;

  if(tCnt > uiD_warmupNum)
  {
      float cv = Cal_CV(cv_i0_buff,CV_IO_BUFF_MAX);
      g_i0 = Abnormal_Fuc(cv,sfCurrI0-sfLastI0,sfCurrI0);
  }

  /**************************************************/
  
  //Step_1:闂佽法鍠栭幗顏嗘媼鐟欏嫬顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氾拷
  sfCurrI1 = g_i0 * (1.0f + fD_temperCorrect * (fD_temperReal - fD_temperRef));
  if (sfCurrI1 < 0.01f)
  {
    sfCurrI1 = 0.01f;
  }

  if ((tCnt <= uiD_warmupNum )) //婵ɑ鎸抽弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛敓锟�...
  {
    sfLastI2 = sfCurrI1;
    sfLastI3 = sfCurrI1;
    sfLastI0 = sfCurrI0;
    *pGluco = 0.0f;
  }
  else//婵ɑ鎸抽弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氾拷
  {
    if((sfLastI2>cur_error_min_value)
    		  &&(sfLastI2<cur_error_max_value)
    		  &&(sfCurrI1<cur_error_max_value)
    		  &&(sfCurrI1>cur_error_min_value))
     {
        if((sfCurrI1-sfLastI2)>fD_currDeltaLimit)
        {

              sfCurrI2=sfLastI2+fD_currDeltaLimit;
        }
        else if((sfLastI2-sfCurrI1)>fD_currDeltaLimit)
              
              sfCurrI2=sfLastI2-fD_currDeltaLimit;
        else
              sfCurrI2= sfCurrI1;
     }
     else
     {
       sfCurrI2= sfCurrI1;
     }


     if((sfCurrI2>cur_error_min_value)&&(sfCurrI2<cur_error_max_value)&&
        (sfLastI3>cur_error_min_value)&&(sfLastI3<cur_error_max_value))
      {
        sfCurrI3 = fD_smoothCoeff * sfCurrI2 + (1.0f - fD_smoothCoeff) * sfLastI3;
      }
      else{sfCurrI3=sfCurrI2;}   
      
   // }
    //Step_3:妤犵偞濞婇弫鎾诲棘閵堝棗顏堕梺璺ㄥ枎婢规瑧鎷犺鐎氾拷
  
    
    if (sfCurrI3 < 0.01f)
    {
      sfCurrI3 = 0.01f;
    }

    //Step_4:闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏�

   // if(usSampleCnt==(uiD_warmupNum+1))
     sfCurrI4 = sfCurrI3 - fD_currBackground;
    // else
    // sfCurrI4 =sfCurrI1;

    if (sfCurrI4 < 0.01f)
    {
      sfCurrI4 = 0.01f;
    }
    //Step_5:闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏�
    sfCurrI5 = sfCurrI4; //闁活亙绀侀獮鎾绘煥閻斿憡鐏柟椋庡厴閺佹捇寮妶鍡楊伓闂佽法鍠愰弸濠氬箯閻戣姤鏅哥紒娑橆儑楠炲棝鏌ㄩ悢鍛婄伄闁瑰嚖鎷�

   // Sys_Watchdog_Refresh();


    if (sensorK != 0)//(app_env_isfs == 1)// whether the sensor code is entered or calibrated
    {                                     // calcGlucoGain_D03(); //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氱畟fCurrK;
        sfCurrK = fExpValue * stCorrK.corr5 * sensorK;
    }


    //闁圭娲╅、鍛存煥閻斿憡鐏柟椋庡厴閺佹捇寮妶鍡楊伓闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氾拷
    if (usBfFlg == 1) //闂佽法鍠曟繛鍥箟鐎ｎ偄顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氬綊鏌ㄩ悢鍏兼噳缁绢參鏀辩�氾拷
    {                 //闂佽法鍠曟繛鍥箟鐎ｎ偄顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氬綊鏌ㄩ悢鍏兼噳缁绢參鏀辩�氬綊鏌ㄩ悢璇插缁炬澘顦扮�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枙琚欓梺顔哄姂閺佹捇寮妶鍡楊伓闂佽法鍠愰弸濠氬箯閿燂拷
    	usBgCnt++;

      if (sensorK == 0)
      {
        sfCurrK = sfCurrI5 / sfCurrBg;
        sensorK = sfCurrK/(stCorrK.corr5 * fExpValue);   //闂侇偅宀搁弫鎾诲棘閵堝棗顏堕柟绋挎穿椤㈠懘鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闂傚啯鍎奸幓顏堝箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏�
        ucMovFilterIni = MOVE_FIL_WIN;       //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氳瀵奸鍡楀弮闁捐鎷嬮崕鐐烘煥閻旇鈻忓ù鍏肩懄鐎氬湱绮旈崘顔芥櫢濞撴哎鍎荤紞鍥箯閻戣姤鏅搁柨鐕傛嫹
      }
      else
      {
        sfCurrK = fD_SKfilt * sfCurrI5 / sfCurrBg + (1.0f - fD_SKfilt) * sfCurrK;
        ucBgTransientCnt = 2;  //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾剁玻閺嶃劍鐏柟椋庡厴閺佹捇寮妶鍡楊伓闂佽法鍠庢慨顓㈠矗閸屾稓妲烽柣姘摠鐎氬綊鏌ㄩ悢鍛婄伄闁圭兘鏀卞濂告煥閻斿憡鐏柟椋庢焿缁狅綁鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕柛灞筋煼閺佹捇寮妶鍡楊伓闂佽法鍣﹂幏锟�2闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕柣婵嗭躬閺佹挻娼径鍝ヤ覆闁圭兘顥撻～鍫ユ煥閻旀儼瀵ч柨婵嗙箲鐎氬綊鏌ㄩ悤鍌涘
        stCorrK.corr5 = sfCurrK / (fExpValue * sensorK); //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢缂佽尙灏睠orrK.corr5
      }
      usBfFlg = 0;
    }

    //閻炴冻鎷烽梺璺ㄥ枑閺嬪骞忓畡甯嫹婵傚憡鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁瑰嚖鎷�
    if (sfCurrK != 0)
    { //闂佽法鍠撻悰銈夊箻閸涱喖顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氬綊鏌ㄩ悢鍛婄伄闁圭兘鏀遍弲銉╂煥閻斿憡鐏柟椋庡厴閺佹捇寮妶鍡楊伓闂佽法鍠愰弸濠氬箯閿燂拷

        {
            fGluco = sfCurrI5/ sfCurrK;

        }

      // printf("fGluco-g0=%f",fGluco);
#if LOG_DEBUG
      fD_glucoNoDelay = fGluco; //闂佽法鍠愰弸濠氬箯閻戣姤鏅搁柡鍌樺�栫�氬綊鏌ㄩ悢鍛婄伄闁瑰嚖鎷�
#endif
    if(ucBgTransientCnt > 0)
    {//闂佽法鍠曟繛鍥掗弮鍌氱殤闁归鍏橀弫鎾诲棘閵堝棗顏堕柡鍛浮閺佹捇寮妶鍡楊伓闂佽法鍣﹂幏锟�
      fGluco = fGlucoOld + fD_transientSmooth*(fGluco-fGlucoOld);
      *pGluco = fGluco;

      ucBgTransientCnt--;
      if(ucBgTransientCnt == 0) ucMovFilterIni = MOVE_FIL_WIN; //闂佽法鍠曟繛鍥掗弮鍌涱仧闁归鍏橀弫鎾剁磽鐎靛憡顏熼柟椋庡厴閺佹捇寮▎鎰伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闁哄倶鍊栫�氳鎷呭澶嬫櫢闂佹壆鏁哥亸銊╁箯闁垮瑔渚�鏌ㄩ悢鍛婄伄闁瑰嚖鎷�
    }
    else
    {
      //閻炴冻鎷烽梺璺ㄥ枑閺嬪骞忛悜鑺ユ櫢闂佹壆鏁哥亸銊╁箯闁垮瑔渚�鏌ㄩ悢鍛婄伄闁归鍏橀弫鎾诲棘閵堝棗顏堕梺璺ㄥ枑瀹撳孩娼忛悙顒�顏堕梺璺ㄥ枑閺嬪骞忛敓锟�
      {
        fMovFilterG[0] = fMovFilterG[1];
        fMovFilterG[1] = fMovFilterG[2];
        fMovFilterG[2] = fMovFilterG[3];
        fMovFilterG[3] = fGlucoOld;
      }

      //闂佽法鍠栭崣娲偖鐎涙ê顏舵俊顖ょ節閺佹捇寮妶鍡楊伓
      fDataTmp = fD_lagK[0] * (fMovFilterG[1] - fMovFilterG[0]);
      fDataTmp += fD_lagK[1] * (fMovFilterG[2] - fMovFilterG[1]);
      fDataTmp += fD_lagK[2] * (fMovFilterG[3] - fMovFilterG[2]);
      fDataTmp += fD_lagK[3] * (fGluco - fMovFilterG[3]);

      if(ucMovFilterIni > 0)
      {
        ucMovFilterIni--;
        *pGluco = fGluco;
      }
      else
      {
        if( (fMovFilterG[0]<=G1)||(fMovFilterG[0]>=G2)||
            (fMovFilterG[1]<=G1)||(fMovFilterG[1]>=G2)||
            (fMovFilterG[2]<=G1)||(fMovFilterG[2]>=G2)||
            (fMovFilterG[3]<=G1)||(fMovFilterG[3]>=G2)||
            (fGluco<=G1)||fGluco>=G2)
          {
            *pGluco = fGluco;
             //printf("error---------.... I5=%03f\r\n",sfCurrI5);
          } 
        else
          *pGluco = fGluco + fD_lagCoeff * fDataTmp;
      }

     //  printf("*pGluco-G_x14_2=%f\r\n",*pGluco);

    }
        //fGlucoOld=*pGluco;
       fGlucoOld = fGluco ;
    }
    else  //sfcurrk=0
    {
       fGluco=0.1;
      *pGluco=0.1;

    }
    
   // usCurrDbg = (unsigned short)(sfCurrK*1000);//for debug

    sfLastI2 = sfCurrI2;
    sfLastI3 = sfCurrI3;
    sfLastI0 = sfCurrI0;

  } 
  }

/*********************************************************************
*********************************************************************/
void calcGluco_StateBackup(void)
{
#if( defined(BOARD) && (BOARD == BRD_N52832_SDK1702) )
	g_softreset_globle_varible.simpleGlucoCalcCnt =simpleGlucoCalcCnt;
	g_softreset_globle_varible.usSampleCnt = usSampleCnt;
	g_softreset_globle_varible.usBgCnt = usBgCnt;
	g_softreset_globle_varible.usBfFlg = usBfFlg;
	g_softreset_globle_varible.usCurrDbg = usCurrDbg;
	
	g_softreset_globle_varible.sfCurrI0 =sfCurrI0;
	g_softreset_globle_varible.sfCurrI1 =sfCurrI1;
	g_softreset_globle_varible.sfCurrI2 =sfCurrI2;
	g_softreset_globle_varible.sfCurrI3 =sfCurrI3;
	g_softreset_globle_varible.sfCurrI4 =sfCurrI4;
	g_softreset_globle_varible.sfCurrI5 =sfCurrI5;
	g_softreset_globle_varible.sfLastI2 =sfLastI2;
	g_softreset_globle_varible.sfLastI3 =sfLastI3;

	g_softreset_globle_varible.sfCurrBg = sfCurrBg;
	g_softreset_globle_varible.sfCurrSum =sfCurrSum;
	g_softreset_globle_varible.sfCurrK = sfCurrK;
  g_softreset_globle_varible.sensorK = sensorK;//
  
  g_softreset_globle_varible.fMovFilterG[0] = fMovFilterG[0];
	g_softreset_globle_varible.fMovFilterG[1] = fMovFilterG[1];
	g_softreset_globle_varible.fMovFilterG[2] = fMovFilterG[2];
	g_softreset_globle_varible.fMovFilterG[3] = fMovFilterG[3];
	g_softreset_globle_varible.fGluco = fGluco;
	g_softreset_globle_varible.fGlucoOld = fGlucoOld;
	g_softreset_globle_varible.ucMovFilterIni = ucMovFilterIni;
	g_softreset_globle_varible.ucBgTransientCnt = ucBgTransientCnt;
	g_softreset_globle_varible.CodeRecvFlag = app_env_isfs;
#elif( defined(BOARD) && (BOARD == BRD_RSL10_SDK34154) )
//	m_global_variable_backup->simpleGlucoCalcCnt =simpleGlucoCalcCnt;
//	m_global_variable_backup->usSampleCnt = usSampleCnt;
//	m_global_variable_backup->usBgCnt = usBgCnt;
//	m_global_variable_backup->usBfFlg = usBfFlg;
//	m_global_variable_backup->usCurrDbg = usCurrDbg;
//
//	m_global_variable_backup->sfCurrI0 =sfCurrI0;
//	m_global_variable_backup->sfCurrI1 =sfCurrI1;
//	m_global_variable_backup->sfCurrI2 =sfCurrI2;
//	m_global_variable_backup->sfCurrI3 =sfCurrI3;
//	m_global_variable_backup->sfCurrI4 =sfCurrI4;
//	m_global_variable_backup->sfCurrI5 =sfCurrI5;
//	m_global_variable_backup->sfLastI2 =sfLastI2;
//	m_global_variable_backup->sfLastI3 =sfLastI3;
//
//	m_global_variable_backup->sfCurrBg = sfCurrBg;
//	m_global_variable_backup->sfCurrSum =sfCurrSum;
//	m_global_variable_backup->sfCurrK = sfCurrK;
//	m_global_variable_backup->sensorK = sensorK;//
//	m_global_variable_backup->stCorrKcorr5 = stCorrK.corr5;
//
//	m_global_variable_backup->fMovFilterG[0] = fMovFilterG[0];
//	m_global_variable_backup->fMovFilterG[1] = fMovFilterG[1];
//	m_global_variable_backup->fMovFilterG[2] = fMovFilterG[2];
//	m_global_variable_backup->fMovFilterG[3] = fMovFilterG[3];
//	m_global_variable_backup->fGluco = fGluco;
//	m_global_variable_backup->fGlucoOld = fGlucoOld;
//	m_global_variable_backup->ucMovFilterIni = ucMovFilterIni;
//	m_global_variable_backup->ucBgTransientCnt = ucBgTransientCnt;
//	m_global_variable_backup->CodeRecvFlag = app_env_isfs;
#endif
}

void calcGluco_StateRecovery(void)
{
//    simpleGlucoCalcCnt = m_global_variable_backup->simpleGlucoCalcCnt;
//    usSampleCnt = m_global_variable_backup->usSampleCnt;
//    usBgCnt = m_global_variable_backup->usBgCnt;
//    usBfFlg = m_global_variable_backup->usBfFlg;
//    usCurrDbg = m_global_variable_backup->usCurrDbg;
//
//    sfCurrI0 = m_global_variable_backup->sfCurrI0;
//    sfCurrI1 = m_global_variable_backup->sfCurrI1;
//    sfCurrI2 = m_global_variable_backup->sfCurrI2;
//    sfCurrI3 = m_global_variable_backup->sfCurrI3;
//    sfCurrI4 = m_global_variable_backup->sfCurrI4;
//    sfCurrI5 = m_global_variable_backup->sfCurrI5;
//    sfLastI2 = m_global_variable_backup->sfLastI2;
//    sfLastI3 = m_global_variable_backup->sfLastI3;
//
//    sfCurrBg = m_global_variable_backup->sfCurrBg;
//    sfCurrSum = m_global_variable_backup->sfCurrSum;
//    sfCurrK = m_global_variable_backup->sfCurrK;
//    sensorK = m_global_variable_backup->sensorK;
//    stCorrK.corr5 = m_global_variable_backup->stCorrKcorr5;
//
//    fMovFilterG[0] = m_global_variable_backup->fMovFilterG[0];
//    fMovFilterG[1] = m_global_variable_backup->fMovFilterG[1];
//    fMovFilterG[2] = m_global_variable_backup->fMovFilterG[2];
//    fMovFilterG[3] = m_global_variable_backup->fMovFilterG[3];
//    fGluco = m_global_variable_backup->fGluco;
//    fGlucoOld = m_global_variable_backup->fGlucoOld;
//    ucMovFilterIni = m_global_variable_backup->ucMovFilterIni;
//    ucBgTransientCnt = m_global_variable_backup->ucBgTransientCnt;
//    app_env_isfs = m_global_variable_backup->CodeRecvFlag;
}

void calcGluco_StateInit(void)
{
#if( defined(BOARD) && (BOARD == BRD_N52832_SDK1702) )
  if(!softreset_get_flag())
#endif
  {
    simpleGlucoCalcCnt = 0;
    usSampleCnt=0;
    usBgCnt=0;
    usBfFlg = 0;
    usCurrDbg = 0;
    
    sfCurrI0=0;
    sfCurrI1=0;
    sfCurrI2=0;
    sfCurrI3=0;
    sfCurrI4=0;
    sfCurrI5=0;
    sfLastI2=0;
    sfLastI3=0;
    
    sfCurrSum=0;
    sfCurrBg=0;
    sfCurrK=0;
    sensorK=0;

    fMovFilterG[0] = 0;
    fMovFilterG[1] = 0;
    fMovFilterG[2] = 0;
    fMovFilterG[3] = 0;
    fGluco = 0;
    fGlucoOld = 0;
    ucMovFilterIni = MOVE_FIL_WIN;
    ucBgTransientCnt = 0;
    //app_env_isfs = 0;
  }
#if( defined(BOARD) && (BOARD == BRD_N52832_SDK1702) )
  else
  {
    simpleGlucoCalcCnt = g_softreset_globle_varible.simpleGlucoCalcCnt;
    usSampleCnt = g_softreset_globle_varible.usSampleCnt;
    usBgCnt = g_softreset_globle_varible.usBgCnt;
    usBfFlg = g_softreset_globle_varible.usBfFlg;
    usCurrDbg = g_softreset_globle_varible.usCurrDbg;
    
    sfCurrI0 = g_softreset_globle_varible.sfCurrI0;
    sfCurrI1 = g_softreset_globle_varible.sfCurrI1;
    sfCurrI2 = g_softreset_globle_varible.sfCurrI2;
    sfCurrI3 = g_softreset_globle_varible.sfCurrI3;
    sfCurrI4 = g_softreset_globle_varible.sfCurrI4;
    sfCurrI5 = g_softreset_globle_varible.sfCurrI5;
    sfLastI2 = g_softreset_globle_varible.sfLastI2;
    sfLastI3 = g_softreset_globle_varible.sfLastI3;

    sfCurrBg = g_softreset_globle_varible.sfCurrBg;
    sfCurrSum = g_softreset_globle_varible.sfCurrSum;
    sfCurrK = g_softreset_globle_varible.sfCurrK;
    sensorK = g_softreset_globle_varible.sensorK;
    
    fMovFilterG[0] = g_softreset_globle_varible.fMovFilterG[0];
    fMovFilterG[1] = g_softreset_globle_varible.fMovFilterG[1];
    fMovFilterG[2] = g_softreset_globle_varible.fMovFilterG[2];
    fMovFilterG[3] = g_softreset_globle_varible.fMovFilterG[3];
    fGluco = g_softreset_globle_varible.fGluco;
    fGlucoOld = g_softreset_globle_varible.fGlucoOld;
    ucMovFilterIni = g_softreset_globle_varible.ucMovFilterIni;
    ucBgTransientCnt = g_softreset_globle_varible.ucBgTransientCnt;
    app_env_isfs = g_softreset_globle_varible.CodeRecvFlag;
  }
#endif
}


float Glucose_Data[GLUCOSE_DATA_LEN]={1,2,3,4,5,6,7,8,9,10};
/*RLS Filter variable 

*/
static float Temp_parray[RSL_ORDER*RSL_ORDER]={0};
static float Temp2_parray[RSL_ORDER*RSL_ORDER]={0};

t_matrix Temp_Matrix={.x=RSL_ORDER,.y=RSL_ORDER,.parray=Temp_parray};
t_matrix Temp2_Matrix={.x=RSL_ORDER,.y=RSL_ORDER,.parray=Temp2_parray};

static float lam=0.99;


 static void RLSFilter(float *data, float * filter_data,uint16_t data_len, uint16_t order)
{
    /*Initial local Matrix varible ,malloc memory*/
    t_matrix W ={.x=RSL_ORDER,.y = 1,.parray=(float *)malloc(RSL_ORDER*sizeof(float))};//28bytes
    t_matrix WT={.x=1,.y = RSL_ORDER,.parray=(float *)malloc(RSL_ORDER*sizeof(float))}; //28bytes
    t_matrix P ={.x=RSL_ORDER,.y=RSL_ORDER,.parray=(float*)malloc(RSL_ORDER*RSL_ORDER*sizeof(float))};//128byts 
    t_matrix X ={.x=RSL_ORDER,.y=1,.parray=(float*)malloc(1*RSL_ORDER*sizeof(float))};//28bytes
    t_matrix XT={.x=1,.y=RSL_ORDER,.parray=(float*)malloc(1*RSL_ORDER*sizeof(float))};//28bytes
    t_matrix PX ={.x=RSL_ORDER,.y=1,.parray=(float*)malloc(1*RSL_ORDER*sizeof(float))};//28bytes   
   //clear memory
    memset((uint8_t *)W.parray,0,RSL_ORDER*sizeof(float));
    memset((uint8_t *)WT.parray,0,RSL_ORDER*sizeof(float));
    memset((uint8_t *)P.parray,0,RSL_ORDER*RSL_ORDER*sizeof(float));
    memset((uint8_t *)X.parray,0,RSL_ORDER*sizeof(float));
    memset((uint8_t *)XT.parray,0,RSL_ORDER*sizeof(float));
    memset((uint8_t *)PX.parray,0,RSL_ORDER*sizeof(float));

    Convert2_UnitMatix(&P);
       //Matix_Print(&P);
    MulConst_Matrix(&P,lms);
     //Matix_Print(&P);
    
    for(uint16_t i=order;i<data_len;i++)
    {
        memcpy((uint8_t*)X.parray,(uint8_t *)(data+(i-order)),order*sizeof(float));
        //printf("X->CNT:%d\r\n",i);

       // Matix_Print(&X);
        Transpose_Matrix(&W,&WT);
        MulMatrix_Matrix(&WT,&X,&Temp_Matrix); //forcast valued is stored in Temp_Matrix
        //debug
        // printf("y=%8.4f\r\n",Temp_Matrix.parray[0]);
        // printf("data[i]=%8.4f\r\n",data[i]);  

        //
        float e = data[i]-Temp_Matrix.parray[0];
        //printf("e--> cnt=%d: %8.4f\r\n",i,e);
        // cal px,xt
        MulMatrix_Matrix(&P,&X,&PX);
        Transpose_Matrix(&X,&XT);

        MulMatrix_Matrix(&XT,&PX,&Temp_Matrix);

        float denominator =Temp_Matrix.parray[0];

        //cal K=PX
        MulConst_Matrix(&PX,1/(denominator+lam));
       // printf("K=CNT=%d\r\n",i);
        //Matix_Print(&PX);

        //P=P-K@X.T@P
        MulMatrix_Matrix(&PX,&XT,&Temp_Matrix); //Temp_Maxtirx=K@XT
        MulMatrix_Matrix(&Temp_Matrix,&P,&Temp2_Matrix); 
        Matrix_Minus_Matrix(&P,&Temp2_Matrix);
        MulConst_Matrix(&P,1/lam);

       // printf("P---CNT:%d\r\n",i);
       // Matix_Print(&P);

        //W=W+K*e
        MulConst_Matrix(&PX,e);
        Matrix_Add_Matrix(&W,&PX);
       // printf("W---CNT:%d\r\n",i);
       // Matix_Print(&W);
    }

    memcpy((uint8_t *)filter_data,(uint8_t *)data,data_len*4);
    Transpose_Matrix(&W,&WT);

    for(uint16_t i=order;i<data_len;i++)
    {
        memcpy((uint8_t*)X.parray,(uint8_t *)(data+(i-order)),order*sizeof(float));
        MulMatrix_Matrix(&WT,&X,&Temp_Matrix);
        filter_data[i]=Temp_Matrix.parray[0];
    }

    /*free memory*/
    free(W.parray);
    free(WT.parray);
    free(P.parray);
    free(X.parray);
    free(XT.parray);
    free(PX.parray);
}

static float Lag(float * G, float g1,float g2, float x14, float * delay_data, uint16_t len)
{
  delay_data[0]=G[0];
  delay_data[1]=G[1];
  delay_data[2]=G[2];
  delay_data[3]=G[3];
  uint8_t x=0;
  for( x=4;x<len;x++)
    {
            if((G[x]>g1)&&(G[x]<g2)&&
                (G[x-1]>g1)&&(G[x-1]<g2)&&
                (G[x-2]>g1)&&(G[x-2]<g2)&&
                (G[x-3]>g1)&&(G[x-3]<g2)&&
                (G[x-4]>g1)&&(G[x-4]<g2))
            delay_data[x]= G[x] + x14*( 0.1*(G[x-3]-G[x-4])+0.2*(G[x-2]-G[x-3])+
                            0.325*(G[x-1]-G[x-2])+0.325*(G[x]-G[x-1]));
            else
            delay_data[x]=G[x];

    }

    return delay_data[x-1];
}


static float gvp_cal(float g)
{

  //printf("g = %04f\r\n",g);

  float L=0,L0=GVP_SAMPLE_MAX * GVP_SAMPLE_TIME_INTEVAL;

  if(gvp_g_index < GVP_SAMPLE_MAX + 1)
  {
    gvp_g[gvp_g_index++] = g;
    return  GVP_ERROR_MAX +1;
  }
  else
  {
    int x;
    for( x =1; x < GVP_SAMPLE_MAX +1; x++)
    {gvp_g[x-1] =gvp_g[x];}
    gvp_g[x-1] = g;

    // Cal L value
    for(int x= 1; x < GVP_SAMPLE_MAX+1; x++)
    {
       // L += sqrtf(powf(GVP_SAMPLE_TIME_INTEVAL,2) + powf(gvp_g[x]-gvp_g[x-1],2));
       L +=  sqrtf(powf(GVP_SAMPLE_TIME_INTEVAL,2) + powf((gvp_g[x]-gvp_g[x-1])*18.0f,2));
      //  printf("gvg_: L =%04f\r\n",L);
    }


   // printf("GVP = %04f\r\n",L/L0-1);

    //gvp value
    return (L/L0 -1);
  }
}



bool gluco_check_bg( float calbration)
{



	float ratio = ((sfCurrI0>0.01 ? sfCurrI0 : 0.01)/calbration)/sfCurrK;

 // printf("ratio =%04f\r\n",ratio);

//	return  (ratio < SENSOR_RATIO_MAX && ratio > SENSOR_RATIO_MIN ) ? true :false;  //enbale calbration

	return  (ratio <= SENSOR_RATIO_MIN ) ? true :false;  //enbale calbration

}



bool gvg_get_result(void)
{
  return  (gvp < GVP_ERROR_MAX) ? true:false;
}


