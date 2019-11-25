#!/bin/sh

# 1.生成私钥
openssl genrsa -out rythem.pem 2048

# 2.生成 CSR (Certificate Signing Request)
openssl req -subj "/C=CN/ST=sz/L=sz/O=Mocha/OU=AlloyTeam Software/CN=rythem.alloyteam.com/emailAddress=iptton@gmail.com" -new -key rythem.pem -out rythem.csr

# 3.生成自签名证书
openssl x509 -req -days 3650 -in rythem.csr -signkey rythem.pem -out rythem.crt
