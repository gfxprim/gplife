//SPDX-License-Identifier: GPL-2.0-or-later

/*

    Copyright (C) 2007-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <widgets/gp_widgets.h>

#include "board.h"

static struct board *board;

static struct view {
	unsigned int x_off;
	unsigned int y_off;
	unsigned int zoom;
	gp_widget *pixmap;
} view = {
	.zoom = 10,
};

static gp_widget *w, *h;

static void view_draw(gp_pixmap *pixmap, const gp_widget_render_ctx *ctx)
{
	gp_pixel bg = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);
	gp_pixel fg = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	gp_pixel fill = ctx->bg_color;

	if (gp_widgets_color_scheme_get() == GP_WIDGET_COLOR_SCHEME_DARK)
		GP_SWAP(bg, fg);

	gp_fill(pixmap, fill);

	if (view.x_off >= board->w)
		return;

	if (view.y_off >= board->h)
		return;

	unsigned int zoom = view.zoom;

	unsigned int w = GP_MIN(board->w, gp_pixmap_w(pixmap)/zoom);
	unsigned int h = GP_MIN(board->h, gp_pixmap_h(pixmap)/zoom);
	unsigned int x_off = (gp_pixmap_w(pixmap) - w * zoom)/2;
	unsigned int y_off = (gp_pixmap_h(pixmap) - h * zoom)/2;

	unsigned int x, y;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (board_cell_get(board, x+view.x_off, y+view.y_off))
				gp_fill_rect_xywh(pixmap, x_off + x*zoom, y_off + y*zoom, zoom, zoom, fg);
			else
				gp_fill_rect_xywh(pixmap, x_off + x*zoom, y_off + y*zoom, zoom, zoom, bg);
		}
	}
}

static int view_map(unsigned int vx, unsigned int vy, unsigned int *bx, unsigned int *by)
{
	unsigned int zoom = view.zoom;
	size_t pw = gp_widget_pixmap_w(view.pixmap);
	size_t ph = gp_widget_pixmap_h(view.pixmap);
	unsigned int w = GP_MIN(board->w, pw/zoom);
	unsigned int h = GP_MIN(board->h, ph/zoom);
	unsigned int x_off = (pw - w * zoom)/2;
	unsigned int y_off = (ph - h * zoom)/2;

	*bx = (vx-x_off)/zoom + view.x_off;
	*by = (vy-y_off)/zoom + view.y_off;

	if (*bx >= board->w)
		return 0;

	if (*by >= board->h)
		return 0;

	return 1;
}

#define ZOOM_MAX 100

static int view_zoom_in(unsigned int zoom)
{
	if (view.zoom >= ZOOM_MAX)
		return 0;

	if (view.zoom + zoom > ZOOM_MAX)
		view.zoom = ZOOM_MAX;
	else
		view.zoom += zoom;

	return 1;
}

static int view_zoom_out(unsigned int zoom)
{
	if (view.zoom <= 1)
		return 0;

	if (view.zoom - 1 < zoom)
		view.zoom = 1;
	else
		view.zoom -= zoom;

	return 1;
}

static void view_zoom(gp_event *ev)
{
	if (ev->val < 0) {
		if (!view_zoom_out(-ev->val))
			return;
	} else {
		if (!view_zoom_in(ev->val))
			return;
	}

	gp_widget_redraw(view.pixmap);
}

void view_click(gp_event *ev)
{
	unsigned int bx, by;

	if (!view_map(ev->st->cursor_x, ev->st->cursor_y, &bx, &by))
		return;

	board_cell_set(board, bx, by, !board_cell_get(board, bx, by));

	gp_widget_redraw(view.pixmap);
}

static int view_input(gp_widget_event *ev)
{
	switch (ev->input_ev->type) {
	case GP_EV_REL:
		switch (ev->input_ev->code) {
		case GP_EV_REL_WHEEL:
			view_zoom(ev->input_ev);
			return 1;
		break;
		}
	break;
	case GP_EV_KEY:
		if (ev->input_ev->val == GP_BTN_LEFT &&
		    ev->input_ev->code == 1) {
			view_click(ev->input_ev);
		}
	break;
	}

	return 0;
}

static int pixmap_on_event(gp_widget_event *ev)
{
	switch (ev->type) {
	case GP_WIDGET_EVENT_REDRAW:
		view_draw(ev->self->pixmap->pixmap, ev->ctx);
		return 1;
	case GP_WIDGET_EVENT_INPUT:
		return view_input(ev);
	}

	return 0;
}

int step(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	board_tick(board);
	gp_widget_redraw(view.pixmap);
	return 0;
}

uint32_t sim_tmr_callback(gp_timer *self)
{
	board_tick(board);
	gp_widget_redraw(view.pixmap);

	return 0;
}

static gp_timer sim_tmr = {
	.period = 100,
	.callback = sim_tmr_callback,
	.id = "Simulation timer",
};

int run(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widgets_timer_ins(&sim_tmr);

	return 0;
}

int stop(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widgets_timer_rem(&sim_tmr);

	return 0;
}

int randomize(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	board_randomize(board);
	gp_widget_redraw(view.pixmap);

	return 0;
}

int clear(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	board_clear(board);
	gp_widget_redraw(view.pixmap);

	return 0;
}

int zoom_in(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (view_zoom_in(1))
		gp_widget_redraw(view.pixmap);

	return 0;
}

int zoom_out(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (view_zoom_out(1))
		gp_widget_redraw(view.pixmap);

	return 0;
}

int resize(gp_widget_event *ev)
{
	struct board *new;

	if (ev->type == GP_WIDGET_EVENT_NEW) {
		gp_widget_int_min_set(ev->self, 1);
		gp_widget_int_max_set(ev->self, BOARD_SIZE_MAX);
	}

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	new = board_resize(board, gp_widget_int_val_get(w), gp_widget_int_val_get(h));
	if (new)
		board = new;

	gp_widget_redraw(view.pixmap);

	return 0;
}

int file_save(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog *file_save = gp_dialog_file_save_new(NULL, NULL);

	if (gp_dialog_run(file_save) != GP_WIDGET_DIALOG_PATH)
		return 0;

	if (board_save(board, gp_dialog_file_path(file_save))) {
		gp_dialog_msg_printf_run(GP_DIALOG_MSG_ERR,
		                          "Failed to save file",
				          "%s", strerror(errno));
	}

	gp_dialog_free(file_save);
	return 0;
}

int file_open(gp_widget_event *ev)
{
	struct board *new;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_dialog *file_open = gp_dialog_file_open_new(NULL, NULL);

	if (gp_dialog_run(file_open) != GP_WIDGET_DIALOG_PATH)
		return 0;

	new = board_load(gp_dialog_file_path(file_open));
	if (!new) {
		gp_dialog_msg_printf_run(GP_DIALOG_MSG_ERR,
		                          "Failed to load file",
				          "%s", strerror(errno));
	} else {
		board_free(board);
		board = new;

		gp_widget_int_val_set(w, board->w);
		gp_widget_int_val_set(h, board->h);

		gp_widget_redraw(view.pixmap);
	}

	gp_dialog_free(file_open);
	return 0;
}

static gp_app_info app_info = {
	.name = "gplife",
	.desc = "Conway's Game of Life",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://github.com/gfxprim/gplife",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2007-2022"},
		{}
	}
};


int main(int argc, char *argv[])
{
	gp_htable *uids;
	gp_widget *layout = gp_app_layout_load("gplife", &uids);

	gp_app_info_set(&app_info);

	view.pixmap = gp_widget_by_uid(uids, "board", GP_WIDGET_PIXMAP);

	if (view.pixmap) {
		gp_widget_on_event_set(view.pixmap, pixmap_on_event, NULL);
		gp_widget_event_unmask(view.pixmap, GP_WIDGET_EVENT_REDRAW);
		gp_widget_event_unmask(view.pixmap, GP_WIDGET_EVENT_INPUT);
	}

	w = gp_widget_by_cuid(uids, "w", GP_WIDGET_CLASS_INT);
	h = gp_widget_by_cuid(uids, "h", GP_WIDGET_CLASS_INT);

	if (!w || !h) {
		fprintf(stderr, "No size!\n");
		return 1;
	}

	board = board_new(gp_widget_int_val_get(w), gp_widget_int_val_get(h));
	if (!board) {
		fprintf(stderr, "Malloc failed!\n");
	}

	gp_htable_free(uids);

	gp_widgets_main_loop(layout, "gplife", NULL, argc, argv);

	return 0;
}
