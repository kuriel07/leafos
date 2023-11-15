/*!\file 			tk_apis.h
 * \brief     	toolkit engine for shard
 * \details   	toolkit engine for shard
 * \author    	AGP
 * \version   	1.0
 * \date      	2016
 * \copyright 	OrbLeaf Technology
 * \pre       	
 * initial release as desktop console application
\verbatim	
********************************************************************
1.0
 * initial release (2015.12.XX)
 * added support to load icon using GET_DATA (2016.03.16)
 * changed download from base64 string instead of binary data (2016.03.17)
 * added tk_base64_decode to support base64 string from orb-river (sourcecode opensource.apple.com/.../base64.c) (2016.03.17)
 * added backup feature on NVRAM for touchscreen configuration (struct ui_config, only 20 bytes avail) (2016.03.17)
 * confirmed download and install orc file from server to card success using default auth (2016.03.17)
 * added: tk_card_send for sending and receiving data to/from card with it's own response mechanism supporting T=1 and T=0 (2016.03.26)
 * changed: toolkit command fetch added to tk_card_send for T=0 (2016.03.26)
 * added: river search implementation, modified river UI (2016.03.31)
 * added: thermal printer support, modified http_request (2016.04.17)
 * improved: pandora authentication to authenticate card (2016.05.20)
 * added : tk_usb.c for KRON driver VID=0x7EAF&PID=0xE001 (2016.06.11)
 * changed: protocol and port number switching to use url prefix and postfix instead of cmd_qualifier and transport_level tag (2016.06.13)
 * added: support for CoAP, untested (2016.06.13)
 * added: DNS client, RFC1035 support (2016.06.14)
 * added: LibQRGen, QRCode generator library (2016.08.01)
 * added: Jansson, JSON library (2016.08.09)
 * added: libpng support (2016.12.01)
1.1
 * added: orb-weaver virtual machine for device entity (2016.12.16)
 * fixed: corterra ui_combo and search feature for multitasking support (2017.03.23)
 * added: invalid card error handler, tk_main (2017.04.02)
 * added: exception handler, low level implemented on os_init.c (2017.04.08)
 * fixed: wbBuffer data overflow on tk_list_application, caused by invalid length from tk_card_send (2017.04.28)
 * changed: tk_list_application from icon based (ui_icon) to listview w/ predefined icon (ui_app_item) (2017.04.30)
 * added: support for seamless framework invocation using ti_load API (2017.05.11)
 * fixed: some ui panel stacking bug for toolkit operation (2017.05.18)
 * fixed: device sleep adding os_enter_critical to disable os_tick (2017.05.28)
 * added: datetime APIs for decode/encoding ISO8601 (2017.06.04)
 * fixed: orc download sha256 check, missing byte (2017.07.28)
 * fixed: app delete list menu not showing up when application is running (2017.08.17)
 * added: changing screen orientation on tk_setting (2017.08.27)
 * added: auto-sync feature for devlet (2017.09.01)
 * added: detail description when listing devlet (2017.09.04)
 * added: resource management (load/store) to support image rendering from resource (2017.12.08)
 * added: display text to support resource id (2017.12.08)
 * changed: background task (check update, sync devlet, etc) moved to worker task (2018.02.18)
1.2
 * modified: installing new app, will re-init system removing all pre-installed handler (2018.04.07)
 * added: load framework application (program.main) including app icon (2018.05.09)
 * changed: tk_load_configuration now load mcu/board microprocessor id and interfaces support (2018.10.15)
********************************************************************
\endverbatim
 */

#ifndef TK_APIS__H
#define TK_APIS__H
#include "..\..\defs.h"
#ifndef IF_APIS__H
#include "..\..\interfaces\inc\if_apis.h"
#endif
#ifndef _OWL__H
#include "..\..\ether\inc\owl.h"
#endif


