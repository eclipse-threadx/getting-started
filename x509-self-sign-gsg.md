---
title: Provision simulated X.509 device to Azure IoT Hub using C and OpenSSL-generated self-signed certificate
description: This quickstart uses individual enrollments. In this quickstart, you create and provision a simulated X.509 device using C device SDK for Azure IoT Hub Device Provisioning Service (DPS) and OpenSSL to generate a self-signed X509 certificate.
author: smcd253
ms.author: spmcdono
ms.date: 10/19/2020
ms.topic: quickstart
ms.service: iot-dps
services: iot-dps 
ms.custom: mvc
#Customer intent: As a new IoT developer, I want to connect an MXChip to my IoT Hub using Azure's Device Provisioning Service so that I can learn how secure provisioning works with Azure RTOS and a self-signed X509 certificate.
---

# Quickstart: Provision an MXChip using the Azure RTOS Getting Started Guide and an OpenSSL-generated self-signed certificate

In this quickstart, you will learn how to connect an MXChip to Azure IoT Hub using Azure's Device Provisioning Service (DPS) and a self-signed X.509 certificate. You will use OpenSSL on the command line to generate a private key and device certificate to provide the credentials to connect to DPS and IoT Hub. You will then use sample code from the [Azure RTOS](https://github.com/azure-rtos) in the [Getting Started Guide](https://github.com/azure-rtos/getting-started) to handle the certificate and key, connect to DPS, and start sending telemetry to IoT Hub. You will then use the [Azure IoT Explorer](https://github.com/Azure/azure-iot-explorer/releases) to view the telemetry being sent by the device.

**NOTE:** This quickstart is based on the ["Quick Create Simulated Device x509"](https://github.com/MicrosoftDocs/azure-docs/blob/master/articles/iot-dps/quick-create-simulated-device-x509.md) tutorial.

## Prerequisites
**NOTE:** This quickstart is currently only tailored toward a Windows 10 development environment.
Download and install the following:
* [Git for Windows](https://git-scm.com/download/win) 
    * **IMPORTANT**: Make sure you select the option to use Git from the command line. This will allow you to run the OpenSSL commands directly from the command line.
* [Visual Studio Code](https://code.visualstudio.com/download)
**NOTE:** You can install the following with the installation batch script provided in gsg
* [GCC](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
* [CMake](https://cmake.org/)
* [Termite](https://www.compuphase.com/software/termite-3.4.exe)
* [Azure IoT Explorer](https://docs.microsoft.com/en-us/azure/iot-pnp/howto-use-iot-explorer)

## Installation and Setup
Confirm installation of Git, OpenSSL VSCode
```bash
git --version
openssl version
code --version
```

Clone the Getting Started Guide repository
```bash
git clone https://github.com/azure-rtos/getting-started
```

Install required tools using the provided batch script
```bash
getting-started\tools\get-toolchain.bat
```

Confirm installation of CMake
```bash
cmake --version
```

## Create Azure Resources
Create Azure Portal Account
Create Subscription
Create IoT Hub
Create DPS
Link DPS to IoT Hub

## Create Self-Signed Cert
```bash
openssl genrsa -out private_key.pem 2048
# Common Name = global.azure-devices-provisioning.net
# all other metadata does not matter
openssl req -new -key private_key.pem -out cert_sign_req.csr
openssl x509 -req -days 365 -in cert_sign_req.csr -signkey private_key.pem -out X509testcert.pem
openssl x509 -outform DER -inform PEM -in X509testcert.pem -out device_cert_formatted.der
openssl rsa -inform PEM -outform DER -in private_key.pem -out private_key_formatted.der
xxd -i device_cert_formatted.der > cert.c
xxd -i private_key_formatted.der >> cert.c # append key to certificate file
```

1) Create an azure portal account, an IoT Hub, and a Device Provisioning Service (DPS)
2) Generate a self-signed cert using OpenSSL
```bash
openssl genrsa -out private_key.pem 2048
# Common Name = global.azure-devices-provisioning.net
# all other metadata does not matter
openssl req -new -key private_key.pem -out cert_sign_req.csr
openssl x509 -req -days 365 -in cert_sign_req.csr -signkey private_key.pem -out X509testcert.pem
```
3) Create an individual enrollment under your DPS and upload your X509testcert.pem as the primary certificate.
	a. Be sure to link your IoT hub here before saving
4) Follow these steps to set up the client to connect to your DPS
5) Before building the client project, we must build the custom_hsm_example projct to give the client project access to the cert
	a. Open provisioning_client\samples\custom_hsm_example\custom_hsm_example.c
	b. Copy and paste your global device endpoint (usually global.azure-devices-provisioning.net) into the string `COMMON_NAME`
	c. Copy and paste your self-signed certificate X509testcert.pem into the string `CERTIFICATE`
	d. Copy and paste your private key private_key.pem into the string `PRIVATE_KEY`
	e. If you haven't already done so, create a cmake folder under your azuire-iot-sdk-c folder `mkdir cmake`
	f. Now create cmake/hsm `mkdir cmake/hsm`
	g. `cd cmake/hsm`
	h. Now you will build the certificate project `cmake ..\..\provisioning_client\samples\custom_hsm_example`
	i. Now open the Project.sln with Visual Studio and build
6) Now you can configure and build prov_dev_client_sample
	a. Open provisioning_client\samples\prov_dev_client_sample\prov_dev_client_sample.c
	b. Set `global_prov_uri = "global.azure-devices-provisioning.net"` 
	c. Find your ID Scope under your global device endpoint on the DPS overview and set `id_scope = "<your_dps_id_scope>"`
7) Now you can build and run your project
	a. `cd ..`
	b. `cmake -Duse_prov_client:BOOL=ON -Dhsm_custom_lib=<path_to_azure-iot-sdk-c>\azure-iot-sdk-c\cmake\hsm\Debug\custom_hsm_example.lib ..`
	c. Ex: `cmake -Duse_prov_client:BOOL=ON -Dhsm_custom_lib=C:\Users\crazy_iot_guy\iot_projects\azure-iot-sdk-c\cmake\hsm\Debug\custom_hsm_example.lib ..`
	d. 
	e. Now open azure-iot-sdk-c/cmake/azure_iot_sdks.sln in Visual Studio
	f. Go to the Solution Explorer on the right of Visual Studio, right click Provision_Samples/prov_dev_client_sample, and click "Set as Startup Project"
	g. Now at the top of Visual Studio you should see a tab titled "Debug." Click this and then click "Start Without Debugging" or just hit ctrl + F5.

You should now see a new Visual Studio command line terminal pop up displaying the following information:
```bash
Registering Device

Provisioning Status: PROV_DEVICE_REG_STATUS_CONNECTED
Provisioning Status: PROV_DEVICE_REG_STATUS_ASSIGNING
Provisioning Status: PROV_DEVICE_REG_STATUS_ASSIGNING

Registration Information received from service: gettingstarted.azure-devices.net, deviceId: test-dpcs-cert-device
Press enter key to exit:
```
Now go to your IoT Hub on your Azure portal, and on the left side you should see IoT Devices under "Explorers". Click this and you should see a new device with the Device ID "test-dpcs-cert-device" and Authentication Type "SelfSigned"

You have now just automatically provisioned a virtual IoT device using a self-signed certificate and Azure's Device Provisioning Service!

Credits: Jelani Brandon, Ricardo Minguez Pablos (Rido), Ryan Walker
		
		
		
