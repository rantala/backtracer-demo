/*
 * This file is part of Functracer.
 *
 * Copyright (C) 2008,2010 by Nokia Corporation
 *
 * Contact: Eero Tamminen <eero.tamminen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <stdlib.h>
#include <stdio.h> /* XXX DEBUG */
#include <string.h> /* XXX DEBUG */
#include <sys/types.h>
#include <libunwind-ptrace.h>
#include <limits.h>

#include "backtrace.h"

struct bt_data {
	unw_addr_space_t as;
	struct UPT_info *ui;
};

struct bt_data *bt_init(pid_t pid)
{
	struct bt_data *btd;

	btd = malloc(sizeof(struct bt_data));
	if (!btd)
		abort();

	btd->as = unw_create_addr_space(&_UPT_accessors, 0);
	if (!btd->as)
		abort();

	unw_set_caching_policy (btd->as, UNW_CACHE_GLOBAL);
	btd->ui = _UPT_create(pid);

	return btd;
}

int bt_backtrace(struct bt_data *btd, void** frames, char **buffer, int size, int resolve_name)
{
	unw_cursor_t c;
	unw_word_t ip, off;
	int n = 0, ret;
	char buf[512] = "in ";

	if (size == 0)
		return 0;

	if ((ret = unw_init_remote(&c, btd->as, btd->ui)) < 0) {
		printf("unw_init_remote() failed.\n");
		//debug(1, "bt_backtrace(): unw_init_remote() failed, ret=%d", ret);
		return -1;
	}

	do {
		if ((ret = unw_get_reg(&c, UNW_REG_IP, &ip)) < 0) {
			//debug(1, "bt_backtrace(): unw_get_reg() failed, ret=%d", ret);
			return -1;
		}
		frames[n] = (void*)ip;

		if (resolve_name) {
			char* ptr = buf;
			ret = unw_get_proc_name(&c, buf + 3, sizeof(buf) - 3, &off);
			if (ret < 0) {
				ptr = buf + 3;
				strcpy(ptr, "<undefined>");
			}
			else if (off) {
				size_t len = strlen(buf);
				/* Reserve the last 64 bytes for the offset */
				if (len >= sizeof(buf) - 64)
					len = sizeof(buf) - 64;
				sprintf(buf + len, "+0x%lx", (unsigned long)off);
			}
			buffer[n] = strdup(ptr);
		}
		n++;
		if ((ret = unw_step(&c)) < 0) {
			//debug(1, "bt_backtrace(): unw_step() failed, ret=%d", ret);
			return -1;
		}
	} while (ret > 0 && n < size);

	return n;
}

void bt_finish(struct bt_data *btd)
{
	_UPT_destroy(btd->ui);
	unw_destroy_addr_space(btd->as);
	free(btd);
}
