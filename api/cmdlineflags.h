/* SPDX-License-Identifier: MIT */
/**
 * @file cmdlineflags.h
 *
 * @author Lukasz Wiecaszek <lukasz.wiecaszek@gmail.com>
 *
 * This is one and only header file for cmdlineflags library.
 * It exports all needed public symbols and macrodefinitions
 * and shall be included by any file which wants to define
 * and/or parse command line arguments using this library.
 *
 * A little bit about thread-safety (not formally).
 * It is generally not thread-safe to call api functions
 * from multiple threads concurently. But on the other hand,
 * who would like to call for example cmdlineflags_parse() concurently.
 * As in the real live, common sense should prevail.
 */

#ifndef _CMDLINEFLAGS_H_
#define _CMDLINEFLAGS_H_

#if defined(__cplusplus)
    #define LTS_EXTERN extern "C"
#else
    #define LTS_EXTERN extern
#endif

/*===========================================================================*\
 * system header files
\*===========================================================================*/
#include <stdbool.h>

/*===========================================================================*\
 * project header files
\*===========================================================================*/

/*===========================================================================*\
 * preprocessor #define constants and macros
\*===========================================================================*/
#if !defined(CMDLINEFLAGS_SECTION_PREFIX)
    #define CMDLINEFLAGS_SECTION_PREFIX lts_cmdlineflags
#endif

#define CMDLINEFLAGS_SUCCESS  (0)
#define CMDLINEFLAGS_FAILURE (-1)

#define CMDLINEFLAGS_ALIGN 32
#define CMDLINEFLAGS_STRINGIFY(x) #x
#define CMDLINEFLAGS_XSTR(x) CMDLINEFLAGS_STRINGIFY(x)
#define CMDLINEFLAGS_CONCATENATE(a, b) a ## b

#define CMDLINEFLAGS_CONCATENATE_SECTION_SHORTOPTIONS(section)  CMDLINEFLAGS_CONCATENATE(section, _shortoptions)
#define CMDLINEFLAGS_CONCATENATE_SECTION_LONGOPTIONS(section)   CMDLINEFLAGS_CONCATENATE(section, _longoptions)
#define CMDLINEFLAGS_CONCATENATE_SECTION_START(section)         CMDLINEFLAGS_CONCATENATE(__start_, section)
#define CMDLINEFLAGS_CONCATENATE_SECTION_END(section)           CMDLINEFLAGS_CONCATENATE(__stop_, section)

#define CMDLINEFLAGS_SHORTOPTIONS_SECTION_ID     CMDLINEFLAGS_CONCATENATE_SECTION_SHORTOPTIONS(CMDLINEFLAGS_SECTION_PREFIX)
#define CMDLINEFLAGS_SHORTOPTIONS_SECTION_NAME   CMDLINEFLAGS_XSTR(CMDLINEFLAGS_SHORTOPTIONS_SECTION_ID)
#define CMDLINEFLAGS_SHORTOPTIONS_SECTION_START  CMDLINEFLAGS_CONCATENATE_SECTION_START(CMDLINEFLAGS_SHORTOPTIONS_SECTION_ID)
#define CMDLINEFLAGS_SHORTOPTIONS_SECTION_END    CMDLINEFLAGS_CONCATENATE_SECTION_END(CMDLINEFLAGS_SHORTOPTIONS_SECTION_ID)

#define CMDLINEFLAGS_LONGOPTIONS_SECTION_ID      CMDLINEFLAGS_CONCATENATE_SECTION_LONGOPTIONS(CMDLINEFLAGS_SECTION_PREFIX)
#define CMDLINEFLAGS_LONGOPTIONS_SECTION_NAME    CMDLINEFLAGS_XSTR(CMDLINEFLAGS_LONGOPTIONS_SECTION_ID)
#define CMDLINEFLAGS_LONGOPTIONS_SECTION_START   CMDLINEFLAGS_CONCATENATE_SECTION_START(CMDLINEFLAGS_LONGOPTIONS_SECTION_ID)
#define CMDLINEFLAGS_LONGOPTIONS_SECTION_END     CMDLINEFLAGS_CONCATENATE_SECTION_END(CMDLINEFLAGS_LONGOPTIONS_SECTION_ID)

#define CMDLINEFLAGS_GLOBAL_MODULE _

/* https://www.youtube.com/watch?v=ohDB5gbtaEQ */
#define CMDLINEFLAGS_NO_ARGUMENT       0
#define CMDLINEFLAGS_REQUIRED_ARGUMENT 1

