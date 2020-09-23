/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _AZURE_CONFIG_H
#define _AZURE_CONFIG_H

typedef enum
{
    None         = 0,
    WEP          = 1,
    WPA_PSK_TKIP = 2,
    WPA2_PSK_AES = 3
} WiFi_Mode;

// ----------------------------------------------------------------------------
// WiFi connection information
// ----------------------------------------------------------------------------
#define WIFI_SSID     "This LAN is Our LAN"
#define WIFI_PASSWORD "weedistight"
#define WIFI_MODE     WPA2_PSK_AES

// ----------------------------------------------------------------------------
// Azure IoT Hub Connection Transport
// Define this to use the nx client, otherwise MQTT
// ----------------------------------------------------------------------------
#define USE_NX_CLIENT_PREVIEW

// ----------------------------------------------------------------------------
// Azure IoT Dynamic Provisioning Service
// Define this to use the DPS service, otherwise direct IoT Hub
// ----------------------------------------------------------------------------
#define ENABLE_DPS

// ----------------------------------------------------------------------------
// Azure IoT Hub config
// ----------------------------------------------------------------------------
#define IOT_HUB_HOSTNAME "gettingstarted.azure-devices.net"
#define IOT_DEVICE_ID    "test-gsg-dps-devid"
#define IOT_PRIMARY_KEY  "Chyz57v9QXOvinUBHbOviNsHnzmD5wc/+SfWa29d89yDdl5ZsnajEL7BrFDHeKYL5TmZMwlIphmTCtm6GgoSgg=="

// ----------------------------------------------------------------------------
// Azure IoT DPS config
// ----------------------------------------------------------------------------
#define IOT_DPS_ENDPOINT        "global.azure-devices-provisioning.net"
#define IOT_DPS_ID_SCOPE        "0ne0017EBDB"
#define IOT_DPS_REGISTRATION_ID "test-gsg-dps"

