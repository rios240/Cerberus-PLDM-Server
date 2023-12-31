// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "testing.h"
#include "host_fw/host_irq_handler.h"
#include "testing/mock/host_fw/bmc_recovery_mock.h"
#include "testing/mock/host_fw/host_irq_control_mock.h"
#include "testing/mock/host_fw/host_processor_mock.h"
#include "testing/engines/hash_testing_engine.h"
#include "testing/engines/rsa_testing_engine.h"


TEST_SUITE_LABEL ("host_irq_handler");


/**
 * Dependencies for testing.
 */
struct host_irq_handler_testing {
	HASH_TESTING_ENGINE hash;				/**< Hash engine to use for reset validation. */
	RSA_TESTING_ENGINE rsa;					/**< RSA engine to use for reset validation. */
	struct host_processor_mock host;		/**< Mock for host processor. */
	struct bmc_recovery_mock recovery;		/**< Mock for BMC watchdog boot recovery. */
	struct host_irq_control_mock irq;		/**< Mock for host IRQ controller. */
	struct host_irq_handler test;			/**< Host interrupt handler under test. */
};

/**
 * Initialize testing dependencies.
 *
 * @param test The testing framework.
 * @param host The testing components to initialize.
 */
static void host_irq_handler_testing_init_dependencies (CuTest *test,
    struct host_irq_handler_testing *host)
{
	int status;

	status = HASH_TESTING_ENGINE_INIT (&host->hash);
	CuAssertIntEquals (test, 0, status);

	status = RSA_TESTING_ENGINE_INIT (&host->rsa);
	CuAssertIntEquals (test, 0, status);

	status = host_processor_mock_init (&host->host);
	CuAssertIntEquals (test, 0, status);

	status = bmc_recovery_mock_init (&host->recovery);
	CuAssertIntEquals (test, 0, status);

	status = host_irq_control_mock_init (&host->irq);
	CuAssertIntEquals (test, 0, status);
}

/**
 * Initialize a host interrupt handler for testing.
 *
 * @param test The testing framework.
 * @param host The testing components to initialize.
 */
void host_irq_handler_testing_init (CuTest *test, struct host_irq_handler_testing *host)
{
    int status;

	host_irq_handler_testing_init_dependencies (test, host);

	status = host_irq_handler_init (&host->test, &host->host.base, &host->hash.base,
		&host->rsa.base, &host->recovery.base);
	CuAssertIntEquals (test, 0, status);
}

/**
 * Initialize a host interrupt handler without recovery for testing.
 *
 * @param test The testing framework.
 * @param host The testing components to initialize.
 */
void host_irq_handler_testing_init_no_recovery (CuTest *test, struct host_irq_handler_testing *host)
{
    int status;

	host_irq_handler_testing_init_dependencies (test, host);

	status = host_irq_handler_init (&host->test, &host->host.base, &host->hash.base,
		&host->rsa.base, NULL);
	CuAssertIntEquals (test, 0, status);
}

/**
 * Helper to validate mocks and release testing dependencies.
 *
 * @param test The testing framework.
 * @param host The testing components to release.
 */
void host_irq_handler_testing_release_dependencies (CuTest *test,
	struct host_irq_handler_testing *host)
{
    int status;

	status = host_processor_mock_validate_and_release (&host->host);
	status |= bmc_recovery_mock_validate_and_release (&host->recovery);
	status |= host_irq_control_mock_validate_and_release (&host->irq);

	CuAssertIntEquals (test, 0, status);

	HASH_TESTING_ENGINE_RELEASE (&host->hash);
	RSA_TESTING_ENGINE_RELEASE (&host->rsa);
}

/**
 * Release a test instance and validate all mocks.
 *
 * @param test The testing framework.
 * @param host The testing components to release.
 */
void host_irq_handler_testing_validate_and_release (CuTest *test,
	struct host_irq_handler_testing *host)
{
	host_irq_handler_release (&host->test);

