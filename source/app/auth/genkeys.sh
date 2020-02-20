#! /bin/bash
#
# Copyright © 2013-2020 Graphia Technologies Ltd.
#
# This file is part of Graphia.
#
# Graphia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Graphia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
#

openssl genpkey -algorithm RSA -out private_auth_key.pem -pkeyopt rsa_keygen_bits:4096
openssl rsa -pubout -in private_auth_key.pem -out public_auth_key.pem

openssl rsa -in private_auth_key.pem -pubout -outform DER -out public_auth_key.der
openssl pkcs8 -topk8 -inform PEM -outform DER -in private_auth_key.pem -out private_auth_key.der -nocrypt

