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

#ifndef RMD160_H
#define RMD160_H

#define RMD160_SZ 20
typedef struct rmd160 rmd160_t;
unsigned int rmd160tsize(void);
void rmd160init(rmd160_t *);
void rmd160update(rmd160_t *, const unsigned char *, unsigned int);
void rmd160final(rmd160_t *, unsigned char *); /* RMD160_SZ */
void rmd160hmac(const unsigned char *k, unsigned int kl, const unsigned char *d, unsigned int dl, unsigned char *h); /* RMD160_SZ */
void rmd160hex(const unsigned char *, char *); /* RMD160_SZ, 2 * RMD160_SZ (not null-terminated) */

#endif /* RMD160_H */
