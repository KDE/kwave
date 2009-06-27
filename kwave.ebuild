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

EAPI="2"
KDE_MINIMAL="4.2"
KDE_LINGUAS="cs de en fr"
inherit kde4-base

DESCRIPTION="Kwave is a sound editor for KDE."
HOMEPAGE="http://kwave.sourceforge.net/"
SRC_URI="mirror://sourceforge/kwave/${P}-1.tar.bz2"

SLOT="kde-4"
LICENSE="GPL-2"
KEYWORDS="~x86 ~amd64 ~ppc"
IUSE="alsa debug doc flac mp3 ogg oss phonon mmx"

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

src_prepare() {
	kde4-base_src_prepare
}

src_configure() {
	use mmx && append-flags "-mmmx"

	use alsa   || mycmakeargs+=" -DWITH_ALSA=OFF"
	use doc    || mycmakeargs+=" -DWITH_DOC=OFF"
	use flac   || mycmakeargs+=" -DWITH_FLAC=OFF"
	use mp3    && mycmakeargs+=" -DWITH_MP3=ON"
	use ogg    || mycmakeargs+=" -DWITH_OGG=OFF"
	use oss    || mycmakeargs+=" -DWITH_OSS=OFF"
	use debug  && mycmakeargs+=" -DDEBUG=ON"
	use phonon && mycmakeargs+=" -DWITH_PHONON=ON"

	kde4-base_src_configure
}
