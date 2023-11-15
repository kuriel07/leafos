#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include "..\..\build.h"
#include "..\..\config.h"
#include "..\..\gui\inc\ui_resources.h"
#include "..\..\gui\inc\ui_keyboard.h"
#include "..\..\gui\inc\ui_chart.h"
#include "..\..\gui\inc\ui_button.h"
#include "..\..\gui\inc\ui_panel.h"
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
#include <math.h>
#include "arm_math.h"
#include "arm_const_structs.h"

extern const unsigned char g_spectral_app_png48[3705];

static void spectral_exit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(spectral_exit_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	//uint32 longtime = 0;
	//ui_object * screen;
	ui_object * cfBtn;
	//tk_clear_subconfig_menu(ctx);
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "spectral"));
	//disable marker
	cfBtn = ui_get_object_by_name(ctx->display, "spectralApp") ;
	if(cfBtn != NULL) {
		((ui_icon_tool *)cfBtn)->show_marker = FALSE;
	}
	//ui_clear_dispstack(ctx->display);
	//tk_clear_body(ctx);
	OS_DEBUG_EXIT();
}

//performance buffer as global variable to be shared between windows
#define SPECTRAL_BUF_SIZE			128
#define CHANNEL_BUF_SIZE			AUDIO_SAMPLING_FREQ/2				//2048
#define FFT_INPUT_SIZE				2048
#define FFT_OUTPUT_SIZE				FFT_INPUT_SIZE/2
int16 g_spectral_buffer[SPECTRAL_BUF_SIZE];
int16 g_audio_buffer[AUDIO_SAMPLING_FREQ];
//__ALIGN_BEGIN int16 g_audio_buffer[AUDIO_SAMPLING_FREQ] __attribute__((section(".RW_ERAM2"))) __ALIGN_END ;

static uint8 g_spectral_mode = 1;



void ui_series_spectral_render(gui_handle_p display, ui_chart_area * area, ui_series * series);
void ui_series_line_render(gui_handle_p display, ui_chart_area * area, ui_series * series);

void spectral_generate_hanning(float * buffer, size_t size) {
	int i=0;
	for(i=0;i<size;i++) {
		buffer[i] = 1 - (0.5*(1+cos(2*PI*i/size)));
	}
}