/* STK TAG SET */
#define STK_TAG_NONE				0x00	//process all tag (proprietary)
#define STK_TAG_CMD_DETAIL			0x01
#define STK_TAG_DEV_ID				0x02	
#define STK_TAG_RESULT				0x03
#define STK_TAG_DURATION			0x04
#define STK_TAG_ALPHA				0x05
#define STK_TAG_ADDRESS				0x06
#define STK_TAG_CAPABILITY			0x07
#define STK_TAG_SUBADDRESS			0x08
#define STK_TAG_SS_STRING			0x09
#define STK_TAG_USSD_STRING			0x0A
#define STK_TAG_SMS_TPDU			0x0B
#define STK_TAG_CB_PAGE				0x0C
#define STK_TAG_TEXT_STRING			0x0D
#define STK_TAG_TONE				0x0E
#define STK_TAG_ITEM				0x0F
#define STK_TAG_ITEM_ID				0x10
#define STK_TAG_RESPONSE_LENGTH		0x11
#define STK_TAG_FILE_LIST			0x12
#define STK_TAG_LOCATION_INFO		0x13
#define STK_TAG_IMEI				0x14
#define STK_TAG_HELP_REQUEST		0x15
#define STK_TAG_NMR					0x16
#define STK_TAG_DEFAULT_TEXT		0x17
#define STK_TAG_EVENT_LIST			0x19
#define STK_TAG_CAUSE				0x1A
#define STK_TAG_LOCATION_STATUS		0x1B
#define STK_TAG_TRANSACTION_ID		0x1C
#define STK_TAG_BCCH_CHANNEL_LIST	0x1D
#define STK_TAG_ICON_IDENTIFIER		0x1E
#define STK_TAG_ICON_LIST			0x1F
#define STK_TAG_CARD_READER_STATUS	0x20
#define STK_TAG_TIMER_IDENTIFIER	0x24
#define STK_TAG_TIMER_VALUE			0x25
#define STK_TAG_DATETIMEZONE		0x26
#define STK_TAG_DATETIMEFORMAT	0x27			/* proprietary tag */
#define STK_TAG_IMDT_RESPONSE		0x2B
#define STK_TAG_DTMF_STRING			0x2C
//class B
#define STK_TAG_AT_COMMAND			0x28
#define STK_TAG_AT_RESPONSE			0x29
#define STK_TAG_LANGUAGE			0x2D
#define STK_TAG_TIMING_ADVANCE		0x2E
//class C
#define STK_TAG_BROWSER_ID					0x30
#define STK_TAG_URL								0x31
#define STK_TAG_BEARER							0x32
#define STK_TAG_PROVISIONING					0x33
#define STK_TAG_BROWSER_TERMINATION	0x34
#define STK_TAG_BEARER_DESC					0x35
#define STK_TAG_CHANNEL_DATA				0x36
#define STK_TAG_CHANNEL_DATA_LENGTH	0x37
#define STK_TAG_CHANNEL_STATUS			0x38
#define STK_TAG_BUFFER_SIZE					0x39
#define STK_TAG_TRANSPORT_LEVEL			0x3C

//proprietary payload tags
#define STK_TAG_PAYLOAD_PLAIN		0x40
#define STK_TAG_PAYLOAD_JSON		0x41
#define STK_TAG_PAYLOAD_OWB		0x42		//proprietary orbweaver object
//scripting tags
#define STK_TAG_PARAM					0x60		//card->terminal
#define STK_TAG_CLASS					0x61		
#define STK_TAG_METHOD					0x62 
#define STK_TAG_ALIAS						0x63
#define STK_TAG_INSTANCE				0x64		
#define STK_TAG_VENDOR					0x65
#define STK_TAG_APPNAME				0x66
#define STK_TAG_RETURN					0x67		//terminal->card
#define STK_TAG_ERROR					0x6E

//reserved tags
#define STK_TAG_RESOURCE_ID			0x70		

#define STK_TAG_ALL					0xFF		//process all tag (proprietary)
#define STK_TAG_START				0xF0

/* STK CMD DETAIL TYPE */
#define STK_CMD_REFRESH 		1
#define STK_CMD_MORE_TIME		2
#define STK_CMD_POLL_INTERVAL	3
#define STK_CMD_POLLING_OFF		4
#define STK_CMD_SETUP_EVENT_LIST	5
#define STK_CMD_SETUP_CALL		0x10
#define STK_CMD_SEND_SS			0x11
#define STK_CMD_SEND_USSD		0x12
//- "12" = Reserved for SEND USSD;
#define STK_CMD_SEND_SHORT_MESSAGE 	0x13
#define STK_CMD_SEND_DTMF			0x14
#define STK_CMD_LAUNCH_BROWSER		0x15

#define STK_CMD_PLAY_TONE			0x20
#define STK_CMD_DISPLAY_TEXT		0x21
#define STK_CMD_GET_INKEY			0x22
#define STK_CMD_GET_INPUT			0x23
#define STK_CMD_SELECT_ITEM		0x24
#define STK_CMD_SETUP_MENU		0x25
#define STK_CMD_PROVIDE_LOCAL_INFORMATION	0x26
#define STK_CMD_TIMER_MANAGEMENT	0x27
#define STK_CMD_SET_UP_IDLE_TEXT	0x28

