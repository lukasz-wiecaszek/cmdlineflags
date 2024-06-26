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
#include <stdbool.h>

/*===========================================================================*\
 * project header files
\*===========================================================================*/
#include <version.h>
#include <cmdlineflags/cmdlineflags.h>

/*===========================================================================*\
 * preprocessor #define constants and macros
\*===========================================================================*/
// clang-format off
#define PRINT_ERROR(...)                      \
    if (cmdlineflags_cfg.emit_debug_messages) \
        fprintf(stderr, __VA_ARGS__)
// clang-format on

/*===========================================================================*\
 * local type definitions
\*===========================================================================*/

/*===========================================================================*\
 * global (external linkage) object definitions
\*===========================================================================*/

/*===========================================================================*\
 * local (internal linkage) function declarations
\*===========================================================================*/
static int cmdlineflags_compare(const struct cmdlineflags* l, const struct cmdlineflags* r);
static void cmdlinefags_add(const struct cmdlineflags** cmdlineflags, size_t n_options, const struct cmdlineflags* it);
static int cmdlinefags_build_help_msg(const struct cmdlineflags** cmdlineflags, size_t n_options, char* msg, unsigned size);
static const struct cmdlineflags** cmdlineflags_combine_options(size_t* n_options, bool sort);

/*===========================================================================*\
 * local (internal linkage) object definitions
\*===========================================================================*/
// clang-format off
static const struct cmdlineflags cmdlineflags_short_options_0
    __attribute__((__section__(CMDLINEFLAGS_SHORTOPTIONS_SECTION_NAME)))
    __attribute__((__used__))
    __attribute__((aligned(CMDLINEFLAGS_ALIGN))) = {0};

static const struct cmdlineflags cmdlineflags_longoptions_0
    __attribute__((__section__(CMDLINEFLAGS_LONGOPTIONS_SECTION_NAME)))
    __attribute__((__used__))
    __attribute__((aligned(CMDLINEFLAGS_ALIGN))) = {0};
// clang-format on

static struct cmdlineflags_cfg cmdlineflags_cfg = {
    .emit_debug_messages = 1,
};

/*===========================================================================*\
 * static inline (internal linkage) function definitions
\*===========================================================================*/
static inline int cmdlineflags_longoptionscmp(char const* str1, char const* str2)
{
    char c1;
    char c2;
    int d;

    do {
        c1 = *str1;
        if (c1 == '_')
            c1 = '-';

        c2 = *str2;
        if (c2 == '_')
            c2 = '-';

        d = c1 - c2;
    } while (d == 0 && *str1++ && *str2++);

    return d;
}

static inline char* cmdlineflags_underscore2dash(char* dest, const char* src, size_t n)
{
    if (n != 0) {
        char c;
        n--;
        for (size_t i = 0; (i < n) && (*src != '\0'); ++i) {
            c = *src++;
            *dest++ = c != '_' ? c : '-'; /* I shouldn't leave it as it is ;-) */
        }
        *dest = '\0';
    }

    return dest;
}

static inline const struct cmdlineflags* cmdlineflags_get_shortoption(const char* module, char shortoption)
{
    const struct cmdlineflags* const cmdlineflags_start_addr = &CMDLINEFLAGS_SHORTOPTIONS_SECTION_START;
    const struct cmdlineflags* const cmdlineflags_end_addr = &CMDLINEFLAGS_SHORTOPTIONS_SECTION_END;
    const struct cmdlineflags* it;

    if (module == NULL)
        module = CMDLINEFLAGS_XSTR(CMDLINEFLAGS_GLOBAL_MODULE);

    for (it = cmdlineflags_start_addr; it < cmdlineflags_end_addr; ++it)
        if ((it->module != NULL) && (!strcmp(it->module, module)))
            if (it->option.u.shortoption == shortoption)
                return it;

    return NULL;
}

