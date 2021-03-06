/* SPDX-License-Identifier: MIT */
/**
 * @file cmdlineflags.c
 *
 * @author Lukasz Wiecaszek <lukasz.wiecaszek@gmail.com>
 */

/*===========================================================================*\
 * system header files
\*===========================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*===========================================================================*\
 * project header files
\*===========================================================================*/
#include <cmdlineflags.h>

/*===========================================================================*\
 * preprocessor #define constants and macros
\*===========================================================================*/
#define PRINT_ERROR(...) \
    if (cmdlineflags_cfg.emit_debug_messages) fprintf(stderr, __VA_ARGS__)

/*===========================================================================*\
 * local type definitions
\*===========================================================================*/
typedef enum {false, true} bool;

/*===========================================================================*\
 * global (external linkage) object definitions
\*===========================================================================*/

/*===========================================================================*\
 * local (internal linkage) function declarations
\*===========================================================================*/
static int cmdlineflags_compare(const struct cmdlineflags* l, const struct cmdlineflags* r);
static void cmdlinefags_add(const struct cmdlineflags** cmdlineflags, size_t n_options, const struct cmdlineflags* it);
static int cmdlinefags_build_help_msg(const struct cmdlineflags** cmdlineflags, size_t n_options, char* msg, unsigned size);
static const struct cmdlineflags** cmdlineflags_combine_options(size_t* n_options);

/*===========================================================================*\
 * local (internal linkage) object definitions
\*===========================================================================*/
static const struct cmdlineflags cmdlineflags_short_options_0
    __attribute__((__section__(CMDLINEFLAGS_SHORTOPTIONS_SECTION_NAME)))
    __attribute__((__used__))
    __attribute__((aligned(CMDLINEFLAGS_ALIGN))) = {0};

static const struct cmdlineflags cmdlineflags_longoptions_0
    __attribute__((__section__(CMDLINEFLAGS_LONGOPTIONS_SECTION_NAME)))
    __attribute__((__used__))
    __attribute__((aligned(CMDLINEFLAGS_ALIGN))) = {0};

static struct cmdlineflags_cfg cmdlineflags_cfg = {
    .emit_debug_messages = 1,
};

/*===========================================================================*\
 * static inline (internal linkage) function definitions
\*===========================================================================*/
static inline const struct cmdlineflags*
    cmdlineflags_get_shortoption(const char* module, char shortoption)
{
    const struct cmdlineflags* const cmdlineflags_start_addr = &CMDLINEFLAGS_SHORTOPTIONS_SECTION_START;
    const struct cmdlineflags* const cmdlineflags_end_addr = &CMDLINEFLAGS_SHORTOPTIONS_SECTION_END;
    const struct cmdlineflags* it;

    if (module == NULL)
        module = CMDLINEFLAGS_XSTR(CMDLINEFLAGS_NO_MODULE);

    for (it = cmdlineflags_start_addr; it < cmdlineflags_end_addr; ++it)
        if ((it->module != NULL) && (!strcmp(it->module, module)))
            if (it->option.u.shortoption == shortoption)
                return it;

    return NULL;
}

static inline const struct cmdlineflags*
    cmdlineflags_get_longoption(const char* module, const char* longoption)
{
    const struct cmdlineflags* const cmdlineflags_start_addr = &CMDLINEFLAGS_LONGOPTIONS_SECTION_START;
    const struct cmdlineflags* const cmdlineflags_end_addr = &CMDLINEFLAGS_LONGOPTIONS_SECTION_END;
    const struct cmdlineflags* it;

    if (module == NULL)
        module = CMDLINEFLAGS_XSTR(CMDLINEFLAGS_NO_MODULE);

    for (it = cmdlineflags_start_addr; it < cmdlineflags_end_addr; ++it)
        if ((it->module != NULL) && (!strcmp(it->module, module)))
            if (!strcmp(it->option.u.longoption, longoption))
                return it;

    return NULL;
}

