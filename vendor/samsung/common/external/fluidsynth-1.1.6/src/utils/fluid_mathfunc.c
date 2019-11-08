#include "fluid_mathfunc.h"

double g_atof(const char* str)
{
 double  num1 = 0 , num2 = 0 , i = 1;     // num1 = 정수자리 , num2 =소수자리 i=소수점
    int   flag = 0;            // 부호 판단 위한 변수
 
 if( *str == '-' || (*str >= '0' && *str <= '9'))      // 첫 글자가 '-' 또는 '0' ~ '9' 이면
 {
  if(*str == '-')
  {
   flag++;              // 음수 부호 
   str++;              // str포인터 변수 증가
  }
 
  while( *str != '.' && (*str >= '0' && *str <= '9') )   // str포인터가 '-' 이고 '0' ~ '9' 이면
  {
   num1 = ( num1 * 10 ) + ( *str - '0' );      // num1 값 구함
   str++;              // str 포인터 증가
  }

  if ( *str == '.' )            // 현재 str포인터가 .이라면
  {
   str++;
   while( *str >= '0' && *str <= '9' )       // str포인터가 '0' ~ '9' 이면
   {
    num2 = ( num2 * 10 ) + ( *str - '0' );     // num2 값 구함
    str++;             // str 포인터 증가
    i *= 0.1;            // 루프만큼 소수점 자리 증가
   }
   
  }
 
  return !flag ? (num1 + ( num2 * i ) ) : -(num1 + ( num2 * i) ); // flag 값에 따라 리턴값 음 양수 달라짐  
 }
 else
  return 0;              // 실패시 0리턴
}
