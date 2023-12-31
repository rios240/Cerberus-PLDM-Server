// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef PLDM_FW_UA_ALL_TESTS_H_
#define PLDM_FW_UA_ALL_TESTS_H_

#include "testing.h"
#include "platform_all_tests.h"
#include "common/unused.h"


/**
 * Add all tests for components in the 'PLDM_FW_UA' directory.
 *
 * Be sure to keep the test suites in alphabetical order for easier management.
 *
 * @param suite Suite to add the tests to.
 */
static void add_all_pldm_firmware_tests (CuSuite *suite)
{
    /* This is unused when no tests will be executed. */
	UNUSED (suite);

    TESTING_RUN_SUITE (pldm_firmware_update_interface);
}

#endif /* PLDM_FW_UA_ALL_TESTS_H_ */