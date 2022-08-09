//SPDX-License-Identifier: GPL-2.0-or-later

/*

    Copyright (C) 2007-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef BOARD_H
#define BOARD_H

struct board {
	unsigned int w;
	unsigned int h;

	uint8_t cells[];
};

struct board *board_new(unsigned int w, unsigned int h);

void board_randomize(struct board *self);

void board_clear(struct board *self);

void board_print(struct board *self);

void board_tick(struct board *self);

struct board *board_resize(struct board *self, unsigned int new_w, unsigned int new_h);

static inline uint8_t board_cell_get(struct board *self,
                                     unsigned int x, unsigned int y)
{
	return self->cells[(y+2) * (self->w+2) + (x+1)];
}

static inline void board_cell_set(struct board *self,
                                  unsigned int x, unsigned int y,
                                  uint8_t val)
{
	self->cells[(y+2) * (self->w+2) + (x+1)] = val;
}


#endif /* BOARD_H */
