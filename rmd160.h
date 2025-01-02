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

typedef struct rmd160 rmd160_t;
unsigned int rmd160tsize(void);
void rmd160init(rmd160_t *);
void rmd160update(rmd160_t *, const unsigned char *, unsigned int);
void rmd160final(rmd160_t *, unsigned char *); /* 20 unsigned char (160 bits) */
void rmd160hmac(const unsigned char *k, unsigned int kl, const unsigned char *d, unsigned int dl, unsigned char *h); /* 20 unsigned char (160 bits) */
void rmd160hex(const unsigned char *, char *); /* 20 unsigned char (160 bits), 40 char (not null-terminated) */