#define STK_CMD_OPEN_CHANNEL		0x40
#define STK_CMD_CLOSE_CHANNEL		0x41 
#define STK_CMD_RECEIVE_DATA		0x42
#define STK_CMD_SEND_DATA			0x43
#define STK_CMD_GET_CHANNEL_STAT	0x44

/* extended interface */
#define STK_CMD_SEND_DEVICE					0x50
#define STK_CMD_SEND_REQ						0x51
#define STK_CMD_SEND_PRINTER				0x52
#define STK_CMD_INVOKE_KERNEL				0x6F
#define STK_CMD_LOAD_RESOURCE				0x60
#define STK_CMD_STORE_RESOURCE			0x61
#define STK_CMD_COMPARE_RESOURCE		0x62

/* HTTP backend APIs */
#define STK_CMD_SEND_DEVICE_EXT			0x70
#define STK_CMD_SEND_REQ_EXT			0x71

//- "27" to "FF" are reserved values

#define ENV_TAG_SMS_PP		0xD1
#define ENV_TAG_SMS_CB		0xD2
#define ENV_TAG_MENU		0xD3
#define ENV_TAG_CALL		0xD4
#define ENV_TAG_MOSM		0xD5
#define ENV_TAG_EVENT		0xD6
#define ENV_TAG_TIMER		0xD7

#define FETCH_TAG_PROSIM 	0xD0

/* STK DEVICE SOURCE/DESTINATION SET */
#define STK_DEV_KEYPAD				0x01
#define STK_DEV_DISPLAY				0x02
#define STK_DEV_EARPIECE			0x03
#define STK_DEV_READER				0x10
#define STK_DEV_CHANNEL				0x20
#define STK_DEV_SIM					0x81
#define STK_DEV_ME					0x82
#define STK_DEV_NETWORK				0x83

#define STK_RES_SUCCESS				0x00	/* success */
#define STK_RES_PARTIAL				0x01
#define STK_RES_MISSING				0x02
#define STK_RES_REFRESH				0x03
#define STK_RES_NO_ICON				0x04
#define STK_RES_MOD_CALL			0x05
#define STK_RES_LIMIT_SERVICE		0x06
#define STK_RES_TERMINATED			0x10  
#define STK_RES_BACKWARD			0x11
#define STK_RES_NO_USER_RESPONSE	0x12
#define STK_RES_HELP_REQUIRED		0x13
#define STK_RES_TRANSACTION_ABORT	0x14

#define STK_RES_ME_FAIL				0x20	/* warning */
#define STK_RES_NETWORK_FAIL 		0x21
#define STK_RES_USER_TIMEOUT		0x22
#define STK_RES_USER_ABORT			0x23

#define STK_RES_ME_ERROR			0x30	/* error */
#define STK_RES_ME_TYPE_ERROR		0x31
#define STK_RES_ME_DATA_ERROR		0x32
#define STK_RES_ME_NUM_ERROR		0x33
#define STK_RES_SS_ERROR			0x34
#define STK_RES_SMS_ERROR			0x35
#define STK_RES_ERROR				0x36

#define RESULT_POR_OK				0
#define RESULT_RCCCDS_FAILED		1
#define RESULT_CNTR_LOW				2
#define RESULT_CNTR_HIGH			3
#define RESULT_CNTR_BLOCKED			4
#define RESULT_CNTR_CIPHER_ERROR	5
#define RESULT_MEMORY_INSUFFICIENT	7

#define EVENT_MT_CALL				0
#define EVENT_CALL_CONNECTED		1
#define EVENT_CALL_DISCONNECTED		2
#define EVENT_LOCATION_STATUS		3
#define EVENT_USER_ACTIVITY			4
#define EVENT_IDLE_SCREEN_AVAIL		5
#define EVENT_CARD_READER_STAT		6
#define EVENT_LANGUAGE_SELECT		7
#define EVENT_BROWSER_TERMINATION	8
#define EVENT_DATA_AVAILABLE		9
#define EVENT_CHANNEL_STATUS		0x0A

