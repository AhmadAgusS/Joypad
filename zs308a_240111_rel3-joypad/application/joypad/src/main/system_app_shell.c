/*
 * Copyright (c) 2019 Actions Semi Co., Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief system app shell.
 */
#include <os_common_api.h>
#include <mem_manager.h>
#include <msg_manager.h>
#include <stdio.h>
#include <string.h>
#include <shell/shell.h>
#include <stdlib.h>
#include <app_switch.h>
#include <sys_event.h>
#ifdef CONFIG_PROPERTY
#include <property_manager.h>
#endif
#ifdef CONFIG_BT_MANAGER
#include "bt_manager.h"
#endif


#ifdef CONFIG_SHELL
static int shell_cmd_switch(int argc, char *argv[])
{
	if (argc > 1)
		app_switch(argv[1], APP_SWITCH_CURR, true);

	return 0;
}

static int shell_input_key_event(const struct shell *shell, size_t argc, char *argv[])
{
	if (argv[1] != NULL) {
		uint32_t key_event;
		key_event = strtoul(argv[1], (char **) NULL, 0);
		sys_event_report_input(key_event);
	}
	return 0;
}

static int shell_set_config(const struct shell *shell, size_t argc, char *argv[])
{
#ifdef CONFIG_PROPERTY
	int ret = 0;

	if (argc < 2) {
		ret = property_set(argv[1], argv[1], 0);
	} else {
		ret = property_set(argv[1], argv[2], strlen(argv[2]));
	}

	if (ret < 0) {
		ret = -1;
	} else {
		property_flush(NULL);
	}
#endif

	SYS_LOG_INF("set config %s : %s ok\n", argv[1], argv[2]);
	return 0;
}

static int shell_dump_bt_info(const struct shell *shell, size_t argc, char *argv[])
{
#ifdef CONFIG_BT_MANAGER
	bt_manager_dump_info();
#endif
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(acts_app_cmds,
	SHELL_CMD(set_config, NULL, "set system config", shell_set_config),
	SHELL_CMD(switch, NULL, "switch <app_name>", shell_cmd_switch),
	SHELL_CMD(input, NULL, "input key event", shell_input_key_event),
	SHELL_CMD(btinfo, NULL, "dump bt info", shell_dump_bt_info),
	SHELL_SUBCMD_SET_END
);

static int cmd_acts_app(const struct shell *shell, size_t argc, char **argv)
{
	if (argc == 1) {
		shell_help(shell);
		return SHELL_CMD_HELP_PRINTED;
	}

	shell_error(shell, "%s unknown parameter: %s", argv[0], argv[1]);

	return -EINVAL;
}

SHELL_CMD_REGISTER(app, &acts_app_cmds, "Application shell commands", cmd_acts_app);
#endif