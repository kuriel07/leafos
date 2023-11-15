#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_toolbox.h"
#include "..\inc\ui_resources.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "jpeglib.h"
#include <setjmp.h>

typedef struct render_context {
	gui_handle_p display;
	ui_rect * rect;
} render_context;

struct jpg_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;        /* for return to caller */
};
typedef struct jpg_error_mgr *jpg_error_ptr;

METHODDEF(void)
jpg_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  jpg_error_ptr myerr = (jpg_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

static void ui_png_render_callback_center(struct png_handle * handle, uint16 offset, uint8 * data, uint32 length) {
	OS_DEBUG_ENTRY(ui_png_render_callback_center);
	render_context * rctx = (render_context *)handle->ioctx;
	gui_handle_p display = rctx->display;
	uint8 * dtptr;
	uint32 i,j;
	uint8 r,g,b;
	uint16 width = handle->width;
	uint16 height = handle->height;
	uint16 x = rctx->rect->x;
	uint16 y = rctx->rect->y;
	//add some offset to put rendered image in the middle
	if(rctx->rect->w > handle->width) x += (rctx->rect->w - handle->width) / 2;
	if(rctx->rect->h > handle->height) y += (rctx->rect->h - handle->height) / 2;
	for(i=0;i<length&& offset < rctx->rect->h; offset++) {
		//set display area
		display->set_area(display, x, y + offset, handle->width, 1);
		display->begin_burst(display);
		switch(handle->pxfmt) {
			case UI_RGB332:
				
				break;
			case UI_RGB565:
				for(j=0;j<handle->width && j<rctx->rect->w;j++) {
					dtptr = data + (j * 3);
					display->put_pixel(display, IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]));
				}
				break;
			case UI_RGB888:
				for(j=0;j<handle->width && j<rctx->rect->w;j++) {
					dtptr = data + (j * 3);
					display->put_pixel(display, IF_COLOR_RGB888(dtptr[0], dtptr[1], dtptr[2]));
				}
				break;
			case UI_ARGB8888:
				for(j=0;j<handle->width && j<rctx->rect->w;j++) {
					dtptr = data + (j * 3);
					display->put_pixel(display, IF_COLOR_ARGB8888(dtptr[0], dtptr[1], dtptr[2]));
				}
				break;
		}
		//advance data pointer
		i += (handle->width * 3);
		data += (handle->width * 3);
	}
	OS_DEBUG_EXIT();
}

static void ui_png_render_callback_fill(struct png_handle * handle, uint16 offset, uint8 * data, uint32 length) {
	OS_DEBUG_ENTRY(ui_png_render_callback_fill);
	render_context * rctx = (render_context *)handle->ioctx;
	gui_handle_p display = rctx->display;
	uint8 * dtptr;
	int16 i,j,k;
	uint8 r,g,b;
	uint32 color;
	uint16 width = handle->width;
	uint16 height = handle->height;
	uint16 x = rctx->rect->x;
	uint16 y = rctx->rect->y;
	uint16 half_h = height / 2;
	uint16 half_w = width / 2;
	uint8 * prev_line = data;
	int16 ygap = (rctx->rect->h - height);
	int16 wgap = (rctx->rect->w - width);
	uint16 start_w = half_w;
	uint16 end_w = width;
	if(wgap < 0) { 
		half_w += ((wgap / 2) );		//wgap is negative, either use abs(x) or reverse operation
		start_w -= ((wgap / 2) );		//wgap is negative, either use abs(x) or reverse operation
		end_w = width;
	}
	//add some offset to put rendered image in the middle (fill half first)
	for(i=0;i<length&& offset < half_h; offset++) {
		//set display area
		display->set_area(display, x, y + offset, rctx->rect->w, 1);
		display->begin_burst(display);
		for(j=0;j<width && j < half_w;j++) {
			dtptr = data + (j * 3);
			display->put_pixel(display, IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]));
		}
		//stretch horizontal
		color = IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]);
		for(k=0;k<wgap;k++) {
			display->put_pixel(display, color);
		}
		for(j=start_w;j<end_w;j++) {
			dtptr = data + (j * 3);
			display->put_pixel(display, IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]));
		}
		//advance data pointer
		i += (handle->width * 3);
		prev_line = data;
		data += (handle->width * 3);
	}
	//fill gap (if exist)
	if(offset == half_h && ygap > 0) {
		for(i=0;i<length&& offset <(half_h + ygap + 1); offset++) {
			//set display area
			display->set_area(display, x, y + offset, rctx->rect->w, ygap);
			display->begin_burst(display);
			for(j=0;j<width && j < half_w;j++) {
				dtptr = prev_line + (j * 3);
				display->put_pixel(display, IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]));
			}
			//stretch horizontal
			color = IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]);
			for(k=0;k<wgap;k++) {
				display->put_pixel(display, color);
			}
			for(j=start_w;j<end_w;j++) {
				dtptr = prev_line + (j * 3);
				display->put_pixel(display, IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]));
			}
		}
	} else {
		if(ygap < 0) ygap = 0;
		//fill second half
		for(i=0;i<length&& offset >= half_h && offset <rctx->rect->h; offset++) {
			//set display area
			display->set_area(display, x, y + ygap + offset, rctx->rect->w, 1);
			display->begin_burst(display);
			for(j=0;j<width && j < half_w;j++) {
				dtptr = data + (j * 3);
				display->put_pixel(display, IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]));
			}
			//stretch horizontal
			color = IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]);
			for(k=0;k<wgap;k++) {
				display->put_pixel(display, color);
			}
			for(j=start_w;j<end_w;j++) {
				dtptr = data + (j * 3);
				display->put_pixel(display, IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]));
			}
			//advance data pointer
			i += (handle->width * 3);
			data += (handle->width * 3);
		}
	}
	OS_DEBUG_EXIT();
}

