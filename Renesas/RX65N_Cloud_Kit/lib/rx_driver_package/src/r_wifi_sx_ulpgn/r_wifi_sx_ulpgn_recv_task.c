#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__CCRL__) || defined(__ICCRL78__) || defined(__RL)
#include <stdint.h>
#endif

#include <tx_api.h>
#include <tx_thread.h>
#include <tx_semaphore.h>
#if defined(__CCRL__) || defined(__ICCRL78__) || defined(__RL)
#include "task.h"
#endif

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#include "platform.h"
#include "r_sci_rx_if.h"
#elif(__CCRL__)
#include "rl_platform.h"
#include "r_sci_wrapper_if.h"
#endif
#include "r_byteq_if.h"
#include "r_wifi_sx_ulpgn_if.h"
#include "r_wifi_sx_ulpgn_private.h"

#if(1 == WIFI_CFG_USE_CALLBACK_FUNCTION)
void WIFI_CFG_CALLBACK_FUNCTION_NAME(void *p_args);
void (* const p_wifi_callback)(void *p_args) = WIFI_CFG_CALLBACK_FUNCTION_NAME;
#else
void (* const p_wifi_callback)(void *p_args) = NULL;
#endif


uint32_t g_wifi_response_recv_count;
uint32_t tmp_recvcnt;
uint32_t g_wifi_response_recv_status;

char g_wifi_response_last_string[15];
uint8_t g_wifi_response_last_string_recv_count;

uint8_t g_wifi_macaddress[6];
wifi_ip_configuration_t g_wifi_ipconfig;
uint32_t g_wifi_dnsaddress;
uint32_t g_wifi_dnsserver_address;
uint32_t g_wifi_dnsquery_subcount;

uint32_t g_wifi_atustat_recv;
uint32_t g_wifi_atustat_sent;

wifi_scan_result_t *gp_wifi_ap_results;
uint32_t g_wifi_aplistmax;
uint32_t g_wifi_aplist_stored_num;
uint32_t g_wifi_aplist_count;
uint32_t g_wifi_aplist_subcount;

uint8_t g_wifi_current_ssid[33];

uint8_t g_wifi_socket_status;

wifi_socket_t g_wifi_socket[WIFI_CFG_CREATABLE_SOCKETS];



wifi_at_execute_queue_t g_wifi_at_execute_queue[10];
uint8_t g_wifi_set_queue_index;
uint8_t g_wifi_get_queue_index;

#define WIFI_RECV_STACK_SIZE (2048U)
static void wifi_recv_task( ULONG input );
TX_THREAD g_wifi_recv_task_handle;
TX_SEMAPHORE g_wifi_recv_task_semaphore;
uint8_t g_wifi_recv_stack[WIFI_RECV_STACK_SIZE];


static void wifi_analyze_ipaddress_string(uint8_t *pstring);
static void wifi_analyze_get_macaddress_string(uint8_t *pstring);
static void wifi_analyze_get_aplist_string(uint8_t *pstring);
static void wifi_analyze_get_dnsquery_string(uint8_t *pstring);
static void wifi_analyze_get_dnsaddress_string(uint8_t *pstring);
static void wifi_analyze_get_sent_recv_size_string(uint8_t *pstring);
static void wifi_analyze_get_current_ssid_string(uint8_t *pstring);
void wifi_analyze_get_socket_status_string(uint8_t *pstring);

static uint8_t IsNumberCheck(uint8_t c);
static void wifi_analyze_get_certificate_string(uint8_t *pstring);
uint8_t g_certificate_str[256]= {0};
uint8_t g_certificate_numstr[256]= {0};

wifi_certificate_infomation_t g_wifi_certificate_information[10];

uint8_t g_number_flg = 1;
uint8_t g_certificate_list_flg = 0;

