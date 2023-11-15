#ifndef IF_PNG__H
#include "defs.h"
#include "config.h"
#include "if_file.h"

typedef struct png_handle {
	if_file * file;
	uint16 max_w;
	uint16 max_h;
	uint32 back_color;
	//current image properties
	uint16 pxfmt;
	uint16 width;
	uint16 height;
	uint16 rsv;
	//callback operation
	void * ioctx;				//callback context
	void (* read_callback)(struct png_handle * , uint16 offset, uint8 * data, uint32 length);
	void (* write_callback)(struct png_handle * , uint16 offset, uint8 * data, uint32 length);
} png_handle;

png_handle * if_png_create(if_file * file, uint16 pxfmt, uint16 max_w, uint16 max_h, uint32 back_color);
void if_png_set_callback(png_handle * handle, void * ctx, 
		void (* read_callback)(struct png_handle * , uint16, uint8 *, uint32), 
		void (* write_callback)(struct png_handle * , uint16, uint8 *, uint32)
			);
uint8 if_png_get_size(png_handle * handle, uint16 * w, uint16 * h);
uint16 if_png_decode(png_handle * handle);
void if_png_release(png_handle *);

#define IF_PNG__H
#endif
		