#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include "..\..\build.h"
#include "..\..\config.h"
#include "..\..\gui\inc\ui_resources.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jansson.h"
#if SHARD_SSL_SUPPORT
#include "wolfssl\version.h"
#endif
#include "libpng\png.h"
#include "zlib\zlib.h"
#include "jconfig.h"

extern uint8 gb_tk_state;
#define gba_apdu_data_field (gba_apdu_buffer + 5)  
extern uint8 gba_net_buffer[NET_BUFFER_SIZE];
extern uint16 gba_orc_size;
extern uint8 gba_grid[20];

//declared on tk_setting.c
extern uint8_c gba_river_icon[597];
extern uint8_c gba_default_icon[597];
extern uint8_c gba_stop_icon[597];

/////auto play mechanism/////
static void tk_record_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_record_click);
	tk_context_p tctx = NULL;
	ui_toggle * playBtn;
	//check smart card context
	tctx = obj->target;
	if(tctx == NULL) goto exit_record_click;
	//check toggle state
	if(((ui_toggle *)obj)->state & UI_TOGGLE_DISABLED) goto exit_record_click;
	//disable play btn
	playBtn = (ui_toggle *)ui_get_object_by_name((gui_handle_p)params, "autoPlayBtn") ;
	if(((ui_toggle *)obj)->state & UI_TOGGLE_ON) {
		if(playBtn != NULL) playBtn->state = UI_TOGGLE_OFF;
		((ui_toggle *)obj)->state = UI_TOGGLE_OFF;
		tctx->runstate = 0;
	} else {
		if(playBtn != NULL) playBtn->state |=UI_TOGGLE_DISABLED;
		((ui_toggle *)obj)->state |= UI_TOGGLE_ON;
		tctx->runstate = TK_AUTOPLAY_RECORD;
		tctx->offset = SHARD_AUTOREC_START;
		if(if_card_state(tctx->cctx) == 0) gb_tk_state = TK_STATE_LIST_APP;
	}
	ui_reinitialize_object(obj);
	exit_record_click:
	OS_DEBUG_EXIT();
	return;
}

void tk_recstop_btn_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_recstop_btn_click);
	ui_toggle * btn;
	tk_context_p tctx;
	((ui_icon_tool *)obj)->bitmap[0].bitmap = NULL;
	((ui_icon_tool *)obj)->bitmap[0].bmpsize = 0;
	((ui_icon_tool *)obj)->bitmap[1].bitmap = NULL;
	((ui_icon_tool *)obj)->bitmap[1].bmpsize = 0;
	ui_set_text((ui_text *)obj, (uint8 *)"");
	((ui_object *)obj)->handler = NULL;
	tctx = ((ui_object *)obj)->target;
	if(tctx != NULL) tctx->runstate = 0;
	ui_reinitialize_object((ui_object *)obj);
	//autoplay button if exist
	btn = (ui_toggle *)ui_get_object_by_name(params, "autoPlayBtn");
	if(btn != NULL) {
		//btn->state = 0;
		ui_reinitialize_object((ui_object *)btn);
	}
	OS_DEBUG_EXIT();
	return;
}

static void tk_autobtn_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_autobtn_click);
	tk_context_p tctx = NULL;
	ui_object * actbtn;
	ui_toggle * abtn = (ui_toggle *)obj;
	//check toolkit context
	tctx = obj->target;
	if(tctx == NULL) goto exit_autobtn_click;
	if(abtn == NULL) goto exit_autobtn_click;
	//switch state
	abtn->state = ++abtn->state % 3;
	tk_set_action(tctx, (uint8 *)"", NULL, 0, NULL);			//clear action button if exist
	switch(abtn->state & 0x03) {
		case 0:
			tctx->runstate = 0;
			break;
		case 1:
			tctx->runstate = TK_AUTOPLAY_ENABLED;
			tctx->offset = SHARD_AUTOREC_START;
			break;
		case 2:
			tctx->runstate = TK_AUTOPLAY_RECORD;
			tctx->offset = SHARD_AUTOREC_START;
			//actbtn = tk_set_action(tctx, (uint8 *)"Stop", (uint8 *)gba_stop_icon, sizeof(gba_stop_icon), tk_recstop_btn_click);			//set action button to stop recording
			actbtn = tk_set_action(tctx, (uint8 *)"Stop", (uint8 *)image_png_stop, sizeof(image_png_stop), tk_recstop_btn_click);			//set action button to stop recording
			if(actbtn != NULL) actbtn->target = tctx;
			break;
	}
	ui_reinitialize_object(obj);
	tk_save_configuration(tctx);
	exit_autobtn_click:
	OS_DEBUG_EXIT();
	return;
}
/////auto play mechanism/////


//exit configuration menu
static void tk_clear_subconfig_menu(tk_context_p ctx) {
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "loginForm"));
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "aboutForm"));
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "passForm"));
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "devletForm"));
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "secForm"));
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "netConfForm"));
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "staipForm"));
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "devNameForm"));
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "btListForm"));
}

static void tk_config_exit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_config_exit_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	uint32 longtime = 0;
	ui_object * screen;
	ui_object * cfBtn;
	tk_clear_subconfig_menu(ctx);
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "settings"));
	//disable marker
	cfBtn = ui_get_object_by_name(ctx->display, "mySetting") ;
	if(cfBtn != NULL) {
		((ui_icon_tool *)cfBtn)->show_marker = FALSE;
	}
	//ui_clear_dispstack(ctx->display);
	//tk_clear_body(ctx);
	OS_DEBUG_EXIT();
}

static void tk_config_about_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_config_about_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	uint8 tbuf[400];
	uint8 lbuf[64];
	uint8 esid_buffer[40];
	ui_object * instance;
	uint8 numline = 6;
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "aboutForm");
	tk_bin2hex(ctx->esid, SHARD_ESID_SIZE, esid_buffer);
	sprintf((char *)tbuf, "Kron version %d.%d.%d\nOrbleaf Technology 2016\nOrb-Weaver %0d.%0d\nESID (%s)\n\n3rd party libraries :", MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER, SHARD_OWL_SUPPORTED >> 4, SHARD_OWL_SUPPORTED & 0x0F, esid_buffer);
	
#if SHARD_SSL_SUPPORT
	sprintf((char *)lbuf, "\nWolfSSL %s", LIBWOLFSSL_VERSION_STRING);
	strcat((char *)tbuf, (const char *)lbuf);
	numline++;
#endif
	sprintf((char *)lbuf, "\nlibpng %s", PNG_LIBPNG_VER_STRING);
	strcat((char *)tbuf, (const char *)lbuf);
	numline++;
	sprintf((char *)lbuf, "\nlibjpg-turbo %d.%d", JPEG_LIB_VERSION>>4, JPEG_LIB_VERSION & 0x0F);
	strcat((char *)tbuf, (const char *)lbuf);
	numline++;
	sprintf((char *)lbuf, "\nzlib %s", ZLIB_VERSION);
	strcat((char *)tbuf, (const char *)lbuf);
	numline++;
	ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, numline, UI_FONT_DEFAULT, tbuf)));
	ui_add_body(ctx->display, (instance  = ui_button_create(UI_COLOR_WHITE, (uint8 *)"OK", 1, tk_config_exit_click)));
	if(instance != NULL) instance->target = ctx;
	OS_DEBUG_EXIT();
}

