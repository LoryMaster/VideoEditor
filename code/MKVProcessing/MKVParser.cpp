#include "MKVProcessing\MKVParser.h"

u64 GetVIntSize(u8 *Data, u32 *numBytes)
{
	u64 Result = 0;
	u32 DataInLittleEndian = *(u32 *)Data;
	DataInLittleEndian = ByteSwap32(DataInLittleEndian); 
	u32 leadingZeroes = LeadingZeros(DataInLittleEndian);
	u32 howManyBytes = 1 + leadingZeroes;
	*numBytes = howManyBytes;

	u8  data8 = 0;
	u16	data16 = 0;
	u32	data32 = 0;
	u64	data64 = 0;

	switch (howManyBytes)
	{
		case 1: data8  = (*(Data) & ~(1u << (7 - leadingZeroes)));  Result = data8; break;
		case 2: data16 = *((u16 *)Data); data16 = ByteSwap16(data16); data16 &= ~(1u << (15 - leadingZeroes)); Result = data16; break;
		case 4: data32 = *((u32 *)Data); data32 = ByteSwap32(data32); data32 &= ~(1u << (31 - leadingZeroes)); Result = data32; break;
		case 8: data64 = *((u64 *)Data); data64 = ByteSwap64(data64); data64 &= ~(1ull << (63 - leadingZeroes)); Result = data64; break;
		default: OutputDebugStringA("Error when calculating VIntSizeBSR. Bytes in size not supported.\n"); break;
	}
	return Result;
}

void getData(void **Data, void *dest)
{
	u32 numBytes = 0;
	u8 *destByte = 0;

	u32 vintSize = (u32)GetVIntSize(*(u8 **)Data, &numBytes);
	*(u8 **)Data += numBytes;

	u64 temp = 0;

	switch (vintSize)
	{
		case 1: 
			ls_memcpy(*(void **)Data, dest, vintSize);
			break;
		case 2:
			ls_memcpy(*(void **)Data, dest, vintSize);
			*(u16 *)dest = ByteSwap16(*(u16 *)dest);
			break;
		case 3:
			ls_memcpy(*(void **)Data, dest, vintSize);
			*(u32 *)dest = ByteSwap32(*(u32 *)dest);
			destByte = (u8 *)dest;
			destByte[0] = destByte[1]; destByte[1] = destByte[2]; destByte[2] = destByte[3]; destByte[3] = 0;
			break;
		case 4:
			ls_memcpy(*(void **)Data, dest, vintSize);
			*(u32 *)dest = ByteSwap32(*(u32 *)dest);
			break;
		case 8:
			ls_memcpy(*(void **)Data, dest, vintSize);
			*(u64 *)dest = ByteSwap64(*(u64 *)dest);
			break;
		case 16:
			ls_memcpy(*(void **)Data, dest, vintSize);
			temp = ByteSwap64(*((u64 *)dest + 1));
			*((u64 *)dest + 1) = ByteSwap64(*(u64 *)dest);
			*(u64 *)dest = temp;
			break;
		default:
			OutputDebugStringA("Not handled size when getting vInt.\n");
			break;
	}

	*(u8 **)Data += vintSize;
	return;
}

void getBinaryData(void **Data, void **dest, u32 *destSize)
{
	HANDLE HeapHandle = GetProcessHeap();

	u32 numBytes = 0;
	u32 vintSize = (u32)GetVIntSize(*(u8 **)Data, &numBytes);
	*destSize = vintSize;
	*(u8 **)Data += numBytes;

	*dest = HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, vintSize);
	ls_memcpy(*(void **)Data, *dest, vintSize);

	*(u8 **)Data += vintSize;
	return;
}

void getString(void **Data, char **dest)
{
	u32 numBytes = 0;
	u32 stringSize = (u32)GetVIntSize(*(u8 **)Data, &numBytes) + 1; /*+1 Because of null-termination*/
	*(u8 **)Data += numBytes;

	*dest = (char *)ls_heapAlloc(stringSize*(sizeof(char)));
	ls_strncpy(*Data, *(void **)dest, stringSize);
	*(u8 **)Data += (stringSize - 1);
}