static inline bool cmdlineflags_is_nonoption(const char* option)
{
    return (option[0] != '-') || (option[1] == '\0');
}

static inline bool cmdlineflags_is_longoption(const char* option)
{
    return (option[1] == '-');
}

/*===========================================================================*\
 * global (external linkage) function definitions
\*===========================================================================*/
int cmdlineflags_parse(int argc, char * const argv[])
{
    int argv_index;
    const char* module;

    if (argc < 1)
        return CMDLINEFLAGS_FAILURE;

    module = NULL;

    /* Start with the ARGV[1] and scan until first non-option argument */
    for (argv_index = 1; argv_index < argc; argv_index++) {
        const char* arg = argv[argv_index];

        if (arg == NULL)
            return CMDLINEFLAGS_FAILURE;

        if (!strcmp(arg, "--"))
            return argv_index + 1; /* The special ARGV-element '--' means end of options */

        if (cmdlineflags_is_nonoption(arg)) {
            if (!module) /* First non-option is treated as a module option */
                module = arg;
            else
                return argv_index;
        }
        else {
            bool is_longoption = cmdlineflags_is_longoption(arg);
            if (is_longoption) {
                const char* longoption_start = arg + 2; /* Skip the initial '--' */
                const char* longoption_end;
                const char* longoption;
                const struct cmdlineflags* cmdlineflags;

                for (longoption_end = longoption_start;
                    *longoption_end != '\0' && *longoption_end != '='; longoption_end++);

                longoption = strndup(longoption_start, longoption_end - longoption_start);
                if (!longoption)
                    return CMDLINEFLAGS_FAILURE;

                cmdlineflags = cmdlineflags_get_longoption(module, longoption);
                if (cmdlineflags) {
                    if (cmdlineflags->flags == CMDLINEFLAGS_NO_ARGUMENT) {
                        if (*longoption_end == '\0') {
                            if (cmdlineflags->u.f0(&cmdlineflags->option) != 0)
                                return free((void*)longoption), argv_index + 1;
                        }
                        else
                            PRINT_ERROR("%s: option '--%s' doesn't allow an argument\n", argv[0], longoption);
                    }
                    else {
                        if (*longoption_end != '\0') {
                            if (cmdlineflags->u.f1(&cmdlineflags->option, longoption_end + 1) != 0)
                                return free((void*)longoption), argv_index + 1;
                        }
                        else
                        if ((argv_index + 1) < argc) {
                            if (cmdlineflags->u.f1(&cmdlineflags->option, argv[++argv_index]) != 0)
                                return free((void*)longoption), argv_index + 1;
                        }
                        else
                            PRINT_ERROR("%s: option '%s' requires an argument\n", argv[0], longoption);
                    }
                }
                else
                    PRINT_ERROR("%s: unrecognized option '--%s'\n", argv[0], longoption);

                free((void*)longoption);
            }
            else {
                const char* nextchar = arg + 1; /* Skip the initial '-' */
                const struct cmdlineflags* cmdlineflags;
                char c;

                while ((c = *nextchar++) != '\0') {
                    cmdlineflags = cmdlineflags_get_shortoption(module, c);
                    if (cmdlineflags) {
                        if (cmdlineflags->flags == CMDLINEFLAGS_NO_ARGUMENT) {
                            if (cmdlineflags->u.f0(&cmdlineflags->option) != 0)
                                return argv_index + 1;
                        }
                        else {
                            /* This is an option that requires an argument. */
                            if (*nextchar != '\0') {
                                if (cmdlineflags->u.f1(&cmdlineflags->option, nextchar) != 0)
                                    return argv_index + 1;
                                /* If we end this ARGV-element by taking the rest as an argument,
                                   we must advance to the next element now. */
                                break;
                            }
                            else
                            if ((argv_index + 1) < argc) {
                                if (cmdlineflags->u.f1(&cmdlineflags->option, argv[++argv_index]) != 0)
                                    return argv_index + 1;
                            }
                            else
                                PRINT_ERROR("%s: option requires an argument -- '%c'\n", argv[0], c);
                        }
                    }
                    else
                        PRINT_ERROR("%s: invalid option -- '%c'\n", argv[0], c);
                }
            }
        }
    }

    return argv_index;
}