static void tk_config_orientation_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_config_about_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	uint8 orientation = ((gui_handle_p)ctx->display)->orientation;
	orientation++;
	ctx->config->orientation = (orientation & 0x03);
	//((gui_handle_p)ctx->display)->orientation = (orientation & 0x03);
	//rotate orientation
	//ui_switch_orientation(ctx->display, orientation);
	//reinitialize all objects
	//ui_reinitialize_object(((gui_handle_p)ctx->display)->header);
	//ui_reinitialize_object(((gui_handle_p)ctx->display)->body);
	//ui_reinitialize_object(((gui_handle_p)ctx->display)->meta);
	//save curent configuration
	tk_save_configuration(ctx);
	OS_DEBUG_EXIT();
}

static void tk_delete_app_click(ui_object * obj, void * params) {
	//gb_tk_state = TK_STATE_LIST_DELETE;
	tk_context_p ctx = obj->target;
	if(ctx == NULL) return;
	if(if_card_state(ctx->cctx) != 0)  { gb_tk_state = TK_STATE_CARD_DISCONNECTED; return; }
	if(tk_list_app_for_delete(ctx) != 0) { gb_tk_state = TK_STATE_CARD_DISCONNECTED; return; }
}

void tk_load_configuration(tk_context_p ctx) {
	if(ctx == NULL) return;
	if_flash_data_read(NULL, 0, (uint8 *)ctx->config, sizeof(tk_config));		//read current configuration
	if_flash_get_esid(ctx->esid, SHARD_ESID_SIZE);
	ctx->dev_id = ((ctx->esid[2] << 8) | ctx->esid[3]) & 0xFFF;
	if(ctx->config->state & TK_CONFIG_STATE_AUTOPLAY_NOT_ENABLED) ctx->runstate &= ~TK_AUTOPLAY_ENABLED;
	else ctx->runstate |= TK_AUTOPLAY_ENABLED;
	//set screen orientation
	if(ctx->config->orientation != 0xFF) { 
		((gui_handle_p)ctx->display)->orientation = ctx->config->orientation;
	} else 
		ctx->config->orientation = 0;
	//set screen brightness
	if(ctx->config->brightness != 0xFF) {
		((gui_handle_p)ctx->display)->brightness = ctx->config->brightness;
	} else 
		ctx->config->brightness = 50; 
	switch(ctx->dev_id) {
		case STM32F746_SERIES:
			ctx->interfaces = TK_INTERFACE_LCD | TK_INTERFACE_TCC | TK_INTERFACE_AUD | TK_INTERFACE_SDC | TK_INTERFACE_HDC | TK_INTERFACE_USB;
			break;
		case STM32F765_SERIES:
		case STM32F407_SERIES:
			ctx->interfaces = TK_INTERFACE_LCD | TK_INTERFACE_TCC | TK_INTERFACE_NET | TK_INTERFACE_ICC |
								TK_INTERFACE_NFC | TK_INTERFACE_GPS | TK_INTERFACE_BLE | TK_INTERFACE_USB;
			break;
		case STM32F413_SERIES:
			ctx->interfaces = TK_INTERFACE_LCD | TK_INTERFACE_PAD | TK_INTERFACE_NET | TK_INTERFACE_NFC |
								 TK_INTERFACE_GPS | TK_INTERFACE_USB;
			break;
	}
		
}

void tk_save_configuration(tk_context_p ctx) {
	if(ctx == NULL) return;
	if(ctx->runstate & TK_AUTOPLAY_ENABLED) ctx->config->state &= ~TK_CONFIG_STATE_AUTOPLAY_NOT_ENABLED;
	else ctx->config->state |= TK_CONFIG_STATE_AUTOPLAY_NOT_ENABLED;
	//set screen orientation and brightness
	//ctx->config->orientation = ((gui_handle_p)ctx->display)->orientation;
	ctx->config->brightness = ((gui_handle_p)ctx->display)->brightness;
	if_flash_data_write(NULL, 0, (uint8 *)ctx->config, sizeof(tk_config));		//read current configuration
}

uint8 tk_sync_time(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_config_about_click);
	datetime dtime;
	uint8 ret = -1;
	int8 tz;
	if(if_time_is_running() && (ctx->flag & TK_FLAG_SYNCHRONIZED)) goto exit_sync_time;			//check if RTC is already running
	if_time_get(&dtime);
	tz = dtime.tz;
	
	if_net_wake(ctx->netctx);
	if(ntp_get_time(ctx->netctx, (uint8 *)"time-a.nist.gov", &dtime) == 0) {
		dtime.tz = tz;
		if_time_set(&dtime);
		ctx->flag |= TK_FLAG_SYNCHRONIZED;
		tk_console_print(ctx, "time synchronized");
		ret = 0;
	}
	
	if_net_sleep(ctx->netctx);
	exit_sync_time:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 tk_check_update(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_check_update);
	uint8 * url;
	int avail;
	uint8 * sgn;
	json_t * json;
    json_error_t error;
	int rv;
	uint8 ret = -1;
	uint8 * stime;
	uint16 len;
	uint32 dlen;
	uint32 filetime;
	uint32 temp_addr;
	ssl_cert_p cert = NULL;
	net_request req;
	int8 tz;
	datetime dtime;
	uint8 sha_chksum[IF_KRONOS_SIGNATURE_LEN];
	bfd_info * b_info = (bfd_info *)0x08080000;
	uint8 hx_devsign[IF_KRONOS_SIGNATURE_LEN * 2 + 1];
	char devbuf[32];
#if SHARD_SSL_SUPPORT
	ssl_handle_p ssdl;
#endif
	uint8 url_buffer[256];
	static int32 chkupd_cntr = -1;
	//TK_COS_STAT_UPDATE_AVAIL
	if(chkupd_cntr > 0) {
		chkupd_cntr--;
		goto exit_check_update;
	}
	sprintf(devbuf, "%s%04X", HARDWARE_ID, SHARD_WIFI_MODULE);
	tk_bin2hex(b_info->signature, IF_KRONOS_SIGNATURE_LEN, hx_devsign);
	if_net_wake(ctx->netctx);
