#include "defs.h"
#include "config.h"
#include "..\inc\if_apis.h"
#include "png.h"
#include "..\inc\if_png.h"


#if NET_BUFFER_SIZE > 8960
#define gba_png_buffer	(png_bytep *)((gba_net_buffer + NET_BUFFER_SIZE) - 8960)
#else
static png_bytep gba_png_buffer[8960];
#endif

void if_png_user_error_func (png_structp a, png_const_charp p) {

}

void if_png_user_warn_func (png_structp a, png_const_charp p) {

}

void if_png_user_read_data(png_structp read_ptr, png_bytep data, png_size_t length) {
	void * handle = png_get_io_ptr(read_ptr);
	if_file_fread(data, 1, length, handle);
}

void * if_png_user_alloc(png_structp png_ptr, size_t size) {
	void * ptr = os_alloc(size);
	//if(ptr == NULL) ptr = gba_net_buffer + 0x8000;
	return ptr;
}	

void if_png_user_free(png_structp png_ptr, void * p) {
	//if(p == (gba_net_buffer + 0x8000)) return;
	os_free(p);
}

png_handle * if_png_create(if_file * file, uint16 pxfmt, uint16 max_w, uint16 max_h, uint32 back_color) {
	png_handle * handle = os_alloc(sizeof(png_handle));
	memset(handle, 0, sizeof(png_handle));
	handle->file = file;
	handle->max_w = max_w;
	handle->max_h = max_h;
	handle->pxfmt = pxfmt;
	handle->back_color = back_color;
	handle->read_callback = NULL;
	handle->write_callback = NULL;
	return handle;
}

void if_png_set_callback(png_handle * handle, void * ctx, 
		void (* read)(struct png_handle * , uint16, uint8 *, uint32), 
		void (* write)(struct png_handle * , uint16, uint8 *, uint32)
			) {
		if(handle == NULL) return;
		handle->ioctx = ctx;
		handle->read_callback = read;
		handle->write_callback = write;
}

void if_png_release(png_handle *handle) {
	os_free(handle);
}

uint8 if_png_get_size(png_handle * handle, uint16 * w, uint16 * h) {
	unsigned char header[8];
	unsigned char is_png;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_infop end_ptr = NULL;
	unsigned int img_w, img_h, img_bitdepth, img_coltype, img_inttype, index;
	int ret = -1;
	png_bytep * row_pointers[7] = { gba_png_buffer, gba_png_buffer + 1280, gba_png_buffer + 2560, gba_png_buffer + 3840, gba_png_buffer + 5120, gba_png_buffer + 6400, gba_png_buffer + 7680 };
	png_color_16 background_color16;
	png_color_8 background_color8;
	unsigned char num_pass;
	if_file * fp = handle->file;
	if(!fp) return -1;
	if(if_file_fread(header, 1, 8, fp) != 8) return -1;
	is_png = !png_sig_cmp(header, 0, 8);
	if(!is_png) return -1;
	if_file_fseek(fp, 0, SEEK_SET);
   //create new png context
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
	   (png_voidp)"Error", if_png_user_error_func, if_png_user_warn_func);
   if(!png_ptr) return -1;
	png_set_mem_fn(png_ptr, NULL, if_png_user_alloc, if_png_user_free);
   //create new info struct
   info_ptr = png_create_info_struct(png_ptr);
   if(!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return -1;
   }
   //set exception handler
 #ifdef PNG_SETJMP_SUPPORTED
   if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return ret;
   }
#endif
   //png_init_io(png_ptr, fp);
   //use custom read function
   png_set_read_fn(png_ptr, fp, if_png_user_read_data); 
   png_set_error_fn(png_ptr, (png_voidp)"Error", if_png_user_error_func, if_png_user_warn_func);
   //set maximum image size
   png_set_user_limits(png_ptr, handle->max_w, handle->max_h);
   //read info struct for metadata
   png_read_info(png_ptr, info_ptr);
   //png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_SCALE_16 | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_GRAY_TO_RGB, NULL);
   //get image properties
   w[0] = png_get_image_width(png_ptr, info_ptr);
   h[0] = png_get_image_height(png_ptr, info_ptr);
   ret = 0;
   //png_read_end(png_ptr, end_ptr);
   //release all context
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   return ret;
}