#define TK_TERMINAL_PROFILE_DOWNLOAD		 		0
#define TK_TERMINAL_SMSPP							1
#define TK_TERMINAL_SMSCB							2
#define TK_TERMINAL_MENU_SELECT						3
#define TK_TERMINAL_TIMER							5
#define TK_TERMINAL_CALL_CONTROL_USIM				6	
#define TK_TERMINAL_RESULT							8
#define TK_TERMINAL_MOSM							11
#define TK_TERMINAL_UCS2_ENTRY						13
#define TK_TERMINAL_UCS2_DISPLAY					14
#define TK_TERMINAL_DISPLAY_TEXT					16
#define TK_TERMINAL_GET_INKEY						17
#define TK_TERMINAL_GET_INPUT						18
#define TK_TERMINAL_MORE_TIME						19
#define TK_TERMINAL_PLAY_TONE						20
#define TK_TERMINAL_POLL_INTERVAL					21
#define TK_TERMINAL_POLLING_OFF					22
#define TK_TERMINAL_REFRESH						23
#define TK_TERMINAL_SELECT_ITEM					24
#define TK_TERMINAL_SEND_SM						25
#define TK_TERMINAL_SEND_SS						26
#define TK_TERMINAL_SEND_USSD						27
#define TK_TERMINAL_SETUP_CALL						28
#define TK_TERMINAL_SETUP_MENU						29
#define TK_TERMINAL_LOCATION_IMEI					30
#define TK_TERMINAL_NMR							31
#define TK_TERMINAL_SETUP_EVENT_LIST				32
#define TK_TERMINAL_EVENT_MTCALL					33
#define TK_TERMINAL_EVENT_CALL_CONNECTED			34
#define TK_TERMINAL_EVENT_CALL_DISCONNECTED		35
#define TK_TERMINAL_EVENT_LOCATION_STATUS			36
#define TK_TERMINAL_EVENT_USER_ACTIVITY			37
#define TK_TERMINAL_EVENT_IDLE_SCREEN_AVAIL		38
#define TK_TERMINAL_EVENT_READER_STATUS			39
#define TK_TERMINAL_EVENT_LANGUAGE_SELECT			40
#define TK_TERMINAL_EVENT_BROWSER_TERMINATION		41
#define TK_TERMINAL_EVENT_DATA_AVAILABLE			42
#define TK_TERMINAL_EVENT_CHANNEL_STATUS			43
#define TK_TERMINAL_EVENT_ACCESS_TECH_CHANGE		44
#define TK_TERMINAL_EVENT_DISPLAY_PARAMS_CHANGE  	45
#define TK_TERMINAL_EVENT_LOCAL_CONNECTION		  	46
#define TK_TERMINAL_EVENT_NETWORK_SEARCH_CHANGE  	47
#define TK_TERMINAL_POWER_ON_CARD				  	48
#define TK_TERMINAL_POWER_OFF_CARD				  	49
#define TK_TERMINAL_PERFORM_CARD_APDU			  	50
#define TK_TERMINAL_GET_READER_STATUS			  	51
#define TK_TERMINAL_GET_READER_ID				  	52
#define TK_TERMINAL_TIMER_START_STOP			  	56
#define TK_TERMINAL_TIMER_VALUE				  	57
#define TK_TERMINAL_DATETIME				  		58
#define TK_TERMINAL_SETUP_IDLE_TEXT			  	60
#define TK_TERMINAL_RUN_AT_CMD					  	61
#define TK_TERMINAL_SEND_DTMF					  	65
#define TK_TERMINAL_LANGUAGE_SETTING				67
#define TK_TERMINAL_TIMING_ADVANCE				  	68
#define TK_TERMINAL_LANGUAGE_NOTIF				  	69
#define TK_TERMINAL_LAUNCH_BROWSER				  	70
#define TK_TERMINAL_GET_ACCESS_TECH			  	71
#define TK_TERMINAL_SOFTKEY_SELECT_ITEM		  	72
#define TK_TERMINAL_SOFTKEY_SETUP_MENU			  	73
#define TK_TERMINAL_OPEN_CHANNEL				  	88
#define TK_TERMINAL_CLOSE_CHANNEL				  	89
#define TK_TERMINAL_RECEIVE_DATA				  	90
#define TK_TERMINAL_SEND_DATA					  	91
#define TK_TERMINAL_GET_CHANNEL_STATUS			  	92
#define TK_TERMINAL_SERVICE_SEARCH				  	93
#define TK_TERMINAL_GET_SERVICE_INFO			  	94
#define TK_TERMINAL_DECLARE_SERVICE			  	95

//response packet status error
#define RESPONSE_PKT_POR_OK				0
#define RESPONSE_PKT_AUTH_FAIL			1
#define RESPONSE_PKT_CNTR_LOW			2
#define RESPONSE_PKT_CNTR_HIGH			3
#define RESPONSE_PKT_CNTR_BLOCKED		4
#define RESPONSE_PKT_CIPHER_ERROR		5
#define RESPONSE_PKT_UNKNOWN_ERROR		6
#define RESPONSE_PKT_MEMORY_ERROR		7
#define RESPONSE_PKT_MORE_TIME			8
#define RESPONSE_PKT_TAR_UNKNOWN		9
#define RESPONSE_PKT_POR_NONE			0xFF		//no status (use command format)

