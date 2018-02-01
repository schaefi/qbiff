#
# spec file for package qbiff (Version 7.2)
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
BuildRequires:  fdupes cmake openssl
BuildRequires:  libqt4 libqt4-devel libqt4-x11
BuildRequires:  openssl-devel
BuildRequires:  libkde4-devel
BuildRequires:  cmake
%if 0%{?suse_version} > 1140
BuildRequires:  systemd
%endif
Summary:      Yet another biff implementation SSL server/client based
Version:      7.2
Release:      16
Group:        System/X11/Utilities
License:      Other License(s), see package, GPL
Source:       qbiff.tar.bz2
Source1:      qbiff.sysconfig
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
%kde4_runtime_requires
Requires:     vim

%description
This package contains the qbiff application, which is a simple
button bar notifiying about new mail and allows to run an application
of your choice to read the mail

%package -n qbiffd
Requires(pre): %fillup_prereq
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
(
	mkdir build && cd build && \
	cmake ../ -DCMAKE_INSTALL_PREFIX="/usr"
)
make -C build all

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

make -C build DESTDIR="%{buildroot}" install

install -m 755 $RPM_BUILD_ROOT/usr/bin/qbiff  $RPM_BUILD_ROOT/usr/bin/qbiffd
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

%if %{suse_version} <= 1140
mkdir -p $RPM_BUILD_ROOT/etc/init.d
install -m 755 sysvinit/qbiffd $RPM_BUILD_ROOT/etc/init.d
rm -f %{buildroot}%{_sbindir}/rcqbiffd
%{__ln_s} ../../etc/init.d/qbiffd %{buildroot}%{_sbindir}/rcqbiffd
%else
mkdir -p $RPM_BUILD_ROOT/usr/lib/systemd/system
install -m 755 systemd/qbiffd.service $RPM_BUILD_ROOT/usr/lib/systemd/system
%endif

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

%preun -n qbiffd
%{stop_on_removal qbiffd}

%post -n qbiffd
%{fillup_and_insserv}

%postun -n qbiffd
%{restart_on_update qbiffd}
%{insserv_cleanup}

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

#=================================================
# qbiff server files...      
#-------------------------------------------------
%files -n qbiffd
%defattr(-,root,root)
%dir /usr/share/qbiff
/usr/bin/qbiffd
/usr/bin/qbiff-server
/usr/share/qbiff/cert-server
%if %{suse_version} <= 1140
%attr(0755,root,root) %{_initddir}/qbiffd
%{_sbindir}/rcqbiffd
%else
%{_unitdir}/qbiffd.service
%endif
%{_fillupdir}/sysconfig.qbiffd
