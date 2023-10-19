/*
 * Copyright 2023 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stddef.h>
#include <string.h>
#include <openssl/provider.h>
#include <openssl/params.h>
#include <openssl/core_names.h>
#include <openssl/self_test.h>
#include <openssl/evp.h>
#include "testutil.h"

#ifdef _WIN32
# include <direct.h>
# define DIRSEP "/\\"
# ifndef __BORLANDC__
#  define chdir _chdir
# endif
# define DIRSEP_PRESERVE 0
#elif !defined(OPENSSL_NO_POSIX_IO)
# include <unistd.h>
# ifndef OPENSSL_SYS_VMS
#  define DIRSEP "/"
#  define DIRSEP_PRESERVE 0
# else
#  define DIRSEP "/]:"
#  define DIRSEP_PRESERVE 1
# endif
#else
/* the test does not work without chdir() */
# define chdir(x) (-1);
# define DIRSEP "/"
#  define DIRSEP_PRESERVE 0
#endif

typedef enum OPTION_choice {
    OPT_ERR = -1,
    OPT_EOF = 0,
    OPT_FAIL,
    OPT_TEST_ENUM
} OPTION_CHOICE;

static OSSL_LIB_CTX *libctx = NULL;
static int expect_failure = 0;

/* changes path to that of the filename and returns new config filename */
static char *change_path(const char *file)
{
    char *s = OPENSSL_strdup(file);
    char *p = s;
    char *last = NULL;
    int ret = 0;
    char *new_config_name = NULL;

    if (s == NULL)
        return NULL;

    while ((p = strpbrk(p, DIRSEP)) != NULL) {
        last = p++;
    }
    if (last == NULL)
        goto err;
    
    last[DIRSEP_PRESERVE] = 0;
    ret = chdir(s);
    if (ret == 0)
        new_config_name = strdup(last + DIRSEP_PRESERVE + 1);
 err:
    OPENSSL_free(s);
    return new_config_name;
}

static int test_include_default_provider(void)
{
    if (OSSL_PROVIDER_available(libctx, "null") != 1) {
        if (expect_failure)
            return 1;
        opt_printf_stderr("Null provider is missing\n");
        return 0;
    }
    if (OSSL_PROVIDER_available(libctx, "default") != 1) {
        if (expect_failure)
            return 1;
        opt_printf_stderr("Default provider is missing\n");
        return 0;
    }
    if (expect_failure)
        return 0;
    return 1;
}

const OPTIONS *test_get_options(void)
{
    static const OPTIONS test_options[] = {
        OPT_TEST_OPTIONS_WITH_EXTRA_USAGE("config_file\n"),
        { "f", OPT_FAIL, '-', "A failure is expected" },
        { NULL }
    };
    return test_options;
}

int setup_tests(void)
{
    OPTION_CHOICE o;
    char *config_file = NULL;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_FAIL:
            expect_failure = 1;
        break;
        case OPT_TEST_CASES:
           break;
        default:
        case OPT_ERR:
            return 0;
        }
    }

    libctx = OSSL_LIB_CTX_new();
    if (!TEST_ptr(libctx))
        return 0;
    /*
     * For this test we need to chdir as we use relative
     * path names in the config files.
     */
    config_file = test_get_argument(0);
    if (!TEST_ptr(config_file)) {
        opt_printf_stderr("No file argument\n");
        return 0;
    }
    config_file = change_path(config_file);
    if (!TEST_ptr(config_file) || !OSSL_LIB_CTX_load_config(libctx, config_file)) {
        OPENSSL_free(config_file);
        opt_printf_stderr("Failed to load config\n");
        return 0;
    }
    OPENSSL_free(config_file);

    ADD_TEST(test_include_default_provider);
    return 1;
}

void cleanup_tests(void)
{
    OSSL_LIB_CTX_free(libctx);
}