	host_irq_handler_testing_release_dependencies (test, host);
}


/*******************
 * Test cases
 *******************/

static void host_irq_handler_test_init (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = host_irq_handler_init (&handler.test, &handler.host.base, &handler.hash.base,
		&handler.rsa.base, &handler.recovery.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrNotNull (test, handler.test.power_on);
	CuAssertPtrNotNull (test, handler.test.enter_reset);
	CuAssertPtrNotNull (test, handler.test.exit_reset);
	CuAssertPtrNotNull (test, handler.test.assert_cs0);
	CuAssertPtrNotNull (test, handler.test.assert_cs1);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_init_no_recovery (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = host_irq_handler_init (&handler.test, &handler.host.base, &handler.hash.base,
		&handler.rsa.base, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_init_null (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = host_irq_handler_init (NULL, &handler.host.base, &handler.hash.base, &handler.rsa.base,
		&handler.recovery.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init (&handler.test, NULL, &handler.hash.base, &handler.rsa.base,
		&handler.recovery.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init (&handler.test, &handler.host.base, NULL, &handler.rsa.base,
		&handler.recovery.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init (&handler.test, &handler.host.base, &handler.hash.base, NULL,
		&handler.recovery.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	host_irq_handler_testing_release_dependencies (test, &handler);
}

static void host_irq_handler_test_init_with_irq_ctrl (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = host_irq_handler_init_with_irq_ctrl (&handler.test, &handler.host.base,
		&handler.hash.base, &handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrNotNull (test, handler.test.power_on);
	CuAssertPtrNotNull (test, handler.test.enter_reset);
	CuAssertPtrNotNull (test, handler.test.exit_reset);
	CuAssertPtrNotNull (test, handler.test.assert_cs0);
	CuAssertPtrNotNull (test, handler.test.assert_cs1);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_init_with_irq_ctrl_no_recovery (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = host_irq_handler_init_with_irq_ctrl (&handler.test, &handler.host.base,
		&handler.hash.base, &handler.rsa.base, NULL, &handler.irq.base);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_init_with_irq_ctrl_null (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = host_irq_handler_init_with_irq_ctrl (NULL, &handler.host.base, &handler.hash.base,
		&handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init_with_irq_ctrl (&handler.test, NULL, &handler.hash.base,
		&handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init_with_irq_ctrl (&handler.test, &handler.host.base, NULL,
		&handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init_with_irq_ctrl (&handler.test, &handler.host.base,
		&handler.hash.base, NULL, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init_with_irq_ctrl (&handler.test, &handler.host.base,
		&handler.hash.base, &handler.rsa.base, &handler.recovery.base, NULL);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	host_irq_handler_testing_release_dependencies (test, &handler);
}

static void host_irq_handler_test_init_enable_exit_reset (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = mock_expect (&handler.irq.mock, handler.irq.base.enable_exit_reset, &handler.irq, 0,
		MOCK_ARG (true));
	status |= mock_expect (&handler.irq.mock, handler.irq.base.enable_exit_reset, &handler.irq, 0,
		MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host_irq_handler_init_enable_exit_reset (&handler.test, &handler.host.base,
		&handler.hash.base, &handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, 0, status);

	CuAssertPtrNotNull (test, handler.test.power_on);
	CuAssertPtrNotNull (test, handler.test.enter_reset);
	CuAssertPtrNotNull (test, handler.test.exit_reset);
	CuAssertPtrNotNull (test, handler.test.assert_cs0);
	CuAssertPtrNotNull (test, handler.test.assert_cs1);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_init_enable_exit_reset_no_recovery (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = mock_expect (&handler.irq.mock, handler.irq.base.enable_exit_reset, &handler.irq, 0,
		MOCK_ARG (true));
	status |= mock_expect (&handler.irq.mock, handler.irq.base.enable_exit_reset, &handler.irq, 0,
		MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = host_irq_handler_init_enable_exit_reset (&handler.test, &handler.host.base,
		&handler.hash.base, &handler.rsa.base, NULL, &handler.irq.base);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_init_enable_exit_reset_null (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = host_irq_handler_init_enable_exit_reset (NULL, &handler.host.base, &handler.hash.base,
		&handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init_enable_exit_reset (&handler.test, NULL, &handler.hash.base,
		&handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init_enable_exit_reset (&handler.test, &handler.host.base, NULL,
		&handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init_enable_exit_reset (&handler.test, &handler.host.base,
		&handler.hash.base, NULL, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_init_enable_exit_reset (&handler.test, &handler.host.base,
		&handler.hash.base, &handler.rsa.base, &handler.recovery.base, NULL);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	host_irq_handler_testing_release_dependencies (test, &handler);
}

static void host_irq_handler_test_init_enable_exit_reset_irq_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = mock_expect (&handler.irq.mock, handler.irq.base.enable_exit_reset, &handler.irq,
		HOST_IRQ_CTRL_EXIT_RESET_FAILED, MOCK_ARG (true));
	CuAssertIntEquals (test, 0, status);

	status = host_irq_handler_init_enable_exit_reset (&handler.test, &handler.host.base,
		&handler.hash.base, &handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, HOST_IRQ_CTRL_EXIT_RESET_FAILED, status);

	host_irq_handler_testing_release_dependencies (test, &handler);
}

static void host_irq_handler_test_release_null (CuTest *test)
{
	TEST_START;

	host_irq_handler_release (NULL);
}

static void host_irq_handler_test_release_irq_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_dependencies (test, &handler);

	status = mock_expect (&handler.irq.mock, handler.irq.base.enable_exit_reset, &handler.irq, 0, MOCK_ARG (true));
	CuAssertIntEquals (test, 0, status);

	status = host_irq_handler_init_enable_exit_reset (&handler.test, &handler.host.base,
		&handler.hash.base, &handler.rsa.base, &handler.recovery.base, &handler.irq.base);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&handler.irq.mock, handler.irq.base.enable_exit_reset, &handler.irq,
		HOST_IRQ_CTRL_EXIT_RESET_FAILED, MOCK_ARG (false));
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_enter_reset (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.recovery.mock, handler.recovery.base.on_host_reset,
		&handler.recovery, 0);
	status |= mock_expect (&handler.host.mock, handler.host.base.soft_reset, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.enter_reset (&handler.test);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_enter_reset_no_recovery (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_no_recovery (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.soft_reset, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	CuAssertIntEquals (test, 0, status);

	status = handler.test.enter_reset (&handler.test);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_enter_reset_null (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = handler.test.enter_reset (NULL);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_enter_reset_host_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.recovery.mock, handler.recovery.base.on_host_reset,
		&handler.recovery, 0);
	status |= mock_expect (&handler.host.mock, handler.host.base.soft_reset, &handler.host,
		HOST_PROCESSOR_SOFT_RESET_FAILED, MOCK_ARG_PTR (&handler.hash),
		MOCK_ARG_PTR (&handler.rsa));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.enter_reset (&handler.test);
	CuAssertIntEquals (test, HOST_PROCESSOR_SOFT_RESET_FAILED, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_exit_reset (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.recovery.mock, handler.recovery.base.on_host_out_of_reset,
		&handler.recovery, 0);
	CuAssertIntEquals (test, 0, status);

	handler.test.exit_reset (&handler.test);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_exit_reset_no_recovery (CuTest *test)
{
	struct host_irq_handler_testing handler;

	TEST_START;

	host_irq_handler_testing_init_no_recovery (test, &handler);

	handler.test.exit_reset (&handler.test);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_exit_reset_null (CuTest *test)
{
	struct host_irq_handler_testing handler;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	handler.test.exit_reset (NULL);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_assert_cs0 (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.recovery.mock, handler.recovery.base.on_host_cs0,
		&handler.recovery, 0);
	CuAssertIntEquals (test, 0, status);

	handler.test.assert_cs0 (&handler.test);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_assert_cs0_no_recovery (CuTest *test)
{
	struct host_irq_handler_testing handler;

	TEST_START;

	host_irq_handler_testing_init_no_recovery (test, &handler);

	handler.test.assert_cs0 (&handler.test);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_assert_cs0_null (CuTest *test)
{
	struct host_irq_handler_testing handler;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	handler.test.assert_cs0 (NULL);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_assert_cs1 (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.recovery.mock, handler.recovery.base.on_host_cs1,
		&handler.recovery, 0, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	CuAssertIntEquals (test, 0, status);

	status = handler.test.assert_cs1 (&handler.test);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_assert_cs1_no_recovery (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init_no_recovery (test, &handler);

	status = handler.test.assert_cs1 (&handler.test);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_assert_cs1_null (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = handler.test.assert_cs1 (NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_assert_cs1_recovery_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.recovery.mock, handler.recovery.base.on_host_cs1,
		&handler.recovery, BMC_RECOVERY_CS1_FAILED, MOCK_ARG_PTR (&handler.hash),
		MOCK_ARG_PTR (&handler.rsa));
	CuAssertIntEquals (test, 0, status);

	status = handler.test.assert_cs1 (&handler.test);
	CuAssertIntEquals (test, BMC_RECOVERY_CS1_FAILED, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_alternate_hash (CuTest *test)
{
	struct host_irq_handler_testing handler;
	HASH_TESTING_ENGINE hash2;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = HASH_TESTING_ENGINE_INIT (&hash2);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host, 0,
		MOCK_ARG_PTR (&hash2), MOCK_ARG_PTR (&handler.rsa));
	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, &hash2.base);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);

	HASH_TESTING_ENGINE_RELEASE (&hash2);
}

static void host_irq_handler_test_power_on_validation_fail (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_blank_fail (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		FLASH_UTIL_UNEXPECTED_VALUE, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_unknown_version (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_FW_UTIL_UNSUPPORTED_VERSION, MOCK_ARG_PTR (&handler.hash),
		MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_flash_rollback (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa), MOCK_ARG (true),
		MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_flash_rollback_alternate_hash (CuTest *test)
{
	struct host_irq_handler_testing handler;
	HASH_TESTING_ENGINE hash2;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = HASH_TESTING_ENGINE_INIT (&hash2);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&hash2), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&hash2), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host, 0,
		MOCK_ARG_PTR (&hash2), MOCK_ARG_PTR (&handler.rsa), MOCK_ARG (true), MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, &hash2.base);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);

	HASH_TESTING_ENGINE_RELEASE (&hash2);
}

static void host_irq_handler_test_power_on_flash_rollback_validation_fail (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		RSA_ENGINE_BAD_SIGNATURE, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa), MOCK_ARG (true),
		MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_flash_rollback_blank_fail (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		FLASH_UTIL_UNEXPECTED_VALUE, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa), MOCK_ARG (true),
		MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_flash_rollback_unknown_version (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_FW_UTIL_UNSUPPORTED_VERSION, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa), MOCK_ARG (true),
		MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_flash_rollback_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa), MOCK_ARG (true),
		MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_flash_rollback_no_rollback (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_NO_ROLLBACK, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, 0, MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_flash_rollback_rollback_dirty (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_DIRTY, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, 0, MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_apply_recovery_image (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, 0, MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_apply_recovery_image_alternate_hash (CuTest *test)
{
	struct host_irq_handler_testing handler;
	HASH_TESTING_ENGINE hash2;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = HASH_TESTING_ENGINE_INIT (&hash2);
	CuAssertIntEquals (test, 0, status);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&hash2), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&hash2), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&hash2), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&hash2), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image, &handler.host,
		0, MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, &hash2.base);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);

	HASH_TESTING_ENGINE_RELEASE (&hash2);
}

static void host_irq_handler_test_power_on_apply_recovery_image_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_IMG_FAILED, MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, 0, MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_apply_recovery_image_retry_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_IMG_FAILED, MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_IMG_FAILED, MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, HOST_PROCESSOR_RECOVERY_IMG_FAILED, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_apply_recovery_image_unsupported (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_UNSUPPORTED, MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, HOST_PROCESSOR_RECOVERY_UNSUPPORTED, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_apply_recovery_image_no_image (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_NO_RECOVERY_IMAGE, MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, false, NULL);
	CuAssertIntEquals (test, HOST_PROCESSOR_NO_RECOVERY_IMAGE, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_apply_recovery_image_unsupported_allow_unsecure (
	CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_UNSUPPORTED, MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.bypass_mode, &handler.host, 0,
		MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, true, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_apply_recovery_image_no_image_allow_unsecure (
	CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_NO_RECOVERY_IMAGE, MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.bypass_mode, &handler.host, 0,
		MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, true, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_bypass_mode (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_IMG_FAILED, MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_IMG_FAILED, MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.bypass_mode, &handler.host, 0,
		MOCK_ARG (true));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, true, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_bypass_mode_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_IMG_FAILED, MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_IMG_FAILED, MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.bypass_mode, &handler.host,
		HOST_PROCESSOR_BYPASS_FAILED, MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.bypass_mode, &handler.host, 0,
		MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, true, NULL);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_bypass_mode_retry_error (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));
	status |= mock_expect (&handler.host.mock, handler.host.base.power_on_reset, &handler.host,
		HOST_PROCESSOR_POR_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.flash_rollback, &handler.host,
		HOST_PROCESSOR_ROLLBACK_FAILED, MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa),
		MOCK_ARG (true), MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_IMG_FAILED, MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.apply_recovery_image,
		&handler.host, HOST_PROCESSOR_RECOVERY_IMG_FAILED, MOCK_ARG (true));

	status |= mock_expect (&handler.host.mock, handler.host.base.bypass_mode, &handler.host,
		HOST_PROCESSOR_BYPASS_FAILED, MOCK_ARG (true));
	status |= mock_expect (&handler.host.mock, handler.host.base.bypass_mode, &handler.host,
		HOST_PROCESSOR_BYPASS_FAILED, MOCK_ARG (false));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.power_on (&handler.test, true, NULL);
	CuAssertIntEquals (test, HOST_PROCESSOR_BYPASS_FAILED, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_power_on_null (CuTest *test)
{
	struct host_irq_handler_testing handler;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = handler.test.power_on (NULL, false, NULL);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_set_host (CuTest *test)
{
	struct host_irq_handler_testing handler;
	struct host_processor_mock host_new;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = host_processor_mock_init (&host_new);
	CuAssertIntEquals (test, 0, status);

	status = host_irq_handler_set_host (&handler.test, &host_new.base);
	CuAssertIntEquals (test, 0, status);

	/* Check that the new instance will be called in response to an event. */
	status = mock_expect (&handler.recovery.mock, handler.recovery.base.on_host_reset,
		&handler.recovery, 0);
	status |= mock_expect (&host_new.mock, host_new.base.soft_reset, &host_new, 0,
		MOCK_ARG_PTR (&handler.hash), MOCK_ARG_PTR (&handler.rsa));

	CuAssertIntEquals (test, 0, status);

	status = handler.test.enter_reset (&handler.test);
	CuAssertIntEquals (test, 0, status);

	status = host_processor_mock_validate_and_release (&host_new);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}

static void host_irq_handler_test_set_host_null (CuTest *test)
{
	struct host_irq_handler_testing handler;
	struct host_processor_mock host_new;
	int status;

	TEST_START;

	host_irq_handler_testing_init (test, &handler);

	status = host_processor_mock_init (&host_new);
	CuAssertIntEquals (test, 0, status);

	status = host_irq_handler_set_host (NULL, &host_new.base);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_irq_handler_set_host (&handler.test, NULL);
	CuAssertIntEquals (test, HOST_IRQ_HANDLER_INVALID_ARGUMENT, status);

	status = host_processor_mock_validate_and_release (&host_new);
	CuAssertIntEquals (test, 0, status);

	host_irq_handler_testing_validate_and_release (test, &handler);
}


TEST_SUITE_START (host_irq_handler);

TEST (host_irq_handler_test_init);
TEST (host_irq_handler_test_init_no_recovery);
TEST (host_irq_handler_test_init_null);
TEST (host_irq_handler_test_init_with_irq_ctrl);
TEST (host_irq_handler_test_init_with_irq_ctrl_no_recovery);
TEST (host_irq_handler_test_init_with_irq_ctrl_null);
TEST (host_irq_handler_test_init_enable_exit_reset);
TEST (host_irq_handler_test_init_enable_exit_reset_no_recovery);
TEST (host_irq_handler_test_init_enable_exit_reset_null);
TEST (host_irq_handler_test_init_enable_exit_reset_irq_error);
TEST (host_irq_handler_test_release_null);
TEST (host_irq_handler_test_release_irq_error);
TEST (host_irq_handler_test_enter_reset);
TEST (host_irq_handler_test_enter_reset_no_recovery);
TEST (host_irq_handler_test_enter_reset_null);
TEST (host_irq_handler_test_enter_reset_host_error);
TEST (host_irq_handler_test_exit_reset);
TEST (host_irq_handler_test_exit_reset_no_recovery);
TEST (host_irq_handler_test_exit_reset_null);
TEST (host_irq_handler_test_assert_cs0);
TEST (host_irq_handler_test_assert_cs0_no_recovery);
TEST (host_irq_handler_test_assert_cs0_null);
TEST (host_irq_handler_test_assert_cs1);
TEST (host_irq_handler_test_assert_cs1_no_recovery);
TEST (host_irq_handler_test_assert_cs1_null);
TEST (host_irq_handler_test_assert_cs1_recovery_error);
TEST (host_irq_handler_test_power_on);
TEST (host_irq_handler_test_power_on_alternate_hash);
TEST (host_irq_handler_test_power_on_validation_fail);
TEST (host_irq_handler_test_power_on_blank_fail);
TEST (host_irq_handler_test_power_on_unknown_version);
TEST (host_irq_handler_test_power_on_error);
TEST (host_irq_handler_test_power_on_flash_rollback);
TEST (host_irq_handler_test_power_on_flash_rollback_alternate_hash);
TEST (host_irq_handler_test_power_on_flash_rollback_validation_fail);
TEST (host_irq_handler_test_power_on_flash_rollback_blank_fail);
TEST (host_irq_handler_test_power_on_flash_rollback_unknown_version);
TEST (host_irq_handler_test_power_on_flash_rollback_error);
TEST (host_irq_handler_test_power_on_flash_rollback_no_rollback);
TEST (host_irq_handler_test_power_on_flash_rollback_rollback_dirty);
TEST (host_irq_handler_test_power_on_apply_recovery_image);
TEST (host_irq_handler_test_power_on_apply_recovery_image_alternate_hash);
TEST (host_irq_handler_test_power_on_apply_recovery_image_error);
TEST (host_irq_handler_test_power_on_apply_recovery_image_retry_error);
TEST (host_irq_handler_test_power_on_apply_recovery_image_unsupported);
TEST (host_irq_handler_test_power_on_apply_recovery_image_no_image);
TEST (host_irq_handler_test_power_on_apply_recovery_image_unsupported_allow_unsecure);
TEST (host_irq_handler_test_power_on_apply_recovery_image_no_image_allow_unsecure);
TEST (host_irq_handler_test_power_on_bypass_mode);
TEST (host_irq_handler_test_power_on_bypass_mode_error);
TEST (host_irq_handler_test_power_on_bypass_mode_retry_error);
TEST (host_irq_handler_test_power_on_null);
TEST (host_irq_handler_test_set_host);
TEST (host_irq_handler_test_set_host_null);

TEST_SUITE_END;
