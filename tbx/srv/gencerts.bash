echop() {
	echo "--> ${green} toolbox/tbx ssl test$norm"
}
for i in signedcerts private 
do 
	[ -d $i ] || mkdir $i
done
CNF=$PWD/config.cnf
SRV=$PWD/srv.cnf

echo $bold$blue"create index.txt"$norm
echo '01' > serial  && touch index.txt 
echo $bold$blue"create $CNF file"$norm
cat >$CNF<<EOB1
# Default configuration to use when one is not provided on the command line.
#
[ ca ]
default_ca      = local_ca
#
#
# Default location of directories and files needed to generate certificates.
#
[ local_ca ]
certificate     = $PWD/cacert.pem
database        = $PWD/index.txt
new_certs_dir   = $PWD/signedcerts
private_key     = $PWD/private/cakey.pem
serial          = $PWD/serial
#       
#
# Default expiration and encryption policies for certificates.
#
default_crl_days        = 365
default_days            = 1825
default_md              = sha1
#       
policy          = local_ca_policy
x509_extensions = local_ca_extensions
#
#
# Copy extensions specified in the certificate request
#
copy_extensions = copy
#       
#
# Default policy to use when generating server certificates.  The following
# fields must be defined in the server certificate.
#
[ local_ca_policy ]
commonName              = supplied
stateOrProvinceName     = supplied
countryName             = supplied
emailAddress            = supplied
organizationName        = supplied
organizationalUnitName  = supplied
#       
#
# x509 extensions to use when generating server certificates.
#
[ local_ca_extensions ]
basicConstraints        = CA:false
#       
#
# The default root certificate generation policy.
#
[ req ]
default_bits    = 2048
default_keyfile = $PWD/private/cakey.pem
default_md      = sha1
#       
prompt                  = no
distinguished_name      = root_ca_distinguished_name
x509_extensions         = root_ca_extensions
#
#
# Root Certificate Authority distinguished name.  Change these fields to match
# your local environment!
#
[ root_ca_distinguished_name ]
commonName              = toolbox/tbx test certificate
stateOrProvinceName     = 
countryName             = FR
emailAddress            = root@localhost
organizationName        = org
#       
[ root_ca_extensions ]
basicConstraints        = CA:true
EOB1

export OPENSSL_CONF=$CNF
echop
echo $bold$yellow"Create certificate:"$norm
openssl req -x509 -newkey rsa:2048 -out cacert.pem -outform PEM -days 1825
openssl x509 -in cacert.pem -out cacert.crt
echo $bold$blue"create $SRV file"$norm
cat >$SRV<<EOB2
#
# srv.cnf
#
[ req ]
prompt                  = no
distinguished_name      = server_distinguished_name
req_extensions          = v3_req

[ server_distinguished_name ]
commonName              = localhost
countryName             = FR
emailAddress            = root@localhost
organizationName        = toolbox/tbx test
organizationalUnitName  = ssl test 

[ v3_req ]
basicConstraints        = CA:FALSE
keyUsage                = nonRepudiation, digitalSignature, keyEncipherment
subjectAltName          = @alt_names

[ alt_names ]
email                   = root@localhost
DNS.0                   = 8.8.8.8 
DNS.1                   = 8.8.4.4
EOB2
echop
export OPENSSL_CONF=$SRV
echo $bold$yellow"Create tempkey & server key:"$norm
openssl req -newkey rsa:1024 -keyout tempkey.pem -keyform PEM -out tempreq.pem -outform PEM 
echo $bold$yellow"convert tempkey to unencrypted key:"$norm
openssl rsa < tempkey.pem > server_key.pem 
export OPENSSL_CONF=$CNF
echo $bold$yellow"Sign certificate:"$norm
openssl ca -in tempreq.pem -out server_crt.pem 
echo $bold$blue"clean up"$norm
rm -f tempkey.pem && rm -f tempreq.pem
cat server_key.pem server_crt.pem > hold.pem
mv      hold.pem server_crt.pem
chmod   400      server_crt.pem
echo $bold$yellow"Sign certificate:"$norm
openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem 
openssl pkcs12 -export -out mycert.pfx -in mycert.pem -name "toolbox/tbx ssl test" 
echo $bold$yellow"generate private key:"$norm
openssl req -new -newkey rsa:1024 -nodes -keyout mykey.pem -out myreq.pem 
