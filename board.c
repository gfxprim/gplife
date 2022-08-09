//SPDX-License-Identifier: GPL-2.0-or-later

/*

    Copyright (C) 2007-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "board.h"

void board_clear(struct board *self)
{
	memset(self->cells, 0, (self->w + 2) * (self->h + 4));
}

struct board *board_new(unsigned int w, unsigned int h)
{
	struct board *ret;

	ret = malloc(sizeof(struct board) + (w + 2) * (h + 4));
	if (!ret)
		return NULL;

	ret->w = w;
	ret->h = h;

	board_clear(ret);

	return ret;
}

static inline size_t board_idx(struct board *self, unsigned int x, unsigned int y)
{
	return y * (self->w+2) + x;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct board *board_resize(struct board *self, unsigned int new_w, unsigned int new_h)
{
	struct board *new = board_new(new_w, new_h);

	if (!new)
		return NULL;

	unsigned int x, y;
	unsigned int w = MIN(self->w, new_w);
	unsigned int h = MIN(self->h, new_h);

	for (y = 2; y < h+2; y++) {
		for (x = 1; x < w+1; x++)
			new->cells[board_idx(new, x, y)] = self->cells[board_idx(self, x, y)];
	}

	free(self);

	return new;
}

static inline uint8_t cell_get(struct board *self, unsigned int x, unsigned int y)
{
	return self->cells[board_idx(self, x, y)];
}

static inline void cell_set(struct board *self, unsigned int x, unsigned int y, uint8_t val)
{
	self->cells[board_idx(self, x, y)] = val;
}

void board_randomize(struct board *self)
{
	unsigned int x, y;

	for (y = 2; y < self->h+2; y++) {
		for (x = 1; x < self->w+1; x++)
			cell_set(self, x, y, random() % 2);
	}
}

static uint8_t rules_lookup[2][9] = {
	/* dead cell */
	{0, 0, 0, 1, 0, 0, 0, 0, 0},
	/* alive cell */
	{0, 0, 1, 1, 0, 0, 0, 0, 0}
};

void board_tick(struct board *self)
{
	unsigned int x, y;
	uint8_t sums[self->w+2];
	uint8_t tmp[self->w+1][3];

	memset(sums, 0, sizeof(sums));
	memset(tmp, 0, sizeof(tmp));

	for (x = 1; x < self->w+1; x++)
		sums[x] = cell_get(self, x, 1) + cell_get(self, x, 2);

	for (y = 2; y < self->h+2; y++) {
		for (x = 1; x < self->w+1; x++) {
			sums[x] -= cell_get(self, x, y-2);
			sums[x] += cell_get(self, x, y+1);
		}

		for (x = 1; x < self->w+1; x++) {
			cell_set(self, x, y-2, tmp[x][(y-2)%3]);

			uint8_t cell = cell_get(self, x, y);
			uint8_t sum = sums[x-1] + sums[x] + sums[x+1] - cell;

			tmp[x][y%3] = rules_lookup[cell][sum];
		}
	}

	for (x = 1; x < self->w+1; x++) {
		cell_set(self, x, y-2, tmp[x][(y-2)%3]);
		cell_set(self, x, y-1, tmp[x][(y-1)%3]);
	}
}

void board_print(struct board *self)
{
	unsigned int x, y;

	for (y = 2; y < self->h+2; y++) {
		for (x = 1; x < self->w+1; x++) {
			printf("%s", cell_get(self, x, y) ? "()" : "--");
		}
		printf("\n");
	}

	printf("\n");
}

