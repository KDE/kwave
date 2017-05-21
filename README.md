# Kwave
_A sound editor built on KDE Frameworks 5._


### Kwave is hosted on KDE and on SourceForge, so you are welcome to visit the Kwave homepage at http://kwave.sourceforge.net for details.         
--- 

If you are interested what has been done and what has to be done, then
look at the files "CHANGES" and "TODO" included in this package.

The project is developed and published under the GNU GENERAL PUBLIC LICENSE
(Version 2, from June 1991), take a look at the file "GNU-LICENSE" included
in the source package.

# 0. PREFACE:

Please note that required packages for compiling Kwave vary between
different distributions and versions. It is practically impossible
to give a complete list of packages and versions, so if you can't
get along, please feel free to ask on the Kwave mailing list!

# 1. COMPILING MANUALLY:

\>>> _NOTE: You don't have to do this on an rpm based system, you can
          easily build a rpm package of Kwave. Please skip this and
	  advance to section 2._

It's quite simple. First get the source archive, unpack it in a driectory
of your choice and then generate it in a separate build directory.
For example:


```
cd ${HOME}/src
tar -xvzf kwave-0.9.3-1.tar.bz2
mkdir kwave-build
cd kwave-build
cmake ../kwave-0.9.3

make
make install
```

# 2. RPM-SUPPORT:

You can build nice binary and/or source rpm packages of kwave in one
of the following ways. Note: Either you do everything as root or you
must have write permissions in the following directories:

* /tmp
* /usr/src/packages/BUILD
* /usr/src/packages/RPMS
* /usr/src/packages/SOURCES
* /usr/src/packages/SPECS
* /usr/src/packages/SRPMS

Installing the rpm package normally requires root permissions.


## 2.1 Building a binary rpm from archive file:


Simply get the source archive (for example kwave-0.9.3-1.tar.bz2) and
then type:

```rpmbuild -ta kwave-0.9.3-1.tar.bz2```

If you want to build for a different architecture than your current, you
might want to add the parameter "--target ...", for example:

```rpmbuild -ta --target i586 kwave-0.9.3-1.tar.bz2```

compiles for i586 architecture.


## 2.2 Building a source and binary rpm from the source tree (GIT):

Assuming that you already have unpacked the source archive in some
directory, like in chapter 1, but instead of the combination
"make" / "make install" you do

`make src_rpm`          (and you get only a source rpm)
***OR***
`make rpm`              (and you get both, a binary and a source rpm)

