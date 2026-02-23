/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Raw/Src/httpd_cg_ssi.c
  * @author  MCD Application Team
  * @brief   Webserver SSI and CGI handlers
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "defines.h"
#include "lwip/debug.h"
#include "lwip/tcp.h"
#include "lwip/apps/httpd.h"
#include "http_cgi_ssi.h"


#include <string.h>
#include <stdlib.h>

tSSIHandler ADC_Page_SSI_Handler;
uint32_t ADC_not_configured=1;

/* we will use character "t" as tag for CGI */
char const* TAGCHAR="t";
char const** TAGS=&TAGCHAR;

/* CGI handler for LED control */
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

/* Html request for "/leds.cgi" will start LEDS_CGI_Handler */
const tCGI LEDS_CGI={"/leds.cgi", LEDS_CGI_Handler};

/* Cgi call table, only one CGI used */
tCGI CGI_TAB[1];


/**
  * @brief  CGI handler for LEDs control 
  */
const char * LEDS_CGI_Handler(int iIndex, int /*iNumParams*/, char * /*pcParam*/ [], char * /*pcValue*/[])
{
  /* We have only one SSI handler iIndex = 0 */
  if (iIndex==0)
  {
  }
  /* uri to send after cgi call*/
  return "/STM32F4xxLED.html";  
}

u16_t ADC_Handler(int /*iIndex*/, char * /*pcInsert*/, int /*iInsertLen*/)
{
    return 0;
}

/**
  * @brief  Http webserver Init
  */
void http_server_init(void)
{
  /* Httpd Init */
  httpd_init();

  /* configure SSI handlers (ADC page SSI) */
  http_set_ssi_handler(ADC_Handler, (char const **)TAGS, 1);

  /* configure CGI handlers (LEDs control CGI) */
  CGI_TAB[0] = LEDS_CGI;
  http_set_cgi_handlers(CGI_TAB, 1);
}
