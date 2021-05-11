#ifndef R_WIFI_SX_ULPGN_IF_H
#define R_WIFI_SX_ULPGN_IF_H

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#include "r_sci_rx_if.h"
#include "platform.h"
#endif
#if defined(__CCRL__) || defined(__ICCRL78__) || defined(__RL)
//#include "FreeRTOS.h"
#include "r_sci_wrapper_if.h"
#include "rl_platform.h"
#include "r_cg_serial.h"
#endif

#include "r_byteq_if.h"
#include "r_wifi_sx_ulpgn_config.h"

/* Driver Version Number. */
#define WIFI_SX_ULPGN_VERSION_MAJOR           (1)
#define WIFI_SX_ULPGN_VERSION_MINOR           (00)

#define WIFI_SOCKET_IP_PROTOCOL_UDP (5)
#define WIFI_SOCKET_IP_PROTOCOL_TCP (6)

#define WIFI_SOCKET_IP_VERSION_4    (4)

#define WIFI_IPV4BYTE_TO_ULONG(in_addr1, in_addr2, in_addr3, in_addr4) \
	(((uint32_t)in_addr1 & 0x000000FF) << 24) | (((uint32_t)in_addr2 & 0x000000FF) << 16) |\
    (((uint32_t)in_addr3 & 0x000000FF) << 8)  | (((uint32_t)in_addr4 & 0x000000FF))

#define WIFI_ULONG_TO_IPV4BYTE_1(in_ul) (uint8_t)(((uint32_t)in_ul) >> 24)
#define WIFI_ULONG_TO_IPV4BYTE_2(in_ul) (uint8_t)(((uint32_t)in_ul) >> 16)
#define WIFI_ULONG_TO_IPV4BYTE_3(in_ul) (uint8_t)(((uint32_t)in_ul) >> 8)
#define WIFI_ULONG_TO_IPV4BYTE_4(in_ul) (uint8_t)((uint32_t)in_ul)

typedef enum
{
	WIFI_SECURITY_OPEN            = 0,
	WIFI_SECURITY_WEP,
	WIFI_SECURITY_WPA,
	WIFI_SECURITY_WPA2,
	WIFI_SECURITY_UNDEFINED,
}wifi_security_t;

typedef enum 			// Wi-Fi APIエラーコード
{
	WIFI_SUCCESS            = 0,	// 成功
	WIFI_ERR_PARAMETER	    = -1,	// 引数が無効です。
	WIFI_ERR_ALREADY_OPEN   = -2,	// すでに初期化済みです
	WIFI_ERR_NOT_OPEN       = -3,	// 初期化していません
	WIFI_ERR_SERIAL_OPEN    = -4,	// シリアルの初期化ができません。
	WIFI_ERR_MODULE_COM     = -5,	// WiFiモジュールとの通信に失敗しました。
	WIFI_ERR_NOT_CONNECT    = -6,	// アクセスポイントに接続していません。
	WIFI_ERR_SOCKET_NUM     = -7,	// 利用可能なソケットがありません。
	WIFI_ERR_SOCKET_CREATE  = -8,	// ソケットを作成できません。
	WIFI_ERR_CHANGE_SOCKET  = -9,	// ソケットを切り替えられません。
	WIFI_ERR_SOCKET_CONNECT = -10,	// ソケットに接続できません。
	WIFI_ERR_BYTEQ_OPEN     = -11,	// BYTEQの割り当てに失敗しました。
	WIFI_ERR_SOCKET_TIMEOUT = -12,	// ソケットの送信でタイムアウトしました。
	WIFI_ERR_TAKE_MUTEX     = -13,	// Mutexの取得に失敗しました。
	WIRI_ERR_FLASH_WRITE    = -14,  // WiFiモジュールFLASHへの書き込みに失敗しました。
	WIRI_ERR_FLASH_ERASE    = -15,  // WiFiモジュールFLASHへの消去に失敗しました。
	WIRI_ERR_FLASH_READ     = -16,  // WiFiモジュールFLASHへの読み出しに失敗しました。
} wifi_err_t;