static void spectral_task() {
	OS_DEBUG_ENTRY(spectral_task);
	tk_context_p ctx;
	uint16 i,j;
	ui_chart * chart = NULL;
	ui_series * spal_perf = NULL;
	ctx = os_get_context();
	arm_rfft_instance_q15 rfft15;
	//arm_rfft_instance_f32 rfft32;
	//arm_cfft_radix4_instance_f32 cfftr4q15;
	int16 g_fft_input_buffer[FFT_INPUT_SIZE];
	int16 g_fft_output_buffer[FFT_OUTPUT_SIZE];
	//float g_fft_audio_buffer[AUDIO_SAMPLING_FREQ];
	int16 g_audio_buffer_right[CHANNEL_BUF_SIZE];
	int16 g_audio_buffer_left[CHANNEL_BUF_SIZE];
	#define HALF_CHANNEL_BUF_SIZE			CHANNEL_BUF_SIZE/2				//1024
	#define QUAD_CHANNEL_BUF_SIZE			HALF_CHANNEL_BUF_SIZE/2		//512
	float hanning[SPECTRAL_BUF_SIZE];
	g_spectral_mode = 0;
	spectral_generate_hanning(hanning, SPECTRAL_BUF_SIZE);
	
	while(1) {
		os_wait(100);		//tick at 1 second
		chart = (ui_chart *)ui_get_object_by_name(ctx->display, "spal_chart");
		if(chart != NULL) {
			spal_perf = ui_chart_get_series_by_name(chart, "spectral");
			if(spal_perf != NULL) {
				//shift elements to left
				//memcpy(g_spectral_buffer, g_spectral_buffer + 1, sizeof(g_spectral_buffer) - sizeof(uint16));
				//tk_memcpy(g_cpu_perf_buffer, g_cpu_perf_buffer + 1, sizeof(g_cpu_perf_buffer) - sizeof(uint16));
				
				//get audio signal from microphone
				ctx->audio->read(ctx->audio, (uint16 *)g_audio_buffer, sizeof(g_audio_buffer));
				//for(i=0;i<AUDIO_SAMPLING_FREQ;i++) g_fft_audio_buffer[i] = g_audio_buffer[i];
				//continue;
				for(i=0;i<AUDIO_SAMPLING_FREQ;i+=2) {
					g_audio_buffer_left[i/2] = g_audio_buffer[i] ;
					g_audio_buffer_right[i/2] = g_audio_buffer[i+1];
				}
				//re-sampling for fft
				for(i=0,j=0;i<HALF_CHANNEL_BUF_SIZE;i+=(HALF_CHANNEL_BUF_SIZE / (FFT_INPUT_SIZE / 2)),j++) {
					g_audio_buffer_left[j] = g_audio_buffer_left[i] ;
				}
				if(g_spectral_mode & 0x01) {
					//frequency domain selected
					chart->area.type = UI_CHART_AREA_TYPE_S1;
					spal_perf->render = ui_series_spectral_render;
					chart->area.mode = UI_CHART_MODE_ABSOLUTE;
					chart->area.axis[1].max = 1024;
					strncpy(chart->title, "Spectrum Analyzer", UI_MAX_OBJECT_NAME);
					snprintf(chart->area.axis[0].name, UI_MAX_OBJECT_NAME, "%s", "Frequency");
					memset(g_fft_input_buffer, 0, sizeof(g_fft_input_buffer));
					arm_rfft_init_q15(&rfft15, FFT_INPUT_SIZE/2, 0, 1);
					arm_rfft_q15(&rfft15, g_audio_buffer_left, g_fft_input_buffer);
					for(i=0;i<FFT_INPUT_SIZE;i++) {
						g_fft_input_buffer[i] <<= 7;
					}
					//if(arm_rfft_init_q15(&S, 8192, 1, 1) != ARM_MATH_SUCCESS)
						//return state_error;
					
					arm_cmplx_mag_q15(g_fft_input_buffer, g_fft_output_buffer, FFT_OUTPUT_SIZE);
					//for(i=0;i<(FFT_OUTPUT_SIZE / 2);i++) {
					//	if(g_fft_output_buffer[i] > 1024)  g_spectral_buffer[i] = 1024;
					//	else if(g_fft_output_buffer[i] < 0) g_spectral_buffer[i] = 0;
					//	else g_spectral_buffer[i] = g_fft_output_buffer[i] ;
						//g_spectral_buffer[i] *= hanning[i];
					//}
					
					for(i=0,j=0;i<(FFT_OUTPUT_SIZE / 2);i+=((FFT_OUTPUT_SIZE / 2) / SPECTRAL_BUF_SIZE),j++) {
						//g_audio_buffer_left[j] = g_audio_buffer_left[i] ;
						if(g_fft_output_buffer[i] > 1024)  g_spectral_buffer[j] = 1024;
						else if(g_fft_output_buffer[i] < 0) g_spectral_buffer[j] = 0;
						else g_spectral_buffer[j] = g_fft_output_buffer[i] ;
					}
					
					//apply hanning if requested
					if(g_spectral_mode & 0x02) {
						for(i=0;i<SPECTRAL_BUF_SIZE;i++) 
							g_spectral_buffer[i] = g_spectral_buffer[i] * hanning[i];
					}
					if(g_spectral_mode & 0x02) {
						
					}
					g_spectral_buffer[0] = 0;
					for(i=0;i<SPECTRAL_BUF_SIZE;i++) 
							((uint16 *)(spal_perf->elements))[i] = (g_spectral_buffer[i] + ((uint16 *)(spal_perf->elements))[i]) / 2;
					//memcpy(spal_perf->elements, g_spectral_buffer, spal_perf->length * spal_perf->type);
				} else {
					//time domain selected
					chart->area.type = UI_CHART_AREA_TYPE_H2;
					chart->area.axis[1].max= 0;
					//chart->area.mode = UI_CHART_MODE_RELATIVE;
					chart->area.mode = UI_CHART_MODE_ABSOLUTE;
					chart->area.axis[1].max = 100;
					strncpy(chart->title, "Oscilloscope", UI_MAX_OBJECT_NAME);
					snprintf(chart->area.axis[0].name, UI_MAX_OBJECT_NAME, "%s", "Time");
					spal_perf->render = ui_series_line_render;
					
					//re-sampling
					//for(i=0;i<CHANNEL_BUF_SIZE;i+=(CHANNEL_BUF_SIZE / SPECTRAL_BUF_SIZE)) 
					//	g_spectral_buffer[i/ (CHANNEL_BUF_SIZE / SPECTRAL_BUF_SIZE)] = g_audio_buffer[i] ;
					for(i=0,j=0;i<HALF_CHANNEL_BUF_SIZE;i+=(HALF_CHANNEL_BUF_SIZE / SPECTRAL_BUF_SIZE),j++) {
						if(g_audio_buffer_left[i] > 100)  g_spectral_buffer[j] = 100;
						else if(g_audio_buffer_left[i] < -100) g_spectral_buffer[j] = -100;
						else g_spectral_buffer[j] = g_audio_buffer_left[i] ;
					}
					//apply hanning if requested
					if(g_spectral_mode & 0x02) {
						for(i=0;i<SPECTRAL_BUF_SIZE;i++) 
							g_spectral_buffer[i] = g_spectral_buffer[i] * hanning[i];
					}
					//for(i=0;i<SPECTRAL_BUF_SIZE;i++) 
					//		((uint16 *)(spal_perf->elements))[i] = (g_spectral_buffer[i] + ((uint16 *)(spal_perf->elements))[i]) / 2;
					memcpy(spal_perf->elements, g_spectral_buffer, spal_perf->length * spal_perf->type);
				}
			
				ui_reinitialize_object((ui_object *)chart);				//invalidate chart
			}
		}
	}
	
	OS_DEBUG_EXIT();
}