static inline const struct cmdlineflags* cmdlineflags_get_longoption(const char* module, const char* longoption)
{
    const struct cmdlineflags* const cmdlineflags_start_addr = &CMDLINEFLAGS_LONGOPTIONS_SECTION_START;
    const struct cmdlineflags* const cmdlineflags_end_addr = &CMDLINEFLAGS_LONGOPTIONS_SECTION_END;
    const struct cmdlineflags* it;

    if (module == NULL)
        module = CMDLINEFLAGS_XSTR(CMDLINEFLAGS_GLOBAL_MODULE);

    for (it = cmdlineflags_start_addr; it < cmdlineflags_end_addr; ++it)
        if ((it->module != NULL) && (!strcmp(it->module, module)))
            if (!cmdlineflags_longoptionscmp(it->option.u.longoption, longoption))
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
const char* cmdlineflags_version(void)
{
    return PROJECT_VER;
}

int cmdlineflags_parse(int argc, char* const argv[])
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
        } else {
            bool is_longoption = cmdlineflags_is_longoption(arg);
            if (is_longoption) {
                const char* longoption_start = arg + 2; /* Skip the initial '--' */
                const char* longoption_end;
                const char* longoption;
                const struct cmdlineflags* cmdlineflags;

                for (longoption_end = longoption_start; *longoption_end != '\0' && *longoption_end != '='; longoption_end++)
                    ;

                longoption = strndup(longoption_start, longoption_end - longoption_start);
                if (!longoption)
                    return CMDLINEFLAGS_FAILURE;

                cmdlineflags = cmdlineflags_get_longoption(module, longoption);
                if (cmdlineflags) {
                    if (cmdlineflags->flags == CMDLINEFLAGS_NO_ARGUMENT) {
                        if (*longoption_end == '\0') {
                            if (cmdlineflags->u.f0(&cmdlineflags->option) != 0)
                                return free((void*)longoption), argv_index + 1;
                        } else
                            PRINT_ERROR("%s: option '--%s' doesn't allow an argument\n", argv[0], longoption);
                    } else {
                        if (*longoption_end != '\0') {
                            if (cmdlineflags->u.f1(&cmdlineflags->option, longoption_end + 1) != 0)
                                return free((void*)longoption), argv_index + 1;
                        } else if ((argv_index + 1) < argc) {
                            if (cmdlineflags->u.f1(&cmdlineflags->option, argv[++argv_index]) != 0)
                                return free((void*)longoption), argv_index + 1;
                        } else
                            PRINT_ERROR("%s: option '--%s' requires an argument\n", argv[0], longoption);
                    }
                } else {
                    if (module) {
                        PRINT_ERROR("%s: unrecognized option '--%s' for '%s' module\n", argv[0], longoption, module);
                    } else {
                        PRINT_ERROR("%s: unrecognized option '--%s'\n", argv[0], longoption);
                    }
                }

                free((void*)longoption);
            } else {
                const char* nextchar = arg + 1; /* Skip the initial '-' */
                const struct cmdlineflags* cmdlineflags;
                char c;

                while ((c = *nextchar++) != '\0') {
                    cmdlineflags = cmdlineflags_get_shortoption(module, c);
                    if (cmdlineflags) {
                        if (cmdlineflags->flags == CMDLINEFLAGS_NO_ARGUMENT) {
                            if (cmdlineflags->u.f0(&cmdlineflags->option) != 0)
                                return argv_index + 1;
                        } else {
                            /* This is an option that requires an argument. */
                            if (*nextchar != '\0') {
                                if (cmdlineflags->u.f1(&cmdlineflags->option, nextchar) != 0)
                                    return argv_index + 1;
                                /* If we end this ARGV-element by taking the rest as an argument,
                                   we must advance to the next element now. */
                                break;
                            } else if ((argv_index + 1) < argc) {
                                if (cmdlineflags->u.f1(&cmdlineflags->option, argv[++argv_index]) != 0)
                                    return argv_index + 1;
                            } else
                                PRINT_ERROR("%s: option '-%c' requires an argument\n", argv[0], c);
                        }
                    } else {
                        if (module) {
                            PRINT_ERROR("%s: unrecognized option '-%c' for '%s' module\n", argv[0], c, module);
                        } else {
                            PRINT_ERROR("%s: unrecognized option '-%c'\n", argv[0], c);
                        }
                    }
                }
            }
        }
    }

    return argv_index;
}

int cmdlineflags_get_help_msg(char* msg, unsigned size, bool sort)
{
    int n;
    size_t n_options;
    char null_msg_buffer[1];
    const struct cmdlineflags** cmdlineflags;

    if (msg == NULL)
        msg = null_msg_buffer;

    cmdlineflags = cmdlineflags_combine_options(&n_options, sort);
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
        if (!strcmp(l->module, CMDLINEFLAGS_XSTR(CMDLINEFLAGS_GLOBAL_MODULE)))
            return -1;
        else if (!strcmp(r->module, CMDLINEFLAGS_XSTR(CMDLINEFLAGS_GLOBAL_MODULE)))
            return +1;
        else
            return status;
    }

    if ((l->option.type == CMDLINEFLAGS_SHORTOPTION) && ((r->option.type == CMDLINEFLAGS_LONGOPTION)))
        status = l->option.u.shortoption - r->option.u.longoption[0];
    else if ((l->option.type == CMDLINEFLAGS_LONGOPTION) && ((r->option.type == CMDLINEFLAGS_SHORTOPTION))) {
        status = l->option.u.longoption[0] - r->option.u.shortoption;
    } else if ((l->option.type == CMDLINEFLAGS_LONGOPTION) && ((r->option.type == CMDLINEFLAGS_LONGOPTION)))
        status = strcmp(l->option.u.longoption, r->option.u.longoption);
    else
        status = l->option.u.shortoption - r->option.u.shortoption;

    return status;
}