#if 0
	len = http_send(ctx->netctx, IF_HTTP_TYPE_GET, (uint8 *)"http://orbleaf.com/updates/kron/v10/alive.php", 80, NULL, 0, gba_net_buffer);
	if(len == 0) goto exit_check_update;
	sprintf((char *)url_buffer, "https://orbleaf.com/updates/kron/v10/check.php?v=%02X&b=%d&dev=%s&sh=%s", MAJOR_VERSION<<4 | MINOR_VERSION, BUILD_NUMBER, devbuf, hx_devsign);
	if(if_net_ssl_init(ctx->netctx, (uint8 *)orbleaf_cert_der_1024, sizeof(orbleaf_cert_der_1024)) != 0) goto exit_check_update;
	len = https_send(ctx->netctx, IF_HTTP_TYPE_GET, url_buffer, 443, NULL, 0, gba_net_buffer);
	if_net_ssl_release(ctx->netctx);
#else
	sprintf((char *)url_buffer, "http://orbleaf.com/updates/kron/v10/check.php?v=%02X&b=%d&dev=%s&sh=%s", MAJOR_VERSION<<4 | MINOR_VERSION, BUILD_NUMBER, devbuf, hx_devsign);
	net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert);
	len = http_send(ctx->netctx, &req, NULL, NULL, 0, gba_net_buffer);
#endif
	//network sleep
	if_net_sleep(ctx->netctx);
	gba_net_buffer[len] = 0;
	if(len != 0) {
		json = json_loads((const char *)gba_net_buffer, JSON_DECODE_ANY, &error);
		rv = json_unpack(json, "{s:b, s:s}", "avail", &avail, "stime", &stime);
		if(!if_time_is_running() || ((ctx->flag & TK_FLAG_SYNCHRONIZED) == 0)) {		//sync to server (in-case NTP failed)
			if(stime != NULL) {
				if_time_get(&dtime);
				tz = dtime.tz;
				filetime = tk_iso8601_decode((char *)stime);
				tk_iso8601_from_filetime(&dtime, filetime);
				dtime.tz = tz;
				if_time_set(&dtime);
				ctx->flag |= TK_FLAG_SYNCHRONIZED;
			}
		}
		json_object_clear(json);
		if(avail) {
			ctx->cos_status |= TK_COS_STAT_UPDATE_AVAIL;
			tk_console_print(ctx, "update available");
		}
		chkupd_cntr = 300;
		ret = 0;
	}
	exit_check_update:
	OS_DEBUG_EXIT();
	return ret;
}

//update checking
static void tk_config_update_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_config_update_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	uint16 len;
	json_t *json;
	uint32 dlen;
	uint32 temp_addr;
    json_error_t error;
	uint8 * url;
	int avail;
	ssl_cert_p cert = NULL;
	net_request req;
	uint8 * sgn;
	bfd_info * b_info = (bfd_info *)0x08080000;
	uint8 hx_devsign[IF_KRONOS_SIGNATURE_LEN * 2 + 1];
	uint8 sha_chksum[IF_KRONOS_SIGNATURE_LEN];
	int rv;
#if SHARD_SSL_SUPPORT
	ssl_handle_p ssdl;
#endif
	uint8 url_buffer[256];
	ui_alert * alert = NULL;
	tk_bin2hex(b_info->signature, IF_KRONOS_SIGNATURE_LEN, hx_devsign);		//convert to hex
	if_net_wake(ctx->netctx);
	alert = ui_alert_show(ctx->display, (uint8 *)"Checking", (uint8 *)"Please Wait...", UI_ALERT_INFO, UI_ALERT_ICON_SECURE);
#if SHARD_SSL_SUPPORT
	net_request_struct(&req, IF_HTTP_TYPE_GET, (uint8 *)"http://orbleaf.com/updates/kron/v10/alive.php", 80, cert);
	len = http_send(ctx->netctx, &req, NULL, NULL, 0, gba_net_buffer);
	if(len == 0) {
		if(alert != NULL) ui_alert_close(ctx->display, alert);
		alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Unable to Connect", UI_ALERT_INFO | UI_ALERT_BUTTON_OK, UI_ALERT_ICON_NETWORK);
		goto exit_conf_update_click;
	}
	sprintf((char *)url_buffer, "https://orbleaf.com/updates/kron/v10/check.php?v=%02X&b=%d&dev=%s&sh=%s", MAJOR_VERSION<<4 | MINOR_VERSION, BUILD_NUMBER, HARDWARE_ID, hx_devsign);
	//if(if_net_ssl_init(ctx->netctx, (uint8 *)orbleaf_cert_der_1024, sizeof(orbleaf_cert_der_1024)) != 0) goto exit_conf_update_click;
	if((cert = if_ssl_create_cert((uint8 *)orbleaf_cert_der_1024, sizeof(orbleaf_cert_der_1024))) == NULL) goto exit_conf_update_click;
	net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 443, cert);
	len = https_send(ctx->netctx, &req, NULL, NULL, 0, gba_net_buffer);
	//if_net_ssl_release(ctx->netctx);
	if_ssl_release_cert(cert);
#else
	sprintf((char *)url_buffer, "http://orbleaf.com/updates/kron/v10/check.php?v=%02X&b=%d&dev=%s&sh=%s", MAJOR_VERSION<<4 | MINOR_VERSION, BUILD_NUMBER, HARDWARE_ID, hx_devsign);
	net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert);
	len = http_send(ctx->netctx, &req, NULL, NULL, 0, gba_net_buffer);
#endif
	gba_net_buffer[len] = 0;
    json = json_loads((const char *)gba_net_buffer, JSON_DECODE_ANY, &error);
	rv = json_unpack(json, "{s:b, s:s, s:s}", "avail", &avail, "url", &url, "sgn", &sgn);
	if(alert != NULL) ui_alert_close(ctx->display, alert);
	if(avail == TRUE) {
		alert = ui_alert_show(ctx->display, (uint8 *)"Downloading", (uint8 *)"Screen may Hanged\nPlease Wait...", UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
		//update available
		tk_hex2bin(sgn, sha_chksum);
		dlen = if_download_update(ctx->netctx, url, &temp_addr, sha_chksum);
		if(alert != NULL) ui_alert_close(ctx->display, alert);
		if(dlen != 0) {
			//firmware ready (should display alert box, requesting restart)
			alert = ui_alert_show(ctx->display, (uint8 *)"Installing Firmware", (uint8 *)"Do Not Turn Off.", UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
			//if_system_reset();
			if_load_new_firmware();
		}
	} else {
		alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"No Update Available", UI_ALERT_INFO | UI_ALERT_BUTTON_OK, UI_ALERT_ICON_NETWORK);
	}
	json_object_clear(json);
	exit_conf_update_click:
	if_net_sleep(ctx->netctx);
	OS_DEBUG_EXIT();
	return;
}