uint16 if_png_decode(png_handle * handle)
{
	unsigned char header[8];
	unsigned char is_png;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_infop end_ptr = NULL;
	unsigned int img_w, img_h, img_bitdepth, img_coltype, img_inttype, index;
	int ret = 0;
	png_bytep * row_pointers[7] = { gba_png_buffer, gba_png_buffer + 1280, gba_png_buffer + 2560, gba_png_buffer + 3840, gba_png_buffer + 5120, gba_png_buffer + 6400, gba_png_buffer + 7680 };
	png_color_16 background_color16;
	png_color_8 background_color8;
	unsigned char num_pass;
	if_file * fp = handle->file;
	//fprintf(STDERR,
   //    " test ignored because libpng was not built with read support\n");
   /* And skip this test */
   //return PNG_LIBPNG_VER < 10600 ? 0 : 77;
   //FILE * fp = fopen("E:\\Research\\libpng\\lpng1626\\projects\\visualc71\\Win32_DLL_Debug\\Test\\pngtest3.png", "rb");
	//fp = if_file_open_mem(data_in, length);
	if(!fp) return -1;
	if(if_file_fread(header, 1, 8, fp) != 8) return -1;
	is_png = !png_sig_cmp(header, 0, 8);
	if(!is_png) return -1;
	if_file_fseek(fp, 0, SEEK_SET);
   //create new png context
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
	   (png_voidp)"Error", if_png_user_error_func, if_png_user_warn_func);
   if(!png_ptr) return -1;
	png_set_mem_fn(png_ptr, NULL, if_png_user_alloc, if_png_user_free);
   //create new info struct
   info_ptr = png_create_info_struct(png_ptr);
   if(!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return -1;
   }
   //set exception handler
 #ifdef PNG_SETJMP_SUPPORTED
   if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return -1;
   }
#endif
   //png_init_io(png_ptr, fp);
   //use custom read function
   png_set_read_fn(png_ptr, fp, if_png_user_read_data); 
   png_set_error_fn(png_ptr, (png_voidp)"Error", if_png_user_error_func, if_png_user_warn_func);
   //set maximum image size
   png_set_user_limits(png_ptr, handle->max_w, handle->max_h);
   //read info struct for metadata
   png_read_info(png_ptr, info_ptr);
   //png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_SCALE_16 | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_GRAY_TO_RGB, NULL);
   //get image properties
   img_w = png_get_image_width(png_ptr, info_ptr);
   img_h = png_get_image_height(png_ptr, info_ptr);
   img_bitdepth = png_get_bit_depth(png_ptr, info_ptr);
   img_coltype = png_get_color_type(png_ptr, info_ptr);
   img_inttype = png_get_interlace_type(png_ptr, info_ptr);
   //if(handle->max_w < img_w) goto exit_png_decode;
   //if(handle->max_h < img_h) goto exit_png_decode;
   //set current image dimension
   handle->width = img_w;
   handle->height = img_h;
	//set background color
	background_color16.index = handle->back_color >> 24;
	background_color16.red = ((handle->back_color >> 16) & 0xFF);
	background_color16.green = ((handle->back_color >> 8) & 0xFF);
	background_color16.blue = (handle->back_color & 0xFF);
	background_color16.gray = 0;
   //set default background
   png_set_background(png_ptr, (png_const_color_16p)&background_color16, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1);
   if(img_inttype == PNG_INTERLACE_NONE) {
	   //row_pointers = png_get_rows(png_ptr, info_ptr);
	   for(index=0;index<img_h;index++) {
			//row_pointers[0] = row;
			//png_read_row(png_ptr, row, NULL);
		   if(index == 0x10) {
			   index = 0x10;
		   }
			png_read_rows(png_ptr, (png_bytepp)row_pointers, NULL, 1);
			//row = row_pointers[index];
			//dump_row(row);
		   if(handle->read_callback != NULL) handle->read_callback(handle, index, (uint8 *)gba_png_buffer, (3 * img_w));
	   }
   } else if(img_inttype == PNG_INTERLACE_ADAM7) {
		num_pass = png_set_interlace_handling(png_ptr);
		for(index=0;index<img_h;index++) {
			//row_pointers[0] = row;
			png_read_rows(png_ptr, (png_bytepp)row_pointers, NULL, num_pass);
			//row = row_pointers[index];
			//dump_row(row);
		   if(handle->read_callback != NULL) handle->read_callback(handle, index, (uint8 *)gba_png_buffer, (3 * img_w * num_pass));
	   }
   } else {
	   exit_png_decode:
	   ret = -1;
   }
   //png_read_end(png_ptr, end_ptr);
   //release all context
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   return ret;
}