static void cmdlinefags_add(const struct cmdlineflags** cmdlineflags, size_t n_options, const struct cmdlineflags* it)
{
    size_t i;

    for (i = 0; i < n_options; ++i)
        if (cmdlineflags_compare(it, cmdlineflags[i]) < 0)
            break;

    memmove(cmdlineflags + i + 1, cmdlineflags + i, (n_options - i) * sizeof(struct cmdlineflags*));
    cmdlineflags[i] = it;
}

static int cmdlinefags_build_help_msg(const struct cmdlineflags** cmdlineflags, size_t n_options, char* msg, unsigned size)
{
    int n;
    char prefix[128];
    char longoption[sizeof(prefix)];
    int status;
    size_t remaining;
    size_t i;
    const char* module;

    n = 0;
    remaining = size;
    module = NULL;

    for (i = 0; i < n_options; ++i) {
        const struct cmdlineflags* it = cmdlineflags[i];

        if (strcmp(it->module, CMDLINEFLAGS_XSTR(CMDLINEFLAGS_GLOBAL_MODULE))) {
            if ((module == NULL) || (strcmp(module, it->module))) {
                module = it->module;
                status = snprintf(msg, remaining, "\n%s\n", it->module);
                if (status < 0)
                    return CMDLINEFLAGS_FAILURE;

                n += status;
                if (n < size) {
                    remaining -= status;
                    msg += status;
                } else
                    remaining = 0;
            }
        }

        if (it->option.type == CMDLINEFLAGS_SHORTOPTION) {
            const struct cmdlineflags* sibbling = it->sibbling;
            if (sibbling == NULL) {
                if (it->flags == CMDLINEFLAGS_NO_ARGUMENT)
                    status = snprintf(prefix, sizeof(prefix), "-%c", it->option.u.shortoption);
                else
                    status = snprintf(prefix, sizeof(prefix), "-%c <arg>", it->option.u.shortoption);
            } else {
                cmdlineflags_underscore2dash(longoption, sibbling->option.u.longoption, sizeof(longoption));
                if (it->flags == CMDLINEFLAGS_NO_ARGUMENT)
                    status = snprintf(prefix, sizeof(prefix), "-%c, --%s", it->option.u.shortoption, longoption);
                else
                    status = snprintf(prefix, sizeof(prefix), "-%c, --%s <arg>", it->option.u.shortoption, longoption);
            }
        } else if (it->option.type == CMDLINEFLAGS_LONGOPTION) {
            cmdlineflags_underscore2dash(longoption, it->option.u.longoption, sizeof(longoption));
            if (it->flags == CMDLINEFLAGS_NO_ARGUMENT)
                status = snprintf(prefix, sizeof(prefix), "--%s", longoption);
            else
                status = snprintf(prefix, sizeof(prefix), "--%s <arg>", longoption);
        } else {
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
        } else
            remaining = 0;
    }

    return n;
}

static const struct cmdlineflags** cmdlineflags_combine_options(size_t* n_options, bool sort)
{
    const struct cmdlineflags* const shortoptions_start_addr = &CMDLINEFLAGS_SHORTOPTIONS_SECTION_START;
    const struct cmdlineflags* const shortoptions_end_addr = &CMDLINEFLAGS_SHORTOPTIONS_SECTION_END;
    ptrdiff_t n_shortoptions_distance = shortoptions_end_addr - shortoptions_start_addr;

    const struct cmdlineflags* const longoptions_start_addr = &CMDLINEFLAGS_LONGOPTIONS_SECTION_START;
    const struct cmdlineflags* const longoptions_end_addr = &CMDLINEFLAGS_LONGOPTIONS_SECTION_END;
    ptrdiff_t n_longoptions_distance = longoptions_end_addr - longoptions_start_addr;

    if (--n_shortoptions_distance < 0)
        return NULL;

    if (--n_longoptions_distance < 0)
        return NULL;

    size_t distance = n_shortoptions_distance + n_longoptions_distance;
    size_t n = 0;

    const struct cmdlineflags** cmdlineflags = calloc(distance, sizeof(struct cmdlineflags*));
    if (cmdlineflags != NULL) {
        const struct cmdlineflags* it;

        for (it = shortoptions_start_addr; it < shortoptions_end_addr; ++it) {
            if (it->module != NULL) {
                if (sort)
                    cmdlinefags_add(cmdlineflags, n++, it);
                else
                    cmdlineflags[n++] = it;
            }
        }

        for (it = longoptions_start_addr; it < longoptions_end_addr; ++it) {
            if ((it->module != NULL) && (it->sibbling != it)) {
                if (sort)
                    cmdlinefags_add(cmdlineflags, n++, it);
                else
                    cmdlineflags[n++] = it;
            }
        }

        if (!sort) {
            size_t i;
            for (i = 0; i < n / 2; ++i) {
                const struct cmdlineflags* it = cmdlineflags[i];
                cmdlineflags[i] = cmdlineflags[n - i - 1];
                cmdlineflags[n - i - 1] = it;
            }
        }
    }

    return *n_options = n, cmdlineflags;
}