typedef enum
{
	WIFI_EVENT_WIFI_REBOOT = 0,
	WIFI_EVENT_WIFI_DISCONNECT,
	WIFI_EVENT_SERIAL_OVF_ERR,
	WIFI_EVENT_SERIAL_FLM_ERR,
	WIFI_EVENT_SERIAL_RXQ_OVF_ERR,
	WIFI_EVENT_RCV_TASK_RXB_OVF_ERR,
	WIFI_EVENT_SOCKET_CLOSED,
	WIFI_EVENT_SOCKET_RXQ_OVF_ERR,
} wifi_err_event_enum_t;

typedef struct
{
		uint8_t ssid[ 33 ];			//SSID�̊i�[�̈�
		uint8_t bssid[ 6 ];			//BSSID�̊i�[�̈�
		wifi_security_t security;	//�Z�L�����e�B�^�C�v�̊i�[�̈�
		int8_t rssi;		     	//�M�����x�̊i�[�̈�
		int8_t channel;				//�`�����l���ԍ��̊i�[�̈�
		uint8_t hidden;				//Hidden�`�����l���̊i�[�̈�
} wifi_scan_result_t;

typedef struct
{
		uint32_t ipaddress;	//IP�A�h���X
		uint32_t subnetmask;	//�T�u�l�b�g�}�X�N
		uint32_t gateway;	//�Q�[�g�E�F�C
} wifi_ip_configuration_t;

typedef struct
{
	wifi_err_event_enum_t event;
	uint32_t socket_number;
}wifi_err_event_t;

typedef enum
{
    ULPGN_SOCKET_STATUS_CLOSED            = 0,
    ULPGN_SOCKET_STATUS_SOCKET,
    ULPGN_SOCKET_STATUS_BOUND,
    ULPGN_SOCKET_STATUS_LISTEN,
    ULPGN_SOCKET_STATUS_CONNECTED,
    ULPGN_SOCKET_STATUS_MAX,
} sx_ulpgn_socket_status_t;

typedef struct
{
	char      host_name[256];
	uint32_t  host_address;
	uint8_t   Certificate_id;
}WIFI_CertificateProfile_t;

typedef struct {
	uint8_t certificate_file[20];
	uint8_t certificate_number;
//	wifi_err_t error_flag;
	void *next_certificate_name;
}wifi_certificate_infomation_t;

/* Configuration */
//#define ULPGN_USE_UART_NUM (1)
#define ULPGN_CFG_SOCKET_CHANGE_TIMEOUT_VALUE    200
#define ULPGN_CFG_SOCKET_CHANGE_TIMEOUT_PERIOD   BSP_DELAY_MILLISECS


#define ULPGN_RETURN_TEXT_OK          "OK\r\n"
#define ULPGN_RETURN_TEXT_CONNECT     "CONNECT\r\n"
#define ULPGN_RETURN_TEXT_RING        "RING\r\n"
#define ULPGN_RETURN_TEXT_NO_CARRIER  "NO_CARRIER\r\n"
#define ULPGN_RETURN_TEXT_ERROR       "ERROR\r\n"
#define ULPGN_RETURN_TEXT_NO_DIALTONE "NO_DIALTONE\r\n"
#define ULPGN_RETURN_TEXT_BUSY        "BUSY\r\n"
#define ULPGN_RETURN_TEXT_NO_ANSWER   "NO_ANSWER\r\n"
#define ULPGN_RETURN_TEXT_LENGTH      (13+1) /* strlen(ULPGN_RETURN_TEXT_NO_DIALTONE)+1 */

#define ULPGN_SOCKET_STATUS_TEXT_CLOSED          "CLOSED"
#define ULPGN_SOCKET_STATUS_TEXT_SOCKET          "SOCKET"
#define ULPGN_SOCKET_STATUS_TEXT_BOUND           "BOUND"
#define ULPGN_SOCKET_STATUS_TEXT_LISTEN          "LISTEN"
#define ULPGN_SOCKET_STATUS_TEXT_CONNECTED       "CONNECTED"

