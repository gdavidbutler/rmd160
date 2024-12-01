/*
 * rmd160 - a small RIPEMD-160 / RIPEMD-320 implementation
 * Copyright (C) 2018-2024 G. David Butler <gdb@dbSystems.com>
 *
 * This file is part of rmd160
 *
 * rmd160 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmd160 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

typedef struct rmd320 rmd320_t;
unsigned int rmd320tsize(void);
void rmd320init(rmd320_t *);
void rmd320update(rmd320_t *, const unsigned char *, unsigned int);
void rmd320final(rmd320_t *, unsigned char *); /* 40 unsigned char (320 bits) */
void rmd320hex(const unsigned char *, char *); /* 40 unsigned char (320 bits), 80 char (not null-terminated) */