static void tk_login_now_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_login_now_click);
	uint8 param_buffer[256];
	uint8 * token;
	uint8 * uid;
	uint8 * status;
	uint8 username[128];
	uint8 password[128];
	ssl_cert_p cert = NULL;
	net_request req;
	uint8 esid_string[(SHARD_ESID_SIZE * 2) + 1];
	tk_context_p ctx = (tk_context_p)obj->target;
	tk_config * conf = ctx->config;
	ui_alert * alert = NULL;
	json_t * json;
	int rv;
	json_error_t error;
	ui_textbox * uname_box;
	ui_textbox * upass_box;
	uint16 response_length;
	if(ctx == NULL) goto exit_login_action;
	uname_box = (ui_textbox *)ui_get_object_by_name(ctx->display, "my_username");
	if(uname_box == NULL) goto exit_login_action;
	upass_box = (ui_textbox *)ui_get_object_by_name(ctx->display, "my_password");
	if(upass_box == NULL) goto exit_login_action;
	net_get_uri_string(uname_box->content, username);			//convert to html escape string
	net_get_uri_string(upass_box->content, password);				//convert to html escape string
	//get ESID hexstring
	memset(esid_string, 0, sizeof(esid_string));
	tk_bin2hex(ctx->esid, SHARD_ESID_SIZE, esid_string);
	sprintf((char *)param_buffer, "uname=%s&upass=%s&esid=%s", username, password, esid_string);
	//wake wifi
	if_net_wake(ctx->netctx);
	alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Connecting...", UI_ALERT_INFO, UI_ALERT_ICON_SECURE);
	//sending reqeuest to river service (list application)
#if SHARD_SSL_SUPPORT
	//always use SSL operation
	//if(if_net_ssl_init(ctx->netctx, (uint8 *)orbleaf_cert_der_1024, sizeof(orbleaf_cert_der_1024)) != 0) goto exit_login_action;
	if((cert = if_ssl_create_cert((uint8 *)orbleaf_cert_der_1024, sizeof(orbleaf_cert_der_1024))) == NULL) goto exit_login_action;
	net_request_struct(&req, IF_HTTP_TYPE_POST, (uint8 *)"http://orbleaf.com/apis/river/sgin.php", 443, cert);
	response_length = https_send(ctx->netctx, &req, NULL, param_buffer, strlen((const char *)param_buffer), gba_net_buffer);
	//if_net_ssl_release(ctx->netctx);
	if_ssl_release_cert(cert);
#else			
	net_request_struct(&req, IF_HTTP_TYPE_POST, (uint8 *)"http://orbleaf.com/apis/river/sgin.php", 80, NULL);
	response_length = http_send(ctx->netctx, &req, NULL, param_buffer, strlen((const char *)param_buffer), gba_net_buffer);
#endif
	//network sleep
	if_net_sleep(ctx->netctx);
	gba_net_buffer[response_length] = 0;
    json = json_loads((const char *)gba_net_buffer, JSON_DECODE_ANY, &error);
	if(json != NULL) {
		rv = json_unpack(json, "{s:s, s:s, s:s}", "token", &token, "uid", &uid, "status", &status);
		//should process data here
		if(status != NULL && strcmp((char *)status, "success") == 0) {
			conf->state &= ~TK_CONFIG_STATE_NOT_LOGGED;		//set logged state
			strcpy((char *)conf->uid, (const char *)uid);
			strcpy((char *)conf->token, (const char *)token);
			strcpy((char *)conf->username, (const char *)uname_box->content);
			strcpy((char *)conf->password, (const char *)upass_box->content);
			//save configuration
			if_config_write(conf, sizeof(tk_config));
		}
		//release json object
		json_object_clear(json);
	}
	ui_alert_close(ctx->display, alert);
	//exit from login screen
	exit_login_action:
	tk_config_exit_click(obj, params);
	OS_DEBUG_EXIT();
}

static void tk_config_login_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_config_login_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_textbox * textbox;
	uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * instance;
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "loginForm");
	memset(tbuf, 0, sizeof(tbuf));
	ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, (uint8 *)"Email : ")));
	ydiv += ((ui_rect *)instance)->h;
	ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_ALPHANUM, (uchar *)tbuf, 64, 1)));	
	ui_set_object_name((ui_object *)textbox, "my_username");
	ydiv += ((ui_rect *)textbox)->h;
	ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, (uint8 *)"Password : ")));
	ydiv += ((ui_rect *)instance)->h;
	ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_PASSWORD, (uchar *)tbuf + 64, 64, 1)));
	ui_set_object_name((ui_object *)textbox, "my_password");		
	ydiv += ((ui_rect *)textbox)->h;
	//create buttonset
	ui_add_body(ctx->display, (ui_object *)(instance = ui_buttonset_create(UI_BUTTON_SET_OK | UI_BUTTON_SET_CANCEL, 1, tk_login_now_click, tk_config_exit_click))) ;
	if(instance != NULL) { 
		instance->target = ctx;
	}
	OS_DEBUG_EXIT();
}

static uint8 tk_config_delete_user_configuration(tk_context_p ctx) {
	tk_config * conf = ctx->config;
	if(ctx == NULL) return -1;
	conf = ctx->config;
	conf->state |= TK_CONFIG_STATE_NOT_LOGGED;		//clear logged state
	//clear configuration
	memset((char *)conf->uid, 0, SHARD_UID_SIZE);					//clear uid
	memset((char *)conf->token, 0, SHARD_TOKEN_SIZE);			//clear token
	memset((char *)conf->username, 0, SHARD_UNAME_SIZE);	//clear username
	memset((char *)conf->password, 0,  SHARD_PASS_SIZE);		//clear password
	memset((char *)conf->devlet_name, 0, TK_MAX_AIDLEN);				//clear devlet name
	memset((char *)conf->devlet_hash, 0, SHARD_HASH_SIZE);
	conf->devlet_size = 0;
	conf->devlet_build = 0;
	if_config_write(conf, sizeof(tk_config));
	//erase residual code application area (EAL FDP_RIP F.9)
	if_flash_code_erase();
	return 0;
}

static void tk_config_logoff_now_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_config_logoff_now_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_textbox * upass_box;
	uint8 password[128];
	uint8 passlen;
	tk_config * conf = ctx->config;
	upass_box = (ui_textbox *)ui_get_object_by_name(ctx->display, "my_password");
	if(upass_box == NULL) goto exit_logoff_action;
	passlen = net_get_uri_string(upass_box->content, password);				//convert to html escape string
	if(strncmp((const char *)password, (char *)conf->devpass, SHARD_PASS_SIZE) == 0) {
		//try delete user configuration
		if(tk_config_delete_user_configuration(ctx) == 0) {
			tk_kernel_init(ctx);			//re-init application framework
			tk_config_exit_click(obj, params);
		}
	} else {
		//invalid password
		ui_alert_show(ctx->display, (uint8 *)"Error", (uint8 *)"Invalid Password", UI_ALERT_INFO | UI_ALERT_BUTTON_OK, UI_ALERT_ICON_ERROR);
	}
	exit_logoff_action:
	OS_DEBUG_EXIT();
}

