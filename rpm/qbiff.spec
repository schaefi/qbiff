#
# spec file for package qbiff (Version 7.3)
#
# Copyright (c) 2004 SUSE LINUX AG, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
#
#Compat macro for new _fillupdir macro introduced in Nov 2017
%if ! %{defined _fillupdir}
  %define _fillupdir /var/adm/fillup-templates
%endif

Name: qbiff
BuildRequires:  fdupes openssl libXtst-devel
BuildRequires:  kconfigwidgets-devel ki18n-devel kwallet-devel
BuildRequires:  libopenssl-devel
BuildRequires:  cmake extra-cmake-modules
%if 0%{?suse_version} > 1140
BuildRequires:  systemd
%endif
Summary:      Yet another biff implementation SSL server/client based
Version:      7.3
Release:      16
Group:        System/X11/Utilities
License:      Other License(s), see package, GPL
Source:       qbiff.tar.bz2
Source1:      qbiff.sysconfig
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Requires:     vim

%description
This package contains the qbiff application, which is a simple
button bar notifiying about new mail and allows to run an application
of your choice to read the mail

%package -n qbiffd
Requires(pre): %fillup_prereq
%if 0%{?suse_version} >= 1210
BuildRequires:  systemd-rpm-macros
%{?systemd_requires}
%else
Requires(pre):  %insserv_prereq
%endif
Summary:  Server part of qbiff
Group:    System/X11/Utilities

%description -n qbiffd
This package contains the qbiff server, which takes control over the
status of the maildir formatted mails on the machine the server
runs on.

Authors:
--------
    Marcus Schaefer <ms@suse.de>
#=================================================
# Preparation...
#-------------------------------------------------
%prep
%setup -n qbiff

%build
test -e /.buildenv && . /.buildenv
export CFLAGS=$RPM_OPT_FLAGS CXXFLAGS="$RPM_OPT_FLAGS" \
#=================================================
# build sources
#-------------------------------------------------
make -C qbiff all
make -C qbiffd all

#=================================================
# install sources
#-------------------------------------------------
%install
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/sbin
mkdir -p $RPM_BUILD_ROOT/usr/share/qbiff
mkdir -p $RPM_BUILD_ROOT/usr/share/qbiff/cert-server
mkdir -p $RPM_BUILD_ROOT/usr/share/qbiff/cert-client
mkdir -p $RPM_BUILD_ROOT/usr/share/qbiff/pixmaps

make -C qbiff/build DESTDIR="%{buildroot}" install
make -C qbiffd/build DESTDIR="%{buildroot}" install

install -m 755 qbiff-client         $RPM_BUILD_ROOT/usr/bin
install -m 755 qbiff-server         $RPM_BUILD_ROOT/usr/bin
install -m 755 readmail             $RPM_BUILD_ROOT/usr/share/qbiff
install -m 755 readmail.private     $RPM_BUILD_ROOT/usr/share/qbiff
install -m 644 readmail.txt         $RPM_BUILD_ROOT/usr/share/qbiff
install -m 644 readmail.private.txt $RPM_BUILD_ROOT/usr/share/qbiff

install -m 644 pixmaps/newmail.png  $RPM_BUILD_ROOT/usr/share/qbiff/pixmaps
install -m 644 pixmaps/nomail.png   $RPM_BUILD_ROOT/usr/share/qbiff/pixmaps
install -m 644 pixmaps/private.png  $RPM_BUILD_ROOT/usr/share/qbiff/pixmaps
install -m 644 pixmaps/public.png   $RPM_BUILD_ROOT/usr/share/qbiff/pixmaps
install -m 644 pixmaps/shape.xpm    $RPM_BUILD_ROOT/usr/share/qbiff/pixmaps

mkdir -p $RPM_BUILD_ROOT/usr/lib/systemd/user
install -m 755 systemd/qbiffd.service $RPM_BUILD_ROOT/usr/lib/systemd/user
install -m 755 systemd/qbiff.service $RPM_BUILD_ROOT/usr/lib/systemd/user

%{__install} -D -m 0644 %{S:1} %{buildroot}%{_fillupdir}/sysconfig.qbiffd

install -m 644 cert-server/rootcert.pem \
	$RPM_BUILD_ROOT/usr/share/qbiff/cert-server
install -m 644 cert-server/dh1024.pem \
	$RPM_BUILD_ROOT/usr/share/qbiff/cert-server
install -m 644 cert-server/dh512.pem \
	$RPM_BUILD_ROOT/usr/share/qbiff/cert-server
install -m 644 cert-server/server.pem \
	$RPM_BUILD_ROOT/usr/share/qbiff/cert-server

install -m 644 cert-client/client.pem \
	$RPM_BUILD_ROOT/usr/share/qbiff/cert-client
install -m 644 cert-client/rootcert.pem \
	$RPM_BUILD_ROOT/usr/share/qbiff/cert-client

%fdupes $RPM_BUILD_ROOT/usr/share/qbiff

%clean
%{__rm} -rf %{buildroot}

%post -n qbiffd
%{fillup_only -n qbiffd}

#=================================================
# qbiff files...      
#-------------------------------------------------
%files
%defattr(-,root,root)
%dir /usr/share/qbiff
/usr/bin/qbiff
/usr/bin/qbiff-client
%config /usr/share/qbiff/readmail
%config /usr/share/qbiff/readmail.private
/usr/share/qbiff/readmail.txt
/usr/share/qbiff/readmail.private.txt
/usr/share/qbiff/pixmaps
/usr/share/qbiff/cert-client
/usr/lib/systemd/user/qbiff.service

#=================================================
# qbiff server files...      
#-------------------------------------------------
%files -n qbiffd
%defattr(-,root,root)
%dir /usr/share/qbiff
/usr/bin/qbiffd
/usr/bin/qbiff-server
/usr/share/qbiff/cert-server
/usr/lib/systemd/user/qbiffd.service
%{_fillupdir}/sysconfig.qbiffd
