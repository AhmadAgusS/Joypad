/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <zephyr.h>
#include <sys/printk.h>
#include "cJSON.h"

static const unsigned int resolution_numbers[3][2] = {
	{1280, 720},
	{1920, 1080},
	{3840, 2160}
};

static const char *json = "{\n\
\t\"name\":\t\"Awesome 4K\",\n\
\t\"resolutions\":\t[{\n\
\t\t\t\"width\":\t1280,\n\
\t\t\t\"height\":\t720\n\
\t\t}, {\n\
\t\t\t\"width\":\t1920,\n\
\t\t\t\"height\":\t1080\n\
\t\t}, {\n\
\t\t\t\"width\":\t3840,\n\
\t\t\t\"height\":\t2160\n\
\t\t}]\n\
}";

static const char *json_without_hd = "{\n\
\t\t\"name\": \"lame monitor\",\n\
\t\t\"resolutions\":\t[{\n\
\t\t\t\"width\":\t640,\n\
\t\t\t\"height\":\t480\n\
\t\t}]\n\
}";

/* securely comparison of floating-point variables */
static cJSON_bool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

static char* create_monitor(void)
{
    char *string = NULL;
    cJSON *name = NULL;
    cJSON *resolutions = NULL;
    cJSON *resolution = NULL;
    cJSON *width = NULL;
    cJSON *height = NULL;
    size_t index = 0;

    cJSON *monitor = cJSON_CreateObject();
    if (monitor == NULL)
    {
        goto end;
    }

    name = cJSON_CreateString("Awesome 4K");
    if (name == NULL)
    {
        goto end;
    }
    /* after creation was successful, immediately add it to the monitor,
     * thereby transferring ownership of the pointer to it */
    cJSON_AddItemToObject(monitor, "name", name);

    resolutions = cJSON_CreateArray();
    if (resolutions == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(monitor, "resolutions", resolutions);

    for (index = 0; index < (sizeof(resolution_numbers) / (2 * sizeof(int))); ++index)
    {
        resolution = cJSON_CreateObject();
        if (resolution == NULL)
        {
            goto end;
        }
        cJSON_AddItemToArray(resolutions, resolution);

        width = cJSON_CreateNumber(resolution_numbers[index][0]);
        if (width == NULL)
        {
            goto end;
        }
        cJSON_AddItemToObject(resolution, "width", width);

        height = cJSON_CreateNumber(resolution_numbers[index][1]);
        if (height == NULL)
        {
            goto end;
        }
        cJSON_AddItemToObject(resolution, "height", height);
    }

    string = cJSON_Print(monitor);
    if (string == NULL)
    {
        printk("Failed to print monitor.\n");
    }

end:
    cJSON_Delete(monitor);
    return string;
}

static char *create_monitor_with_helpers(void)
{
    char *string = NULL;
    cJSON *resolutions = NULL;
    size_t index = 0;

    cJSON *monitor = cJSON_CreateObject();

    if (cJSON_AddStringToObject(monitor, "name", "Awesome 4K") == NULL)
    {
        goto end;
    }

    resolutions = cJSON_AddArrayToObject(monitor, "resolutions");
    if (resolutions == NULL)
    {
        goto end;
    }

    for (index = 0; index < (sizeof(resolution_numbers) / (2 * sizeof(int))); ++index)
    {
        cJSON *resolution = cJSON_CreateObject();

        if (cJSON_AddNumberToObject(resolution, "width", resolution_numbers[index][0]) == NULL)
        {
            goto end;
        }

        if(cJSON_AddNumberToObject(resolution, "height", resolution_numbers[index][1]) == NULL)
        {
            goto end;
        }

        cJSON_AddItemToArray(resolutions, resolution);
    }

    string = cJSON_Print(monitor);
    if (string == NULL) {
        printk("Failed to print monitor.\n");
    }

end:
    cJSON_Delete(monitor);
    return string;
}

/* return 1 if the monitor supports full hd, 0 otherwise */
static int supports_full_hd(const char * const monitor)
{
    const cJSON *resolution = NULL;
    const cJSON *resolutions = NULL;
    const cJSON *name = NULL;
    int status = 0;
    cJSON *monitor_json = cJSON_Parse(monitor);
    if (monitor_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printk("Error before: %s\n", error_ptr);
        }
        status = 0;
        goto end;
    }

    name = cJSON_GetObjectItemCaseSensitive(monitor_json, "name");
    if (cJSON_IsString(name) && (name->valuestring != NULL))
    {
        //printk("Checking monitor \"%s\"\n", name->valuestring);
    }

    resolutions = cJSON_GetObjectItemCaseSensitive(monitor_json, "resolutions");
    cJSON_ArrayForEach(resolution, resolutions)
    {
        cJSON *width = cJSON_GetObjectItemCaseSensitive(resolution, "width");
        cJSON *height = cJSON_GetObjectItemCaseSensitive(resolution, "height");

        if (!cJSON_IsNumber(width) || !cJSON_IsNumber(height))
        {
            status = 0;
            goto end;
        }

        if (compare_double(width->valuedouble, 1920) && compare_double(height->valuedouble, 1080))
        {
            status = 1;
            goto end;
        }
    }

end:
    cJSON_Delete(monitor_json);
    return status;
}

int main(void)
{
    char *monitor;
	
    /* print the version */
    printk("cJSON Version: %s\n", cJSON_Version());

    /* json create test */
	printk("create_monitor test ");
    monitor = create_monitor();
	if (strcmp(json, monitor) != 0) {
		printk("failed!\njson:\n%s\nerr:\n%s\n", json, monitor);
	} else {
		printk("success!\n");
	}
    free(monitor);

    /* json create with helpers test */
	printk("create_monitor_with_helpers test ");
    monitor = create_monitor_with_helpers();
	if (strcmp(json, monitor) != 0) {
		printk("failed!\njson:\n%s\nerr:\n%s\n", json, monitor);
	} else {
		printk("success!\n");
	}
    free(monitor);

    /* json parser test */
	printk("json parser1 test ");
    if (supports_full_hd(json) != 1) {
		printk("failed!\n");
	} else {
		printk("success!\n");
	}
	printk("json parser2 test ");
    if (supports_full_hd(json_without_hd) != 0) {
		printk("failed!\n");
	} else {
		printk("success!\n");
	}
	
	while (1) {
		k_sleep(K_MSEC(1000));
	}
}