static void tk_config_logoff_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_config_logoff_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_textbox * textbox;
	uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * instance;
	ui_object * screen;
	ui_object * btn;
	ui_alert * alert = ui_alert_show(ctx->display, (uint8 *)"Confirmation", (uint8 *)"Do you want to unregister this device?", UI_ALERT_BUTTON_OK | UI_ALERT_BUTTON_CANCEL, UI_ALERT_ICON_INFO);
	//start ui rendering presentation (enter loop)	
	btn = ui_wait_user_input(ctx->display);
	ui_alert_close(ctx->display, alert);
	if(ui_alert_result(ctx->display) == UI_ALERT_RESULT_OK) {
		//check if device password has been set or password state disabled
		if(ctx->config->devpass[0] == 0xFF || (ctx->config->state & TK_CONFIG_STATE_PASSWORD_OFF) != 0) {			//if device password not set
			//device logoff without password confirmation
			if(tk_config_delete_user_configuration(ctx) == 0) {
				tk_kernel_init(ctx);
				tk_config_exit_click(obj, params);
			}
		} else {
			//device logoff require password confirmation
			//ui_clear_dispstack(ctx->display);
			ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "settings"));
			screen = ui_push_screen(ctx->display, NULL);
			ui_set_object_name(screen, "settings");
			ui_set_text(screen, "settings");
			memset(tbuf, 0, sizeof(tbuf));
			ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, (uint8 *)"Password : ")));
			ydiv += ((ui_rect *)instance)->h;
			ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_PASSWORD, (uchar *)tbuf + 64, 64, 1)));
			ui_set_object_name((ui_object *)textbox, "my_password");		
			ydiv += ((ui_rect *)textbox)->h;
			
			//create buttonset
			ui_add_body(ctx->display, (ui_object *)(instance = ui_buttonset_create(UI_BUTTON_SET_OK | UI_BUTTON_SET_CANCEL, 1, tk_config_logoff_now_click, tk_config_exit_click))) ;
			if(instance != NULL) { 
				instance->target = ctx;
			}
		}
	}
	OS_DEBUG_EXIT();
}

static void tk_config_devlet_click(ui_object * obj, void * params) {
	tk_kernel_devlet_list(obj->target);
}

///////////////////////////////////////////////SECURITY//////////////////////////////////////////

void tk_back_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_back_click);
	tk_context_p ctx = (tk_context_p)sender->target;
	ui_pop_screen(ctx->display);
	OS_DEBUG_EXIT();
}

void tk_chpass_submit_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_chpass_submit_click);
	tk_context_p ctx = (tk_context_p)sender->target;
	ui_textbox * newpass = (ui_textbox *)ui_get_object_by_name(ctx->display, "new_pass");
	ui_textbox * confpass = (ui_textbox *)ui_get_object_by_name(ctx->display, "conf_pass");
	ui_textbox * oldpass = (ui_textbox *)ui_get_object_by_name(ctx->display, "old_pass");
	
	if(ctx->config->devpass[0] != 0xFF && oldpass != NULL) {			//in-case new device
		if(strncmp((const char *)oldpass->content, (char *)ctx->config->devpass, SHARD_PASS_SIZE) != 0) {
			//invalid password, display alert
			ui_alert_show(ctx->display, (uint8 *)"Error", (uint8 *)"Invalid Password", UI_ALERT_INFO | UI_ALERT_BUTTON_OK, UI_ALERT_ICON_ERROR);
			goto exit_chpass_submit_click;
		}
	}
	//check for password confirmation
	if(strncmp((const char *)newpass->content, (char *)confpass->content, SHARD_PASS_SIZE) != 0) {
		//invalid password, display alert
		ui_alert_show(ctx->display, (uint8 *)"Error", (uint8 *)"Confirmation Failed", UI_ALERT_INFO | UI_ALERT_BUTTON_OK, UI_ALERT_ICON_ERROR);
		goto exit_chpass_submit_click;
	}
	//update password
	strncpy((char *)ctx->config->devpass, (char *)newpass->content, SHARD_PASS_SIZE);
	if_config_write(ctx->config, sizeof(tk_config));
	tk_back_click(sender, params);	
	exit_chpass_submit_click:
	OS_DEBUG_EXIT();
	return;
}

void tk_security_chpass_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_security_chpass_click);
	tk_context_p ctx = (tk_context_p)sender->target;
	uint8 tbuf[256];
	ui_textbox * textbox;
	uint16 ydiv = 0;
	ui_object * instance;
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "passForm");
	memset(tbuf, 0, sizeof(tbuf));
	if(ctx->config->devpass[0] != 0xFF) {			//in-case new device
		ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, (uint8 *)"Old Password : ")));
		ydiv += ((ui_rect *)instance)->h;
		ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_PIN, (uchar *)tbuf + 64, 32, 1)));
		ui_set_object_name((ui_object *)textbox, "old_pass");		
		ydiv += ((ui_rect *)textbox)->h;
	}
	ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, (uint8 *)"New Password : ")));
	ydiv += ((ui_rect *)instance)->h;
	ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_PIN, (uchar *)tbuf, 32, 1)));	
	ui_set_object_name((ui_object *)textbox, "new_pass");
	ydiv += ((ui_rect *)textbox)->h;
	ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, (uint8 *)"Confirm Password : ")));
	ydiv += ((ui_rect *)instance)->h;
	ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_PIN, (uchar *)tbuf + 64, 32, 1)));
	ui_set_object_name((ui_object *)textbox, "conf_pass");		
	ydiv += ((ui_rect *)textbox)->h;
	
	//create buttonset
	ui_add_body(ctx->display, (ui_object *)(instance = ui_buttonset_create(UI_BUTTON_SET_OK | UI_BUTTON_SET_CANCEL, 1, tk_chpass_submit_click, tk_back_click))) ;
	if(instance != NULL) { 
		instance->target = ctx;
	}
	OS_DEBUG_EXIT();
}

void tk_security_swpass_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_security_swpass_click);
	tk_context_p ctx = (tk_context_p)sender->target;
	uint8 tbuf[256];
	if(ctx->config->state & TK_CONFIG_STATE_PASSWORD_OFF) {
		ctx->config->state &= ~TK_CONFIG_STATE_PASSWORD_OFF;	//enable password
		ui_set_text((ui_text *)sender, (uint8 *)"Disable Password");
	} else {
		ctx->config->state |= TK_CONFIG_STATE_PASSWORD_OFF;		//disable password
		ui_set_text((ui_text *)sender, (uint8 *)"Enable Password");
	}
	if_config_write(ctx->config, sizeof(tk_config));
	ui_reinitialize_object(sender);
	OS_DEBUG_EXIT();
}