#define TK_TAG_NOT_FOUND				-1
#define TK_BUFFER_SIZE					267

#define TK_TEA_MAXKEY					0x10

//toolkit kron gui states
#define TK_STATE_LIST_APP				0
#define TK_STATE_LIST_MENU			1
#define TK_STATE_LIST_EVENT			2
#define TK_STATE_LIST_DELETE			3
#define TK_STATE_EXECUTE				4
#define TK_STATE_IDLE						7
//CAT API states
#define TK_STATE_DISPLAY_TEXT			8
#define TK_STATE_SELECT_ITEM				9
#define TK_STATE_GET_INPUT				10
#define TK_STATE_GET_DATETIME			11
#define TK_STATE_SETUP_MENU				13
#define TK_STATE_LOCAL_INFO				14
#define TK_STATE_ENVELOPE_MENU		0x41
#define TK_STATE_SEND_DEVICE				0x50
#define TK_STATE_SEND_REQ					0x51
#define TK_STATE_SEND_PRINTER			0x52
#define TK_STATE_INVOKE_KERNEL			0x6F
#define TK_STATE_LOAD_RESOURCE		0x70
#define TK_STATE_STORE_RESOURCE		0x71
#define TK_STATE_COMPARE_RESOURCE	0x72
//internal events states
#define TK_STATE_USB_COMMAND		0xB1
#define TK_STATE_USB_SELECT			0xB2
#define TK_STATE_DELETE_APP			0xE4
#define TK_STATE_USB_DELETE_APP	0xE5
#define TK_STATE_CONFIG_ERROR				0xEE
#define TK_STATE_RIVER							0xFE
#define TK_STATE_RIVER_DOWNLOAD			0xFD
#define TK_STATE_RIVER_SEARCH				0xFC
#define TK_STATE_RIVER_SEARCH_QUERY		0xFB
#define TK_STATE_RIVER_CAT_CHANGED		0xFA
#define TK_STATE_RIVER_ERROR					0xF9
#define TK_STATE_RIVER_USB_DOWNLOAD	0xF8

//#define TK_STATE_DEVLET_LIST					0xDE
//#define TK_STATE_DEVLET_DOWNLOAD		0xDD

#define TK_STATE_SELECT_MENU		0x21
#define TK_STATE_TRIGGER_EVENT		0x22
#define TK_STATE_TRIGGER_TIMER		0x23
#define TK_STATE_CARD_DISCONNECTED		0xFF

#define TK_AUTOPLAY_ENABLED			0x80
#define TK_AUTOPLAY_RECORD			0x08
#define TK_AUTOPLAY_RUN				0x01

#define TK_FLAG_SECURE					0x80		//tk secure channel
#define TK_FLAG_CARD_DORMANT		0x40		//current card lifecycle
#define TK_FLAG_CARD_SYNCHED		0x20		//request for activation already sent
#define TK_FLAG_SYNCHRONIZED		0x02		//time synchronization

#define TK_COS_CONFIG_SIZE				0x08
//COS status
#define TK_COS_STAT_VM_STARTED	0x80		//Orb-Weaver started
#define TK_COS_STAT_PICC_AVAIL		0x02
#define TK_COS_STAT_UPDATE_AVAIL	0x40

#define TK_UCMD_NONE					0x00
#define TK_UCMD_STATUS					0xC0
#define TK_UCMD_INFO						0xC1
#define TK_UCMD_LIST_APP				0xCA
#define TK_UCMD_SELECT_APP			0xC3
#define TK_UCMD_SELECT_MENU			0xC4
#define TK_UCMD_DOWNLOAD_APP		0xCC
#define TK_UCMD_DELETE_APP			0xCD
//ucmd communication tag
#define TK_UCMD_RESULT					0x03
#define TK_UCMD_ALPHA					0x05
#define TK_UCMD_TEXT						0x0D
#define TK_UCMD_ITEM						0x0F

#define OWB_TAG_SEQ						16	//SEQUENCE, SEQUENCE OF
#define OWB_TAG_SET						17	//SET, SET OF
#define OWB_TAG_OBJDESC				7	//ObjectDescriptor

#define TK_MAX_AIDLEN					16

#define TK_ERR_NONE					0
#define TK_ERR_NETWORK				1
#define TK_ERR_AUTH					2
#define TK_ERR_INSTALL_ORC		3
#define TK_ERR_INSTALL_APP			4
#define TK_ERR_LOAD_ORC			5
#define TK_ERR_ORC_DATA			6
#define TK_ERR_ORC_EXCEEDED		7
#define TK_ERR_HASH_INVALID		8

