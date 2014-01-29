Sceadan v 1.2.1
===============

Sceadan (pronounced "scee-den") is a Systematic Classification Engine for Advanced Data ANalysis tool.

Sceadan is originally Old English / Proto-Germanic for "to classify."

	Copyright (c) 2012-2013 The University of Texas at San Antonio

**License: GPLv2**

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**Written by:** 

Dr. Nicole Beebe (nicole.beebe@utsa.edu) and Lishu Liu, Department of Information Systems and Cyber Security, and Laurence Maddox, Department of Computer Science, University of Texas at San Antonio, One UTSA Circle, San Antonio, Texas 78209

Purpose
----

Sceadan is a C program that classifies an input file, or set of files, as a specific file type absent magic numbers (signatures, footers, etc.) and file system metadata.  The input file may be a complete file, fragment of a file, network stream, etc...any block of input data is saved into a file.  It classifies the data type based on a series of features, or characteristics, about the data using a model created from past machine learning experiments.   

Build Instructions
----
	./configure
	make
	make install

Usage
-----

	sceadan_app <target> 0


If `target`  is a directory, Sceadan will operate on each file within the directory individual, otherwise Sceadan will classify the single file referenced.

0 means classify in "container mode," which means the entire file will be used for classification.  

NOTE: In FUTURE releases, a non-zero <block_size> will be permissible, where any <block_size> in bytes can be specified.   


User Modification Instructions
----

**Specify alternate model file:**
To use a liblinear produced model file other than our default, you need to:

* Place the desired model file in the src directory
* Modify the model file specification in `sceadan_predict.c` `#define MODEL "<<YOUR FILE NAME HERE>>"`
* NOTE: Feature order of the model file MUST match the feature order used by Sceadan.  Sceadan produces a normalized, concatenated unigram-bigram frequency vector from the input file, placing bigrams in array order before unigrams.  Specifically 0x0000-0x00FF 0xFF00-0xFFFF 0x00-0xFF. 

**Change randomness threshold:** Prediction of the RANDOM DATA CLASS is based on an entropy threshold.  This version sets the threshold to entropy=0.995.  To change that threshold, modify the `#define RANDOMNESS_THRESHOLD (.995)` line in `sceadan_sceadan_predict.c`


Capability and Design
----

Default support vector machine model used:

- Uses linear kernel function 
- Uses L2 regularized L2 loss function, primal solver (Liblinear solver_type L2R_L2LOSS_SVC) (e=0.005)
- Uses C=256 for the penalty parameter for the error term
- The model was trained on 1,800 items in each of 48 file type classes
- The features used for classification in this version are the concatenated bi-gram and unigram count vectors (features 1-65536 are bigrams \x0000-\x00FF then \xFF00-\xFFFF, features 65537-65792 are unigrams\x00-\xFF).

This version was statistically trained to identify 48 of the following file/data types:
	
	/* file type description: Plain text
	   file extensions: .text, .txt */
	TEXT = 1,

	/* file type description: Delimited
	   file extensions: .csv */
	CSV = 2,

	/* file type description: Log files
	   file extensions: .log */
	LOG = 3,

	/* file type description: HTML
	   file extensions: .html */
	HTML = 4,

	/* file type description: xml
	   file extensions: .xml */
	XML = 5,

	/* file type description: JSON records
	   file extensions: .json */
	JSON = 7,

	/* file type description: JavaScript code
	   file extensions: .js */
	JS = 8,

	/* file type description: Java Source Code
	   file extensions: .java */
	JAVA = 9,

	/* file type description: css
	   file extensions: .css */
	CSS = 10,

	/* file type description: Base64 encoding
	   file extensions: .b64 */
	B64 = 11,

	/* file type description: Base85 encoding
	   file extensions: .a85 */
	A85 = 12,

	/* file type description: Hex encoding
	   file extensions: .b16 */
	B16 = 13,

	/* file type description: URL encoding
	   file extensions: .urlencoded */
	URL = 14,

	/* file type description: Rich Text File
	   file extensions: .rtf */
	RTF = 16,

	/* file type description: Thunderbird Mail Files (data and index)
	   file extensions: .msf and no extension */
	TBIRD = 17,

	/* file type description: MS Outlook PST files
	   file extensions: .pst */
	PST = 18,

	/* file type description: Portable Network Graphic
	   file extensions:  */
	PNG = 19,

	/* file type description: GIF
	   file extensions: .gif */
	GIF = 20,

	/* file type description: Bi-tonal images
	   file extensions: .tif, .tiff */
	TIF = 21,

	/* file type description: JBIG2
	   file extensions: .jb2 */
	JB2 = 22,

	/* file type description: ZLIB - DEFLATE compression
	   file extensions: .gz, .gzip, .tgz, .z, .taz */
	GZ = 23,

	/* file type description: ZLIB - DEFLATE compression
	   file extensions: .zip */
	ZIP = 24,

	/* file type description: BZ2
	   file extensions: .bz, .tbz, .bz2, bzip2, .tbz2 */
	BZ2 = 27,

	/* file type description: PDF
	   file extensions: .pdf */
	PDF = 28,

	/* file type description: MS-DOCX
	   file extensions: .docx */
	DOCX = 29,

	/* file type description: MS-XLSX
	   file extensions: .xlsx */
	XLSX = 30,

	/* file type description: MS-PPTX
	   file extensions: .pptx */
	PPTX = 31,

	/* file type description: JPG
	   file extensions: .jpg, .jpeg */
	JPG = 32,

	/* file type description: MP3
	   file extensions: .mp3 */
	MP3 = 33,

	/* file type description: AAC
	   file extensions: .m4a */
	M4A = 34,

	/* file type description: H264
	   file extensions: .mp4 */
	MP4 = 35,

	/* file type description: AVI
	   file extensions: .avi */
	AVI = 36,

	/* file type description: WMV
	   file extensions: .wmv */
	WMV = 37,

	/* file type description: FLV
	   file extensions: .flv */
	FLV = 38,

	/* file type description: Windows Audio File
	   file extensions: .wav */
	WAV = 40,

	/* file type description: Apple Quicktime Move
	   file extensions: .mov */
	MOV = 42,

	/* file type description: MS-DOC
	   file extensions: .doc */
	DOC = 43,

	/* file type description: MS-XLS
	   file extensions: .xls */
	XLS = 44,

	/* file type description: MS-PPT
	   file extensions: .ppt */
	PPT = 45,

	/* file type description: FS-FAT
	   file extensions: .fat */
	FAT = 46,

	/* file type description: FS-NTFS
	   file extensions: .ntfs */
	NTFS = 47,

	/* file type description: FS-EXT
	   file extensions: .ext3 */
	EXT3 = 48,

	/* file type description: Windows Portable Executables (PE)
	   file extensions: .exe */
	EXE = 49,

	/* file type description: Windows Dynamic Link Library files
	   file extensions: .dll */
	DLL = 50,

	/* file type description: Linux Executables
	   file extensions: .elf */
	ELF = 51,

	/* file type description: Bitmap
	   file extensions: .bmp */
	BMP = 52,

