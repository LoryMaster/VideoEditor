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
	s64 nanoseconds;
};

struct SegmentUID
{
	u64 LowBits;
	u64 HighBits;
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

enum TrackType
{
	TRACK_VIDEO		= 1,
	TRACK_AUDIO		= 2,
	TRACK_COMPLEX	= 3,
	TRACK_LOGO		= 0x10,
	TRACK_SUBTITLE	= 0x11,
	TRACK_BUTTONS	= 0x12,
	TRACK_CONTROL	= 0x20
};

#pragma region TrackTranslateIDs
#define TRACKTRANSLATEMASTER		0x2466
#define	TRACKTRANSLATEEDITIONUID	0xFC66
#define TRACKTRANSLATECODEC			0xBF66
#define	TRACKTRANSLATETRACKID		0xA566
#pragma endregion

struct TrackTranslate
{
	u32			size;
	u64			TrackTranslateEditionUID;
	u8			TrackTranslateCodec;
	u64			TrackTranslateTrackID;
};

#pragma region MasteringMetadataIDs
#define MASTMETADATAMASTER					0xD055
#define MASTMETADATAPRIMARYRCHROMATICITYX	0xD155
#define MASTMETADATAPRIMARYRCHROMATICITYY	0xD255
#define MASTMETADATAPRIMARYGCHROMATICITYX	0xD355
#define MASTMETADATAPRIMARYGCHROMATICITYY	0xD455
#define MASTMETADATAPRIMARYBCHROMATICITYX	0xD555
#define MASTMETADATAPRIMARYBCHROMATICITYY	0xD655
#define MASTMETADATAWHITEPOINTCHROMATICITYX	0xD755
#define MASTMETADATAWHITEPOINTCHROMATICITYY	0xD855
#define MASTMETADATALUMINANCEMAX			0xD955
#define MASTMETADATALUMINANCEMIN			0xDA55
#pragma endregion

struct MasteringMetadata
{
	u32			size;

	f32			PrimaryRChromaticityX;
	f32			PrimaryRChromaticityY;
	f32			PrimaryGChromaticityX;
	f32			PrimaryGChromaticityY;
	f32			PrimaryBChromaticityX;
	f32			PrimaryBChromaticityY;
	f32			WhitePointChromaticityX;
	f32			WhitePointChromaticityY;
	f32			LuminanceMax;
	f32			LuminanceMin;
};

#pragma region ColourIDs
#define COLOURMASTER				0xB055
#define COLOURMATRIXCOEFFICIENTS	0xB155
#define COLOURBITSPERCHANNEL		0xB255
#define COLOURCHROMASUBSAMPLINGHORZ 0xB355
#define COLOURCHROMASUBSAMPLINGVERT 0xB455
#define COLOURCBSUBSAMPLINGHORZ		0xB555
#define COLOURCBSUBSAMPLINGVERT		0xB655
#define COLOURCHROMASITINGHORZ		0xB755
#define COLOURCHROMASITINGVERT		0xB855
#define COLOURRANGE					0xB955
#define COLOURTRANSFERCHARACT		0xBA55
#define COLOURPRIMARIES				0xBB55
#define COLOURMAXCLL				0xBC55
#define COLOURMAXFALL				0xBD55
#pragma endregion

struct Colour
{
	u32					size;

	u8					MatrixCoefficients;
	u32					BitsPerChannel;
	u32					ChromaSubsamplingHorz;
	u32					ChromaSubsamplingVert;
	u32					CbSubsamplingHorz;
	u32					CbSubsamplingVert;
	u8					ChromaSitingHorz;
	u8					ChromaSitingVert;
	u8					Range;
	u8					TransferCharacteristics;
	u8					Primaries;
	u32					MaxCLL;
	u32					MaxFALL;
	MasteringMetadata	Metadata;
};

#pragma region VideoIDs
#define	VIDEOMASTER				0xE0
#define VIDEOFLAGINTERLACED		0x9A
#define VIDEOFIELDORDER			0x9D
#define VIDEOSTEREOMODE			0xB853
#define VIDEOALPHAMODE			0xC053
#define VIDEOPIXELWIDTH			0xB0
#define VIDEOPIXELHEIGHT		0xBA
#define VIDEOPIXELCROPBOTTOM	0xAA54
#define VIDEOPIXELCROPTOP		0xBB54
#define VIDEOPIXELCROPLEFT		0xCC54
#define VIDEOPIXELCROPRIGHT		0xDD54
#define VIDEODISPLAYWIDTH		0xB054
#define VIDEODISPLAYHEIGHT		0xBA54
#define VIDEODISPLAYUNIT		0xB254
#define VIDEOASPECTRATIOTYPE	0xB354
#define VIDEOCOLOURSPACE		0x24B52E
	#define VIDEOCOLOURSPACE16	0xB52E
	#define VIDEOCOLOURSPACE24	0x24
#pragma endregion

struct Video
{
	u32			size;