Instructions on how to get the sources via GIT can be found in the
Kwave handbook and on the Kwave project homepage at SourceForge
(see http://sourceforge.net/p/kwave/code/ci/master/tree/).

Please note that the online documentation in not included in GIT, it will
created from the corresponding docbook source which is included.
This requires a working docbook/sgml environment that is sometimes
problematic (see below).


# 3. RELOCATING THE BINARY RPM:

The binary rpm package of kwave is "relocatable". This means that you can
build the package on a system with the KDE base directory set to some
location (like for example /usr) and install it on some other system
and/or into a different directory.

```rpm -Uvh --prefix=/usr/local kwave-0.9.3-1.i586.rpm```

This modifies the path where the program is installed and it is strongly
recommended that this is a directory that is contained in the KDEDIRS
environment variable, otherwise Kwave would be unable to find it's menu
configuration, plugins, preset files and so on...

# 4. REQUIREMENTS:

\>>> __Please read the preface (section 0) before this__ <<<

The revision codes (numbers after the '-') should not be so important.
As a rule of thumb one can say that the nearer your version number is
the better it will work.

For compilation you need a working autoconf/automake environment, a good
C/C++ compiler, the qt and the kde libraries.

I am currently developing under a Gentoo Linux distribution
(i586 architecture) using at least the following packages:

* cmake >= 2.8.12			(the cmake build system)
* gcc >= 4.7				(the C / C++ compiler)
* make >= 3.80
* libstdc++-v3 >= 3.3.4			(C++ library, including STL)
* glibc >= 2.3				(the GNU C library)
* kernel-headers			(needed by glibc-devel)
* gettext >= 0.12			(for internationalization)
* rpm >= 4.0				(optional, for rpm support)
* ImageMagick >= 6.1			(needed to create scaled Kwave icons)
* libmad + libmad-devel >= 0.15		(for MP3 import)
* id3lib >= 3.8.1			(for MP3 tag import, see www.id3lib.org)
* libogg >= 1.1.2			(for Ogg/Vorbis import/export)
* libvorbis >= 1.1.0			(for Ogg/Vorbis import/export)
* flac >= 1.2.0				(for FLAC import/export)
* fftw >= 3.0				(for Sonagram plugin / FFT)
* pulseaudio >= 0.9.16			(for record/playback via PulseAudio)
* samplerate >= 0.1.3			(for sample rate conversion)
* gettext				(for msgmerge)

Some tools that are normally installed in every distribution:
* sed, bash, msgcat, msgmerge, msgfmt, xgettext, cat, find, bzip2, perl

Nearly everything from ftp.kde.org, especially the following
packets should be sufficient for building:

[WARNING: this list might be a bit outdated]

* the following Qt packages Qt >= 5.4.0
  - qtcore
  - qtconcurrent
  - qtnetwork
  - qtgui
  - qtmultimedia
  - qtwidgets
* the following KDE Frameworks packages, KF5 >= 5.2.0
  - extra-cmake-modules
  - kconfig
  - kconfigwidgets
  - kcoreaddons
  - kcrash
  - kdbusaddons
  - kdoctools
  - ki18n
  - kiconthemes
  - kio
  - kservice
  - ktextwidgets
  - kxmlgui
  - kwidgetsaddons

additionally, for generating the online help;

  - poxml (for po2xml and xml2pot)
  - kdoctools (for checkXML5)

## known distributions / architectures:

see the Kwave homepage: http://kwave.sf.net/distributions.html

Compilation might also work on many other distributions and different
versions of compilers and libraries, or even on different architectures.

So if you have success in compiling and using kwave under a different system,
please let me know !

If the program does NOT compile, please let me know too - and/or consider
an update of your system if your packages are older than those mentioned
above.

## currently tested distributions:

see http://kwave.sourceforge.net/distributions.html

# 5. KNOWN PROBLEMS / SOME HINTS:

* ## missing files:

  Depending on your distribution, one ore more components might require
  additional packages. Often you will need the "-devel" packages for
  building (only) Kwave, e.g. it might not be sufficient to have the
  package "libaudiofile" installed, additionally the corresponding package
  "audiofile-devel" might be required too!

  Hint for SuSE users:
  => on CD1 (or on the DVD1) there is a file named "ARCHIVES.gz".
     If you want to find out which package does contain a missing program or
     file, you can do the following:

     ```gzip -dc ARCHIVES.gz | grep name_of_the_missing_file```

* ## warnings about invalid character set when creating online documentation:

  This is quite "normal", because some tools are not aware of the
  UTF-8 encoding that is used in the .docbook and the .po files.

* ## hint: compiling in parallel with distcc:

  I successfully compiled Kwave on many systems with the following
  method:

  `CC=distcc CXX="distcc g++" cmake `  ...your parameters...
  `make -j <JOBS>`

  (replace <JOBS> with a number of parallel jobs, for example the
   number of CPUs on all hosts + number of distcc hosts)

# 6. SOME HISTORY:

This project has been started by Martin Wilz in summer 1998 and has been
developed and improved by him an some other people. In November 1999 I started
to fix some little bugs here and there and stepped into the source code of
the program deeper and deeper. Up to today I have extended, rewritten or
revised nearly every component of the program and spend much time on it.
Since summer 1999 I have taken over the project leadership and I am still
working on it.

So good luck, and feel free to keep me informed about bugs and wishes...

   Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

