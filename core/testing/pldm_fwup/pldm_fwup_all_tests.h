#ifndef PLDM_FWUP_ALL_TESTS_H_
#define PLDM_FWUP_ALL_TESTS_H_


#include "testing.h"
#include "platform_all_tests.h"
#include "common/unused.h"


static void add_all_pldm_fwup_tests (CuSuite *suite)
{
    UNUSED (suite);

    TESTING_RUN_SUITE (pldm_fwup_interface);
}





#endif /* PLDM_FWUP_ALL_TESTS_H_ */