void ParseSegmentInfo(void **Data, Segment *segment)
{
	switch (**(u16 **)Data)
	{
		case SEGMENTUID:			 *(u8 **)Data += 2; getData(Data, (void *)&segment->Info.SegUID); break;
		case SEGMENTFILENAME:		 *(u8 **)Data += 2; getString(Data, &segment->Info.SegFileName); break;
		case 0xB93C:				 if (*((*(u8 **)Data) + 2) == 0x23) { *(u8 **)Data += 3; getData(Data, (void *)&segment->Info.PrevSegUID); } break;
		case 0x833C:				 if (*((*(u8 **)Data) + 2) == 0xAB) { *(u8 **)Data += 3; getString(Data, &segment->Info.PrevSegFileName); } break;
		case 0xB93E:				 if (*((*(u8 **)Data) + 2) == 0x23) { *(u8 **)Data += 3; getData(Data, (void *)&segment->Info.NextSegUID); } break;
		case 0x833E:				 if (*((*(u8 **)Data) + 2) == 0xBB) { *(u8 **)Data += 3; getString(Data, &segment->Info.NextSegFileName); } break;
		case SEGMENTFAMILY:			 *(u8 **)Data += 2; getData(Data, (void *)&segment->Info.SegmentFamily); break;
		case CHAPTERTRANSMASTER:	 *(u8 **)Data += 2; getData(Data, (void *)&segment->Info.ChTranslate->size); break;
		case CHAPTERTRANSEDITIONUID: *(u8 **)Data += 2; getData(Data, (void *)&segment->Info.ChTranslate->ChapterTranslateEditionUID); break;
		case CHAPTERTRANSCODEC:		 *(u8 **)Data += 2; getData(Data, (void *)&segment->Info.ChTranslate->ChapterTranslateCodec); break;
		case CHAPTERTRANSID:		 *(u8 **)Data += 2; getData(Data, (void *)&segment->Info.ChTranslate->ChapterTranslateID); break;
		case 0xD72A:				 if (*((*(u8 **)Data) + 2) == 0xB1) { *(u8 **)Data += 3; getData(Data, (void *)&segment->Info.TimecodeScale); } break;
		case DURATION:				 *(u8 **)Data += 2; getData(Data, (void *)&segment->Info.Duration); break;
		case DATEUTC:				 *(u8 **)Data += 2; getData(Data, (void *)&segment->Info.DateUTC); break;
		case TITLE:					 *(u8 **)Data += 2; getString(Data, &segment->Info.Title); break;
		case MUXINGAPP:				 *(u8 **)Data += 2; getString(Data, &segment->Info.MuxingApp); break;
		case WRITINGAPP:			 *(u8 **)Data += 2; getString(Data, &segment->Info.WritingApp); break;
		default:
			break;
	}
}