extern uint8_t ULPGN_USE_UART_NUM;

extern uint8_t g_wifi_createble_sockets;
extern uint8_t wifi_command_port;
extern uint8_t wifi_data_port;

int32_t R_WIFI_SX_ULPGN_GetTcpSocketStatus(uint8_t socket_number);

wifi_err_t R_WIFI_SX_ULPGN_Open(void);
wifi_err_t R_WIFI_SX_ULPGN_Close(void);
wifi_err_t R_WIFI_SX_ULPGN_Scan(wifi_scan_result_t *ap_results,uint32_t max_networks, uint32_t *exist_ap_count);
wifi_err_t R_WIFI_SX_ULPGN_SetDnsServerAddress(uint32_t dnsaddress1, uint32_t dnsaddress2);
wifi_err_t R_WIFI_SX_ULPGN_Connect (const uint8_t *ssid, const uint8_t *pass, uint32_t security, uint8_t dhcp_enable, wifi_ip_configuration_t *ipconfig);
wifi_err_t R_WIFI_SX_ULPGN_Disconnect (void);
int32_t    R_WIFI_SX_ULPGN_IsConnected (void);
wifi_err_t R_WIFI_SX_ULPGN_GetMacAddress (uint8_t *mac_address);
wifi_err_t R_WIFI_SX_ULPGN_GetIpAddress (wifi_ip_configuration_t *ip_config);
wifi_err_t R_WIFI_SX_ULPGN_GetDnsServerAddress(uint32_t *dns_address);
int32_t    R_WIFI_SX_ULPGN_CreateSocket(uint32_t type, uint32_t ip_version);
wifi_err_t R_WIFI_SX_ULPGN_ConnectSocket(int32_t socket_number, uint32_t ip_address, uint16_t port,char *destination);
wifi_err_t R_WIFI_SX_ULPGN_CloseSocket(int32_t socket_number);
int32_t    R_WIFI_SX_ULPGN_SendSocket (int32_t socket_number, uint8_t *data, int32_t length, uint32_t timeout_ms);
int32_t    R_WIFI_SX_ULPGN_ReceiveSocket (int32_t socket_number, uint8_t *data, int32_t length, uint32_t timeout_ms);
wifi_err_t R_WIFI_SX_ULPGN_ShutdownSocket (int32_t socket_number);
wifi_err_t R_WIFI_SX_ULPGN_DnsQuery (uint8_t *domain_name, uint32_t *ip_address);
wifi_err_t R_WIFI_SX_ULPGN_Ping (uint32_t ip_address, uint16_t count, uint32_t interval_ms);
wifi_err_t R_WIFI_SX_ULPGN_WriteServerCertificate(uint32_t data_id,uint32_t data_type,const uint8_t *certificate,uint32_t certificate_length);

/* Request TLS to each SOCKET */
wifi_err_t R_WIFI_SX_ULPGN_RequestTlsSocket (int32_t socket_number);

/* Server Certificate File Write/Erase Function */
wifi_err_t R_WIFI_SX_ULPGN_EraseServerCertificate (uint8_t *certificate_name);
wifi_err_t  R_WIFI_SX_ULPGN_GetServerCertificate (wifi_certificate_infomation_t *wifi_certificate_information);
wifi_err_t	R_WIFI_SX_ULPGN_EraseAllServerCertificate(void);
wifi_err_t	R_WIFI_SX_ULPGN_SetCertificateProfile(uint8_t certificate_id, uint32_t ipaddress, char *servername);

/* Driver Version */
uint32_t   R_WIFI_SX_ULPGN_GetVersion(void);

void r_wifi_sx_ulpgn_give_semaphore(void);

#endif /* #define SX_ULPGN_DRIVER_H */
