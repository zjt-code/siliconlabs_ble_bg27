#include"polyfit.h"

float polyfit(float *x, float *y, int n, int degree, float *coefficients, float *fit) 
{
    float X[MAX_POINTS][DEGREE+1]; //120 bytes
    float XT[DEGREE+1][MAX_POINTS]; //120 bytes
    float XTX[DEGREE+1][DEGREE+1];  //36bytes
    float XTX_inv[DEGREE+1][DEGREE+1]; //36bytes
    float XTY[DEGREE+1]; //12byts
    float XTX_inv_XTY[DEGREE+1]; //12byts
    
    int i, j, k;
    
    // 鏋勫缓X鐭╅樀
    for (i = 0; i < n; i++) {
        for (j = 0; j <= degree; j++) {
            X[i][j] = powf(x[i], j);
        }
    }
    
    // 璁＄畻XT鐭╅樀
    for (i = 0; i <= degree; i++) {
        for (j = 0; j < n; j++) {
            XT[i][j] = X[j][i];
        }
    }
    
    // 璁＄畻XTX鐭╅樀
    for (i = 0; i <= degree; i++) {
        for (j = 0; j <= degree; j++) {
            XTX[i][j] = 0;
            for (k = 0; k < n; k++) {
                XTX[i][j] += XT[i][k] * X[k][j];
            }
        }
    }
    
    // 璁＄畻XTX鐭╅樀鐨勯�嗙煩闃�
    // 鍒╃敤楂樻柉-绾﹀綋娑堝厓娉�
    for (i = 0; i <= degree; i++) {
        for (j = 0; j <= degree; j++) {
            if (i == j) {
                XTX_inv[i][j] = 1;
            } else {
                XTX_inv[i][j] = 0;
            }
        }
    }
    
    for (k = 0; k <= degree; k++) {
        float pivot = XTX[k][k];
        
        for (j = 0; j <= degree; j++) {
            XTX[k][j] /= pivot;
            XTX_inv[k][j] /= pivot;
        }
        
        for (i = 0; i <= degree; i++) {
            if (i != k) {
                float factor = XTX[i][k];
                
                for (j = 0; j <= degree; j++) {
                    XTX[i][j] -= factor * XTX[k][j];
                    XTX_inv[i][j] -= factor * XTX_inv[k][j];
                }
            }
        }
    }
    
    // 璁＄畻XTY鐭╅樀
    for (i = 0; i <= degree; i++) {
        XTY[i] = 0;
        for (j = 0; j < n; j++) {
            XTY[i] += XT[i][j] * y[j];
        }
    }
    
    // 璁＄畻XTX_inv_XTY
    for (i = 0; i <= degree; i++) {
        XTX_inv_XTY[i] = 0;
        for (j = 0; j <= degree; j++) {
            XTX_inv_XTY[i] += XTX_inv[i][j] * XTY[j];
        }
    }
    
    // 璁剧疆鎷熷悎澶氶」寮忕殑绯绘暟
    for (i = 0; i <= degree; i++) {
        coefficients[i] = XTX_inv_XTY[i];
    }
    
    // 璁＄畻鎷熷悎璇樊
    float error = 0;
    for (i = 0; i < n; i++) {
        float y_fit = 0;
        for (j = 0; j <= degree; j++) {
            y_fit += coefficients[j] * powf(x[i], j);
        }
        fit[i]=y_fit;
        error += powf(y[i] - y_fit, 2);
        
       // printf("yfit[%d]=%8.4f\r\n",i,y_fit);
    }
    
    return error;
}