void ParseSegmentTracks(void **Data, TrackEntry *Entry)
{
	u32 numBytes = 0;

	switch (**(u8 **)Data)
	{
	case TRACKNUMBER:				(*(u8 **)Data)++; getData(Data, (void *)&Entry->TrackNumber); return;
	case TRACKTYPE:					(*(u8 **)Data)++; getData(Data, (void *)&Entry->Type); return;
	case TRACKFLAGENABLED:			(*(u8 **)Data)++; getData(Data, (void *)&Entry->FlagEnabled); return;
	case TRACKFLAGDEFAULT:			(*(u8 **)Data)++; getData(Data, (void *)&Entry->FlagDefault); return;
	case TRACKFLAGLACING:			(*(u8 **)Data)++; getData(Data, (void *)&Entry->FlagLacing); return;
	case TRACKCODECID:				(*(u8 **)Data)++; getString(Data, &Entry->CodecID); return;
	case TRACKCODECDECODEALL:		(*(u8 **)Data)++; getData(Data, (void *)&Entry->Type); return;
	case VIDEOMASTER:				(*(u8 **)Data)++; Entry->Video.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case VIDEOFLAGINTERLACED:		(*(u8 **)Data)++; getData(Data, (void *)&Entry->Video.FlagInterlaced); return;
	case VIDEOFIELDORDER:			(*(u8 **)Data)++; getData(Data, (void *)&Entry->Video.FieldOrder); return;
	case VIDEOPIXELWIDTH:			(*(u8 **)Data)++; getData(Data, (void *)&Entry->Video.PixelWidth); return;
	case VIDEOPIXELHEIGHT:			(*(u8 **)Data)++; getData(Data, (void *)&Entry->Video.PixelHeight); return;
	case AUDIOMASTER:				(*(u8 **)Data)++; Entry->Audio.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case AUDIOSAMPLINGFREQ:			(*(u8 **)Data)++; getData(Data, (void *)&Entry->Audio.SamplingFrequency); return;
	case AUDIOCHANNELS:				(*(u8 **)Data)++; getData(Data, (void *)&Entry->Audio.Channels); return;
	case TRACKOPMASTER:				(*(u8 **)Data)++; Entry->Operations.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case TRACKCOMBINEDPLANESMASTER: (*(u8 **)Data)++; Entry->Operations.PlaneTracks.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case TRACKPLANEMASTER:			(*(u8 **)Data)++; Entry->Operations.PlaneTracks.Plane.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case TRACKPLANEUID:				(*(u8 **)Data)++; getData(Data, (void *)&Entry->Operations.PlaneTracks.Plane.TrackPlaneUID); return;
	case TRACKPLANETYPE:			(*(u8 **)Data)++; getData(Data, (void *)&Entry->Operations.PlaneTracks.Plane.TrackPlaneType); return;
	case TRACKJOINBLOCKSMASTER:		(*(u8 **)Data)++; Entry->Operations.JoinBlocks.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case TRACKJOINUID:				(*(u8 **)Data)++; getData(Data, (void *)&Entry->Operations.JoinBlocks.TrackJoinUID); return;
	}

	switch (**(u16 **)Data)
	{
	case TRACKUID:								*(u8 **)Data += 2; getData(Data, (void *)&Entry->TrackUID); return;
	case TRACKFLAGFORCED:						*(u8 **)Data += 2; getData(Data, (void *)&Entry->FlagForced); return;
	case TRACKMINCACHE:							*(u8 **)Data += 2; getData(Data, (void *)&Entry->MinCache); return;
	case TRACKMAXCACHE:							*(u8 **)Data += 2; getData(Data, (void *)&Entry->MaxCache); return;
	case TRACKDEFDURATION16:					if (*(*(u8 **)Data + 2) == TRACKDEFDURATION24) { *(u8 **)Data += 3; getData(Data, (void *)&Entry->DefaultDuration); } return;
	case TRACKDEFDECODEDFIELDDUR16:				if (*(*(u8 **)Data + 2) == TRACKDEFDECODEDFIELDDUR24) { *(u8 **)Data += 3; getData(Data, (void *)&Entry->DefaultDecodingFieldDuration); } return;
	case TRACKMAXBLOCKADDITIONID:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->MaxBlockAdditionID); return;
	case TRACKNAME:								*(u8 **)Data += 2; getString(Data, &Entry->Name); return;
	case TRACKLANGUAGE16:						if (*(*(u8 **)Data + 2) == TRACKLANGUAGE24) { *(u8 **)Data += 3; getString(Data, &Entry->Language); } return;
	case TRACKCODECPRIVATE:						*(u8 **)Data += 2; getBinaryData(Data, (void **)&Entry->CodecPrivate, &Entry->CodecPrivateSize); return;
	case TRACKCODECNAME16:						if (*(*(u8 **)Data + 2) == TRACKCODECNAME24) { *(u8 **)Data += 3; getString(Data, &Entry->CodecName); }return;
	case TRACKATTACHMENTLINK:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->AttachmentLink); return;
	case TRACKOVERLAY:							*(u8 **)Data += 2; getData(Data, (void *)&Entry->TrackOverlay); return;
	case TRACKCODECDELAY:						*(u8 **)Data += 2; getData(Data, (void *)&Entry->CodecDelay); return;
	case TRACKSEEKPREROLL:						*(u8 **)Data += 2; getData(Data, (void *)&Entry->SeekPreRoll); return;
	case TRACKTRANSLATEMASTER:					*(u8 **)Data += 2; Entry->TrackTranslate.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case TRACKTRANSLATEEDITIONUID:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->TrackTranslate.TrackTranslateEditionUID); return;
	case TRACKTRANSLATECODEC:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->TrackTranslate.TrackTranslateCodec); return;
	case TRACKTRANSLATETRACKID:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->TrackTranslate.TrackTranslateTrackID); return;
	case VIDEOSTEREOMODE:						*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.StereoMode); return;
	case VIDEOALPHAMODE:						*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.AlphaMode); return;
	case VIDEOPIXELCROPBOTTOM:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.PixelCropBottom); return;
	case VIDEOPIXELCROPTOP:						*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.PixelCropTop); return;
	case VIDEOPIXELCROPLEFT:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.PixelCropLeft); return;
	case VIDEOPIXELCROPRIGHT:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.PixelCropRight); return;
	case VIDEODISPLAYWIDTH:						*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.DisplayWidth); return;
	case VIDEODISPLAYHEIGHT:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.DisplayHeight); return;
	case VIDEODISPLAYUNIT:						*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.DisplayUnit); return;
	case VIDEOASPECTRATIOTYPE:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.AspectRatioType); return;
	case VIDEOCOLOURSPACE16:					if (*(*(u8 **)Data + 2) == VIDEOCOLOURSPACE24) { *(u8 **)Data += 3; getData(Data, (void *)&Entry->Video.ColourSpace); } return;
	case COLOURMASTER:							*(u8 **)Data += 2; Entry->Video.ColourInfo.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case COLOURMATRIXCOEFFICIENTS:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.MatrixCoefficients); return;
	case COLOURBITSPERCHANNEL:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.BitsPerChannel); return;
	case COLOURCHROMASUBSAMPLINGHORZ:			*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.ChromaSubsamplingHorz); return;
	case COLOURCHROMASUBSAMPLINGVERT:			*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.ChromaSubsamplingVert); return;
	case COLOURCBSUBSAMPLINGHORZ:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.CbSubsamplingHorz); return;
	case COLOURCBSUBSAMPLINGVERT:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.CbSubsamplingVert); return;
	case COLOURCHROMASITINGHORZ:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.ChromaSitingHorz); return;
	case COLOURCHROMASITINGVERT:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.ChromaSitingVert); return;
	case COLOURRANGE:							*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Range); return;
	case COLOURTRANSFERCHARACT:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.TransferCharacteristics); return;
	case COLOURPRIMARIES:						*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Primaries); return;
	case COLOURMAXCLL:							*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.MaxCLL); return;
	case COLOURMAXFALL:							*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.MaxFALL); return;
	case MASTMETADATAMASTER:					*(u8 **)Data += 2; Entry->Video.ColourInfo.Metadata.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case MASTMETADATAPRIMARYRCHROMATICITYX:		*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.PrimaryRChromaticityX); return;
	case MASTMETADATAPRIMARYRCHROMATICITYY:		*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.PrimaryRChromaticityY); return;
	case MASTMETADATAPRIMARYGCHROMATICITYX:		*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.PrimaryGChromaticityX); return;
	case MASTMETADATAPRIMARYGCHROMATICITYY:		*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.PrimaryGChromaticityY); return;
	case MASTMETADATAPRIMARYBCHROMATICITYX:		*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.PrimaryBChromaticityX); return;
	case MASTMETADATAPRIMARYBCHROMATICITYY:		*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.PrimaryBChromaticityY); return;
	case MASTMETADATAWHITEPOINTCHROMATICITYX:	*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.WhitePointChromaticityX); return;
	case MASTMETADATAWHITEPOINTCHROMATICITYY:	*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.WhitePointChromaticityY); return;
	case MASTMETADATALUMINANCEMAX:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.LuminanceMax); return;
	case MASTMETADATALUMINANCEMIN:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Video.ColourInfo.Metadata.LuminanceMin); return;
	case AUDIOOUTSAMPLINGFREQ:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Audio.OutputSamplingFrequency); return;
	case AUDIOBITDEPTH:							*(u8 **)Data += 2; getData(Data, (void *)&Entry->Audio.BitDepth); return;
	case CONTENTENCODINGSMASTER:				*(u8 **)Data += 2; Entry->Encodings.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case CONTENTENCODINGMASTER:					*(u8 **)Data += 2; Entry->Encodings.Encoding.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case CONTENTENCODINGORDER:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.ContentEncodingOrder); return;
	case CONTENTENCODINGSCOPE:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.ContentEncodingScope); return;
	case CONTENTENCODINGTYPE:					*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.ContentEncodingType); return;
	case CONTENTENCODINGCOMPMASTER:				*(u8 **)Data += 2; Entry->Encodings.Encoding.Compression.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case CONTENTENCODINGCOMPALGO:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.Compression.ContentCompAlgo); return;
	case CONTENTENCODINGCOMSETTING:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.Compression.ContentCompSettings); return;
	case CONTENTENCODINGENCRYPTIONMASTER:		*(u8 **)Data += 2; Entry->Encodings.Encoding.Encryption.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case CONTENTENCODINGENCALGO:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.Encryption.ContentEncAlgo); return;
	case CONTENTENCODINGENCKEYID:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.Encryption.ContentEncKeyID); return;
	case CONTENTENCODINGSIGNATURE:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.Encryption.ContentSignature); return;
	case CONTENTENCODINGSIGKEYID:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.Encryption.ContentSigKeyID); return;
	case CONTENTENCODINGSIGALGO:				*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.Encryption.ContentSigAlgo); return;
	case CONTENTENCODINGSIGHASHALGO:			*(u8 **)Data += 2; getData(Data, (void *)&Entry->Encodings.Encoding.Encryption.ContentSigHashAlgo); return;
	}

	OutputDebugStringA("Not handled 8-bit or 16-bit case in ParseSegmentTracks.\n");
	return;
}