static void tk_config_security_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_config_security_click);
	tk_context_p ctx = (tk_context_p)sender->target;
	uint8 tbuf[400];
	uint8 lbuf[64];
	ui_object * list;
	uint8 esid_buffer[40];
	ui_object * instance;
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "secForm");
	list = ui_list_create(0, 
		0, 
		((ui_rect *)((gui_handle_p)ctx->display)->body)->w, 
		((ui_rect *)((gui_handle_p)ctx->display)->body)->h);
	if(list != NULL) {
		ui_set_object_name(list, "secList");
		ui_add_body(ctx->display, list);
		//Back
		ui_list_add(list, (instance  = ui_item_create((uint8 *)"Back", NULL, 0, 0, tk_back_click)));
		if(instance != NULL) instance->target = ctx;
		//Change Password
		ui_list_add(list, (instance  = ui_item_create((uint8 *)"Change Password", NULL, 0, 0, tk_security_chpass_click)));
		if(instance != NULL) instance->target = ctx;
		//Enable/Disable Password Security
		if(ctx->config->state & TK_CONFIG_STATE_PASSWORD_OFF) {
			ui_list_add(list, (instance  = ui_item_create((uint8 *)"Enable Password", NULL, 0, 0, tk_security_swpass_click)));
			if(instance != NULL) instance->target = ctx;
		} else {
			ui_list_add(list, (instance  = ui_item_create((uint8 *)"Disable Password", NULL, 0, 0, tk_security_swpass_click)));
			if(instance != NULL) instance->target = ctx;
		}
	}
	OS_DEBUG_EXIT();
}
/////////////////////////////////////////END OF SECURITY////////////////////////////////////////
ui_select_item_list g_timezone_list[] = {
	{-12, "GMT-12"}, {-11, "GMT-11"},	{-10, "GMT-10"},
	{-9, "GMT-9"},	{-8, "GMT-8"},	{-7, "GMT-7"},
	{-6, "GMT-6"},	{-5, "GMT-5"},	{-4, "GMT-4"},
	{-3, "GMT-3"},	{-2, "GMT-2"},	{-1, "GMT-1"},	{0, "GMT+0"},
	{1, "GMT+1"},	{2, "GMT+2"},	{3, "GMT+3"},	{4, "GMT+4"},
	{5, "GMT+5"},	{6, "GMT+6"},	{7, "GMT+7"},	{8, "GMT+8"},
	{9, "GMT+9"},	{10, "GMT+10"}, {11, "GMT+11"}, {12, "GMT+12"}
};

static void tk_timezone_changed(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_timezone_changed);
	int8 tz;
	datetime dval;
	ui_select_item_list *cur;
	if_time_get(&dval);
	tz = dval.tz;
	cur = &g_timezone_list[((ui_select_item *)sender)->index];
	if(cur->value != tz) {
		dval.tz = cur->value;
		if_time_set(&dval);
	}
	OS_DEBUG_EXIT();
}

/////////////////////////////////////////NETWORK CONFIG////////////////////////////////////////
static void tk_staip_submit_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_staip_submit_click);
	uint8 ip_addr[4];
	tk_context_p ctx = (tk_context_p)sender->target;
	ui_textbox * textbox = (ui_textbox *)ui_get_object_by_name(ctx->display, "conf_staip");
	if(textbox != NULL && ctx != NULL) {
		if(net_text_2_ip4ddr(textbox->content, ip_addr) == 0) {
			//valid ip address
			memcpy(ctx->netctx->staip, ip_addr, 4);				//load configured ip address to net config
			memcpy(ctx->config->net_static_ip, ip_addr, 4);		//load configured ip address to current config
			tk_save_configuration(ctx);
		}
	}
	OS_DEBUG_EXIT();
}

static void tk_devname_submit_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_devname_submit_click);
	tk_context_p ctx = (tk_context_p)sender->target;
	ui_textbox * textbox = (ui_textbox *)ui_get_object_by_name(ctx->display, "conf_name");
	if(textbox != NULL && ctx != NULL) {
		memcpy(ctx->netctx->name, textbox->content, SHARD_MAX_NODE_NAME);				//load configured ip address to net config
		memcpy(ctx->config->net_nodename, textbox->content, SHARD_MAX_NODE_NAME);		//load configured ip address to current config
		tk_save_configuration(ctx);
	}
	OS_DEBUG_EXIT();
}

static void tk_set_staip_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_set_staip_click);
	tk_context_p ctx = (tk_context_p)sender->target;
	uint8 tbuf[256];
	ui_textbox * textbox;
	uint16 ydiv = 0;
	ui_object * instance;
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "staipForm");
	memset(tbuf, 0, sizeof(tbuf));
	ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, (uint8 *)"Static IP : ")));
	ydiv += ((ui_rect *)instance)->h;
	sprintf((char *)tbuf + 64, "%d.%d.%d.%d", ctx->config->net_static_ip[0], ctx->config->net_static_ip[1], ctx->config->net_static_ip[2], ctx->config->net_static_ip[3]);
	ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_IP, (uchar *)tbuf + 64, 32, 1)));
	ui_set_object_name((ui_object *)textbox, "conf_staip");		
	ydiv += ((ui_rect *)textbox)->h;
	//create buttonset
	ui_add_body(ctx->display, (ui_object *)(instance = ui_buttonset_create(UI_BUTTON_SET_OK | UI_BUTTON_SET_CANCEL, 1, tk_staip_submit_click, tk_back_click))) ;
	if(instance != NULL) { 
		instance->target = ctx;
	}
	OS_DEBUG_EXIT();
}

static void tk_set_devname_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_set_staip_click);
	tk_context_p ctx = (tk_context_p)sender->target;
	uint8 tbuf[256];
	ui_textbox * textbox;
	uint16 ydiv = 0;
	ui_object * instance;
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "devNameForm");
	memset(tbuf, 0, sizeof(tbuf));
	ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, (uint8 *)"Device Name : ")));
	ydiv += ((ui_rect *)instance)->h;
	//sprintf((char *)tbuf + 64, "%d.%d.%d.%d", ctx->config->net_static_ip[0], ctx->config->net_static_ip[1], ctx->config->net_static_ip[2], ctx->config->net_static_ip[3]);
	sprintf((char *)tbuf + 64, "%s", ctx->netctx->name);
	ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_ALPHANUM, (uchar *)tbuf + 64, 15, 1)));
	ui_set_object_name((ui_object *)textbox, "conf_name");		
	ydiv += ((ui_rect *)textbox)->h;
	//create buttonset
	ui_add_body(ctx->display, (ui_object *)(instance = ui_buttonset_create(UI_BUTTON_SET_OK | UI_BUTTON_SET_CANCEL, 1, tk_devname_submit_click, tk_back_click))) ;
	if(instance != NULL) { 
		instance->target = ctx;
	}
	OS_DEBUG_EXIT();
}