static void spectral_hanning_btn_click(ui_object * obj, void * display) {
	if(g_spectral_mode  & 0x02) {
		g_spectral_mode &= ~0x02;
		//ui_set_toggle_state(obj, UI_TOGGLE_OFF);
	} else {
		g_spectral_mode |= 0x02;
		//ui_set_toggle_state(obj, UI_TOGGLE_ON);
	}
}
static void spectral_time_btn_click(ui_object * obj, void * display) {
	g_spectral_mode = 0;
}
static void spectral_freq_btn_click(ui_object * obj, void * display) {
	g_spectral_mode = 1;
}

extern const unsigned char g_filter_add_png32[2178] ;
extern const unsigned char g_filter_remove_png32[2172] ;
const ui_resource g_filter_add32 = {
	sizeof(g_filter_add_png32), (unsigned char *)g_filter_add_png32
};
const ui_resource g_filter_remove32 = {
	sizeof(g_filter_remove_png32), (unsigned char *)g_filter_remove_png32
};
static void spectral_show(tk_context_p ctx) {
	OS_DEBUG_ENTRY(spectral_show);
	//ui_toggle * play;
	//ui_datetime * dtime;
	//datetime dval;
	//ui_object * list;
	//uint32 longtime;
	ui_object * obj;
	//ui_object * calc_keypad1, * calc_keypad2;
	//uint16 keypad_height = 0;
	//uint16 y = 0;
	//tk_config * conf = ctx->config;
	//ui_object * lcd;
	ui_chart * chart;
	ui_object * panel;
	ui_series * series;
	os_task * spaltask = NULL;
	//ui_rect rect = { 0, 0, 48, 48 };
	ui_object * btn1, * btn2, * btn3;
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	//ui_clear_dispstack(ctx->display);
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	obj = ui_get_object_by_name(ctx->display, "spectralApp") ;
	if(obj != NULL) {
		((ui_icon_tool *)obj)->show_marker = TRUE;
	}
	obj = ui_get_object_by_name(ctx->display, "spectral") ;
	if(obj != NULL) {
		ui_remove_screen(ctx->display, obj);
	}
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "spectral");
	ui_set_text((ui_text *)screen, (uint8 *)"spectral");
	ui_add_body(ctx->display, (panel = ui_panel_create(UI_ALIGN_VERTICAL, 96, 32)));
	
	
	ui_add_object(panel, (btn1 = ui_button_create(UI_COLOR_BLUE, (uchar *)"Time", 1, spectral_time_btn_click)));
	ui_add_object(panel, (btn2 = ui_button_create(UI_COLOR_BLUE, (uchar *)"Freq", 2, spectral_freq_btn_click)));
	ui_add_object(panel, (btn3 = ui_button_create(UI_COLOR_BLUE, (uchar *)"Hanning", 3, spectral_hanning_btn_click)));
	ui_set_align(btn1, UI_ALIGN_HORIZONTAL);
	ui_set_size(btn1, 96, 32);
	ui_set_position(btn1, 0, 0);
	ui_set_align(btn2, UI_ALIGN_HORIZONTAL);
	ui_set_size(btn2, 96, 32);
	ui_set_position(btn2, 96, 0);
	ui_set_align(btn3, UI_ALIGN_HORIZONTAL);
	ui_set_size(btn3, 96, 32);
	ui_set_position(btn3, 0, 0);
	
	
	
	series = ui_series_create_spectral("spectral", sizeof(uint16), UI_COLOR_ORANGE, UI_COLOR_LIME, 128, (void *)g_spectral_buffer);
	ui_add_body(ctx->display, (chart = (ui_chart *)ui_chart_create("spal_chart", "Spectral", series)));
	chart->area.axis[1].max = 1024;
	//((ui_chart *)chart)->area.mode = UI_CHART_MODE_RELATIVE;
	ui_set_align((ui_object *)chart, UI_ALIGN_VERTICAL);
	ui_set_size((ui_object *)chart, 280, 216);
	//ui_set_position((ui_object *)chart, 0, 32);
	//ui_set_size((ui_object *)chart, screen->base.w, screen->base.h - 32);
	
	spaltask = os_find_task_by_name("spaltask");			//spectrum analyzer task
	if(spaltask == NULL) {
		memset(g_spectral_buffer, 0, sizeof(g_spectral_buffer));		//clear performance buffer
		spaltask = os_create_task(ctx, spectral_task, "spaltask", 500, 128000);
	}
	
	//ui_set_size(chart, screen->base.w, screen->base.h);
	//ui_add_body(ctx->display, (lcd = ui_numbox_create((uint8 *)"", 40)));
	//ui_set_object_name(lcd, "calc_lcd");
	//ui_set_content(lcd, "0");
	//y = screen->rect.y;
	//y += lcd->rect.h + 8;
	//keypad_height = screen->rect.h -  (lcd->rect.h + 8);
	
	if(((gui_handle_p)ctx->display)->orientation & 0x01) {
		//ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w, keypad_height / 2, 0)));
		//ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x, y + (keypad_height / 2), screen->rect.w, keypad_height / 2, 1)));
	} else {
		//ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w/2, keypad_height, 0)));
		//ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x + (screen->rect.w/2), y, screen->rect.w/2, keypad_height, 1)));
	}
	
	OS_DEBUG_EXIT();
}