void ParseSegmentChapters(void **Data, EditionEntry *Entry)
{
	u32 numBytes = 0;

	ChapterAtom *Atom = &Entry->Atoms[Entry->numAtoms - 1];

	switch (**(u8 **)Data)
	{
	case ATOMMASTER:		
		(*(u8 **)Data)++; Entry->numAtoms++; 
		Entry->Atoms[Entry->numAtoms - 1].size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); 
		(*(u8 **)Data) += numBytes; Entry->Atoms[Entry->numAtoms - 1].FlagEnabled = -1;	Entry->Atoms[Entry->numAtoms - 1].FlagHidden = -1; 
		return;
	case ATOMTIMESTART:		(*(u8 **)Data)++; getData(Data, (void *)&Atom->ChapterTimeStart); return;
	case ATOMTIMEEND:		(*(u8 **)Data)++; getData(Data, (void *)&Atom->ChapterTimeEnd); return;
	case ATOMFLAGHIDDEN:	(*(u8 **)Data)++; getData(Data, (void *)&Atom->FlagHidden); return;
	case CHAPTRACKMASTER:	(*(u8 **)Data)++; Atom->TrackList.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case CHAPTRACKNUMBER:	(*(u8 **)Data)++; getData(Data, (void *)&Atom->TrackList.ChapterTrackNumber); return;
	case CHAPDISPLAYMASTER: (*(u8 **)Data)++; Atom->Display.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case CHAPDISPLAYSTRING: (*(u8 **)Data)++; getString(Data, &Atom->Display.ChapString); return;
	}

	switch (**(u16 **)Data)
	{
	case EDITIONUID: *(u8 **)Data += 2; getData(Data, (void *)&Entry->EditionUID); return;
	case EDITIONFLAGHIDDEN: *(u8 **)Data += 2; getData(Data, (void *)&Entry->FlagHidden); return;
	case EDITIONFLAGDEFAULT: *(u8 **)Data += 2; getData(Data, (void *)&Entry->FlagDefault); return;
	case EDITIONFLAGORDERED: *(u8 **)Data += 2; getData(Data, (void *)&Entry->FlagOrdered); return;
	case ATOMUID: *(u8 **)Data += 2; getData(Data, (void *)&Atom->ChapterUID); return;
	case ATOMSTRINGUID: *(u8 **)Data += 2; getString(Data, &Atom->ChapterStringUID); return;
	case ATOMFLAGENABLED: *(u8 **)Data += 2; getData(Data, (void *)&Atom->FlagEnabled); return;
	case ATOMSEGMENTUID:*(u8 **)Data += 2; getBinaryData(Data, (void **)&Atom->ChapterSegmentUID, &Atom->ChapterSegmentUIDSize); return;
	case ATOMSEGEDITIONUID: *(u8 **)Data += 2; getData(Data, (void *)&Atom->ChapterSegmentEditionUID); return;
	case ATOMPHYSICALEQUIV: *(u8 **)Data += 2; getData(Data, (void *)&Atom->ChapterPhysicalEquiv); return;
	case CHAPDISPLAYLANGUAGE:*(u8 **)Data += 2; getString(Data, &Atom->Display.ChapLanguage); return;
	case CHAPDISPLAYCOUNTRY: *(u8 **)Data += 2; getString(Data, &Atom->Display.ChapCountry); return;
	case CHAPPROCMASTER: *(u8 **)Data += 2; Atom->Proc.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case CHAPPROCCODECID: *(u8 **)Data += 2; getData(Data, (void *)&Atom->Proc.ProcessCodecID); return;
	case CHAPPROCPRIVATE: *(u8 **)Data += 2; getBinaryData(Data, (void **)&Atom->Proc.ProcessPrivate, &Atom->Proc.ProcessPrivateSize); return;
	case CHAPPROCCOMMANDMASTER: *(u8 **)Data += 2; Atom->Proc.Commands.size = (u32)GetVIntSize(*(u8 **)Data, &numBytes); (*(u8 **)Data) += numBytes; return;
	case CHAPPROCTIME: *(u8 **)Data += 2; getData(Data, (void *)&Atom->Proc.Commands.ProcessTime); return;
	case CHAPPROCDATA: *(u8 **)Data += 2; getBinaryData(Data, (void **)&Atom->Proc.Commands.ProcessData, &Atom->Proc.Commands.ProcessDataSize); return;
	}

	OutputDebugStringA("Not handled 8-bit or 16-bit case in ParseSegmentChapters.\n");
	return;
}

