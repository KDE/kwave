%define ver 0.5.2
%define release 11
%define filelist %{name}/files.list.%{name}
%define rpm_opt_flags ${RPM_OPT_FLAGS}

Summary:        KDE, Qt, WAV, Sound, FFT, Sonagram, Sample, Effects, 24 Bit
Name:           kwave
Version: 	0.5.2
Release: 	11
Source0: 	kwave-0.5.2-11.tar.gz
Group:  	X11/Applications/Sound
Copyright:	 GPL
URL:            http://fs.spinfo.uni-koeln.de/~kwave
Vendor:          mwilz@ernie.mi.uni-koeln.de (Martin Wilz)
BuildRoot: 	/tmp/kwave-%{version}.root
Provides:           kwave
Requires: 	libkwave >= 0.3.8-2

%description
    A sound editor for KDE, beta release.

%prep

%setup -n kwave
rm -rf "$RPM_BUILD_ROOT"
rm -f config.cache
touch config.in
echo %{topdir}
./configure --with-install-root=$RPM_BUILD_ROOT \
	--enable-shared=yes --enable-static=no --enable-debug=no
# make distclean

%build -n kwave
cd libgui
make DESTDIR="$RPM_BUILD_ROOT" SUID_ROOT=""
cd ..
export LD_LIBRARY_PATH=`pwd`/libgui/.libs:${LD_LIBRARY_PATH}
make DESTDIR="$RPM_BUILD_ROOT" SUID_ROOT=""

%install -n kwave
rm -rf "$RPM_BUILD_ROOT"
cd libgui
make DESTDIR="$RPM_BUILD_ROOT" SUID_ROOT="" install
cd ..
make DESTDIR="$RPM_BUILD_ROOT" SUID_ROOT="" install
cd "$RPM_BUILD_ROOT"
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > "$RPM_BUILD_DIR"/%{filelist}
find . -type f | sed 's,^\.,\%attr(-\,root\,root) \%dir ,' >> "$RPM_BUILD_DIR"/%{filelist}
find . -type l | sed 's,^\.,\%attr(-\,root\,root) \%dir ,' >> "$RPM_BUILD_DIR"/%{filelist}
# remove files/directories that should not get into the package
mv "$RPM_BUILD_DIR"/%{filelist} /tmp/files.lst.tmp
cat /tmp/files.lst.tmp | grep -v /default$ > "$RPM_BUILD_DIR"/%{filelist}

%files -f files.list.%{name}

%doc GNU-LICENSE CHANGES README README.OLD TODO kwave.lsm

%post
ldconfig

%postun
ldconfig
