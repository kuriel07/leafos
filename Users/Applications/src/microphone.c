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
#include "..\..\gui\inc\ui_slider.h"
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
#include "usbd_customhid.h"
#include "..\..\core\inc\os_msg.h"


__ALIGN_BEGIN uint8 g_usb_audio_buffer[48312 * 3] __attribute__((section(".RW_ERAM2"))) __ALIGN_END;
uint32 g_audio_buffer_head = 0;
uint32 g_audio_buffer_tail = 0;
#if 1
extern const unsigned char g_microphone_appicon_png48[2128];

static void microphone_exit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(microphone_exit_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	//uint32 longtime = 0;
	//ui_object * screen;
	ui_object * cfBtn;
	//tk_clear_subconfig_menu(ctx);
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "microphone"));
	//disable marker
	cfBtn = ui_get_object_by_name(ctx->display, "microphoneApp") ;
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
#define FFT_INPUT_SIZE				512
#define FFT_OUTPUT_SIZE				FFT_INPUT_SIZE/2
//static int16 g_spectral_buffer[SPECTRAL_BUF_SIZE] __attribute__((section(".RW_ERAM1")));
//static int16 g_audio_buffer[AUDIO_SAMPLING_FREQ] __attribute__((section(".RW_ERAM1")));
extern int16 g_spectral_buffer[SPECTRAL_BUF_SIZE];
extern int16 g_audio_buffer[AUDIO_SAMPLING_FREQ];
uint8 g_spectral_mic_mode = 0;

void ui_series_spectral_render(gui_handle_p display, ui_chart_area * area, ui_series * series);
void ui_series_line_render(gui_handle_p display, ui_chart_area * area, ui_series * series);

extern USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size);



void gen_sinusoidal(int16 amp, uint16 freq, uint16 total_period, int16 * buffer, uint32 sz) {
	float period = (float)total_period / freq;
	for(int i=0;i<period;i++) {
		int16 res = (int16)(sin((((float)i / period) * 6.28)) * amp);
		for(float j=0;j<total_period;j+=period) {
			int32 index = (j + i);
			if(index <= ((sz / sizeof(int16)) - 1) ) {
				buffer[index] = res;
				//buffer[index +0] = 0;
				//buffer[index +0] = res;
				//buffer[index +1] = res >> 8;
			}
		}
	}
}



