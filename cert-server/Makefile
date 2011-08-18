
RM = /bin/rm
CAT = /bin/cat
OPENSSL = /usr/bin/openssl

CERTS = root.pem serverCA.pem server.pem

DHPARAMS = dh512.pem dh1024.pem

all: $(CERTS) $(DHPARAMS)

$(CERTS): $(CERTS:.pem=.cnf)
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout rootkey.pem -out rootreq.pem -config root.cnf -passout file:passphrase
	$(OPENSSL) x509 -req -days 3650 -in rootreq.pem -sha1 -extfile root.cnf -extensions certificate_extensions -signkey rootkey.pem -out rootcert.pem -passin file:passphrase
	$(CAT) rootcert.pem rootkey.pem > root.pem
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout serverCAkey.pem -out serverCAreq.pem -config serverCA.cnf -passout file:passphrase
	$(OPENSSL) x509 -req -days 3650 -in serverCAreq.pem -sha1 -extfile serverCA.cnf -extensions certificate_extensions -CA root.pem -CAkey root.pem -CAcreateserial -out serverCAcert.pem -passin file:passphrase
	$(CAT) serverCAcert.pem serverCAkey.pem rootcert.pem > serverCA.pem
	$(OPENSSL) req -newkey rsa:1024 -sha1 -keyout serverkey.pem -out serverreq.pem -config server.cnf -reqexts req_extensions -passout file:passphrase
	$(OPENSSL) x509 -req -days 3650 -in serverreq.pem -sha1 -extfile server.cnf -extensions certificate_extensions -CA serverCA.pem -CAkey serverCA.pem -CAcreateserial -out servercert.pem -passin file:passphrase
	$(CAT) servercert.pem serverkey.pem serverCAcert.pem rootcert.pem > server.pem

certcleanunused:
	$(RM) -f rootkey.pem rootreq.pem root.pem root.srl
	$(RM) -f serverCAkey.pem serverCAreq.pem serverCAcert.pem serverCA.pem serverCA.srl
	$(RM) -f serverkey.pem serverreq.pem servercert.pem

certclean:
	$(RM) -f rootkey.pem rootreq.pem rootcert.pem root.pem root.srl
	$(RM) -f serverCAkey.pem serverCAreq.pem serverCAcert.pem serverCA.pem serverCA.srl
	$(RM) -f serverkey.pem serverreq.pem servercert.pem server.pem

dh512.pem:
	$(OPENSSL) dhparam -check -text -5 512 -out dh512.pem

dh1024.pem:
	$(OPENSSL) dhparam -check -text -5 1024 -out dh1024.pem

dhparamclean:
	$(RM) -f $(DHPARAMS)

clean: certcleanunused
	
distclean: clean certclean dhparamclean
