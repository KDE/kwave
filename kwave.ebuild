# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /cvsroot/kwave/kwave/kwave.ebuild,v 1.9 2006/06/05 13:30:40 the Exp $

inherit kde flag-o-matic

DESCRIPTION="Kwave is a sound editor for KDE."
HOMEPAGE="http://kwave.sourceforge.net/"
SRC_URI="mirror://sourceforge/kwave/${P}.tar.gz"

SLOT="0"
LICENSE="GPL-2"
KEYWORDS="~x86 ~amd64 ~ppc"
IUSE="alsa doc flac mp3 ogg oss mmx"

RDEPEND="kde-base/arts
	alsa? ( media-libs/alsa-lib )
	media-libs/audiofile
	mp3? ( media-libs/id3lib media-libs/libmad )
	ogg? ( media-libs/libogg media-libs/libvorbis )
	flac? ( media-libs/flac )
	sci-libs/gsl"
DEPEND="${RDEPEND}
	>=dev-util/cmake-2.4.6
	|| ( kde-base/kdesdk-misc kde-base/kdesdk )
	|| ( kde-base/kdemultimedia-arts kde-base/kdemultimedia )
	app-text/recode
	media-gfx/imagemagick"
need-kde 3.4

pkg_setup() {
	if ! built_with_use kdelibs arts ; then
		eerror "KWave needs aRts, please rebuild kdelibs with arts use flag enabled."
		die
	fi
}

src_compile() {
	use mmx && append-flags "-mmmx"

	myconf+=" -DWITH_BUILTIN_LIBAUDIOFILE=OFF"
	use alsa || myconf+=" -DWITH_ALSA=OFF"
	use doc  || myconf+=" -DWITH_DOC=OFF"
	use flac || myconf+=" -DWITH_FLAC=OFF"
	use mp3  || myconf+=" -DWITH_MP3=OFF"
	use ogg  || myconf+=" -DWITH_OGG=OFF"
	use oss  || myconf+=" -DWITH_OSS=OFF"

	cmake \
		-DCMAKE_C_COMPILER=$(type -P $(tc-getCC)) \
		-DCMAKE_CXX_COMPILER=$(type -P $(tc-getCXX)) \
		${myconf} \
	|| die "cmake failed"

	emake || die "Error: emake failed!"
}
