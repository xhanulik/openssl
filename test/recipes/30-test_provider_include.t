#! /usr/bin/env perl
# Copyright 2023 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html

use strict;
use warnings;
use OpenSSL::Test qw/:DEFAULT data_file bldtop_dir/;
use OpenSSL::Test::Utils;
use Cwd qw(abs_path);

setup("test_provider_include");

plan skip_all => "test_provider_include doesn't work without posix-io"
    if disabled("posix-io");

delete $ENV{OPENSSL_CONF_INCLUDE};

plan tests => 3;

$ENV{OPENSSL_MODULES} = abs_path(bldtop_dir("providers"));

ok(run(test(["provider_include_test", data_file("null-default.cnf")])), 
   "test null and default provider availability");
ok(run(test(["provider_include_test", "-f", data_file("null.cnf")])),
   "test default provider unavailability");
SKIP: {
    skip "Skipping legacy test", 1
        if disabled("legacy");
    ok(run(test(["provider_include_test", "-legacy", data_file("null-default-legacy.cnf")])),
       "test null, default and legacy provider availability");
}