void ProcessMKVFile(FileInfo *File, Matroska *MkvFile)
{
	//TODO: Temporary??
	HANDLE HeapHandle = GetProcessHeap();


	////////////////////////////////////////////////////
	//	EBML HEADER PARSING
	////////////////////////////////////////////////////
	u8  *Data8 = (u8 *)File->data;
	EBML *header = &MkvFile->header;
	Segment *segment = &MkvFile->segment;

	u32 numBytes = 0;

	if (*((u32 *)Data8) != EBMLMASTER) { OutputDebugStringA("This is not a valid .mkv file\n"); }
	else { Data8 += 4; header->HeaderSize = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; }

	if (*((u16 *)Data8) != EBMLVERSIONID) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8 += 3; header->Version = *Data8; Data8++; }
	
	if (*((u16 *)Data8) != EBMLREADVERSION) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8 += 3; header->ReadVersion = *Data8; Data8++; }
	
	if (*((u16 *)Data8) != EBMLMAXIDLENGTH) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8 += 3; header->MaxIDLength = *Data8; Data8++; }
	
	if (*((u16 *)Data8) != EBMLMAXSIZELENGTH) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8 += 3; header->MaxSizeLength = *Data8; Data8++; }
	
	if (*((u16 *)Data8) != EBMLDOCTYPE) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else
	{
		Data8 += 2;
		getString((void **)&Data8, &header->DocType);
	}
	
	if (*((u16 *)Data8) != EBMLDOCTYPEVER) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8 += 3; header->DocTypeVersion = *Data8; Data8++; }
	
	if (*((u16 *)Data8) != EBMLDOCTYPEREADVER) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8 += 3; header->DocTypeReadVersion = *Data8; Data8++; }


	////////////////////////////////////////////////////
	//	SEGMENT PARSING
	////////////////////////////////////////////////////
	
	if (*((u32 *)Data8) != SEGMENTMASTER) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8 += 4; segment->SegmentSize = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; }
	
	u8 *StartingData = Data8;
	
	u32 it = 0;
	u32 i = 0;

	while ((u32)(Data8 - StartingData) < segment->SegmentSize)
	{
		u8 *Starter = 0;
		u8 *Middler = 0;

		//Deal with the voids in the segment
		u32 VoidDim = 0;
		if (*Data8 == EBMLVOID) { Data8++; VoidDim = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; Data8 += VoidDim; }

		switch (*((u32 *)Data8))
		{
			////////////////////////////////////////////////////
			//	SEEK PARSING
			////////////////////////////////////////////////////
		case SEEKHEAD:
			Data8 += 4; segment->SeekHead.size = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes;

			/*Just to be sure I allocate 16 Seeks, because I actually have no freaking way to determine how many seeks there are other than looking at them all.*/
			//TODO: Temporary??
			segment->SeekHead.Seeks = (Seek *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, sizeof(Seek) * 16);

			it = segment->SeekHead.size;
			i = 0;
			while (it)
			{
				if (*((u16 *)Data8) != SEEK) { OutputDebugStringA("Error when finding all the seeks.\n"); }
				else { Data8 += 2; it -= 2; segment->SeekHead.Seeks[i].size = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; it -= numBytes; }

				if (*((u16 *)Data8) != SEEKID) { OutputDebugStringA("Can't find SeekID.\n"); }
				else { Data8 += 2; getData((void **)(&Data8), (void *)&segment->SeekHead.Seeks[i].SeekID); }

				if (*((u16 *)Data8) != SEEKPOSITION) { OutputDebugStringA("Can't find SeekPos.\n"); }
				else { Data8 += 2; getData((void **)(&Data8), (void *)&segment->SeekHead.Seeks[i].SeekPosition); }

				it -= segment->SeekHead.Seeks[i].size;
				i++;
			}
			break;

			////////////////////////////////////////////////////
			//	SEGMENT INFO PARSING
			////////////////////////////////////////////////////
		case INFOMASTER:
			Data8 += 4; segment->Info.InfoSize = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes;

			Starter = Data8;

			while ((u32)(Data8 - Starter) < segment->Info.InfoSize)
			{
				ParseSegmentInfo((void **)&Data8, segment);
			}
			break;

			////////////////////////////////////////////////////
			//	TRACKS PARSING
			////////////////////////////////////////////////////

		case TRACKMASTER:
			Data8 += 4; segment->Tracks.size = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes;

			//Randomly choosing to allocate 5 tracks. Should be enough space for most use cases.
			segment->Tracks.Entries = (TrackEntry *)HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, sizeof(TrackEntry) * 5);
			i = 0;

			Starter = Data8;
			while ((u32)(Data8 - Starter) < segment->Tracks.size)
			{
				if (*Data8 != TRACKENTRYMASTER) { OutputDebugStringA("Error when parsing tracks.\n"); }
				else { Data8 += 1; segment->Tracks.Entries[i].size = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; }

				segment->Tracks.Entries[i].FlagDefault = -1;
				segment->Tracks.Entries[i].FlagEnabled = -1;
				segment->Tracks.Entries[i].FlagForced = -1;
				segment->Tracks.Entries[i].FlagLacing = -1;

				Middler = Data8;

				while ((u32)(Data8 - Middler) < segment->Tracks.Entries[i].size)
				{
					ParseSegmentTracks((void **)&Data8, &segment->Tracks.Entries[i]);
				}
				i++;
			}
			break;

		case CHAPTERSMASTER:
			Data8 += 4; segment->Chapters.size = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes;

			i = 0;
			Starter = Data8;
			while ((u32)(Data8 - Starter) < segment->Chapters.size)
			{
				if (*(u16 *)Data8 != EDITIONMASTER) { OutputDebugStringA("Error when parsing tracks.\n"); }
				else { Data8 += 2; segment->Chapters.Entries[i].size = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; }

				segment->Chapters.Entries[i].FlagHidden = -1;
				segment->Chapters.Entries[i].FlagDefault = -1;
				segment->Chapters.Entries[i].FlagOrdered = -1;

				Middler = Data8;

				while ((u32)(Data8 - Middler) < segment->Chapters.Entries[i].size)
				{
					ParseSegmentChapters((void **)&Data8, &segment->Chapters.Entries[i]);
				}
				i++;
			}
			segment->Chapters.numEntries = (i + 1);
			break;

		case CLUSTERMASTER:
			break;

		default:
			OutputDebugStringA("Error when parsing segment.\n");
			return;
		}
	}
}

void EditorLoop(MemoryArena *Memory, Matroska *MkvStruct)
{
	static b32 FileLoaded = FALSE;

	if (FileLoaded == FALSE)
	{
		FileInfo MkvFile = {};
		Memory->ReadEntireFile("F:/ProgrammingProjects/VideoEditor/VideoEditor/resources/001.mkv", &MkvFile, &Memory->PermanentMemory);
		ProcessMKVFile(&MkvFile, MkvStruct);

		FileLoaded = TRUE;
	}

}