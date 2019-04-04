#! /bin/bash

openssl genpkey -algorithm RSA -out private_auth_key.pem -pkeyopt rsa_keygen_bits:4096
openssl rsa -pubout -in private_auth_key.pem -out public_auth_key.pem

openssl rsa -in private_auth_key.pem -pubout -outform DER -out public_auth_key.der
openssl pkcs8 -topk8 -inform PEM -outform DER -in private_auth_key.pem -out private_auth_key.der -nocrypt
