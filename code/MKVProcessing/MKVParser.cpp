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
			u64 temp = ByteSwap64(*((u64 *)dest + 1));
			*((u64 *)dest + 1) = ByteSwap64(*(u64 *)dest);
			*(u64 *)dest = temp;
			break;
	}

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

	if(*((u16 *)Data8) != EBMLVERSIONID) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8+=3; header->Version = *Data8; Data8++; }

	if (*((u16 *)Data8) != EBMLREADVERSION) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8+=3; header->ReadVersion = *Data8; Data8++; }

	if (*((u16 *)Data8) != EBMLMAXIDLENGTH) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8+=3; header->MaxIDLength = *Data8; Data8++; }

	if (*((u16 *)Data8) != EBMLMAXSIZELENGTH) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8+=3; header->MaxSizeLength = *Data8; Data8++; }

	if (*((u16 *)Data8) != EBMLDOCTYPE) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else 
	{ 
		Data8+=2;
		getString((void **)&Data8, &header->DocType);
	}

	if (*((u16 *)Data8) != EBMLDOCTYPEVER) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8+=3; header->DocTypeVersion = *Data8; Data8++; }

	if (*((u16 *)Data8) != EBMLDOCTYPEREADVER) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8+=3; header->DocTypeReadVersion = *Data8; Data8++; }


	////////////////////////////////////////////////////
	//	SEGMENT PARSING
	////////////////////////////////////////////////////

	if (*((u32 *)Data8) != SEGMENTMASTER) { OutputDebugStringA("The .mkv file seems corrupted\n"); }
	else { Data8 += 4; segment->SegmentSize = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; }

	u8 *StartingData = Data8;

	u32 it = 0;
	u32 i = 0;

	while((u32)(Data8 - StartingData) < segment->SegmentSize)
	{
		u8 *Starter = 0;

		//Deal with the voids in the segment
		u32 VoidDim = 0;
		if (*Data8 == EBMLVOID) { Data8++; VoidDim = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; Data8 += VoidDim; }

		switch (*((u32 *)Data8))
		{
			////////////////////////////////////////////////////
			//	SEEK PARSING
			////////////////////////////////////////////////////
			case SEEKHEAD:
				{Data8 += 4; segment->SeekHead.size = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; }

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
				//if (*(u32 *)Data8 != INFOMASTER) { OutputDebugStringA("Can't find Info Master of this segment. Mkv corrupted?\n"); }
				{ Data8 += 4; segment->Info.InfoSize = (u32)GetVIntSize(Data8, &numBytes); Data8 += numBytes; }

				Starter = Data8;

				while ((u32)(Data8 - Starter) < segment->Info.InfoSize)
				{
					ParseSegmentInfo((void **)&Data8, segment);
				}
				break;

			////////////////////////////////////////////////////
			//	TRACKS PARSING
			////////////////////////////////////////////////////

			default:
				OutputDebugStringA("Error when parsing segment.\n");
				break;
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