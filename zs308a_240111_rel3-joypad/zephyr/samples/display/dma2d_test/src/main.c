/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(sample, LOG_LEVEL_INF);

#include <zephyr.h>

void test_fill_color(void);
void test_fill_mask(void);
void test_copy(void);
void test_blend(void);
void test_blend_fg(void);
void test_rotate(void);

int main(void)
{
	test_copy();
	return 0;
}