#define __CMDLINEFLAGS_DEFINE_SHORT_OPTION_A(_module_, _shortoption_, _flags_, _function_, _help_)             \
    const struct cmdlineflags cmdlineflags_shortoptions_ ## _module_ ## _ ## _shortoption_                     \
        [sizeof(#_shortoption_) == 2 ? 1 : -1]                                                                 \
        __attribute__((__section__(CMDLINEFLAGS_SHORTOPTIONS_SECTION_NAME)))                                   \
        __attribute__((__used__))                                                                              \
        __attribute__((aligned(CMDLINEFLAGS_ALIGN))) =                                                         \
        {{                                                                                                     \
            .module = #_module_,                                                                               \
            .option = {.type = CMDLINEFLAGS_SHORTOPTION, .u = {.shortoption = #_shortoption_[0]}},             \
            .flags = _flags_,                                                                                  \
            .u = {.f ## _flags_ = _function_},                                                                 \
            .help = _help_,                                                                                    \
            .sibbling = ((const struct cmdlineflags*)0)                                                        \
        }}

#define __CMDLINEFLAGS_DEFINE_SHORT_OPTION_B(_module_, _shortoption_, _flags_, _function_, _help_, _sibbling_) \
    const struct cmdlineflags cmdlineflags_shortoptions_ ## _module_ ## _ ## _shortoption_                     \
        [sizeof(#_shortoption_) == 2 ? 1 : -1]                                                                 \
        __attribute__((__section__(CMDLINEFLAGS_SHORTOPTIONS_SECTION_NAME)))                                   \
        __attribute__((__used__))                                                                              \
        __attribute__((aligned(CMDLINEFLAGS_ALIGN))) =                                                         \
        {{                                                                                                     \
            .module = #_module_,                                                                               \
            .option = {.type = CMDLINEFLAGS_SHORTOPTION, .u = {.shortoption = #_shortoption_[0]}},             \
            .flags = _flags_,                                                                                  \
            .u = {.f ## _flags_ = _function_},                                                                 \
            .help = _help_,                                                                                    \
            .sibbling = cmdlineflags_longoptions_ ## _module_ ## _ ## _sibbling_                               \
        }}

#define __CMDLINEFLAGS_DEFINE_LONG_OPTION_A(_module_, _longoption_, _flags_, _function_, _help_)               \
    const struct cmdlineflags cmdlineflags_longoptions_ ## _module_ ## _ ## _longoption_                       \
        [sizeof(#_longoption_) > 1 ? 1 : -1]                                                                   \
        __attribute__((__section__(CMDLINEFLAGS_LONGOPTIONS_SECTION_NAME)))                                    \
        __attribute__((__used__))                                                                              \
        __attribute__((aligned(CMDLINEFLAGS_ALIGN))) =                                                         \
        {{                                                                                                     \
            .module = #_module_,                                                                               \
            .option = {.type = CMDLINEFLAGS_LONGOPTION, .u = {.longoption = #_longoption_}},                   \
            .flags = _flags_,                                                                                  \
            .u = {.f ## _flags_ = _function_},                                                                 \
            .help = _help_,                                                                                    \
            .sibbling = ((const struct cmdlineflags*)0)                                                        \
        }}

#define __CMDLINEFLAGS_DEFINE_LONG_OPTION_B(_module_, _longoption_, _flags_, _function_, _help_)               \
    const struct cmdlineflags cmdlineflags_longoptions_ ## _module_ ## _ ## _longoption_                       \
        [sizeof(#_longoption_) > 1 ? 1 : -1]                                                                   \
        __attribute__((__section__(CMDLINEFLAGS_LONGOPTIONS_SECTION_NAME)))                                    \
        __attribute__((__used__))                                                                              \
        __attribute__((aligned(CMDLINEFLAGS_ALIGN))) =                                                         \
        {{                                                                                                     \
            .module = #_module_,                                                                               \
            .option = {.type = CMDLINEFLAGS_LONGOPTION, .u = {.longoption = #_longoption_}},                   \
            .flags = _flags_,                                                                                  \
            .u = {.f ## _flags_ = _function_},                                                                 \
            .help = _help_,                                                                                    \
            .sibbling = cmdlineflags_longoptions_ ## _module_ ## _ ## _longoption_                             \
        }}

#define CMDLINEFLAGS_DEFINE_SHORT_OPTION(_module_, _shortoption_, _flags_, _function_, _help_)                 \
    __CMDLINEFLAGS_DEFINE_SHORT_OPTION_A(_module_, _shortoption_, _flags_, _function_, _help_)

#define CMDLINEFLAGS_DEFINE_LONG_OPTION(_module_, _longoption_, _flags_, _function_, _help_)                   \
    __CMDLINEFLAGS_DEFINE_LONG_OPTION_A(_module_, _longoption_, _flags_, _function_, _help_)

#define CMDLINEFLAGS_DEFINE(_module_, _shortoption_, _longoption_, _flags_, _function_, _help_)                \
    __CMDLINEFLAGS_DEFINE_LONG_OPTION_B(_module_, _longoption_, _flags_, _function_, _help_);                  \
    __CMDLINEFLAGS_DEFINE_SHORT_OPTION_B(_module_, _shortoption_, _flags_, _function_, _help_, _longoption_)

/*===========================================================================*\
 * global type definitions
\*===========================================================================*/
struct cmdlineflags_cfg
{
     /* == 0 - do not,
        != 0 - do emit debug messages */
    int emit_debug_messages;
};

enum cmdlineflags_type
{
    CMDLINEFLAGS_SHORTOPTION,
    CMDLINEFLAGS_LONGOPTION
};

struct cmdlineflags_option
{
    enum cmdlineflags_type type;
    union {
        char shortoption;
        const char* longoption;
    } u;
};

struct cmdlineflags
{
    const char* module;
    struct cmdlineflags_option option;
    unsigned flags;
    union {
        int (*f0)(const struct cmdlineflags_option* option);
        int (*f1)(const struct cmdlineflags_option* option, const char* argument);
    } u;
    const char* help;
    const struct cmdlineflags* sibbling;
} __attribute__((aligned(CMDLINEFLAGS_ALIGN)));

/*===========================================================================*\
 * static inline (internal linkage) function definitions
\*===========================================================================*/

/*===========================================================================*\
 * global (external linkage) object declarations
\*===========================================================================*/
LTS_EXTERN const struct cmdlineflags CMDLINEFLAGS_SHORTOPTIONS_SECTION_START;
LTS_EXTERN const struct cmdlineflags CMDLINEFLAGS_SHORTOPTIONS_SECTION_END;

LTS_EXTERN const struct cmdlineflags CMDLINEFLAGS_LONGOPTIONS_SECTION_START;
LTS_EXTERN const struct cmdlineflags CMDLINEFLAGS_LONGOPTIONS_SECTION_END;

/*===========================================================================*\
 * function forward declarations (external linkage)
\*===========================================================================*/
/**
 * Parses the command-line arguments.
 *
 * This function is in many aspects very similar to POSIX's getopt.
 * Out of the three possible semantics of the getopt it uses the POSIXLY_CORRECT one.
 * This means that it stops option processing when the first non-option is seen.
 *
 * @param[in] argc Argument count as passed to the main() on program invocation.
 * @param[in,out] argv Argument vector as passed to the main() on program invocation.
 *
 * @note Please see also unistd.h, and/or getopt.h.
 * @return Index (into argv) of the first nonoptions argument.
 */
LTS_EXTERN int cmdlineflags_parse(int argc, char * const argv[]);

/**
 * Copies help message to the buffer pointed to by 'msg' argument.
 *
 * This function do not copy more than 'size' bytes (including the terminating
 * null byte ('\0')).
 * If the output message was truncated due to this limit then the return value
 * is the number of characters (excluding the terminating null byte)
 * which would have been copied to the output buffer if enough space had been available.
 * Thus, a return value of 'size' or more means that the output message was truncated.
 *
 * One of the techniques which may be used with this function is to pass
 * 'size' of 0 to learn the size of overall message first and then in the second call
 * use buffer of sufficient size to be filled with message string.
 *
 * @param[out] msg Pointer to the output buffer to be filled with message string.
 * @param[in] size Maximum number of bytes that shall be copied to the output buffer.
 * @param[in] sort When true, provides sorted view of all options, when false, no sorting is performed.
 *
 * @return Number of characters constituting the help message
 *         (excluding the terminating null byte ('\0')) or a negative value
 *         if an error was encountered.
 */
LTS_EXTERN int cmdlineflags_get_help_msg(char* msg, unsigned size, bool sort);

/**
 * Retrieves current cmdlineflags configuration.
 *
 * @param[out] cfg Pointer to the object to be filled with the current
 *                 cmdlineflags configuration.
 *
 * @return 0 on success, negative value otherwise.
 */
LTS_EXTERN int cmdlineflags_get_cfg(struct cmdlineflags_cfg* cfg);

/**
 * Attempts to set new cmdlineflags configuration.
 *
 * @param[out] cfg Pointer to the object depicting new cmdlineflags configuration.
 *
 * @return 0 on success, negative value otherwise.
 */
LTS_EXTERN int cmdlineflags_set_cfg(const struct cmdlineflags_cfg* cfg);

#endif /* _CMDLINEFLAGS_H_ */
