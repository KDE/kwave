# Copyright 1999-2004 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Id$
# $Header$

inherit eutils

DESCRIPTION="A sound editor for KDE3."
HOMEPAGE="http://kwave.sourceforge.net"
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.gz"
RESTRICT="nomirror"

LICENSE="GPL-2"
IUSE="kdeenablefinal debug mmx"
SLOT="0"
KEYWORDS="~x86"

DEPEND="sys-apps/sed
		sys-apps/gawk
		sys-apps/coreutils
		sys-apps/findutils
		>=sys-devel/autoconf-2.50
		>=sys-devel/automake-1.7.32
		>=sys-devel/gcc-3.2
		>=sys-devel/make-3.80
		>=sys-libs/glibc-2.2.5
		>=sys-devel/gettext-0.11.5
		media-gfx/imagemagick
		app-text/recode
		>=x11-libs/qt-3.0.5
		>=kde-base/kdelibs-3.0.4
		>=kde-base/kdemultimedia-3.0.4
		>=kde-base/kdesdk-3.0.4
		>=kde-base/arts-1.0.4
		>=media-libs/flac-1.1.0
		>=media-libs/libmad-0.14
		>=media-libs/libogg-1.0
		>=media-libs/libvorbis-1.0
		>=media-libs/id3lib-3.8.1
		>=sci-libs/gsl-1.4"

src_compile() {

	local myconf
	myconf=" "

	use kdeenablefinal && myconf="${myconf} --enable-final"
	use debug && myconf="${myconf} --enable-debug"
	use mmx && append-flags -mmmx

	# avoid sandbox warnings
	addpredict ${QTDIR}/etc/settings/.qtrc.lock
	addpredict ${QTDIR}/etc/settings/.qt_plugins_*
	addpredict /var/lib/rpm/__db.Providename

	# configure
	make -f Makefile.dist \
		RPM_OPT_FLAGS="${CFLAGS}" \
		CONFIGURE_OPTS="${myconf} ${CONFIGURE_OPTS}" \
		|| die "./configure failed"

	# compile
	MAKE_FLAGS="${MAKEOPTS}" emake || die "emake failed"
}

src_install() {
	# install
	make DESTDIR=${D} install || die
}
