#ifndef FD_H
#define FD_H
/* type for file descriptors */
typedef int fd_t;
#endif
#ifndef FILE_TYPE_H
#define FILE_TYPE_H
/* data types - NOTE extensions should not be case-sensitive */
typedef enum {
	UNCLASSIFIED = 0,
	TEXT = 1, 	/* file type description: Plain text file extensions: .text, .txt */
	CSV = 2, 	/* file type description: Delimited file extensions: .csv */
	LOG = 3, 	/* file type description: Log files file extensions: .log */
	HTML = 4, 	/* file type description: HTML file extensions: .html */
	XML = 5, 	/* file type description: xml file extensions: .xml */
	ASPX = 6, 	/* file type description: ASPX files file extensions: .aspx */
	JSON = 7, 	/* file type description: JSON records file extensions: .json */
	JS = 8, 	/* file type description: JavaScript code file extensions: .js */
	JAVA = 9, 	/* file type description: Java Source Code file extensions: .java */
	CSS = 10, 	/* file type description: css file extensions: .css */
	B64 = 11, 	/* file type description: Base64 encoding file extensions: .b64 */
	A85 = 12, 	/* file type description: Base85 encoding file extensions: .a85 */
	B16 = 13, 	/* file type description: Hex encoding file extensions: .b16 */
	URL = 14, 	/* file type description: URL encoding file extensions: .urlencoded */
	PS = 15, 	/* file type description: Postscript file extensions: .ps */
	RTF = 16, 	/* file type description: Rich Text File file extensions: .rtf */
	TBIRD = 17, 	/* file type description: Thunderbird Mail Files (data and index) file extensions: .msf and no extension */
	PST = 18, 	/* file type description: MS Outlook PST files file extensions: .pst */
	PNG = 19, 	/* file type description: Portable Network Graphic file extensions:  */
	GIF = 20, 	/* file type description: GIF file extensions: .gif */
	TIF = 21, 	/* file type description: Bi-tonal images file extensions: .tif, .tiff */
	JB2 = 22, 	/* file type description: JBIG2 file extensions: .jb2 */
	GZ = 23, 	/* file type description: ZLIB - DEFLATE compression file extensions: .gz, .gzip, .tgz, .z, .taz */
	ZIP = 24, 	/* file type description: ZLIB - DEFLATE compression file extensions: .zip */
	JAR = 25, 	/* file type description: JAVA Archive file extensions: .jar */
	RPM = 26, 	/* file type description: Resource Package Manager file extensions: .rpm */
	BZ2 = 27, 	/* file type description: BZ2 file extensions: .bz, .tbz, .bz2, bzip2, .tbz2 */
	PDF = 28, 	/* file type description: PDF file extensions: .pdf */
	DOCX = 29, 	/* file type description: MS-DOCX file extensions: .docx */
	XLSX = 30, 	/* file type description: MS-XLSX file extensions: .xlsx */
	PPTX = 31, 	/* file type description: MS-PPTX file extensions: .pptx */
	JPG = 32, 	/* file type description: JPG file extensions: .jpg, .jpeg */
	MP3 = 33, 	/* file type description: MP3 file extensions: .mp3 */
	M4A = 34, 	/* file type description: AAC file extensions: .m4a */
	MP4 = 35, 	/* file type description: H264 file extensions: .mp4 */
	AVI = 36, 	/* file type description: AVI file extensions: .avi */
	WMV = 37, 	/* file type description: WMV file extensions: .wmv */
	FLV = 38, 	/* file type description: FLV file extensions: .flv */
	SWF = 39, 	/* file type description: Shock Wave Flash file extensions: .swf */
	WAV = 40, 	/* file type description: Windows Audio File file extensions: .wav */
	WMA = 41, 	/* file type description: Windows Media File file extensions: .wma */
	MOV = 42, 	/* file type description: Apple Quicktime Move file extensions: .mov */
	DOC = 43, 	/* file type description: MS-DOC file extensions: .doc */
	XLS = 44, 	/* file type description: MS-XLS file extensions: .xls */
	PPT = 45, 	/* file type description: MS-PPT file extensions: .ppt */
	FAT = 46, 	/* file type description: FS-FAT file extensions: .fat */
	NTFS = 47, 	/* file type description: FS-NTFS file extensions: .ntfs */
	EXT3 = 48, 	/* file type description: FS-EXT file extensions: .ext3 */
	EXE = 49, 	/* file type description: Windows Portable Executables (PE) file extensions: .exe */
	DLL = 50, 	/* file type description: Windows Dynamic Link Library files file extensions: .dll */
	ELF = 51, 	/* file type description: Linux Executables file extensions: .elf */
	BMP = 52, 	/* file type description: Bitmap file extensions: .bmp */
	AES = 53, 	/* file type description: ENCRYPTED file extensions: N/A (filename: AES256*) */
	RAND = 54, 	/* file type description: RANDOM file extensions: N/A (filename: Random*) */
	PPS = 55,
	UCV_CONST = 100,
	BCV_CONST = 101,
	TCV_CONST = 102
} file_type_e;

#endif
