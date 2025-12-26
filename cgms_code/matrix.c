#include<stdint.h>
#include<stdbool.h>
#include<stdio.h>
#include"matrix.h"

//Auther:       created by zhangjingtao
//Date:         2023.10.12
//Version :     v1.0.0


void Convert2_UnitMatix(t_matrix *mat)
{    
    for(uint8_t x=0,y=0;x<mat->x && y<mat->y;x++,y++)
    {
     mat->parray[x*(mat->y+1)]=1;
    }
}
//

void Transpose_Matrix( t_matrix *mat,t_matrix *mat_T)
{
    float value;

    mat_T->x = mat->y;
    mat_T->y = mat->x;

    for(uint16_t x=0; x<mat->x;x++)
    {
        for(uint16_t y=0; y<mat->y;y++)
        {
            value=GetElement_Matrix(mat, x, y);
           //printf("x=%d,y=%d, value =%6.2f\r\n",x,y ,value);
            SetElement_Matrix(mat_T,y,x,value);
        }
    }
}

void MulConst_Matrix(t_matrix *mat , float val)
{
    float value;

    for(uint16_t x=0; x<mat->x;x++)
    {
        for(uint16_t y=0; y<mat->y;y++)
        {
            value=GetElement_Matrix(mat, x, y)*val;
            SetElement_Matrix(mat,x,y,value);
        }
    }

}
/*
*
return : 0 ,invalide parameters
*/

bool MulMatrix_Matrix(t_matrix *mat_a,t_matrix *mat_b, t_matrix *mat_val)
{
    if(mat_a->y != mat_b->x)return 0;

    for(uint16_t val_x=0; val_x<mat_val->x;val_x++)
    {
        for(uint16_t val_y=0; val_y<mat_val->y;val_y++)
        {
            float val_x_y =0; 
            
            for(uint16_t lop=0; lop< mat_a->y; lop++)
            {                
            val_x_y +=GetElement_Matrix(mat_a,val_x,lop)*GetElement_Matrix(mat_b,lop,val_y);
            }
            SetElement_Matrix(mat_val,val_x,val_y,val_x_y);
        }
    }
    return 1;
}

bool Matrix_Add_Matrix(t_matrix *mat_a, t_matrix *mat_b)
{   
    if((mat_a->x != mat_b->x)||(mat_a->y !=mat_b->y))return 0;

    for(uint16_t val_x=0; val_x<mat_a->x;val_x++)
    {
        for(uint16_t val_y=0; val_y<mat_a->y;val_y++)
        { 
              
            float value =GetElement_Matrix(mat_a,val_x,val_y)+GetElement_Matrix(mat_b,val_x,val_y);
            SetElement_Matrix(mat_a,val_x,val_y,value);
        }

    }
    return 1;
}

bool Matrix_Minus_Matrix(t_matrix *mat_a, t_matrix *mat_b)
{
    if((mat_a->x != mat_b->x)||(mat_a->y !=mat_b->y))return 0;

        for(uint16_t val_x=0; val_x<mat_a->x;val_x++)
    {
        for(uint16_t val_y=0; val_y<mat_a->y;val_y++)
        { 
              
            float value =GetElement_Matrix(mat_a,val_x,val_y)-GetElement_Matrix(mat_b,val_x,val_y);
            SetElement_Matrix(mat_a,val_x,val_y,value);
        }

    }
    return 1;
}

/***************************/
float GetElement_Matrix(t_matrix *mat,uint16_t x, uint16_t y)
{
    return (mat->parray[x*mat->y +y]);
}

/**************************/
void SetElement_Matrix(t_matrix *mat,uint16_t x, uint16_t y, float value)
{
    mat->parray[x*mat->y +y]=value;

}
/******************************/

void Matix_Print(t_matrix *mat)
{
    printf("x_max=%d, y_max =%d\r\n",mat->x,mat->y);

    printf("{\r\n");
    uint16_t y_max = mat->y,ycnt=0;;

    for(uint16_t x=0;x< (mat->x * mat->y);x++)
    {
        printf("%10.6f   ",mat->parray[x]);
           
        if(ycnt>=(y_max-1))
        { ycnt=0,printf("\r\n");}
        else
        ycnt++;
    }

    printf("}\r\n");
}