void resize_sampling(int16 * source, uint32 source_size, uint8 * dst, uint32 dst_size, uint32 index, uint32 buf_size, uint16 amp) {
	uint32 i,j, k;
	uint8 buf[3];
	volatile float scale = (float)dst_size / (float)source_size;
	volatile float left = 0;
	int16 current, next;
	int16 step;
	index %= buf_size;
	k= index;
	if(scale >= 1) {
		//upscale
		for(i=0;i<(source_size - 1);i++) {
			current = source[i] * amp;
			next = source[i+1] * amp;
			step = (next - current) / scale;
			for(j=0;j<(uint32)(scale + (int)left) ;j++) {
				//24 bit
				int32 temp = (int32)current;
				dst[k+2] = (temp>>8);
				dst[k+1] = temp;
				dst[k+0] = 0;
				k += 3;
				k %= buf_size;		//modulus (circular buffer)
				current += step;
			}
			left = (scale + left) - j;
		}
	} else {
		//downscale
		scale = 1/scale;
		left = 0;
		for(i=0;i<dst_size-1;i++) {
			current = source[(uint32)left] * amp;
			int32 temp = (int32)current;
			dst[k+2] = (temp>>8);
			dst[k+1] = temp;
			dst[k+0] = 0;
			k += 3;
			k %= buf_size;		//modulus (circular buffer)
			left += scale;
		}
	}
	k = k;
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
//static os_tick_callback_counter =0;

int16 g_equalizer[FFT_INPUT_SIZE/2];

static unsigned g_i_volume = 10;
static void graph_slider_changed(ui_object * obj, void * params) {
	ui_slider * slider = (ui_slider *)obj;
	g_i_volume = (slider->base.percent/20);
}

static void microphone_task() {
	OS_DEBUG_ENTRY(microphone_task);
	tk_context_p ctx;
	uint32 i, j;
	ui_chart * chart = NULL;
	ui_series * spal_perf = NULL;
	ctx = os_get_context();
	arm_rfft_instance_q15 rfft15;
	arm_rfft_fast_instance_f32 rfft32;
	//arm_rfft_instance_f32 rfft32;
	//arm_cfft_radix4_instance_f32 cfftr4q15;
	int16 g_fft_input_buffer[FFT_INPUT_SIZE];
	int16 g_fft_output_buffer[FFT_OUTPUT_SIZE];
	//float g_fft_audio_buffer[AUDIO_SAMPLING_FREQ];
	int16 g_audio_buffer_right[CHANNEL_BUF_SIZE];					//4096 bytes
	int16 g_audio_buffer_left[CHANNEL_BUF_SIZE];					//4096 bytes
	#define HALF_CHANNEL_BUF_SIZE			CHANNEL_BUF_SIZE/2				//1024
	#define QUAD_CHANNEL_BUF_SIZE			HALF_CHANNEL_BUF_SIZE/2		//512
	usb_context_p usb = ctx->uctx;		//
	os_task * usb_task = NULL;
	g_audio_buffer_head = 0;
	g_spectral_mic_mode = 0;
	//float32_t g_fft32_input_buffer[FFT_INPUT_SIZE];
	//float32_t g_fft32_audio_buffer[FFT_INPUT_SIZE];
	//float32_t g_fft32_output_buffer[FFT_OUTPUT_SIZE];
	static unsigned last_tail=0;
	for(i=0;i<(FFT_INPUT_SIZE/2);i++) g_equalizer[i] = 1;
	//gen_sinusoidal(2000, 440, 32768, g_usb_audio_buffer, sizeof(g_usb_audio_buffer));
	//gen_sinusoidal(2000, 440, 32768, (uint8 *)g_audio_buffer, sizeof(g_usb_audio_buffer));
	while(1) {
		os_wait(50);		//tick at 1 second
		chart = (ui_chart *)ui_get_object_by_name(ctx->display, "spal_chart");
		if(chart != NULL) {
			spal_perf = ui_chart_get_series_by_name(chart, "spectral");
			if(spal_perf != NULL) {
				//shift elements to left
				//memcpy(g_spectral_buffer, g_spectral_buffer + 1, sizeof(g_spectral_buffer) - sizeof(uint16));
				//tk_memcpy(g_cpu_perf_buffer, g_cpu_perf_buffer + 1, sizeof(g_cpu_perf_buffer) - sizeof(uint16));
				
				//get audio signal from microphone
				ctx->audio->read(ctx->audio, (int16 *)g_audio_buffer , sizeof(g_audio_buffer));		//load only half of AUDIO_SAMPLING_FREQ
				
				for(i=0;i<AUDIO_SAMPLING_FREQ;i+=2) {
					g_audio_buffer_left[i/2] = g_audio_buffer[i] ;
					g_audio_buffer_right[i/2] = g_audio_buffer[i+1];
				}
				g_audio_buffer_head = last_tail + (6000*3); 
				if(g_audio_buffer_head < g_audio_buffer_tail) {
					g_audio_buffer_head = g_audio_buffer_tail;
				}
				last_tail = g_audio_buffer_tail;
				//upscale audio for usb
				resize_sampling(g_audio_buffer_left, AUDIO_SAMPLING_FREQ/2, g_usb_audio_buffer, 6000, g_audio_buffer_head, 144000, 100);
				
				
				
				if(g_spectral_mic_mode & 0x01) {
					//re-sampling for fft
					for(i=0,j=0;i<HALF_CHANNEL_BUF_SIZE;i+=(HALF_CHANNEL_BUF_SIZE / (FFT_INPUT_SIZE / 2)),j++) {
						g_audio_buffer_left[j] = g_audio_buffer_left[i] ;
					}
					//frequency domain selected
					chart->area.type = UI_CHART_AREA_TYPE_S1;
					spal_perf->render = ui_series_spectral_render;
					chart->area.mode = UI_CHART_MODE_ABSOLUTE;
					chart->area.axis[1].max = 1024;
					strncpy(chart->title, "Spectrum Analyzer", UI_MAX_OBJECT_NAME);
					snprintf(chart->area.axis[0].name, UI_MAX_OBJECT_NAME, "%s", "Frequency");
					memset(g_fft_input_buffer, 0, sizeof(g_fft_input_buffer));
					
					//forward fourier transform
					arm_rfft_init_q15(&rfft15, FFT_INPUT_SIZE/2, 0, 1);			//FFT
					arm_rfft_q15(&rfft15, g_audio_buffer_left, g_fft_input_buffer);
					//arm_rfft_fast_init_f32(&rfft32, FFT_INPUT_SIZE/2);	
					//arm_rfft_fast_f32(	&rfft32, g_fft32_audio_buffer, g_fft32_input_buffer, 0 );
					
					//graphic equalizer
					for(i=0;i<FFT_INPUT_SIZE;i++) {
						g_fft_input_buffer[i] <<= 7;
					}
					
					//load magnitude for spectrum analyzer
					arm_cmplx_mag_q15(g_fft_input_buffer, g_fft_output_buffer, FFT_OUTPUT_SIZE);
					//arm_cmplx_mag_f32(g_fft32_input_buffer, g_fft32_output_buffer, FFT_OUTPUT_SIZE);
					
					
					//inverse fourier transform
					///arm_rfft_init_q15(&rfft15, FFT_INPUT_SIZE/2, 1, 1);		//IFFT
					//arm_rfft_q15(&rfft15, g_fft_input_buffer, g_audio_buffer_left);
					
					//arm_rfft_fast_f32	(	&rfft32, g_fft32_input_buffer, g_fft32_audio_buffer, 1 );
					//for(i=0;i<FFT_INPUT_SIZE;i++) g_audio_buffer_left[i] = g_fft32_input_buffer[i];
					
					
					for(i=0,j=0;i<(FFT_OUTPUT_SIZE / 2);i+=((FFT_OUTPUT_SIZE / 2) / SPECTRAL_BUF_SIZE),j++) {
						//g_audio_buffer_left[j] = g_audio_buffer_left[i] ;
						if(g_fft_output_buffer[i] > 1024)  g_spectral_buffer[j] = 1024;
						else if(g_fft_output_buffer[i] < 0) g_spectral_buffer[j] = 0;
						else g_spectral_buffer[j] = g_fft_output_buffer[i] ;
					}
					
					for(i=0;i<SPECTRAL_BUF_SIZE;i++) 
							((uint16 *)(spal_perf->elements))[i] = (g_spectral_buffer[i] + ((uint16 *)(spal_perf->elements))[i]) / 2;
					
				} else {
					//time domain selected
					chart->area.type = UI_CHART_AREA_TYPE_H2;
					chart->area.axis[1].max= 0;
					//chart->area.mode = UI_CHART_MODE_RELATIVE;
					chart->area.mode = UI_CHART_MODE_ABSOLUTE;
					chart->area.axis[1].max = 60;
					strncpy(chart->title, "Oscilloscope", UI_MAX_OBJECT_NAME);
					snprintf(chart->area.axis[0].name, UI_MAX_OBJECT_NAME, "%s", "Time");
					spal_perf->render = ui_series_line_render;
					
					
					//re-sampling
					for(i=0;i<CHANNEL_BUF_SIZE;i+=((CHANNEL_BUF_SIZE) / SPECTRAL_BUF_SIZE)) {
						j = i/ ((CHANNEL_BUF_SIZE) / SPECTRAL_BUF_SIZE);
						if(g_audio_buffer_left[i] > 60)  g_spectral_buffer[j] = 59;
						else if(g_audio_buffer_left[i] < -60) g_spectral_buffer[j] = -59;
						g_spectral_buffer[j] = g_audio_buffer_left[i] ;
					}
					#if false
					for(i=0,j=0;i<HALF_CHANNEL_BUF_SIZE;i+=(HALF_CHANNEL_BUF_SIZE / SPECTRAL_BUF_SIZE),j++) {
						if(g_audio_buffer_left[i] > 128)  g_spectral_buffer[j] = 128;
						else if(g_audio_buffer_left[i] < -128) g_spectral_buffer[j] = -128;
						else g_spectral_buffer[j] = g_audio_buffer_left[i] ;
					}
					#endif
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


static void microphone_show(tk_context_p ctx) {
	OS_DEBUG_ENTRY(microphone_show);
	//ui_toggle * play;
	//ui_datetime * dtime;
	//datetime dval;
	//ui_object * list;
	//uint32 longtime;
	ui_object * obj;
	int i;
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
	obj = ui_get_object_by_name(ctx->display, "microphoneApp") ;
	if(obj != NULL) {
		((ui_icon_tool *)obj)->show_marker = TRUE;
	}
	obj = ui_get_object_by_name(ctx->display, "microphone") ;
	if(obj != NULL) {
		ui_remove_screen(ctx->display, obj);
	}
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "microphone");
	ui_set_text((ui_text *)screen, (uint8 *)"microphone");
	//ui_add_body(ctx->display, (panel = ui_panel_create(UI_ALIGN_VERTICAL, 96, 32)));
	
	
	//ui_add_object(panel, (btn1 = ui_button_create(UI_COLOR_BLUE, (uchar *)"Time", 1, spectral_time_btn_click)));
	//ui_add_object(panel, (btn2 = ui_button_create(UI_COLOR_BLUE, (uchar *)"Freq", 2, spectral_freq_btn_click)));
	//ui_add_object(panel, (btn3 = ui_button_create(UI_COLOR_BLUE, (uchar *)"Hanning", 3, spectral_hanning_btn_click)));
	//ui_set_align(btn1, UI_ALIGN_HORIZONTAL);
	//ui_set_size(btn1, 96, 32);
	//ui_set_position(btn1, 0, 0);
	//ui_set_align(btn2, UI_ALIGN_HORIZONTAL);
	//ui_set_size(btn2, 96, 32);
	//ui_set_position(btn2, 96, 0);
	//ui_set_align(btn3, UI_ALIGN_HORIZONTAL);
	//ui_set_size(btn3, 96, 32);
	//ui_set_position(btn3, 0, 0);
	
	
	
	series = ui_series_create_spectral("spectral", sizeof(uint16), UI_COLOR_ORANGE, UI_COLOR_LIME, 128, (void *)g_spectral_buffer);
	ui_add_body(ctx->display, (chart = (ui_chart *)ui_chart_create("spal_chart", "Spectral", series)));
	chart->area.axis[1].max = 1024;
	//((ui_chart *)chart)->area.mode = UI_CHART_MODE_RELATIVE;
	ui_set_align((ui_object *)chart, UI_ALIGN_VERTICAL);
	ui_set_size((ui_object *)chart, 280, 120);
	
	//create spacer
	//obj = ui_panel_create(UI_ALIGN_GRID, 16, 120);
	//ui_add_body(ctx->display, obj);
	obj = ui_panel_create(UI_ALIGN_HORIZONTAL, 120, 16);
	ui_add_body(ctx->display, obj);
	obj = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_SMALL, (uchar *)"Volume");
	ui_add_body(ctx->display, obj);
	//for(i=0;i<16;i++) {
	g_i_volume= 100;
		obj = ui_slider_create("", i, UI_SLIDER_HORIZONTAL, 100, graph_slider_changed);		
		ui_set_align((ui_object *)obj, UI_ALIGN_VERTICAL);
		ui_set_size((ui_object *)obj, 120, 24);
		ui_add_body(ctx->display, obj);
	//}
	//ui_set_position((ui_object *)chart, 0, 32);
	//ui_set_size((ui_object *)chart, screen->base.w, screen->base.h - 32);
	
	spaltask = os_find_task_by_name("mictask");			//spectrum analyzer task
	if(spaltask == NULL) {
		memset(g_spectral_buffer, 0, sizeof(g_spectral_buffer));		//clear performance buffer
		spaltask = os_create_task(ctx, microphone_task, "mictask", 17, 312000);
	}
	
	
	
	OS_DEBUG_EXIT();
}

static void microphone_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(microphone_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	//ui_object * instance;
	os_task * spaltask = NULL;
	//ui_textbox * textbox;
	//uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * screen;
	memset(tbuf, 0, sizeof(tbuf));
	if((screen = ui_get_screen_by_name(ctx->display, "microphone")) != NULL) {
		if(((gui_handle_p)ctx->display)->body != screen) {
			//spaltask = os_find_task_by_name("mictask");			//spectrum analyzer task
			//if(spaltask == NULL) {
			//	os_kill_task(spaltask);
			//}
			microphone_exit_click(obj, params);
		} else {
			//ui_remove_screen_unsafe(ctx->display, screen);
			//screen = ui_push_screen(ctx->display, screen);
			//ui_set_object_name(screen, "chart");
			//ui_set_text((ui_text *)screen, (uint8 *)"chart");
		}
	} else {
			microphone_show(ctx);
	}
	OS_DEBUG_EXIT();
	return;
}


void microphone_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(microphone_init);
	ui_object * calc = NULL;
	ui_object * header = handle->header;
	uint16 header_w;
	if(params != NULL && header != NULL) {
		header_w = header->rect.h / 4;
		ui_add_header(handle, (calc = ui_header_create((uint8 *)"microphoneApp", (uint8 *)"microphone", header_w, header_w, sizeof(g_microphone_appicon_png48), (uint8 *)g_microphone_appicon_png48, microphone_click)));
		if(calc != NULL) ui_set_target(calc, params);
	}
	OS_DEBUG_EXIT();
}

void microphone_app_init(gui_handle_p handle, void * params, uint8 id) {
	OS_DEBUG_ENTRY(microphone_app_init);
	ui_object * chart = NULL;
	ui_object * body = handle->body;
	ui_object * obj;
	uint16 header_w = 64;
	if(params != NULL && body != NULL) {
		ui_add_body(handle, (chart = ui_launcher_create((uint8 *)"microphoneApp", (uint8 *)"microphone", header_w, header_w, sizeof(g_microphone_appicon_png48), (uint8 *)g_microphone_appicon_png48, microphone_click)));
		if(chart != NULL) {
			obj = ui_get_object_by_name(handle, "microphone") ;
			if(obj != NULL) {
				((ui_icon_tool *)chart)->show_marker = TRUE;
			}
			ui_set_target(chart, params);
		}
	}
	memset(g_usb_audio_buffer, 0, sizeof(g_usb_audio_buffer));
	//__HAL_RCC_TIM11_CLK_ENABLE();
	//MX_TIM11_Init();
	//NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
	OS_DEBUG_EXIT();
}

#endif


