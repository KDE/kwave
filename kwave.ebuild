# Copyright 1999-2005 Gentoo Foundation
# $Header: /cvsroot/kwave/kwave/kwave.ebuild,v 1.9 2006/06/05 13:30:40 the Exp $

#############################################################################
##                                                                          #
##    This program is free software; you can redistribute it and/or modify  #
##    it under the terms of the GNU General Public License as published by  #
##    the Free Software Foundation; either version 2 of the License, or     #
##    (at your option) any later version.                                   #
##                                                                          #
#############################################################################

EAPI="1"
NEED_KDE="4.1"
inherit kde4-base flag-o-matic

DESCRIPTION="Kwave is a sound editor for KDE."
HOMEPAGE="http://kwave.sourceforge.net/"
SRC_URI="mirror://sourceforge/kwave/${P}-2.tar.bz2"

SLOT="kde-4"
LICENSE="GPL-2"
KEYWORDS="~x86 ~amd64 ~ppc"
IUSE="alsa debug doc flac mp3 ogg oss mmx"

LANGS="en de fr"
for X in ${LANGS} ; do
	IUSE="${IUSE} linguas_${X}"
done

RDEPEND="
	alsa? ( media-libs/alsa-lib )
	media-libs/audiofile
	mp3? ( media-libs/id3lib media-libs/libmad )
	ogg? ( media-libs/libogg media-libs/libvorbis )
	flac? ( media-libs/flac )
	sci-libs/fftw"

DEPEND="${RDEPEND}
	>=dev-util/cmake-2.4.6
	|| ( kde-base/kdesdk-misc kde-base/kdesdk )
	media-gfx/imagemagick"

pkg_setup() {
	strip-linguas ${LANGS}
}

src_compile() {
	use mmx && append-flags "-mmmx"

	myconf+=" -DWITH_BUILTIN_LIBAUDIOFILE=OFF"
	use alsa  || myconf+=" -DWITH_ALSA=OFF"
	use doc   || myconf+=" -DWITH_DOC=OFF"
	use flac  || myconf+=" -DWITH_FLAC=OFF"
	use mp3   && myconf+=" -DWITH_MP3=ON"
	use ogg   || myconf+=" -DWITH_OGG=OFF"
	use oss   || myconf+=" -DWITH_OSS=OFF"
	use debug && myconf+=" -DDEBUG=ON"

	cmake \
		-DCMAKE_C_COMPILER=$(type -P $(tc-getCC)) \
		-DCMAKE_CXX_COMPILER=$(type -P $(tc-getCXX)) \
		${myconf} \
	|| die "cmake failed"

	emake || die "Error: emake failed!"
}
