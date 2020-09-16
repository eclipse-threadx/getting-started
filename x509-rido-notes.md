Rido Meeting Notes:

To get x509 to work:

1) Create root cert with script create-RootCerts.ps1
2) Locate root cert file and export using certmgr.msc
3) Copy root cert to DPS Certificates
4) Validate root cert using script create-validationcert.ps1
5) Copy validation string to root cert on DPS Certificates (double click root cert)
6) Create device cert with script create-deviceCerts.ps1
7) Sign device cert with root cert using script create-self-signed-cert.ps1
8) *assuming there is a config string for x509 in getting-started*
    place device cert thumbprint in x509 config string along with scope id, device id, and dps enpoint
9) compile and pray

Resources:
Rido Web Client: https://mqtt.rido.dev/
Cert Scripts: https://github.com/ridomin/RigadoS1-PnPDemo/tree/master/_tools
DeviceClientFactory: https://github.com/ridomin/DeviceClientFactory
