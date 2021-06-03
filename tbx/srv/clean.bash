for i in cacert.crt cacert.pem config.cnf mycert.pem mycert.pfx mykey.pem myreq.pem server_crt.pem server_key.pem srv.cnf private/ signedcerts/ 
do
	if [ -f $i -o -d $i ] 
	then
		echo removing $i...
		rm -rf $i
	fi
done
