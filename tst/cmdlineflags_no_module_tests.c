/* SPDX-License-Identifier: MIT */
/**
 * @file cmdlineflags_no_module_tests.c
 *
 * @author Lukasz Wiecaszek <lukasz.wiecaszek@gmail.com>
 */

/*===========================================================================*\
 * system header files
\*===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMPARE_WITH_GETOPT

#if defined(COMPARE_WITH_GETOPT)
#include <getopt.h>
#endif

/*===========================================================================*\
 * project header files
\*===========================================================================*/
#include <cmdlineflags.h>

/*===========================================================================*\
 * preprocessor #define constants and macros
\*===========================================================================*/

/*===========================================================================*\
 * local type definitions
\*===========================================================================*/

/*===========================================================================*\
 * global (external linkage) object definitions
\*===========================================================================*/

/*===========================================================================*\
 * local (internal linkage) function declarations
\*===========================================================================*/
static int handle_expected_cnt_options(const struct cmdlineflags_option* option, const char* argument);

static int v_option_expected_cnt = 0;
CMDLINEFLAGS_DEFINE(CMDLINEFLAGS_GLOBAL_MODULE, i, expected_v_cnt, \
   CMDLINEFLAGS_REQUIRED_ARGUMENT, handle_expected_cnt_options, "sets expected v counter");

static int c_option_expected_cnt = 0;
CMDLINEFLAGS_DEFINE(CMDLINEFLAGS_GLOBAL_MODULE, j, expected_c_cnt, \
   CMDLINEFLAGS_REQUIRED_ARGUMENT, handle_expected_cnt_options, "sets expected c counter");

static int v_option_actual_cnt = 0;
static int handle_v_option(const struct cmdlineflags_option* option);

CMDLINEFLAGS_DEFINE_SHORT_OPTION(CMDLINEFLAGS_GLOBAL_MODULE, v, \
   CMDLINEFLAGS_NO_ARGUMENT, handle_v_option, "prints version information");

CMDLINEFLAGS_DEFINE_LONG_OPTION(CMDLINEFLAGS_GLOBAL_MODULE, version, \
   CMDLINEFLAGS_NO_ARGUMENT, handle_v_option, "prints version information");

static int c_option_actual_cnt = 0;
static int handle_c_option(const struct cmdlineflags_option* option, const char* argument);

CMDLINEFLAGS_DEFINE_SHORT_OPTION(CMDLINEFLAGS_GLOBAL_MODULE, c, \
   CMDLINEFLAGS_REQUIRED_ARGUMENT, handle_c_option, "configuration file");

CMDLINEFLAGS_DEFINE_LONG_OPTION(CMDLINEFLAGS_GLOBAL_MODULE, configuration, \
   CMDLINEFLAGS_REQUIRED_ARGUMENT, handle_c_option, "configuration file");

/*===========================================================================*\
 * local (internal linkage) object definitions
\*===========================================================================*/

/*===========================================================================*\
 * static inline (internal linkage) function definitions
\*===========================================================================*/

/*===========================================================================*\
 * global (external linkage) function definitions
\*===========================================================================*/
int main(int argc, char* argv[])
{
    int retval = -1;

    do {
        int status;
        int index;
        struct cmdlineflags_cfg cfg;
        char help_message[1024];

        status = cmdlineflags_get_help_msg(NULL, 0);
        if (status < 0)
            break;

        status = cmdlineflags_get_help_msg(help_message, sizeof(help_message));
        if (status < 0)
            break;

        status = cmdlineflags_get_cfg(NULL);
        if (status == 0)
           break;

        status = cmdlineflags_set_cfg(NULL);
        if (status == 0)
           break;

        status = cmdlineflags_get_cfg(&cfg);
        if (status != 0)
           break;

        status = cmdlineflags_set_cfg(&cfg);
        if (status != 0)
           break;

        index = cmdlineflags_parse(argc, argv);
        fprintf(stdout, "cmdlineflags_parse: first nonoption argument: %d\n", index);
        fprintf(stdout, "cmdlineflags_parse: v_option_actual_cnt: %d\n", v_option_actual_cnt);
        fprintf(stdout, "cmdlineflags_parse: c_option_actual_cnt: %d\n", c_option_actual_cnt);

#if defined(COMPARE_WITH_GETOPT)
        static const struct option long_options[] = {
            {"expected_v_cnt", required_argument, 0, 'i'},
            {"expected_c_cnt", required_argument, 0, 'j'},
            {"version",        no_argument,       0, 'v'},
            {"configuration",  required_argument, 0, 'c'},
            {0, 0, 0, 0}
        };

        for (;;) {
            int c = getopt_long(argc, argv, "+i:j:vc:", long_options, 0);
            if (-1 == c)
                break;

            fprintf(stdout, "getopt_long: '%c', optarg: '%s'\n", c, optarg);
        }

        fprintf(stdout, "getopt_long: first nonoption argument: %d\n", optind);
#endif

        if (v_option_actual_cnt != v_option_expected_cnt)
            break;

        if (c_option_actual_cnt != c_option_expected_cnt)
            break;

        retval = 0;
    } while (0);

    return retval;
}

/*===========================================================================*\
 * local (internal linkage) function definitions
\*===========================================================================*/
static int handle_expected_cnt_options(const struct cmdlineflags_option* option, const char* argument)
{
    int retval = -1;

    switch (option->type)
    {
        case CMDLINEFLAGS_SHORTOPTION:
        {
            if (option->u.shortoption == 'i') {
                v_option_expected_cnt = atoi(argument);
                retval = 0;
            }
            else
            if (option->u.shortoption == 'j') {
                c_option_expected_cnt = atoi(argument);
                retval = 0;
            }
            else {
                /* do nothing */
            }

            break;
        }
        case CMDLINEFLAGS_LONGOPTION:
        {
            if (!strcmp(option->u.longoption, "expected_v_cnt")) {
                v_option_expected_cnt = atoi(argument);
                retval = 0;
            }
            else
            if (!strcmp(option->u.longoption, "expected_c_cnt")) {
                c_option_expected_cnt = atoi(argument);
                retval = 0;
            }
            else {
                /* do nothing */
            }

            break;
        }
        default:
        {
            break;
        }
    }

    return retval;
}

static int handle_v_option(const struct cmdlineflags_option* option)
{
    fprintf(stdout, "%s()\n", __func__);
    v_option_actual_cnt++;
    return 0;
}

static int handle_c_option(const struct cmdlineflags_option* option, const char* argument)
{
    fprintf(stdout, "%s(%s)\n", __func__, argument);
    c_option_actual_cnt++;
    return (argument != NULL) ? 0 : ~0;
}