static uint8_t IsNumberCheck(uint8_t c)
{
	if (0x30 <= c && 0x39 >= c)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void wifi_analyze_get_certificate_string(uint8_t *pstring)
{
	uint8_t unum=0;
	uint8_t start_num=0;
	uint8_t ufnum=0;

	if(0==strcmp((char*)pstring,"OK\r\n"))
	{
	}
	else
	{
		if('\r'==*pstring)
		{
			g_certificate_list_flg  = 1;
			g_number_flg = 1;
			unum=0;
			while ('\0' != g_certificate_str[unum])
			{
				if('\r'==g_certificate_str[unum])
				{
					memcpy(&g_wifi_certificate_information[ufnum].certificate_file,&g_certificate_str[start_num],(unum-start_num));
					g_wifi_certificate_information[ufnum].next_certificate_name = &g_wifi_certificate_information[ufnum+1];
					start_num = unum+2;
					ufnum++;
				}
				unum++;
			}
			memset(g_certificate_str,0,sizeof(g_certificate_str));
			memset(g_certificate_numstr,0,sizeof(g_certificate_numstr));
		}
		else
		{
			if(1==g_number_flg)
			{
				/* 数値として処理 */
				memset(g_certificate_str,0,sizeof(g_certificate_str));
				memset(g_certificate_numstr,0,sizeof(g_certificate_numstr));

				strcat((char*)g_certificate_numstr,(char*)pstring);
				for(unum=0;unum<strlen((char*)&g_certificate_numstr);unum++)
				{
					if('\r'==g_certificate_numstr[unum])
					{
						break;
					}
					if(0==IsNumberCheck(g_certificate_numstr[unum]))
					{
						g_number_flg=0;
						break;
					}
				}
				if(1==g_number_flg)
				{
					if (unum==1)
					{
						g_wifi_certificate_information[0].certificate_number = g_certificate_numstr[0] - 0x30;
					}
					else if(unum==2)
					{
						g_wifi_certificate_information[0].certificate_number = (g_certificate_numstr[0] - 0x30)*10 +  (g_certificate_numstr[1] - 0x30);
					}
					else
					{
						g_wifi_certificate_information[0].certificate_number = 0;
					}
				}
				g_number_flg = 0;
				g_certificate_list_flg = 0;
			}
			else
			{
				strcat((char*)g_certificate_str,(char*)pstring);
			}
		}
	}
}

int32_t wifi_start_recv_task( void )
{
	int32_t ret = -1;
	UINT tx_ret;

	tx_ret = tx_semaphore_create(&g_wifi_recv_task_semaphore, "wifi recv", 0u);
    if(tx_ret != TX_SUCCESS) {
        ret = -1;
    } else {
        ret = 0;
    }

    tx_ret = tx_thread_create(&g_wifi_recv_task_handle, "wifi recv", wifi_recv_task, 0u,
            g_wifi_recv_stack, sizeof(g_wifi_recv_stack), WIFI_CFG_TASK_PRIORITY, WIFI_CFG_TASK_PRIORITY,
                                0u, TX_AUTO_START);
    if(tx_ret != TX_SUCCESS) {
        ret = -1;
    } else {
        ret = 0;
    }

	return ret;
}

void wifi_delete_recv_task( void )
{
    /* Delete wifi driver at response tasks. */
    if( g_wifi_recv_task_handle.tx_thread_id == TX_THREAD_ID )
    {
        tx_thread_delete(&g_wifi_recv_task_handle);
    }

    if( g_wifi_recv_task_semaphore.tx_semaphore_id == TX_SEMAPHORE_ID) {
        tx_semaphore_delete(&g_wifi_recv_task_semaphore);
    }
}

static void wifi_recv_task( ULONG input )
{
	uint8_t data;
	uint8_t *presponse_buff;
	sci_err_t sci_ercd;
	int i;
	byteq_err_t byteq_ercd;
	uint32_t psw_value;
	wifi_at_execute_queue_t * pqueue;
	wifi_err_event_t event;
	uint8_t command_port;
	g_wifi_response_recv_count = 0;
	g_wifi_response_last_string_recv_count = 0;
	memset(g_wifi_response_last_string, 0,sizeof(g_wifi_response_last_string));


    for( ; ; )
    {
    	command_port = wifi_command_port;
    	presponse_buff = g_wifi_uart[command_port].response_buff;
    	sci_ercd = R_SCI_Receive(g_wifi_uart[command_port].wifi_uart_sci_handle, &data, 1);
    	if(SCI_ERR_INSUFFICIENT_DATA == sci_ercd)
    	{
    		/* Pause for a short while to ensure the network is not too
         * congested. */
    		//vTaskDelay( 1 );
    		tx_semaphore_get(&g_wifi_recv_task_semaphore, TX_WAIT_FOREVER);

    	}
    	else if(SCI_SUCCESS == sci_ercd)
    	{
#if DEBUGLOG == 2
			R_BSP_CpuInterruptLevelWrite (WIFI_CFG_SCI_INTERRUPT_LEVEL-1);
    		putchar(data);
			R_BSP_CpuInterruptLevelWrite (0);
#endif
			if(1 == g_wifi_transparent_mode)
			{
    			psw_value = R_BSP_CpuInterruptLevelRead();
				R_BSP_CpuInterruptLevelWrite (WIFI_CFG_SCI_INTERRUPT_LEVEL-1);
    			byteq_ercd = R_BYTEQ_Put(g_wifi_socket[0].socket_byteq_hdl, data);
				R_BSP_CpuInterruptLevelWrite (psw_value);
    			if(byteq_ercd != BYTEQ_SUCCESS)
    			{
    				g_wifi_socket[0].put_error_count++;
    			}
			}
			else
			{
				if(g_wifi_response_recv_count >= g_wifi_uart[command_port].response_buff_size -1 )
				{
					event.event = WIFI_EVENT_RCV_TASK_RXB_OVF_ERR;
					if(NULL != p_wifi_callback)
					{
						p_wifi_callback(&event);
					}
				}
				else
				{
					presponse_buff[g_wifi_response_recv_count] = data;
				}
				g_wifi_response_recv_count++;
			}
			if(g_wifi_response_last_string_recv_count >= (sizeof(g_wifi_response_last_string) -1))
			{
				memmove(&g_wifi_response_last_string[0], &g_wifi_response_last_string[1], (sizeof(g_wifi_response_last_string) -1));
				g_wifi_response_last_string[sizeof(g_wifi_response_last_string) -2] = data;
				g_wifi_response_last_string[sizeof(g_wifi_response_last_string) -1] = '\0';
			}
			else
			{
				g_wifi_response_last_string[g_wifi_response_last_string_recv_count] = data;
				g_wifi_response_last_string[g_wifi_response_last_string_recv_count+1] = '\0';
				g_wifi_response_last_string_recv_count++;

			}
    		if('\n' == data)
    		{
				pqueue = wifi_get_current_running_queue();

				if(pqueue->at_command_id == WIFI_COMMAND_GET_IPADDRESS)
				{
					if(0 == strncmp((char *)presponse_buff,"IP:",3))
					{
						wifi_analyze_ipaddress_string(presponse_buff);
					}
				}
				if(pqueue->at_command_id == WIFI_COMMAND_GET_MACADDRESS)
				{
					if(0 == strncmp((char *)presponse_buff,"Mac Addr",8))
					{
						wifi_analyze_get_macaddress_string(presponse_buff);
					}
				}
				if(pqueue->at_command_id == WIFI_COMMAND_GET_DNSADDRESS)
				{
					wifi_analyze_get_dnsaddress_string(presponse_buff);
				}
				if(pqueue->at_command_id == WIFI_COMMAND_SET_DNSQUERY)
				{
					wifi_analyze_get_dnsquery_string(presponse_buff);
				}
				if(pqueue->at_command_id == WIFI_COMMAND_GET_APLIST)
				{
					wifi_analyze_get_aplist_string(presponse_buff);
				}
				if(pqueue->at_command_id == WIFI_COMMAND_GET_SENT_RECV_SIZE)
				{
					wifi_analyze_get_sent_recv_size_string(presponse_buff);
				}
				if(pqueue->at_command_id == WIFI_COMMAND_GET_CURRENT_SSID)
				{
					wifi_analyze_get_current_ssid_string(presponse_buff);
				}
				if(pqueue->at_command_id == WIFI_COMMAND_GET_SOCKET_STATUS)
				{
					wifi_analyze_get_socket_status_string(presponse_buff);
				}
				else if(0 == strcmp((const char *)presponse_buff,WIFI_RETURN_TEXT_ERROR))
				{
					wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_ERROR );
				}
				else if(0 == strcmp((const char *)presponse_buff,WIFI_RETURN_TEXT_BUSY))
				{
					wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_BUSY );
				}
				else if(0 == strcmp((const char *)presponse_buff,WIFI_RETURN_TEXT_NOCARRIER))
				{
					wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_NOCARRIER );
				}
				else if(0 == strcmp((const char *)presponse_buff,WIFI_RETURN_TEXT_NOANSWER))
				{
					wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_ERROR );
				}
				else if(0 == strcmp((const char *)presponse_buff,WIFI_RETURN_TEXT_OK))
				{
					/* 通常のOK　*/
					wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_OK );
				}
				if(pqueue->at_command_id == WIFI_COMMAND_SET_SYSFALSH_READ_DATA)
				{
					wifi_analyze_get_certificate_string(presponse_buff);		/* 証明書確認 */
				}
				else
				{
				}
				memset(presponse_buff, 0, g_wifi_uart[command_port].response_buff_size);
				g_wifi_response_recv_count = 0;
				g_wifi_response_last_string_recv_count = 0;
				g_wifi_response_recv_status = 0x0000;
    		}
    	}
        if(0 != g_wifi_sci_err_flag)
        {
        	switch(g_wifi_sci_err_flag)
        	{
    			case 1:
    				event.event = WIFI_EVENT_SERIAL_RXQ_OVF_ERR;
    				break;
    			case 2:
    				event.event = WIFI_EVENT_SERIAL_OVF_ERR;
    				break;
    			case 3:
    				event.event = WIFI_EVENT_SERIAL_FLM_ERR;
    				break;
        	}
        	g_wifi_sci_err_flag = 0;
    		if(NULL != p_wifi_callback)
    		{
    			p_wifi_callback(&event);
    		}
        }
        for(i = 0;i<g_wifi_createble_sockets;i++)
        {
        	if(g_wifi_socket[i].put_error_count > 0)
        	{
        		g_wifi_socket[i].put_error_count = 0;
    			event.event = WIFI_EVENT_SOCKET_RXQ_OVF_ERR;
    			event.socket_number = i;
    			if(NULL != p_wifi_callback)
    			{
    				p_wifi_callback(&event);
    			}
        	}

        }
    }
}