int cmdlineflags_get_help_msg(char* msg, unsigned size)
{
    int n;
    size_t n_options;
    char null_msg_buffer[1];
    const struct cmdlineflags** cmdlineflags;

    if (msg == NULL) {
        msg = null_msg_buffer;
    }

    cmdlineflags = cmdlineflags_combine_options(&n_options);
    if (cmdlineflags == NULL)
        return CMDLINEFLAGS_FAILURE;

    n = cmdlinefags_build_help_msg(cmdlineflags, n_options, msg, size);

    free(cmdlineflags);

    return n;
}

int cmdlineflags_get_cfg(struct cmdlineflags_cfg* cfg)
{
    int retval = CMDLINEFLAGS_FAILURE;

    do {
        if (cfg == NULL)
            break;

        *cfg = cmdlineflags_cfg;

        retval = CMDLINEFLAGS_SUCCESS;
    } while (0);

    return retval;
}

int cmdlineflags_set_cfg(const struct cmdlineflags_cfg* cfg)
{
    int retval = CMDLINEFLAGS_FAILURE;

    do {
        if (cfg == NULL)
            break;

        cmdlineflags_cfg = *cfg;

        retval = CMDLINEFLAGS_SUCCESS;
    } while (0);

    return retval;
}

/*===========================================================================*\
 * local (internal linkage) function definitions
\*===========================================================================*/
static int cmdlineflags_compare(const struct cmdlineflags* l, const struct cmdlineflags* r)
{
    int status;

    status = strcmp(l->module, r->module);
    if (status) {
        if (!strcmp(l->module, CMDLINEFLAGS_XSTR(CMDLINEFLAGS_NO_MODULE)))
            return -1;
        else
        if (!strcmp(r->module, CMDLINEFLAGS_XSTR(CMDLINEFLAGS_NO_MODULE)))
            return +1;
        else
            return status;
    }

    if ((l->option.type == CMDLINEFLAGS_SHORTOPTION) && ((r->option.type == CMDLINEFLAGS_LONGOPTION)))
        status = l->option.u.shortoption - r->option.u.longoption[0];
    else
    if ((l->option.type == CMDLINEFLAGS_LONGOPTION) && ((r->option.type == CMDLINEFLAGS_SHORTOPTION))) {
        status = l->option.u.longoption[0] - r->option.u.shortoption;
    }
    else
    if ((l->option.type == CMDLINEFLAGS_LONGOPTION) && ((r->option.type == CMDLINEFLAGS_LONGOPTION)))
        status = strcmp(l->option.u.longoption, r->option.u.longoption);
    else
        status = l->option.u.shortoption - r->option.u.shortoption;

    return status;
}

static void cmdlinefags_add(
    const struct cmdlineflags** cmdlineflags, size_t n_options, const struct cmdlineflags* it)
{
    size_t i;

    for (i = 0; i < n_options; ++i)
        if (cmdlineflags_compare(it, cmdlineflags[i]) < 0)
            break;

    memmove(cmdlineflags + i + 1, cmdlineflags + i, (n_options - i) * sizeof(struct cmdlineflags*));
    cmdlineflags[i] = it;
}