static void spectral_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(spectral_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	//ui_object * instance;
	//ui_textbox * textbox;
	//uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * screen;
	memset(tbuf, 0, sizeof(tbuf));
	if((screen = ui_get_screen_by_name(ctx->display, "spectral")) != NULL) {
		if(((gui_handle_p)ctx->display)->body != screen) {
			spectral_exit_click(obj, params);
		} else {
			//ui_remove_screen_unsafe(ctx->display, screen);
			//screen = ui_push_screen(ctx->display, screen);
			//ui_set_object_name(screen, "chart");
			//ui_set_text((ui_text *)screen, (uint8 *)"chart");
		}
	} else {
			spectral_show(ctx);
	}
	OS_DEBUG_EXIT();
	return;
}


void spectral_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(spectral_init);
	ui_object * calc = NULL;
	ui_object * header = handle->header;
	uint16 header_w;
	if(params != NULL && header != NULL) {
		header_w = header->rect.h / 4;
		ui_add_header(handle, (calc = ui_header_create((uint8 *)"spectralApp", (uint8 *)"spectral", header_w, header_w, sizeof(g_spectral_app_png48), (uint8 *)g_spectral_app_png48, spectral_click)));
		if(calc != NULL) ui_set_target(calc, params);
	}
	OS_DEBUG_EXIT();
}

void spectral_app_init(gui_handle_p handle, void * params, uint8 id) {
	OS_DEBUG_ENTRY(spectral_app_init);
	ui_object * chart = NULL;
	ui_object * body = handle->body;
	ui_object * obj;
	uint16 header_w = 64;
	if(params != NULL && body != NULL) {
		ui_add_body(handle, (chart = ui_launcher_create((uint8 *)"spectralApp", (uint8 *)"spectral", header_w, header_w, sizeof(g_spectral_app_png48), (uint8 *)g_spectral_app_png48, spectral_click)));
		if(chart != NULL) {
			obj = ui_get_object_by_name(handle, "spectral") ;
			if(obj != NULL) {
				((ui_icon_tool *)chart)->show_marker = TRUE;
			}
			ui_set_target(chart, params);
		}
	}
	OS_DEBUG_EXIT();
}
