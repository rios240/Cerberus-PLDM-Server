// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef X509_EXTENSION_BUILDER_DME_TESTING_H_
#define X509_EXTENSION_BUILDER_DME_TESTING_H_

#include <stdint.h>
#include <stddef.h>
#include "asn1/x509_extension_builder.h"


extern const uint8_t X509_EXTENSION_BUILDER_DME_TESTING_OID[];
#define	X509_EXTENSION_BUILDER_DME_TESTING_OID_LEN		10

extern const uint8_t X509_EXTENSION_BUILDER_DME_TESTING_DATA_ECC384_SHA384[];
extern const size_t X509_EXTENSION_BUILDER_DME_TESTING_DATA_ECC384_SHA384_LEN;

extern const uint8_t X509_EXTENSION_BUILDER_DME_TESTING_DATA_NO_DEVICE_OID[];
extern const size_t X509_EXTENSION_BUILDER_DME_TESTING_DATA_NO_DEVICE_OID_LEN;

extern const uint8_t X509_EXTENSION_BUILDER_DME_TESTING_DATA_NO_RENEWAL[];
extern const size_t X509_EXTENSION_BUILDER_DME_TESTING_DATA_NO_RENEWAL_LEN;

extern const uint8_t X509_EXTENSION_BUILDER_DME_TESTING_DATA_ECC256_SHA256[];
extern const size_t X509_EXTENSION_BUILDER_DME_TESTING_DATA_ECC256_SHA256_LEN;

extern const struct x509_extension X509_EXTENSION_BUILDER_DME_TESTING_EXTENSION_ECC384_SHA384;
extern const struct x509_extension X509_EXTENSION_BUILDER_DME_TESTING_EXTENSION_NO_DEVICE_OID;
extern const struct x509_extension X509_EXTENSION_BUILDER_DME_TESTING_EXTENSION_NO_RENEWAL;
extern const struct x509_extension X509_EXTENSION_BUILDER_DME_TESTING_EXTENSION_ECC256_SHA256;


#endif /* X509_EXTENSION_BUILDER_DME_TESTING_H_ */