static int cmdlinefags_build_help_msg(
    const struct cmdlineflags** cmdlineflags, size_t n_options, char* msg, unsigned size)
{
    int n;
    char prefix[128];
    int status;
    size_t remaining;
    size_t i;
    const char* module;

    n = 0;
    remaining = size;
    module = NULL;

    for (i = 0; i < n_options; ++i) {
        const struct cmdlineflags* it = cmdlineflags[i];

        if (strcmp(it->module, CMDLINEFLAGS_XSTR(CMDLINEFLAGS_NO_MODULE))) {
            if ((module == NULL) || (strcmp(module, it->module))) {
                module = it->module;
                status = snprintf(msg, remaining, "\n%s\n", it->module);
                if (status < 0)
                    return CMDLINEFLAGS_FAILURE;

                n += status;
                if (n < size) {
                    remaining -= status;
                    msg += status;
                }
                else
                    remaining = 0;
            }
        }

        if (it->option.type == CMDLINEFLAGS_SHORTOPTION) {
            const struct cmdlineflags* sibbling = it->sibbling;
            if (sibbling == NULL) {
                if (it->flags == CMDLINEFLAGS_NO_ARGUMENT)
                    status = snprintf(prefix, sizeof(prefix), "-%c",
                        it->option.u.shortoption);
                else
                    status = snprintf(prefix, sizeof(prefix), "-%c <arg>",
                        it->option.u.shortoption);
            }
            else {
                if (it->flags == CMDLINEFLAGS_NO_ARGUMENT)
                    status = snprintf(prefix, sizeof(prefix), "-%c, --%s",
                        it->option.u.shortoption, sibbling->option.u.longoption);
                else
                    status = snprintf(prefix, sizeof(prefix), "-%c, --%s <arg>",
                        it->option.u.shortoption, sibbling->option.u.longoption);
            }
        }
        else
        if (it->option.type == CMDLINEFLAGS_LONGOPTION) {
            if (it->flags == CMDLINEFLAGS_NO_ARGUMENT)
                status = snprintf(prefix, sizeof(prefix), "--%s",
                    it->option.u.longoption);
            else
                status = snprintf(prefix, sizeof(prefix), "--%s <arg>",
                    it->option.u.longoption);
        }
        else {
            /* do nothing */
        }

        if (status < 0)
            return CMDLINEFLAGS_FAILURE;

        status = snprintf(msg, remaining, "   %-40s : %s\n", prefix, it->help);
        if (status < 0)
            return CMDLINEFLAGS_FAILURE;

        n += status;
        if (n < size) {
            remaining -= status;
            msg += status;
        }
        else
            remaining = 0;
    }

    return n;
}

static const struct cmdlineflags** cmdlineflags_combine_options(size_t* n_options)
{
    const struct cmdlineflags* const shortoptions_start_addr = &CMDLINEFLAGS_SHORTOPTIONS_SECTION_START;
    const struct cmdlineflags* const shortoptions_end_addr = &CMDLINEFLAGS_SHORTOPTIONS_SECTION_END;
    ptrdiff_t n_shortoptions_distance =  shortoptions_end_addr - shortoptions_start_addr;

    const struct cmdlineflags* const longoptions_start_addr = &CMDLINEFLAGS_LONGOPTIONS_SECTION_START;
    const struct cmdlineflags* const longoptions_end_addr = &CMDLINEFLAGS_LONGOPTIONS_SECTION_END;
    ptrdiff_t n_longoptions_distance =  longoptions_end_addr - longoptions_start_addr;

    if (--n_shortoptions_distance < 0)
        return NULL;

    if (--n_longoptions_distance < 0)
        return NULL;

    size_t distance = n_shortoptions_distance + n_longoptions_distance;
    size_t n = 0;

    const struct cmdlineflags** cmdlineflags =
        calloc(distance, sizeof(struct cmdlineflags*));
    if (cmdlineflags != NULL) {
        const struct cmdlineflags* it;

        for (it = shortoptions_start_addr; it < shortoptions_end_addr; ++it)
            if (it->module != NULL)
                cmdlinefags_add(cmdlineflags, n++, it);

        for (it = longoptions_start_addr; it < longoptions_end_addr; ++it)
            if ((it->module != NULL) && (it->sibbling != it))
                cmdlinefags_add(cmdlineflags, n++, it);
    }

    return *n_options = n, cmdlineflags;
}