static void tk_network_setting_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_network_setting_click);
	tk_context_p ctx = (tk_context_p)sender->target;
	uint8 tbuf[400];
	uint8 lbuf[64];
	ui_object * list;
	uint8 esid_buffer[40];
	ui_object * instance;
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "netConfForm");
	list = ui_list_create(0, 
		0, 
		((ui_rect *)((gui_handle_p)ctx->display)->body)->w, 
		((ui_rect *)((gui_handle_p)ctx->display)->body)->h);
	if(list != NULL) {
		ui_set_object_name(list, "netCfList");
		ui_add_body(ctx->display, list);
		//Back
		ui_list_add(list, (instance  = ui_item_create((uint8 *)"Back", NULL, 0, 0, tk_back_click)));
		if(instance != NULL) instance->target = ctx;
		//static ip configuration
		ui_list_add(list, (instance  = ui_item_create((uint8 *)"Device Name", NULL, 0, 0, tk_set_devname_click)));
		if(instance != NULL) instance->target = ctx;
		//static ip configuration
		ui_list_add(list, (instance  = ui_item_create((uint8 *)"IP Address", NULL, 0, 0, tk_set_staip_click)));
		if(instance != NULL) instance->target = ctx;
		//current IP
		//ui_list_add(list, ui_label_create(0, 1, (uint8 *)"Device Info\n"));
		sprintf((char *)tbuf, "Device Info\nIP : %d.%d.%d.%d", ctx->netctx->ipv4[0], ctx->netctx->ipv4[1], 
							ctx->netctx->ipv4[2], ctx->netctx->ipv4[3]);
		//ui_list_add(list, ui_label_create(0, 2, tbuf));
		sprintf((char *)tbuf, "%s\nMAC : %02x:%02x:%02x:%02x:%02x:%02x", tbuf,
							ctx->netctx->mac[0], ctx->netctx->mac[1], 
							ctx->netctx->mac[2], ctx->netctx->mac[3], 
							ctx->netctx->mac[4], ctx->netctx->mac[5]);
		ui_list_add(list, ui_label_create(0, 3, UI_FONT_DEFAULT, tbuf));
	}
	OS_DEBUG_EXIT();
}
//////////////////////////////////END OF NETWORK CONFIG////////////////////////////////////////



/////////////////////////////////////////BLUETOOTH CONFIG////////////////////////////////////////

void tk_btdev_connect(ui_object * sender, void * params) {
	bt_device_p dev = sender->target;
	bt_context_p ctx = dev->ctx;
	ui_alert * alert;
	gui_handle_p display = (gui_handle_p)params;
	alert = ui_alert_show(display, (uint8 *)"Information", (uint8 *)"Connecting...", UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
	if(ctx->state & BLE_STATE_CONNECTED) {
		//ctx->disconnect(ctx);
		if_ble_disconnect(ctx);
	}
	if(if_ble_connect(ctx, dev) == 0) {
		//if_btdev_init(ctx);
		//if(if_ble_open(ctx, 0) == 0) {
			//if_ble_send(ctx, (uint8 *)"test\r\n\r\n\r\n\r\n", 12, 200);
			//if_ble_close(ctx);
		//}
	}
	ui_alert_close(display, alert);
}

void tk_config_btdev_list_click(ui_object * sender, void * params) {
	OS_DEBUG_ENTRY(tk_config_btdev_list_click);
	//load store application
	uint8 url_buffer[256];
	uint8 search_buffer[40] = { 0, 0 };
	uint16 j;
	uint16 size, isize;
	uint16 hh;
	uint16 i, sz;
	bt_device_p iterator;
	uint8 error_code = 0;
	ui_list_scroll * listbox;
	ui_alert * alert;
	gui_handle_p display;
	uint8 dev_count = 0;
	ui_object * obj;
	tk_context_p ctx = sender->target;
	tk_config * conf = ((tk_context_p)sender->target)->config;
	ui_object * screen;
	//screen = ui_get_object_by_name(ctx->display, "btListForm");
	//if(screen != NULL) ui_remove_screen(ctx->display, screen);
	restart_display:
	alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Discovering...", UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
	dev_count = if_ble_dev_list(ctx->bctx);
	if(dev_count == 0) {
		//no device available
		ui_alert_close(ctx->display, alert);
		ui_alert_show(ctx->display, (uint8 *)"Error", (uint8 *)"No Device Available", UI_ALERT_BUTTON_OK, UI_ALERT_ICON_ERROR);
		goto exit_btdev_list;
	}
	ui_alert_close(ctx->display, alert);
	
	i = 0;
	screen = ui_push_screen(ctx->display, NULL);
	if(screen == NULL) goto exit_btdev_list;
	ui_set_object_name(screen, "btListForm");
	display = (gui_handle_p)ctx->display;
	hh = ((ui_rect *)display->body)->h;
	listbox = (ui_list_scroll *)ui_list_create(0, 0, ((ui_rect *)display->body)->w, hh);
	if(listbox == NULL) goto exit_btdev_list;
	ui_add_body(ctx->display, listbox);
	//start iterating device list
	iterator = ctx->bctx->dev_list;
	while(iterator != NULL) {
		ui_list_add((ui_object *)listbox, (obj = (ui_object *)ui_item_create(iterator->name, NULL, 0, 0, tk_btdev_connect)));
		obj->target = iterator;
		iterator = iterator->next;
	}
	exit_btdev_list:
	OS_DEBUG_EXIT();
	return;
}

//////////////////////////////////////END OF BLUETOOTH CONFIG////////////////////////////////////////

static void tk_setting_show_config(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_setting_show_config);
	ui_toggle * play;
	ui_datetime * dtime;
	datetime dval;
	ui_object * list;
	uint32 longtime;
	ui_object * obj;
	tk_config * conf = ctx->config;
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	//ui_clear_dispstack(ctx->display);
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	obj = ui_get_object_by_name(ctx->display, "mySetting") ;
	if(obj != NULL) {
		((ui_icon_tool *)obj)->show_marker = TRUE;
	}
	obj = ui_get_object_by_name(ctx->display, "settings") ;
	if(obj != NULL) {
		ui_remove_screen(ctx->display, obj);
	}
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "settings");
	ui_set_text(screen, "settings");
	list = ui_list_create(0, 
		0, 
		((ui_rect *)((gui_handle_p)ctx->display)->body)->w, 
		((ui_rect *)((gui_handle_p)ctx->display)->body)->h);
	if(list != NULL) {
		ui_add_body(ctx->display, list);
		
		//login button
		if((conf->state & TK_CONFIG_STATE_NOT_LOGGED) == 0) {
			ui_list_add(list, (obj  = ui_item_create((uint8 *)"Unregister", (uint8 *)image_png_unregister24, sizeof(image_png_unregister24), 0, tk_config_logoff_click)));
			if(obj != NULL) obj->target = ctx;
			ui_list_add(list, (obj  = ui_item_create((uint8 *)"Load Framework", (uint8 *)image_png_framework24, sizeof(image_png_framework24), 0, tk_config_devlet_click)));
			if(obj != NULL) obj->target = ctx;
		} else {
			ui_list_add(list, (obj  = ui_item_create((uint8 *)"Register", (uint8 *)image_png_register24, sizeof(image_png_register24), 0, tk_config_login_click)));
			if(obj != NULL) obj->target = ctx;
		}
		if(ctx->bctx->state & BLE_STATE_INITIALIZED) {
			ui_list_add(list, (obj  = ui_item_create((uint8 *)"Devices", (uint8 *)image_png_bluetooth24, sizeof(image_png_bluetooth24), 0, tk_config_btdev_list_click)));
			if(obj != NULL) obj->target = ctx;
		}
		//initialize automaton button
		ui_list_add(list, (ui_object *)(play  = (ui_toggle *)ui_autobtn_create((uint8 *)"autoPlayBtn", ctx, tk_autobtn_click)));
		if(ctx->runstate & TK_AUTOPLAY_ENABLED) {
			play->state = 1;
		} else {
			switch((ctx->runstate & 0x0F)) {
				case TK_AUTOPLAY_RECORD:
					play->state = 2;
					break;
				default: break;
			}
		}
		
		//create timezone selector
		if_time_get(&dval);
		ui_list_add(list, (ui_object *)ui_select_item_create((uint8 *)"myTimezone", dval.tz +12, 25, g_timezone_list, tk_timezone_changed));
	///////////////////////////////////////DATE TIME CONFIGURATION//////////////////////////////////
		//longtime = if_time_get(&dval);
		//time configuration
		//ui_list_add(list, (ui_object *)(dtime = (ui_datetime *)ui_datetime_button_create((uint8 *)"myDateTime", UI_DTSHOW_FULLTIME | UI_DTSHOW_FULLDATE | UI_DTSHOW_SUBMIT, &dval)));
	///////////////////////////////////////DATE TIME CONFIGURATION//////////////////////////////////
		
		if(ctx->cos_config[0] & 0x80) {		//only for development card
			ui_list_add(list, (obj  = ui_item_create((uint8 *)"Delete App", (uint8 *)image_png_exe24, sizeof(image_png_exe24), 0, tk_delete_app_click)));
			if(obj != NULL) obj->target = ctx;
		}
		//about button
		ui_list_add(list, (obj  = ui_item_create((uint8 *)"Security", (uint8 *)image_png_security24, sizeof(image_png_security24), 0, tk_config_security_click)));
		if(obj != NULL) obj->target = ctx;
		//screen orientation button
		ui_list_add(list, (obj  = ui_item_create((uint8 *)"Network", (uint8 *)image_png_network24, sizeof(image_png_network24), 0, tk_network_setting_click)));
		if(obj != NULL) obj->target = ctx;
		//update download button
		if(ctx->cos_status & TK_COS_STAT_UPDATE_AVAIL) {
			//ui_list_add(list, (obj  = ui_item_create((uint8 *)"Download Update", (uint8 *)image_png_download, sizeof(image_png_download), 0, tk_config_update_click)));
			ui_list_add(list, (obj  = ui_item_create((uint8 *)"Download Update", (uint8 *)image_png_update24, sizeof(image_png_update24), 0, tk_config_update_click)));
			if(obj != NULL) obj->target = ctx;
		}
		//screen orientation button
		ui_list_add(list, (obj  = ui_item_create((uint8 *)"Orientation", (uint8 *)image_png_orientation24, sizeof(image_png_orientation24), 0, tk_config_orientation_click)));
		if(obj != NULL) obj->target = ctx;
		//about button
		ui_list_add(list, (obj  = ui_item_create((uint8 *)"About", (uint8 *)image_png_sysinfo24, sizeof(image_png_sysinfo24), 0, tk_config_about_click)));
		if(obj != NULL) obj->target = ctx;
		//initialize exit button
		//ui_list_add(list, (obj  = ui_item_create((uint8 *)"Exit", NULL, 0, 0, tk_config_exit_click)));
		//if(obj != NULL) obj->target = ctx;
	}
	OS_DEBUG_EXIT();
}