// ----------------------------------------------------------------------------
// Azure IoT DPS X509 config
// ----------------------------------------------------------------------------
#define ENABLE_X509
#define IOT_DPS_X509_CERT       "-----BEGIN CERTIFICATE-----\n\
MIID9TCCAt0CFCnQvA476WUk+eosx4i3mqyyJkbgMA0GCSqGSIb3DQEBCwUAMIG2\
MQswCQYDVQQGEwJVUzETMBEGA1UECAwKQ2FsaWZvcm5pYTEVMBMGA1UEBwwMU2Fu\
dGEgTW9uaWNhMRIwEAYDVQQKDAlNaWNyb3NvZnQxEjAQBgNVBAsMCUF6dXJlIElv\
VDEuMCwGA1UEAwwlZ2xvYmFsLmF6dXJlLWRldmljZXMtcHJvdmlzaW9uaW5nLm5l\
dDEjMCEGCSqGSIb3DQEJARYUc2htY2Rvbm9Ab3V0bG9vay5jb20wHhcNMjAwOTE3\
MTYxMzAxWhcNMjEwOTE3MTYxMzAxWjCBtjELMAkGA1UEBhMCVVMxEzARBgNVBAgM\
CkNhbGlmb3JuaWExFTATBgNVBAcMDFNhbnRhIE1vbmljYTESMBAGA1UECgwJTWlj\
cm9zb2Z0MRIwEAYDVQQLDAlBenVyZSBJb1QxLjAsBgNVBAMMJWdsb2JhbC5henVy\
ZS1kZXZpY2VzLXByb3Zpc2lvbmluZy5uZXQxIzAhBgkqhkiG9w0BCQEWFHNobWNk\
b25vQG91dGxvb2suY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA\
nLmKrGMiHe+jb0xa4UNagiwBGNt2f1MudNx7VeKJsI4nmgvNe5Xjnifl1l9EABZ6\
UrkFMEoZoHEYckqBMXPqKLoOBL/h3BF/2EyXtf4d9HFCCEK3k+6qvGFQFb93nGRC\
/7Nr41OFUko7Z0978tFlKQO8I/6T1Ah7U1XhcFOLnVKpmddZ2uMMuuOkYWYWnLQ6\
td7N2jr1xqAp53Wt7JIXwJLEeaq3SsbWXmYymEPG/1zvbAtUDDBifNa5Xau4fqNP\
/RlRWGzpDFRul7qqy2YhbA+nS5UGi2E1jfXZ0Nz/5jJfg+ktiLqMIqr8Fpd6BnvO\
zUtUV18AEbrQDypMRGrsnQIDAQABMA0GCSqGSIb3DQEBCwUAA4IBAQAGiNRzusXZ\
Lwr5Vq+FyX3/JqpU9+avPjzwL6T6kO2elOW4LBsl7IF4BCewDWD7ufZVfIhAPEFX\
rAZQtUbpikzKdiy38SDqBKFhrDJpoDoI85iStpv3FRVQmnMIQYP04hkDjq707s1k\
OgTG5x7lGBZ0NmRfqLjy4VwUXj1+a8LIDCbti7EkMQtWVRUvp7w6BND2mmX8tOTU\
+F6f6gJ3o0NeLZPOrdyluYKflIhfvbFJsZEyPGDq0B4CIZB6sV0TpRn7NgJ01cQj\
Q8s2Dd04bk4IC9MXeNfp+hBo2evdhFQK9JdjheNUgJRoLXvsh8AGW9s2QF4QA65y\
kDKKoZFtMFhc\n\
-----END CERTIFICATE-----"
#define IOT_X509_PRIVATE_KEY    "-----BEGIN PRIVATE KEY-----\n\
MIIEpAIBAAKCAQEAnLmKrGMiHe+jb0xa4UNagiwBGNt2f1MudNx7VeKJsI4nmgvN\
e5Xjnifl1l9EABZ6UrkFMEoZoHEYckqBMXPqKLoOBL/h3BF/2EyXtf4d9HFCCEK3\
k+6qvGFQFb93nGRC/7Nr41OFUko7Z0978tFlKQO8I/6T1Ah7U1XhcFOLnVKpmddZ\
2uMMuuOkYWYWnLQ6td7N2jr1xqAp53Wt7JIXwJLEeaq3SsbWXmYymEPG/1zvbAtU\
DDBifNa5Xau4fqNP/RlRWGzpDFRul7qqy2YhbA+nS5UGi2E1jfXZ0Nz/5jJfg+kt\
iLqMIqr8Fpd6BnvOzUtUV18AEbrQDypMRGrsnQIDAQABAoIBAER3McN5UzAsvgMj\
IhiCagmv25LWXP/mgMcAgsjUPD5q+5XqTgwNQPCYkWmDGJvJNgrpVQDxfhx/C6Ty\
M1o9PzAnh3Q/txWCd40weJ5CsydUSvp82uG/Q8sj3+6ARGqHylUWNq2KkRWP5D+P\
k6BSJZUzUn+O+VmJkwam3+NIPBevk9zckU+PJyOynWHLks6EvxReyjwdAT9rwX1L\
ZWQ3awQ4cXKuX5HFtR3qhxI3sz8p5rgrsLdxVMdsKXVVMyW76mqG1U4HpfRsWggU\
VHPQZJY+UF1sh7Pr5HLggfDQmv2yUumXZYa6ZddCIPNdq84RKyNuaVqqZq3FU1yr\
Q3WCCqkCgYEAzn500VgdI0TP4pbKaN2f59XR85M2gAmDDcSjsb6kAzSNarbD6K0P\
1VPGK/6OKgX4/SDb4VxOlmZ3K+SUQZ/CP+MokD/8aXn3Ajclho8lIs4HtQhvZhN9\
E5EXrCVKnj0senDqXrEL57lINsoqsL313Q07ZRa38rc9GBW9okl0DVMCgYEAwkyB\
rzolC4oOGKEXo//JCdj4NbmH/2IK8z7MvTc0R3tgWBH7unXS1yl+s0Z5NULPI+bE\
SHjj7GDCcIrTd7Pp8xcxcJhHZw2UEh2/0wadO7gPpgu8hDDTjTWaaT7jzlo8M3m7\
fC9B8TSvE7iz+Chp9sm5KFgr3BVWD6gII/3P8E8CgYBjjxjnrmIuFQleKHHi6agl\
cTRPChVm2i8zvjFmi5IscXQVlZUTZA2UP1X29D67CDIqaWNaXtUNMs4WfZj1ov9X\
KkDue8tJV6K+kLQpEm9HPqr8n9qGDrNpcbVGLJyzeq8tQa/eg+JHkU++Cu9WU+Bs\
+eo/LRr3jV2JdUH/MwcfPQKBgQC405pRm5f9wBfkqekZIoqVCYYjZRfPge40sbqF\
C+x5Sw28+W5iUw57iodx9kn6tlDhL/XrksQN6YVNFWJP7h9QFPH8z36ssyf1i8Mb\
9KDBXDfWljd/8Aptu13lW+PMOd4mgVgtq66oypr4wazBFYCnOAEgNa+TLeCuCAOm\
aRzS1QKBgQCyDUvKQ4AKEkRAs8bkbI8tAzs9jf/CL8BMvLapyNMQKVx3qmeH5G41\
5ZPvPRXM4dGPLiSUjyiaWNVBiXKt3eh0N0TGoNA8RpDoxMva8K9mm1mm5+fVlG5w\
F2qLNvpzTk5j9a1YfyhVLZl1QGtAwii/Qc8mwGodfFqtpEzVfevZxQ==\n\
-----END PRIVATE KEY-----"

#endif // _AZURE_CONFIG_H
