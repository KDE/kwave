# Kwave
## a sound editor built on the KDE Frameworks

It is hosted by [KDE](https://invent.kde.org/multimedia/kwave) and
mirrored on [SourceForge](https://sourceforge.net/projects/kwave/).
You are welcome to visit the
[Kwave homepage](http://kwave.sourceforge.net) for details.

If you are interested what has been done and what has to be done, then
look at the files "CHANGES" and "TODO" included in this package.

The project is developed and published under the GNU GENERAL PUBLIC LICENSE
(Version 2, from June 1991), take a look at the file "GNU-LICENSE" included
in the source package.

## Building

It's quite simple. First clone the source repository then build it in a
separate build directory.
For example:

``` bash
git clone https://invent.kde.org/multimedia/kwave.git
cd kwave
mkdir build
cd build
cmake ..

make
make install
```

## Requirements

Please note that required packages for compiling Kwave vary between
different distributions and versions. It is practically impossible
to give a complete list of packages and versions!

Beside the standard tools that most KDE projects require like cmake
and a C++ compiler, you will need a few other libraries:

- libmad (for MP3 import)
- id3lib (for MP3 tag import, see www.id3lib.org)
- libogg (for Ogg/Vorbis import/export)
- libvorbis (for Ogg/Vorbis import/export)
- flac (for FLAC import/export)
- fftw (for Sonagram plugin / FFT)
- pulseaudio (for record/playback via PulseAudio)
- samplerate (for sample rate conversion)

Qt:
  - qtconcurrent
  - qtcore
  - qtmultimedia
  - qtwidgets

KDE Frameworks:
  - extra-cmake-modules
  - karchive
  - kcompletion
  - kconfig
  - kconfigwidgets
  - kcoreaddons
  - kcrash
  - kdbusaddons
  - kdoctools
  - ki18n
  - kiconthemes
  - kio
  - ktextwidgets
  - kxmlgui
  - kwidgetsaddons

known distributions / architectures:
------------------------------------

see the Kwave homepage: http://kwave.sf.net/distributions.html

Compilation might also work on many other distributions and different
versions of compilers and libraries, or even on different architectures.

So if you have success in compiling and using kwave under a different system,
please let me know !

If the program does NOT compile, please let me know too - and/or consider
an update of your system if your packages are older than those mentioned
above.

currently tested distributions:
-------------------------------------------

see http://kwave.sourceforge.net/distributions.html

## Known Problems / Some Hints

* missing files:
  -------------
  Depending on your distribution, one or more components might require
  additional packages. Often you will need the "-devel" packages for
  building (only) Kwave, e.g. it might not be sufficient to have the
  package "libaudiofile" installed, additionally the corresponding package
  "audiofile-devel" might be required too!

  Hint for SuSE users:
  => on CD1 (or on the DVD1) there is a file named "ARCHIVES.gz".
     If you want to find out which package does contain a missing program or
     file, you can do the following:

     gzip -dc ARCHIVES.gz | grep name_of_the_missing_file

* warnings about invalid character set when creating online documentation:
  -----------------------------------------------------------------------
  This is quite "normal", because some tools are not aware of the
  UTF-8 encoding that is used in the .docbook and the .po files.

* hint: compiling in parallel with distcc:
  ---------------------------------------
  I successfully compiled Kwave on many systems with the following
  method:

  ```
  CC=distcc CXX="distcc g++" cmake ...your parameters...
  make -j <JOBS>
  ```

  (replace `<JOBS>` with a number of parallel jobs, for example the
   number of CPUs on all hosts + number of distcc hosts)

## History

This project was started by Martin Wilz in summer 1998 and has been
developed and improved by him and some other people. In November 1999 I started
to fix some little bugs here and there and stepped into the source code of
the program deeper and deeper. Up to today I have extended, rewritten or
revised nearly every component of the program and spend much time on it.
Since summer 1999 I have taken over the project leadership and I am still
working on it.

So good luck, and feel free to keep me informed about bugs and wishes...

   Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

