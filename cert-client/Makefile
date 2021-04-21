RM = /bin/rm
CAT = /bin/cat
OPENSSL = /usr/bin/openssl

CERTS = client.pem

all: $(CERTS)

$(CERTS): $(CERTS:.pem=.cnf)
	ln -sf ../cert-server/root.pem root.pem
	ln -sf ../cert-server/rootcert.pem rootcert.pem
	$(OPENSSL) req -newkey rsa:2048 -sha256 -keyout clientkey.pem -out clientreq.pem -config client.cnf -reqexts req_extensions -passout file:passphrase
	$(OPENSSL) x509 -req -days 3650 -in clientreq.pem -sha256 -extfile client.cnf -extensions certificate_extensions -CA root.pem -CAkey root.pem -CAcreateserial -out clientcert.pem -passin file:passphrase
	$(CAT) clientcert.pem clientkey.pem rootcert.pem > client.pem

certcleanunused:
	$(RM) -f clientkey.pem clientreq.pem clientcert.pem

certclean:
	$(RM) -f clientkey.pem clientreq.pem clientcert.pem client.pem root.srl

clean: certcleanunused

distclean: clean certclean
