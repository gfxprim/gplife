//SPDX-License-Identifier: GPL-2.0-or-later

/*

    Copyright (C) 2007-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef BOARD_H
#define BOARD_H

#include <limits.h>

#define BOARD_SIZE_MAX UINT_MAX

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

/**
 * @brief Saves board in RLE format.
 *
 * @self A borad.
 * @file_name A path to a file to save the board to.
 * @return Zero on success, non-zero otherwise.
 */
int board_save(struct board *self, const char *file_name);

/**
 * @brief Loads board from RLE format.
 *
 * @file_name A path to a file to load the board from.
 * @return A newly allocated and initialized board or NULL in a case of a
 * failure.
 */
struct board *board_load(const char *file_name);

/**
 * @brief Frees board memory.
 *
 * The call is no-op for NULL.
 *
 * @self A board.
 */
void board_free(struct board *self);

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