	u8			FlagInterlaced;
	u8			FieldOrder;
	u8			StereoMode;
	u8			AlphaMode;
	u32			PixelWidth;
	u32			PixelHeight;
	u32			PixelCropBottom;
	u32			PixelCropTop;
	u32			PixelCropLeft;
	u32			PixelCropRight;
	u32			DisplayWidth;
	u32			DisplayHeight;
	u8			DisplayUnit;
	u8			AspectRatioType;
	u32			ColourSpace;
	Colour		ColourInfo;

};

#pragma region AudioIDs

#define AUDIOMASTER				0xE1
#define AUDIOSAMPLINGFREQ		0xB5
#define AUDIOOUTSAMPLINGFREQ	0xB578
#define AUDIOCHANNELS			0x9F
#define AUDIOBITDEPTH			0x6462

#pragma endregion

struct Audio
{
	u32			size;

	f32			SamplingFrequency;
	f32			OutputSamplingFrequency;
	u32			Channels;
	u32			BitDepth;
};

#pragma region TrackOperationIDs
#define TRACKOPMASTER				0xE2
#define TRACKCOMBINEDPLANESMASTER	0xE3
#define TRACKPLANEMASTER			0xE4
#define TRACKPLANEUID				0xE5
#define TRACKPLANETYPE				0xE6
#define TRACKJOINBLOCKSMASTER		0xE9
#define TRACKJOINUID				0xED
#pragma endregion

struct TrackPlane
{
	u32			size;

	u64			TrackPlaneUID;
	u8			TrackPlaneType;
};

struct TrackCombinePlanes
{
	u32			size;

	TrackPlane	Plane;
};

struct TrackJoinBlocks
{
	u32			size;
	u64			TrackJoinUID;
};

struct TrackOperation
{
	u32			size;

	TrackCombinePlanes	PlaneTracks;
	TrackJoinBlocks		JoinBlocks;
};

enum EncodingField
{
	FIELD_FRAMECONTENT		= 0x01,
	FIELD_TRACKPRIVATEDATA	= 0x02,
	FIELD_NEXTENCODING		= 0x04
};

#pragma region ContentEncodingIDs
#define CONTENTENCODINGSMASTER			0x806D
#define CONTENTENCODINGMASTER			0x4062
#define CONTENTENCODINGORDER			0x3150
#define CONTENTENCODINGSCOPE			0x3250
#define CONTENTENCODINGTYPE				0x3350
#define CONTENTENCODINGCOMPMASTER		0x3450
#define CONTENTENCODINGCOMPALGO			0x5442
#define CONTENTENCODINGCOMSETTING		0x5542
#define CONTENTENCODINGENCRYPTIONMASTER	0x3550
#define CONTENTENCODINGENCALGO			0xE147
#define CONTENTENCODINGENCKEYID			0xE247
#define CONTENTENCODINGSIGNATURE		0xE347
#define CONTENTENCODINGSIGKEYID			0xE447
#define CONTENTENCODINGSIGALGO			0xE547
#define CONTENTENCODINGSIGHASHALGO		0xE647
#pragma endregion

struct ContentCompression
{
	u32			size;

	u8			ContentCompAlgo;
	void		*ContentCompSettings;
};

struct ContentEncryption
{
	u32			size;
	u8			ContentEncAlgo;
	void		*ContentEncKeyID;
	void		*ContentSignature;
	void		*ContentSigKeyID;
	u8			ContentSigAlgo;
	u8			ContentSigHashAlgo;
};

struct Encoding
{
	u32					size;

	u64					ContentEncodingOrder;
	u8					ContentEncodingScope;
	u8					ContentEncodingType;
	ContentCompression	Compression;
	ContentEncryption	Encryption;
};

struct ContentEncodings
{
	u32			size;
	Encoding	Encoding;
};

#pragma region TrackIDS
#define TRACKMASTER						0x6BAE5416
#define TRACKENTRYMASTER				0xAE
#define TRACKNUMBER						0xD7
#define TRACKUID						0xC573
#define TRACKTYPE						0x83
#define TRACKFLAGENABLED				0xB9
#define TRACKFLAGDEFAULT				0x88
#define TRACKFLAGFORCED					0xAA55
#define TRACKFLAGLACING					0x9C
#define TRACKMINCACHE					0xE76D
#define TRACKMAXCACHE					0xF86D
#define TRACKDEFDURATION				0x83E323
	#define TRACKDEFDURATION16			0xE323
	#define	TRACKDEFDURATION24			0x83

#define TRACKDEFDECODEDFIELDDUR			0x7A4E23
	#define TRACKDEFDECODEDFIELDDUR16	0x4E23
	#define TRACKDEFDECODEDFIELDDUR24	0x7A

#define TRACKMAXBLOCKADDITIONID			0xEE55
#define TRACKNAME						0x6E53
#define TRACKLANGUAGE					0x9CB522
	#define TRACKLANGUAGE16				0xB522
	#define TRACKLANGUAGE24				0x9C

#define TRACKCODECID					0x86
#define TRACKCODECPRIVATE				0xA263
#define TRACKCODECNAME					0x888625
	#define TRACKCODECNAME16			0x8625
	#define TRACKCODECNAME24			0x88

#define TRACKATTACHMENTLINK				0x4674
#define TRACKCODECDECODEALL				0xAA
#define TRACKOVERLAY					0xAB6F
#define TRACKCODECDELAY					0xAA56
#define TRACKSEEKPREROLL				0xBB56
#pragma endregion

struct TrackEntry
{
	u32					size;