#define TK_APP_STATE_RUNNING	0x80
#define TK_APP_STATE_DORMANT	0x00

#define TK_INTERFACE_LCD		0x00000001
#define TK_INTERFACE_AUD		0x00000002
#define TK_INTERFACE_TCC		0x00000010
#define TK_INTERFACE_PAD		0x00000020
#define TK_INTERFACE_NET		0x00000100
#define TK_INTERFACE_ICC		0x00001000
#define TK_INTERFACE_NFC		0x00002000
#define TK_INTERFACE_GPS		0x00004000
#define TK_INTERFACE_BLE		0x00008000
#define TK_INTERFACE_HDC		0x00400000
#define TK_INTERFACE_SDC		0x00800000
#define TK_INTERFACE_USB		0x01000000

typedef struct tk_context * tk_context_p;
typedef struct tk_config * tk_config_p;
typedef struct tk_app_record * tk_app_record_p;
typedef struct tk_resource_record * tk_resource_record_p;

#define TK_CONFIG_STATE_NOT_LOGGED					0x80
#define TK_CONFIG_STATE_PASSWORD_OFF				0x40
#define TK_CONFIG_STATE_AUTOPLAY_NOT_ENABLED		0x01
typedef struct tk_config {
	ui_config base;
	uint8 ecv;												//etheron kron-OS version
	uint8 state;												//system state								-> state (password, account, autoplay)
	uint8 uid[SHARD_UID_SIZE];					//32 digit uid (16 bytes in hexstring)			-> user identifier			33 bytes
	uint8 token[SHARD_TOKEN_SIZE];			//32 digit token									-> current user token	33 bytes
	uint8 username[SHARD_UNAME_SIZE];		//should be encrypted (cloud integrated)	-> username 65 bytes
	uint8 password[SHARD_PASS_SIZE];			//should be encrypted (cloud integrated)	-> password 65 bytes
	//devlet configuration
	uint8 devlet_name[TK_MAX_AIDLEN];					//current device entity (devlet)				-> current devlet identifier
	uint8 devlet_id[16];										//devlet uid											//16 bytes
	uint32 devlet_build;										//build number
	uint16 devlet_size;											//size
	uint8 devlet_hash[SHARD_HASH_SIZE];					//hash for current devlet (default sha256)		//32 bytes
	//device configuration
	uint8 devpass[SHARD_PASS_SIZE];			//device password								-> current device password 65 bytes
	uint8 orientation;									//screen orientation
	uint8 brightness;
	uint8 dev_rsv[8];									//reserved for device configuration
	//net configuration
	uint8 net_state;									//current network state config
	uint8 net_static_ip[4];								//net static IP (local IP)
	uint8 net_static_mac[6];								//net static MAC (device MAC)
	uint8 net_static_dns_1[4];							//net static dns 1
	uint8 net_static_dns_2[4];							//net static dns 2
	uint8 net_nodename[SHARD_MAX_NODE_NAME];
	uint8 net_resv[16];
} tk_config;

typedef struct tk_context {
	void * display;
	audio_handle_p audio;
	scard_context_p cctx;
#if (SHARD_NFC_TYPE == 0x522)
	rf_context_p rctx;
#endif
#if (SHARD_NFC_TYPE == 0x532)
	nfc_context_p rctx;
#endif
	net_context_p netctx;
	usb_context_p uctx;
	gps_context_p gpsctx;
	tk_config_p config;
	owl_context_p octx;
	bt_context_p bctx;
	dev_node_p devices;
	uint8 batt_percent;
	//card information
	uint8 cos_owver;				//framework version
	uint8 cos_config[TK_COS_CONFIG_SIZE];
	uint32 cos_freespace;		//freespace
	uint32 cos_totalspace;		//totalspace
	uint32 cos_grid;				//group identifier
	uint8 cos_status;				//os status
	//app information
	uint8 app_state;
	uint8 app_name[TK_MAX_AIDLEN];
	uint16 app_bnum;				//build number
	//autoplay context
	uint8 runstate;
	uint16 offset;
	//current card default card manager
	uint8 cmlen;
	uint8 cmaid[TK_MAX_AIDLEN];
	//aid buffer, should clear buffer after operation (for delete operation)
	uint8 aidlen;
	uint8 aidbuf[TK_MAX_AIDLEN];
	//terminal-card authentication context (pandora)
	uint8 flag;			
	uint8 pkey[16];			
	uint8 pseed;
	//terminal-server authentication context (TEA)
	uint8 eak[16];								//key A		(terminal->server)
	uint8 ebk[16];								//key B		(server->terminal)
	uint8 eck[16];								//key C		(CMAC-RMAC)
	//usb command structure
	uint8 ucmd;
	uint8 ulen;
	uint8 ubuf[256];
	//microprocessor/board configuration
	uint16 dev_id;
	uint32 interfaces;							//supported interfaces
	uint8 esid[SHARD_ESID_SIZE];					//etheron shard identifier
} tk_context;