uint8 ui_image_info(ui_resource * image, uint16 pxfmt, uint16 * w, uint16 * h) {
	OS_DEBUG_ENTRY(ui_image_info);
	uint16 xx, yy;
	uint8 ww, hh, num_pal;
	uint8 i, j, c,d;
	uint16 offset;
	uint8 ret = -1;
	uint32 color, c1, c2, c3, c4, c5;
	uint8 index;
	if_file * file;
	png_handle * hpng;
	uint32 csr;
	uint8 * bitmap = image->buffer;
	uint32 bmpsize = image->size;
	struct jpeg_decompress_struct cinfo;
	struct jpg_error_mgr jerr;
	JSAMPARRAY buffer;            /* Output row buffer */
	int row_stride;               /* physical row width in output buffer */
	render_context rctx;
	uint16 xsta, ysta;
	uint8 * objmp = bitmap;
	uint8 * dtptr;
	
	if(objmp[0] == 0x6C) {
		if(objmp[1] > 0x01) goto exit_image_info;		//check version
		if(objmp[5] != 0x02) goto exit_image_info;		//unsupported bit depth (only 2 bit)
		
		w[0] = objmp[6];       		//icon width
		h[0] = objmp[7];      		//icon height
		ret = 0;
	} else if(memcmp(objmp, "\x89PNG", 4) == 0) {
		file = if_file_open_mem(objmp, bmpsize);
		if(file == NULL) goto exit_image_info;
		hpng = if_png_create(file, pxfmt, 64, 64, UI_COLOR_WHITE);
		if_png_set_callback(hpng, &rctx, ui_png_render_callback_center, NULL);
		ret = if_png_get_size(hpng, w, h);
		if_png_release(hpng);
		if_file_fclose(file);
#if 1
	//support for JPG image based on libjpg
	} else if(memcmp(objmp + 6, "JFIF", 4) == 0) {		//check for app0type
		memset(&cinfo, 0, sizeof(struct jpeg_decompress_struct));
		memset(&jerr, 0, sizeof(struct jpg_error_mgr));
		csr = os_enter_critical();
		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = jpg_error_exit;
		jpeg_create_decompress(&cinfo);
		jpeg_mem_src(&cinfo, objmp, bmpsize);
		(void) jpeg_read_header(&cinfo, TRUE);
		w[0] = cinfo.image_width;
		h[0] = cinfo.image_height;
		ret = 0;
		jpeg_destroy_decompress(&cinfo);
		os_exit_critical(csr);
#endif
	}
	exit_image_info:
	OS_DEBUG_EXIT();
	return ret;
}


