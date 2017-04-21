#pragma once

#include "Platform\win32.h"
#include "tools\lscrt.h"

#pragma region EBMLIDS

#define EBMLMASTER			0xA3DF451A
#define EBMLVERSIONID		0x8642
#define EBMLREADVERSION		0xF742
#define EBMLMAXIDLENGTH		0xF242
#define EBMLMAXSIZELENGTH	0xF342
#define EBMLDOCTYPE			0x8242
#define EBMLDOCTYPEVER		0x8742
#define EBMLDOCTYPEREADVER	0x8542

#define EBMLVOID			0xEC
#define EBMLCRC32			0xBF

#pragma endregion

struct EBML
{
	u32		HeaderSize; /*Entire Size in Bytes of the header Including IDs*/

	u8		Version;
	u8		ReadVersion;

	u8		MaxIDLength;
	u8		MaxSizeLength;

	char	*DocType;
	u8		DocTypeVersion;
	u8		DocTypeReadVersion;
};

#pragma region SegmentIDS

#define SEGMENTMASTER		0x67805318
#define SEEKHEAD			0x749B4D11
#define SEEK				0xBB4D
#define SEEKID				0xAB53
#define SEEKPOSITION		0xAC53

#define INFOMASTER			0x66A94915
#define	SEGMENTUID			0xA473
#define SEGMENTFILENAME		0x8473
#define PREVUID				0x23B93C
#define PREVFILENAME		0xAB833C
#define NEXTUID				0x23B93E
#define NEXTFILENAME		0xBB833E
#define SEGMENTFAMILY		0x4444

#define TIMECODESCALE		0xB1D72A
#define DURATION			0x8944
#define DATEUTC				0x6144
#define TITLE				0xA97B
#define MUXINGAPP			0x804D
#define WRITINGAPP			0x4157

#pragma endregion

struct Date
{
	u64 nanoseconds;
};

struct SegmentUID
{
	union
	{
		u64 LowBits;
		u64 HighBits;
	};
};

#pragma region ChapterIDS

#define	CHAPTERTRANSMASTER		0x2469
#define	CHAPTERTRANSEDITIONUID	0xFC69
#define	CHAPTERTRANSCODEC		0xBF69
#define	CHAPTERTRANSID			0xA569

#pragma endregion

struct ChapterTranslate
{
	u32 size;
	u32 ChapterTranslateEditionUID;
	u32 ChapterTranslateCodec;
	u32 ChapterTranslateID;
};

struct SegmentInfo
{
	u32					InfoSize;
	char				*InfoText;

	SegmentUID			SegUID;
	char				*SegFileName;
	SegmentUID			PrevSegUID;
	char				*PrevSegFileName;
	SegmentUID			NextSegUID;
	char				*NextSegFileName;
	SegmentUID			SegmentFamily;

	ChapterTranslate	*ChTranslate;

	u32					TimecodeScale;
	f32					Duration;
	Date				DateUTC;
	char				*Title;
	char				*MuxingApp;
	char				*WritingApp;
};

struct Seek
{
	u32			size;
	u32			SeekID;
	u32			SeekPosition;
};

struct SeekHead
{
	u32			size;
	Seek		*Seeks;
};

struct Segment
{
	u32			SegmentSize;
	SeekHead	SeekHead;
	SegmentInfo Info;
};

struct Matroska
{
	struct EBML		header;
	struct Segment	segment;
};


u64 GetVIntSize(u8 *Data, u32 *numBytes);
void ProcessMKVFile(FileInfo *File, Matroska *MkvFile);
void EditorLoop(MemoryArena *Memory, Matroska *MkvStruct);