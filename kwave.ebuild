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
inherit gnome2-utils kde4-base

DESCRIPTION="Kwave is a sound editor for KDE."
HOMEPAGE="http://kwave.sourceforge.net/"
SRC_URI="mirror://sourceforge/kwave/${P}-1.tar.bz2"

LICENSE="GPL-2"
SLOT="4"
KEYWORDS="~amd64 ~ppc ~x86"
IUSE="alsa debug doc flac mp3 ogg oss phonon pulseaudio +samplerate mmx"

RDEPEND="
	!media-sound/kwave:0
	media-libs/audiofile
	sci-libs/fftw
	alsa? ( media-libs/alsa-lib )
	flac? ( media-libs/flac )
	media-libs/libsamplerate
	mp3? ( media-libs/id3lib media-libs/libmad )
	ogg? ( media-libs/libogg media-libs/libvorbis )
	pulseaudio? ( media-sound/pulseaudio )"

DEPEND="${RDEPEND}
	>=dev-util/cmake-2.6.0
	>=kde-base/kdesdk-misc-${KDE_MINIMAL}[extras]
	media-gfx/imagemagick"

src_configure() {
	use mmx && append-flags -mmmx

	mycmakeargs="${mycmakeargs}
		$(cmake-utils_use_with alsa)
		$(cmake-utils_use_with doc)
		$(cmake-utils_use_with flac)
		$(cmake-utils_use_with mp3)
		$(cmake-utils_use_with ogg)
		$(cmake-utils_use_with oss)
		$(cmake-utils_use_with pulseaudio)
		$(cmake-utils_use_with libsamplerate SAMPLERATE)
		$(cmake-utils_use debug)"

	kde4-base_src_configure
}

src_install() {
	kde4-base_src_install
}

pkg_preinst() {
	gnome2_icon_savelist
}

pkg_postinst() {
	kde4-base_pkg_postinst
	gnome2_icon_cache_update
}

pkg_postrm() {
	kde4-base_pkg_postrm
	gnome2_icon_cache_update
}