#define TK_MAX_ICON_SIZE					1280
#define TK_APPREC_TAG						0xA6
typedef struct tk_app_record {
	uint8 tag;										//tag
	uint8 oid[8];										//oid
	uint8 app_name[TK_MAX_AIDLEN];			//app name
	uint8 hash[SHARD_HASH_SIZE];			//hash
	uint8 desc[SHARD_MAX_DESC + 1];
	uint16 bnum;
	uint16 size;
	uint16 counter;								//install counter
	uint8 icon[TK_MAX_ICON_SIZE];								//16x16 icon
} tk_app_record;

typedef struct tk_resource_record {
	uint16 res_size;
	uint8 * res_ptr;
} tk_resource_record;

extern uint8_c gba_default_icon[597];
extern uint8_c gba_river_icon[597];
extern uint8_c gba_stop_icon[];
extern uint8 gba_apdu_buffer[TK_BUFFER_SIZE];
extern const unsigned char orbleaf_cert_der_1024[0x3c9];
//default server/key certificates for SSL operation
extern const unsigned char server_cacert_der_1024[1198];
extern const unsigned char server_cert_der_1024 [1186] ;
extern const unsigned char server_keys_der_1024 [1193];
//png resources
extern uint8_c image_png_exe24[1568] ;
extern uint8_c image_png_elf24[1908] ;
extern uint8_c image_png_river24[844];
extern uint8_c image_png_bluetooth24[1339];

void tk_memcpy(BYTE * dst, BYTE * src, WORD size) _REENTRANT_ ;
uint8 tk_push(BYTE * buffer, BYTE tag, BYTE length, BYTE * value) _REENTRANT_ ;
uint8 tk_pop(BYTE * buffer, BYTE * tag, WORD * size, BYTE * value) _REENTRANT_ ;
uint16 tk_pop_by_tag(BYTE * buffer_in, BYTE size, BYTE tag, BYTE * buffer_out) _REENTRANT_ ;

uint16 tk_build_apdu(BYTE * buffer, BYTE cla, BYTE ins, BYTE p1, BYTE p2, BYTE lc);
uint16 tk_build_apdu_data(BYTE * buffer, BYTE cla, BYTE ins, BYTE p1, BYTE p2, BYTE lc, BYTE * dfield);
uint16 tk_get_status(BYTE * data, WORD len) ;
uint16 tk_card_send(tk_context_p ctx, uint8 * c_apdu, uint16 clen, uint8 * r_apdu, uint16 *rlen);
uint16 tk_unwrap_response_buffer(tk_context_p ctx, uint8 * buffer, uint16 length) _REENTRANT_ ;
uint16 tk_wrap_data_buffer(tk_context_p ctx, uint8 * buffer, uint16 length) _REENTRANT_ ;
uint8 tk_select_application(tk_context_p tctx, BYTE len, BYTE * aid) ;
void tk_aid2text(BYTE * textbuffer, BYTE maxlen, BYTE * aid, BYTE aidlen);
void tk_load_cos_config(tk_context_p ctx, uint8 * config, uint16 len);
void tk_load_cardconfig(tk_context_p ctx, uint8 * config, uint16 len);
uint8 tk_list_app_for_delete(tk_context_p ctx);


void tk_setting_init(gui_handle_p handle, void * params);
ui_object * tk_set_action(tk_context_p ctx, uint8 * text, uint8 * img, uint32 imgsize, void (* action)(ui_object * obj, void * params)) ;

//default program loop
void tk_main_loop(tk_context_p ctx) ;
void tk_river_loop(tk_context_p ctx);
void tk_river_usb_download(tk_context_p ctx, uint8 * aid, uint8 len);
void tk_app_delete(tk_context_p ctx, uint8 * aid, uint8 aidlen);

//history
uint8 tk_history_count(void * handle);
uint8 tk_history_read(void * handle, uint8 index, tk_app_record * record);
void tk_history_push(void * handle, uint8 * oid, uint8 * app_name, uint8 * desc, uint16 bnum, uint16 size, uint8 * hash);

