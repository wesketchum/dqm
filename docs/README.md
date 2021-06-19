# Data Quality Monitoring
Software and tools for data monitoring

Work in progress

## How to run
The following modules are needed: dataformats, dfmessages, dfmodules, readout
and serialization in 2.6.0. readout has to be in the branch `floriangroetschla/request_types`

## Using the Kafka exporter
The Exporter.hpp module runs using the librdkafka library. To enable this library, once you have set up your build environment run:

source /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products_dev/setup
setup -B librdkafka v1_7_0 -q e19:prof

To authenticate its connection to the broadcast service, it requires 3 certificates: a .pem file, a .crt.pem file, and a .key.pem file. For development, certificates have been generated and stored at /afs/cern.ch/user/p/pahamilt/certs/. If you are running from a location where you cannot access these files, you may want to generate your own and point Exporter.hpp to them.

## Steps for generating your own certificates 
For the .crt.pem and .key.pem files: obtain a .p12 GRID certificate from the CERN certification authority: https://ca.cern.ch/ca/user/Request.aspx?template=ee2user
Then use the following commands to generate the .crt.pem and .key.pem certificates from the .p12 file:

openssl pkcs12 -in <cert.p12> -out <cert.crt.pem> -clcerts -nokeys

openssl pkcs12 -in <cert.p12> -out <cert.key.pem> -nocerts -nodes

For the .pem file: execute the following commands

set -e

curl -k https://cafiles.cern.ch/cafiles/certificates/CERN%20Root%20Certification%20Authority%202.crt -o CERNRootCertificationAuthority2.crt

curl -k https://cafiles.cern.ch/cafiles/certificates/CERN%20Grid%20Certification%20Authority.crt -o CERNGridCertificationAuthority.crt

openssl x509 -in CERNRootCertificationAuthority2.crt -inform DER  -out CERNRoot.pem

cat CERNRoot.pem CERNGridCertificationAuthority.crt > cerncacerts.pem

rm CERNRoot.pem CERNGridCertificationAuthority.crt CERNRootCertificationAuthority2.crt
