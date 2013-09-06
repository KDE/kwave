# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/media-sound/kwave/kwave-0.8.7.ebuild,v 1.2 2012/01/05 12:38:06 johu Exp $

EAPI=5

KDE_HANDBOOK="optional"
KDE_LINGUAS="cs de en_GB es fr"
inherit kde4-base

DESCRIPTION="A sound editor for KDE that can edit many types of audio files"
HOMEPAGE="http://kwave.sourceforge.net/"
SRC_URI="mirror://sourceforge/${PN}/${P}-1.tar.bz2"

LICENSE="BSD GPL-2 LGPL-2
	handbook? ( FDL-1.2 )"
SLOT="4"
KEYWORDS="~amd64 ~ppc ~x86"
IUSE="alsa debug flac handbook mp3 oss opus phonon pulseaudio vorbis"

RDEPEND="
	media-libs/audiofile
	>=sci-libs/fftw-3
	media-libs/libsamplerate
	alsa? ( media-libs/alsa-lib )
	flac? ( media-libs/flac )
	mp3? (
		media-libs/id3lib
		media-libs/libmad
		|| ( media-sound/lame media-sound/twolame media-sound/toolame )
	)
	opus? (
		media-libs/libogg
		media-libs/opus
	)
	phonon? ( media-libs/phonon )
	pulseaudio? ( media-sound/pulseaudio )
	vorbis? (
		media-libs/libogg
		media-libs/libvorbis
	)
"
DEPEND="${RDEPEND}
	$(add_kdebase_dep poxml extras)
	|| ( media-gfx/imagemagick[png,svg] media-gfx/graphicsmagick[imagemagick,png,svg] )
"

DOCS=( AUTHORS CHANGES LICENSES README TODO )

src_configure() {
	local mycmakeargs=(
		$(cmake-utils_use_with alsa ALSA)
		$(cmake-utils_use_with handbook DOC)
		$(cmake-utils_use_with flac FLAC)
		$(cmake-utils_use_with mp3 MP3)
		$(cmake-utils_use_with vorbis OGG_VORBIS)
		$(cmake-utils_use_with opus OGG_OPUS)
		$(cmake-utils_use_with oss OSS)
		$(cmake-utils_use_with phonon PHONON)
		$(cmake-utils_use_with pulseaudio PULSEAUDIO)
		$(cmake-utils_use debug DEBUG)
	)

	kde4-base_src_configure
}
