# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /cvsroot/kwave/kwave/kwave.ebuild,v 1.9 2006/06/05 13:30:40 the Exp $

EAPI="1"
NEED_KDE="4.0"
inherit kde4-base

DESCRIPTION="Kwave is a sound editor for KDE."
HOMEPAGE="http://kwave.sourceforge.net/"
SRC_URI="mirror://sourceforge/kwave/${P}-1.tar.gz"

SLOT="0.8"
LICENSE="GPL-2"
KEYWORDS="~x86 ~amd64 ~ppc"
IUSE="alsa debug doc flac mp3 ogg oss mmx"

RDEPEND="
	alsa? ( media-libs/alsa-lib )
	media-libs/audiofile
	mp3? ( media-libs/id3lib media-libs/libmad )
	ogg? ( media-libs/libogg media-libs/libvorbis )
	flac? ( media-libs/flac )
	sci-libs/gsl"
DEPEND="${RDEPEND}
	>=dev-util/cmake-2.4.6
	|| ( kde-base/kdesdk-misc kde-base/kdesdk )
	app-text/recode
	media-gfx/imagemagick"

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