static void wifi_analyze_ipaddress_string(uint8_t *pstring)
{
	int scanf_ret;
	uint32_t ipaddr[4];
	uint32_t subnetmask[4];
	uint32_t gateway[4];
    scanf_ret = sscanf((const char *)pstring, "IP:%d.%d.%d.%d, Mask:%d.%d.%d.%d, Gateway:%d.%d.%d.%d\r\n", \
                       &ipaddr[0], &ipaddr[1], &ipaddr[2], &ipaddr[3], \
                       &subnetmask[0], &subnetmask[1], &subnetmask[2], &subnetmask[3], \
                       &gateway[0], &gateway[1], &gateway[2], &gateway[3]);
    if(scanf_ret == 12)
    {
        g_wifi_ipconfig.ipaddress = WIFI_IPV4BYTE_TO_ULONG(ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
        g_wifi_ipconfig.subnetmask = WIFI_IPV4BYTE_TO_ULONG(subnetmask[0], subnetmask[1], subnetmask[2], subnetmask[3]);
        g_wifi_ipconfig.gateway = WIFI_IPV4BYTE_TO_ULONG(gateway[0], gateway[1], gateway[2], gateway[3]);
    }
}

static void wifi_analyze_get_macaddress_string(uint8_t *pstring)
{
	int scanf_ret;
	uint32_t macaddr[6];

	scanf_ret = sscanf((const char *)pstring, "Mac Addr     =   %2x:%2x:%2x:%2x:%2x:%2x\r\n",\
			&macaddr[0], &macaddr[1], &macaddr[2], &macaddr[3], &macaddr[4], &macaddr[5]);

	if (scanf_ret == 6)
	{
		g_wifi_macaddress[0] = macaddr[0];
		g_wifi_macaddress[1] = macaddr[1];
		g_wifi_macaddress[2] = macaddr[2];
		g_wifi_macaddress[3] = macaddr[3];
		g_wifi_macaddress[4] = macaddr[4];
		g_wifi_macaddress[5] = macaddr[5];
	}
}

static void wifi_analyze_get_dnsaddress_string(uint8_t *pstring)
{
	int scanf_ret;
	uint32_t dnsaddr[4];
    scanf_ret = sscanf((const char *)pstring, "%d.%d.%d.%d\r\n", \
                       &dnsaddr[0], &dnsaddr[1], &dnsaddr[2], &dnsaddr[3]);
    if(scanf_ret == 4)
    {
        g_wifi_dnsserver_address = WIFI_IPV4BYTE_TO_ULONG(dnsaddr[0], dnsaddr[1], dnsaddr[2], dnsaddr[3]);
    }
}

static void wifi_analyze_get_aplist_string(uint8_t *pstring)
{
	int scanf_ret;
	uint32_t bssid[6];
	uint32_t channel;
	int32_t indicator;
	char *pstr1;
	char *pstr2;

	if(0 == strncmp((char *)pstring, "ssid = ", 7))
	{
		pstr1 =(char*)(pstring+7);
		pstr2 = strchr((const char *)pstr1,'\r');
		if(NULL != pstr2)
		{
			*pstr2 = '\0';
			if(strlen(pstr2) <= 32)
			{
				if(g_wifi_aplist_subcount >= 1)
				{
					g_wifi_aplist_stored_num++;
					gp_wifi_ap_results++;
				}
				if(g_wifi_aplist_stored_num < g_wifi_aplistmax)
				{
					strcpy((char*)(gp_wifi_ap_results->ssid), pstr1);
				}
				g_wifi_aplist_subcount = 1;
			}
		}
	}
	else if(0 == strncmp((char *)pstring, "bssid =", 7))
	{
		scanf_ret = sscanf((char*)pstring, "bssid = %2x:%2x:%2x:%2x:%2x:%2x\r\n",\
				&bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]);
		if(scanf_ret == 6)
		{
			if(g_wifi_aplist_subcount >= 2)
			{
				g_wifi_aplist_stored_num++;
				gp_wifi_ap_results++;
			}
			if(g_wifi_aplist_stored_num < g_wifi_aplistmax)
			{
				gp_wifi_ap_results->bssid[0] = bssid[0];
				gp_wifi_ap_results->bssid[1] = bssid[1];
				gp_wifi_ap_results->bssid[2] = bssid[2];
				gp_wifi_ap_results->bssid[3] = bssid[3];
				gp_wifi_ap_results->bssid[4] = bssid[4];
				gp_wifi_ap_results->bssid[5] = bssid[5];
			}
			g_wifi_aplist_subcount = 2;

		}
	}
	else if(0 == strncmp((char *)pstring, "channel =", 9))
	{
		scanf_ret = sscanf((const char *)pstring, "channel = %d\r\n",\
				&channel);
		if(scanf_ret == 1)
		{
			if(g_wifi_aplist_subcount >= 3)
			{
				g_wifi_aplist_stored_num++;
				gp_wifi_ap_results++;
			}
			if(g_wifi_aplist_stored_num < g_wifi_aplistmax)
			{
				gp_wifi_ap_results->channel = channel;
			}
			g_wifi_aplist_subcount = 3;
		}
	}
	else if(0 == strncmp((char *)pstring, "indicator =", 11))
	{
		scanf_ret = sscanf((char *)pstring, "indicator = %d\r\n",\
				&indicator);
		if(scanf_ret == 1)
		{
			if(g_wifi_aplist_subcount >= 4)
			{
				g_wifi_aplist_stored_num++;
				gp_wifi_ap_results++;
			}
			if(g_wifi_aplist_stored_num < g_wifi_aplistmax)
			{
				gp_wifi_ap_results->rssi = indicator;
			}
			g_wifi_aplist_subcount = 4;
		}
	}
	else if (0 == strncmp((char *)pstring, "security = NONE!", 16))
	{
		if(g_wifi_aplist_subcount >= 5)
		{
			g_wifi_aplist_stored_num++;
			gp_wifi_ap_results++;
		}
		if(g_wifi_aplist_stored_num < g_wifi_aplistmax)
		{
			gp_wifi_ap_results->security = WIFI_SECURITY_OPEN;
		}
		g_wifi_aplist_subcount = 5;
		g_wifi_aplist_count++;
	}
	else if (0 == strncmp((char *)pstring, "WPA", 3))
	{
		if(g_wifi_aplist_subcount >= 5)
		{
			g_wifi_aplist_stored_num++;
			gp_wifi_ap_results++;
		}
		if(g_wifi_aplist_stored_num < g_wifi_aplistmax)
		{
			gp_wifi_ap_results->security = WIFI_SECURITY_WPA;
		}
		g_wifi_aplist_subcount = 5;
		g_wifi_aplist_count++;
	}
	else if (0 == strncmp((char *)pstring, "RSN/WPA2", 8))
	{
		if(g_wifi_aplist_subcount >= 5)
		{
			g_wifi_aplist_stored_num++;
			gp_wifi_ap_results++;
		}
		if(g_wifi_aplist_stored_num < g_wifi_aplistmax)
		{
			gp_wifi_ap_results->security = WIFI_SECURITY_WPA2;
		}
		g_wifi_aplist_subcount = 5;
		g_wifi_aplist_count++;
	}
}