void ui_image_render(gui_handle_p display, ui_rect * obj, uint8 * bitmap, uint16 bmpsize, uint8 align) {
	ui_resource res;
	res.buffer = bitmap;
	res.size = bmpsize;
	ui_resource_render(display, obj, &res, align);
}

void ui_resource_render(gui_handle_p display, ui_rect * obj, ui_resource * image, uint8 align) {
	OS_DEBUG_ENTRY(ui_image_render);
	uint16 xx, yy;
	uint8 ww, hh, num_pal;
	uint8 * bitmap = image->buffer;
	uint32 bmpsize = image->size;
	uint8 i, j, c,d;
	uint32 palletes[8];
	uint16 offset;
	uint32 color, c1, c2, c3, c4, c5;
	uint8 index;
	if_file * file;
	png_handle * hpng;
	uint32 csr;
	struct jpeg_decompress_struct cinfo;
	struct jpg_error_mgr jerr;
	JSAMPARRAY buffer;            /* Output row buffer */
	int row_stride;               /* physical row width in output buffer */
	render_context rctx;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 cy = ((ui_rect *)obj)->y, cx = ((ui_rect *)obj)->x;
	uint16 xsta, ysta;
	uint8 * objmp = bitmap;
	uint8 * dtptr;
	uint16 img_w, img_h;
	//return;
#if 0
	ui_image_info(objmp, bmpsize, &img_w, &img_h);
	if(img_w > obj->w) return;		//image out of bounds
	if(img_h > obj->h) return;		//image out of bounds
	//set image alignment
	if((align & UI_IMAGE_ALIGN_FILL) == 0) {
		if(align & UI_IMAGE_ALIGN_BOTTOM) {
			obj->y += (obj->h-img_h);
			if(align & UI_IMAGE_ALIGN_CENTER) obj->y -= ((obj->h-img_h) >> 1);
		} else {
			if(align & UI_IMAGE_ALIGN_CENTER) obj->y += ((obj->h-img_h) >> 1);
		}
		obj->h = img_h;
		if(align & UI_IMAGE_ALIGN_RIGHT) {
			obj->x += (obj->w-img_w);
			if(align & UI_IMAGE_ALIGN_CENTER) obj->x -= ((obj->w-img_w) >> 1);
		} else {
			if(align & UI_IMAGE_ALIGN_CENTER) obj->x += ((obj->w-img_w) >> 1);
		}
		obj->w = img_w;
	}
#endif
	if(bitmap == NULL) goto exit_image_render;
	if(bmpsize == 0) goto exit_image_render;
	if(objmp[0] == 0x6C) {
		if(objmp[1] > 0x01) goto exit_image_render;		//check version
		if(objmp[5] != 0x02) goto exit_image_render;		//unsupported bit depth (only 2 bit)
		
		ww = objmp[6];       		//icon width
		hh = objmp[7];      		//icon height
		num_pal = objmp[8];		//number of palletes
		//should be optimized for aligned memory area such as ARMv4 (which doesn't memory must be aligned within 4 bytes when accessing uint32)
		offset = 9;
		palletes[0] = ((ui_object *)obj)->backcolor;
		for (i = 0; i < num_pal; i++)
		{
			color = (objmp[offset + 1] << 16) |  (objmp[offset + 2] << 8) |  objmp[offset + 3];
			palletes[i + 1] = color;		//shoule be swap when little endian used
			offset += 4;
		}
		xsta = 0;
		ysta = 0;
		if(align & UI_IMAGE_ALIGN_BOTTOM) {
			if(h > hh) cy = (y + h) -hh;
			else ysta = hh-h;
		} else {
			if(h < hh) hh = h;
		}
		if(align & UI_IMAGE_ALIGN_RIGHT) {
			if(w > ww) cx = (x + w) -ww;
			else xsta = ww-w;
		} else {
			if(w < ww) ww = w;
		}
		display->set_area(display, cx, cy, ww, hh);
		display->begin_burst(display);
		for (i=ysta;i < hh; i++)
		{
			for (j=xsta; j < ww; j += 4)
			{
				//pixel data
				c = objmp[offset];
				if(j < (w - 1)) d = objmp[offset + 1];
				else d = c;
				c5 = palletes[d & 0x03];
				//color data
				index = c & 0x03;
				c1 = palletes[index];
				//if(index != 0) display->fill_area(display, color, display->set_area(display, cx + j, cy + i, 1, 1));
				index = (c>>2) & 0x03;
				c2 = palletes[index];
				//if(index != 0) display->fill_area(display, color, display->set_area(display, cx + j + 1, cy + i, 1, 1));
				index = (c>>4) & 0x03;
				c3 = palletes[index];
				//if(index != 0) display->fill_area(display, color, display->set_area(display, cx + j + 2, cy + i, 1, 1));
				index = (c>>6) & 0x03;
				c4 = palletes[index];
				//if(index != 0) display->fill_area(display, color, display->set_area(display, cx + j + 3, cy + i, 1, 1));
				c1 = (c1 >> 1) & 0x7F7F7F7f;
				c2 = (c2 >> 1) & 0x7F7F7F7f;
				c3 = (c3 >> 1) & 0x7F7F7F7f;
				c4 = (c4 >> 1) & 0x7F7F7F7f;
				c5 = (c5 >> 1) & 0x7F7F7F7f;
				//antialias effect
				display->fill_area(display, c1 + c2, display->set_area(display, cx + j, cy + i, 1, 1));
				display->fill_area(display, c2 + c3, display->set_area(display, cx + j + 1, cy + i, 1, 1));
				display->fill_area(display, c3 + c4, display->set_area(display, cx + j + 2, cy + i, 1, 1));
				display->fill_area(display, c4 + c5, display->set_area(display, cx + j + 3, cy + i, 1, 1));
				offset++;
			}
		}
	} else if(memcmp(objmp, "\x89PNG", 4) == 0) {
		file = if_file_open_mem(objmp, bmpsize);
		if(file == NULL) goto exit_image_render;
		if(w < 24) w = 24;		//set minimum width to 24 pixel
		if(h < 24) h = 24;		//set minimum height to 24 pixel
		hpng = if_png_create(file, display->fb_pxfmt, w, h, ((ui_object *)obj)->backcolor);
		if(hpng != NULL) {
			rctx.display = display;
			rctx.rect = obj;
			if(align & UI_IMAGE_ALIGN_FILL) {
				if_png_set_callback(hpng, &rctx, ui_png_render_callback_fill, NULL);
			} else {
				if_png_set_callback(hpng, &rctx, ui_png_render_callback_center, NULL);
			}
			if_png_decode(hpng);
			if_png_release(hpng);
		}
		if_file_fclose(file);
#if 1
	//support for JPG image based on libjpg
	} else if(memcmp(objmp + 6, "JFIF", 4) == 0) {		//check for app0type
		memset(&cinfo, 0, sizeof(struct jpeg_decompress_struct));
		memset(&jerr, 0, sizeof(struct jpg_error_mgr));
		csr = os_enter_critical();
		cinfo.err = jpeg_std_error(&jerr.pub);
		jerr.pub.error_exit = jpg_error_exit;
		//if (setjmp(jerr.setjmp_buffer)) {
		//	jpeg_destroy_decompress(&cinfo);
		//	goto exit_image_render;
		//}
		jpeg_create_decompress(&cinfo);
		jpeg_mem_src(&cinfo, objmp, bmpsize);
		(void) jpeg_read_header(&cinfo, TRUE);
		(void) jpeg_start_decompress(&cinfo);
		row_stride = cinfo.output_width * cinfo.output_components;
		buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
		while (cinfo.output_scanline < cinfo.output_height) {
			(void) jpeg_read_scanlines(&cinfo, buffer, 1);
			display->set_area(display, x, y + cinfo.output_scanline, cinfo.output_width, 1);
			display->begin_burst(display);
			for(j=0;j<cinfo.output_width && j<w;j++) {
				dtptr = (uint8 *)buffer[0] + (j * 3);
				display->put_pixel(display, IF_COLOR_RGB(dtptr[0], dtptr[1], dtptr[2]));
			}
		}
		cinfo.mem->free_pool((j_common_ptr) &cinfo, JPOOL_IMAGE);
		(void) jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		os_exit_critical(csr);
#endif
	}
	exit_image_render:
	OS_DEBUG_EXIT();
	return;
}

