This folder contains a heavily modified & streamlined version of LibXMP, 
created by Claudio Matsuoka.  https://github.com/cmatsuoka/libxmp
-----------------------------
                 __   _____                           
                |  | |__|  |______  ___ ____________  
                |  | |  || __  \  \/  //      ____  \ 
                |  |_|  || |_>  >    <|  Y Y  \ |_>  >
                |____/__||_____/__/\  \__|_|  /  ___/ 
                                    \_/     \/|_|    
                    Extended Module Player Library

                              Version 4.4


OVERVIEW

Libxmp is a library that renders module files to PCM data. It supports
over 90 mainstream and obscure module formats including Protracker (MOD),
Scream Tracker 3 (S3M), Fast Tracker II (XM), and Impulse Tracker (IT).

Many compressed module formats are supported, including popular Unix, DOS,
and Amiga file packers including gzip, bzip2, SQSH, Powerpack, etc.


LIBRARY API

See docs/libxmp.html for the library API reference. The documentation
is also available in man page and PDF formats.


SUPPORTED FORMATS

See docs/formats.txt for the list of supported formats. By default,
Protracker modules are played with a reasonable mix of Protracker 2/3
quirks (because certain Protracker 1/2 bugs are just too weird and
generally not worth emulating). Other Amiga 4 channel MODs are played with
a generic Noisetracker/Protracker-style replayer, Scream Tracker III MODs
are played with a Scream Tracker style replayer and multichannel MOD
variations are played with a Fasttracker II style replayer. Multifile
formats (Startrekker AM, Magnetic Fields, etc). must have all files in
the same directory.


SUPPORTED PACKERS

The following formats have built-in decompressors: bz2, gz, lha, oxm, xz,
Z, zip, ArcFS, arc, MMCMP, PowerPack, !Spark, SQSH, MUSE, LZX, and S404.
Other compressed formats need helpers to be installed on the system:
mo3 (unmo3) and rar (unrar).


AUTHORS AND CONTACT

See docs/CREDITS for the full list of authors. Send your comments
and bug reports to xmp-devel@lists.sourceforge.net, or directly to
cmatsuoka@gmail.com.


LICENSE

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version. This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