static void wifi_analyze_get_dnsquery_string(uint8_t *pstring)
{
	uint32_t ipaddr[4];
	if(0 == strncmp((char*)pstring,"1\r\n",3))
	{
		g_wifi_dnsquery_subcount = 1;
	}
	else if(4 == sscanf((char*)pstring,"%d.%d.%d.%d\r\n",&ipaddr[0],&ipaddr[1],&ipaddr[2],&ipaddr[3]))
	{
		if(1 == g_wifi_dnsquery_subcount)
		{
			g_wifi_dnsaddress = WIFI_IPV4BYTE_TO_ULONG(ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
			g_wifi_dnsquery_subcount = 2;
		}
	}
}
static void wifi_analyze_get_sent_recv_size_string(uint8_t *pstring)
{
	uint32_t recv;
	uint32_t sent;
	if(2 == sscanf((char*)pstring,"recv=%lu sent=%lu\r\n",(uint8_t*)&recv,&sent))
	{
		g_wifi_atustat_recv = recv;
		g_wifi_atustat_sent = sent;
	}
}

static void wifi_analyze_get_current_ssid_string(uint8_t *pstring)
{
	char tag[] = "ssid         =   ";
	char *ptr1;
	char *ptr2;
	if(0 == strncmp((char *)pstring, tag, strlen(tag)))
	{
		ptr1 = (char *)pstring + strlen(tag);
		ptr2 = strchr(ptr1,'\r');
		if(NULL == ptr2)
		{
			ptr2 = '\0';
			if(strlen(ptr2) < 32)
			{
				strcpy((char *)g_wifi_current_ssid, ptr2);
			}
		}
	}
}

void wifi_analyze_get_socket_status_string(uint8_t *pstring)
{
	char * str_ptr;
	int i;

	/* focus 1st line "CLOSED", "SOCKET", "BOUND", "LISTEN" or "CONNECTED".*/
	if( ('A' <= pstring[0]) && (pstring[0] <= 'Z'))
	{
		str_ptr = strchr((const char *)pstring, ',');
	    if(str_ptr != NULL)
	    {
	    	str_ptr = "\0";
	        for(i = 0; i < 5; i++)
	        {
	            if(0 == strcmp((const char *)str_ptr, (const char *)wifi_socket_status_tbl[i]))
	            {
	                break;
	            }
	        }
		    if(i < 5)
		    {
		    	g_wifi_socket_status = i;
		    }
	    }
	}
}