//resources management API
void tk_clear_resources(tk_context_p ctx);
uint8 tk_add_resources(tk_context_p ctx, uint16 len, uint8 * buffer) ;
tk_resource_record_p tk_get_resource(tk_context_p ctx, uint8 id);

//use command handler
uint16 tk_usb_handler(void * ctx, uint8 * c_apdu, uint16 len, uint8 * r_apdu);
void tk_usb_exec(tk_context_p ctx);
void tk_usb_callback(tk_context_p ctx, uint8 * response, uint8 len);

BYTE tkUtf8Check(BYTE * buffer, BYTE size) _REENTRANT_ ;
BYTE tkUtf82Ucs2(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ ;
BYTE tkUtf82Gsm(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ ;
BYTE tkUcs2Utf8(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ ;
BYTE tkGsm2Utf8(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ ;
BYTE tkUcs2Gsm(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ ;
BYTE tkGsm2Ucs(BYTE * buffer, BYTE size, BYTE max_len) _REENTRANT_ ;

uint16 tk_base64_decode(uint8 * buffer, uint16 size);
uint16 tk_base64_encode(uint8 * bytes_to_encode, uint16 in_len, uint8 * outbuf);
uint16 tk_hex2bin(uint8 * hexstring, uint8 * bytes);
uint16 tk_bin2hex(uint8 * bytes, uint16 len, uint8 * hexstring);

//autoplay API
void tk_push_action(tk_context_p tctx, uint8 tag, uint8 * buffer, uint8 length);
uint8 tk_pop_action(tk_context_p tctx, uint8 * tag, uint8 * buffer);

void tk_get_config(tk_config * config) ;
void tk_set_config(tk_config * config) ;
uint8 tk_gen_hash(uint8 * buffer, uint16 len, uint8 * signature);

void tk_init_gui(tk_context_p handle);
void tk_clear_body(tk_context_p ctx);
void tk_console_print(void * ctx, char * text);
void tk_show_menu(tk_context_p ctx);
ui_object * tk_present(tk_context_p ctx);
ui_object * tk_wait_user_input(tk_context_p ctx);
void tk_select_item(tk_context_p display, BYTE len, BYTE * tags);
void tk_get_input(tk_context_p display, uint8 len, uint8 qualifier, BYTE * tags);
void tk_display_text(tk_context_p display, uint8 mode, BYTE len, BYTE * tags) ;
uint16 tk_load_card_certificate(tk_context_p ctx, uint8 * certbuf);
uint8 tk_load_resource(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags);
uint8 tk_store_resource(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags);
void tk_clear_resources(tk_context_p ctx) ;
uint8 tk_local_info(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags);
uint8 tk_dispatch(tk_context_p ctx, uint16 len, uint8 * buffer);
uint16 tk_response(BYTE status, BYTE len, BYTE * tlv);
uint8 tk_check_update(tk_context_p ctx);
uint8 tk_sync_time(tk_context_p ctx);
void tk_load_configuration(tk_context_p ctx);
void tk_save_configuration(tk_context_p ctx);

//network APIs
uint16 tk_decode_params(uint8 * owb, uint8 * outbuf);

//toolkit function
void tk_kernel_init(tk_context_p ctx);
void tk_kernel_exec(tk_context_p ctx);				//exec program.main
void tk_kernel_kill(tk_context_p ctx);
uint8 tk_kernel_framework_load(tk_context_p ctx, void * handle) ;		//load current framework vf_handle
uint8 tk_kernel_data_load(tk_context_p ctx, void * handle) ;				//load current data vf_handle
void tk_kernel_devlet_list(tk_context_p ctx) ;
uint8 tk_kernel_list_menu(tk_context_p ctx);
uint8 tk_kernel_devlet_sync(tk_context_p ctx);
uint8 tk_kernel_install(tk_context_p ctx, uint8 * name, uint8 * oid, uint8 * sign, uint16 size, uint16 bnum, uint16 mode);
uint16 tk_kernel_invoke(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags);
uint8 tk_kernel_trigger_event(tk_context_p ctx, uint8 event, uint8 argLen, uint8 * arg);
uint8 tk_kernel_execute(tk_context_p ctx);

//datetime APIs
uint32 tk_iso8601_to_filetime(struct datetime * dtime);
uint8 tk_iso8601_from_filetime(struct datetime * dtime, uint32 filetime);
uint32 tk_iso8601_decode(char * str);
void tk_iso8601_encode(struct datetime * dtime, uint8 * buffer) ;



#endif