static uint8 tk_is_valid_password(uint8 * text, uint16 length) {
	uint16 i;
	uint8 c;
	uint8 ret = TRUE;
	for(i=0;i<length && ret == TRUE;i++) {
		c = text[i];
		if(c == 0) break;		//end of string
		if(c < 0x32) ret = FALSE;
		if(c > 127) ret = FALSE;
	}
	return ret;
}

static void tk_config_submit_devpass(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_config_submit_devpass);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_textbox * textbox = (ui_textbox *)ui_get_object_by_name(ctx->display, "cur_pass");
	if(textbox == NULL) { tk_config_exit_click(obj, params); goto exit_config_submit_devpass; }
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "settings"));
	if(!tk_is_valid_password(ctx->config->devpass, SHARD_PASS_SIZE) || strncmp((const char *)textbox->content, (char *)ctx->config->devpass, SHARD_PASS_SIZE) == 0) {
		tk_setting_show_config(ctx);
	} else {
		ui_alert_show(ctx->display, (uint8 *)"Error", (uint8 *)"Invalid Password", UI_ALERT_INFO | UI_ALERT_BUTTON_OK, UI_ALERT_ICON_ERROR);
	}
	exit_config_submit_devpass:
	OS_DEBUG_EXIT();
	return;
}

static void tk_setting_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_setting_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * instance;
	ui_textbox * textbox;
	uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * screen;
	memset(tbuf, 0, sizeof(tbuf));
	if((screen = ui_get_screen_by_name(ctx->display, "settings")) != NULL) {
		if(((gui_handle_p)ctx->display)->body == screen) {
			tk_config_exit_click(obj, params);
		} else {
			tk_clear_subconfig_menu(ctx);
			ui_remove_screen_unsafe(ctx->display, screen);
			screen = ui_push_screen(ctx->display, screen);
			ui_set_object_name(screen, "settings");
			ui_set_text((ui_text *)screen, (uint8 *)"settings");
		}
	} else {
		if((ctx->config->state & TK_CONFIG_STATE_PASSWORD_OFF) == 0) {
			show_config_screen:
			//password activated
			//ui_clear_dispstack(ctx->display);
			screen = ui_push_screen(ctx->display, NULL);
			ui_set_object_name(screen, "settings");
			ui_set_text((ui_text *)screen, (uint8 *)"settings");
			ui_add_body(ctx->display, (instance = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, (uint8 *)"Password : ")));
			ydiv += ((ui_rect *)instance)->h;
			ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_PIN, (uchar *)tbuf + 64, 32, 1)));
			ui_set_object_name((ui_object *)textbox, "cur_pass");		
			ydiv += ((ui_rect *)textbox)->h;
			//create buttonset
			ui_add_body(ctx->display, (ui_object *)(instance = ui_buttonset_create(UI_BUTTON_SET_OK | UI_BUTTON_SET_CANCEL, 1, tk_config_submit_devpass, tk_config_exit_click))) ;
			if(instance != NULL) { 
				instance->target = ctx;
			}
		} else {
			tk_setting_show_config(ctx);
		}
	}
	OS_DEBUG_EXIT();
	return;
}

void tk_setting_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(tk_setting_init);
	ui_object * setting = NULL;
	ui_object * header = handle->header;
	uint16 header_w;
	if(params != NULL && header != NULL) {
		header_w = header->rect.h / 4;
		ui_add_header(handle, (setting = ui_config_create((uint8 *)"mySetting", (uint8 *)"", header_w, header_w, tk_setting_click)));
		if(setting != NULL) ui_set_target(setting, params);
	}
	OS_DEBUG_EXIT();
}