This version of Sceadan detects the following types by rule or pattern:
	
- Random 
- Constant

This version has support for enabling advanced feature extraction and usage in future versions and with use of other model files.

*NOTE:* The formulas contained in the code are not verified as correct and should be used only with extreme caution.

Such features include: 

- Bi-gram entropy
- Hamming weight
- Mean byte value
- Standard deviation of byte values
- Absolute deviation of byte values
- Skewness
- Kurtosis
- Compressed item length -- Burrows-Wheeler algorithm (bzip)
- Compressed item length -- Lempel-Ziv-Welch algorithm (LZW used by zlib and gzip)
- Average contiguity between bytes
- Max byte streak
- Low ASCII frequency
- Medium ASCII frequency
- High ASCII frequency
NOTE: Experimentation and testing showed the bi-gram/unigram byte count vectors achieved higher prediction accuracy than other combinations which included some or all of the above advanced features in a 38-class scenario.

Should advanced features be enabled, the following dependencies will be required:

- zlib v1.2.7
- libbz v1.0.2-5

Modes of operation:

- AVAILABLE: "Container mode": calculates statistics on the entire input item (file)
NOT AVAILABLE: "Block mode": calculates statistics on blocks of specified size within a container file; marches through container at specified byte-level offsets
- NOT AVAILABLE:"Feature vector extraction mode": does not predict type; simply produces vector files compatible with libsvm and liblinear format; useful for researchers who want to use Sceadan to generate vector files for research experiments 


Bugs and Platforms Tested On
----

This version was built for a Linux environment.  It has been tested on Ubuntu 11.10 and 12.04.

This version calculates bigram frequency vectors and bigram entropy without a 1-byte sliding window, however the model file uses a 1-byte sliding window.  This degrades this version's prediction accuracy, although it is still reasonable, given the bigram frequency relationship between the sliding and non-sliding window.

This version calculates hamming weight and entropy values incorrectly.  This has no impact on the default operation, since Sceadan does not use these features in this version's prediction model.


Acknowledgments
----


Foundational research, design, and project management by Nicole L. Beebe, Ph.D. at The University of Texas at San Antonio (UTSA)

Software development by Laurence Maddox and Lishu Liu, UTSA.

Experimentation by Lishu Liu, UTSA.

Special thanks to DJ Bauch, SAIC, for software development assistance.

Special thanks to Minghe Sun, Ph.D., UTSA for support vector machine assistance.

Special thanks to Matt Beebe for tool naming inspiration.

`restart.c` library from UNIX Systems Programming: Communication, Concurrency and Threads, 2nd Edition, by Kay and Steve Robbins, Prentice Hall (2003) (ISBN-10: 0130424110).  Drs. Robbins are professors at UTSA.
It is not necessary and could be removed.  Standard OPEN functions could be used in pace of `R_OPEN` functions for handling input, eliminating the need for `restart.c` and associated files.

**This research was supported in part by a research grant from the U.S. Naval Postgraduate School (Grant No. N00244-11-1-0011), and in part by funding from the UTSA Provost Summer Research Mentorship program.** 