	u32					TrackNumber;
	u64					TrackUID;
	TrackType			Type;
	s8					FlagEnabled;
	s8					FlagDefault;
	s8					FlagForced;
	s8					FlagLacing;
	u32					MinCache;
	u32					MaxCache;
	u64					DefaultDuration;
	u64					DefaultDecodingFieldDuration;
	u64					MaxBlockAdditionID;
	char				*Name;
	char				*Language;
	char				*CodecID;
	void				*CodecPrivate;
	u32					CodecPrivateSize;
	char				*CodecName;
	u64					AttachmentLink;
	u8					CodecDecodeAll;
	u32					TrackOverlay;
	u64					CodecDelay;
	u64					SeekPreRoll;
	TrackTranslate		TrackTranslate;
	Video				Video;
	Audio				Audio;
	TrackOperation		Operations;
	ContentEncodings	Encodings;
};

struct Track
{
	u32			size;
	TrackEntry	*Entries;
};

#pragma region ChapterTrack
#define CHAPTRACKMASTER	0x8F
#define CHAPTRACKNUMBER	0x89
#pragma endregion

struct ChapterTrack
{
	u32		size;
	u32		ChapterTrackNumber[8];
};

#pragma region ChapterDisplay
#define CHAPDISPLAYMASTER	0x80
#define CHAPDISPLAYSTRING	0x85
#define CHAPDISPLAYLANGUAGE	0x7C43
#define CHAPDISPLAYCOUNTRY	0x7E43
#pragma endregion

struct ChapterDisplay
{
	u32			size;

	char		*ChapString;
	char		*ChapLanguage;
	char		*ChapCountry;
};

#pragma region ChapterProcess
#define CHAPPROCMASTER			0x4469
#define CHAPPROCCODECID			0x5569
#define CHAPPROCPRIVATE			0x0D45
#define CHAPPROCCOMMANDMASTER	0x1169
#define CHAPPROCTIME			0x2269
#define CHAPPROCDATA			0x3369
#pragma endregion

struct ProcessCommand
{
	u32		size;
	u64		ProcessTime;
	void	*ProcessData;
	u32		ProcessDataSize;
};

struct ChapterProcess
{
	u32				size;
	u32				ProcessCodecID;
	void			*ProcessPrivate;
	u32				ProcessPrivateSize;
	ProcessCommand	Commands;
};

#pragma region ChapterAtom
#define ATOMMASTER			0xB6
#define ATOMUID				0xC473
#define ATOMSTRINGUID		0x5456
#define ATOMTIMESTART		0x91
#define ATOMTIMEEND			0x92
#define ATOMFLAGHIDDEN		0x98
#define ATOMFLAGENABLED		0x9845
#define ATOMSEGMENTUID		0x676E
#define ATOMSEGEDITIONUID	0xBC6E
#define ATOMPHYSICALEQUIV	0xC363
#pragma endregion

struct ChapterAtom
{
	u32				size;
	u64				ChapterUID;
	char			*ChapterStringUID;
	u64				ChapterTimeStart;
	u64				ChapterTimeEnd;
	s8				FlagHidden;
	s8				FlagEnabled;
	void			*ChapterSegmentUID;
	u32				ChapterSegmentUIDSize;
	u64				ChapterSegmentEditionUID;
	u32				ChapterPhysicalEquiv;
	ChapterTrack	TrackList;
	ChapterDisplay	Display;
	ChapterProcess	Proc;
};

#pragma region Chapters&EditionIDS
#define CHAPTERSMASTER		0x70A74310
#define	EDITIONMASTER		0xB945
#define	EDITIONUID			0xBC45
#define	EDITIONFLAGHIDDEN	0xBD45
#define	EDITIONFLAGDEFAULT	0xDB45
#define	EDITIONFLAGORDERED	0xDD45
#pragma endregion

struct EditionEntry
{
	u32			size;
	u64			EditionUID;
	s8			FlagHidden;
	s8			FlagDefault;
	s8			FlagOrdered;
	ChapterAtom Atoms[4];
	u32			numAtoms;
};

struct Chapters
{
	u32				size;
	EditionEntry	Entries[4];
	u32				numEntries;
};

struct Segment
{
	u32			SegmentSize;

	SeekHead	SeekHead;
	SegmentInfo Info;
	Track		Tracks;
	Chapters	Chapters;
};

struct Matroska
{
	struct EBML		header;
	struct Segment	segment;
};


u64 GetVIntSize(u8 *Data, u32 *numBytes);
void ProcessMKVFile(FileInfo *File, Matroska *MkvFile);
void EditorLoop(MemoryArena *Memory, Matroska *MkvStruct);