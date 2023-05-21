#include "TextureExporterFastPNG.h"
#include "PNGExportOption.h"
#include "AssetExportTask.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

UTextureExporterFastPNG::UTextureExporterFastPNG(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	SupportedClass = UTexture2D::StaticClass();
	PreferredFormatIndex = 0;
	FormatExtension.Add(TEXT("PNG"));
	FormatDescription.Add(TEXT("Portable Network Graphics"));
}

bool UTextureExporterFastPNG::SupportsObject(UObject* Object) const
{
	bool bSupportsObject = false;
	if (Super::SupportsObject(Object))
	{
		UTexture2D* Texture = Cast<UTexture2D>(Object);

		if (Texture)
		{
			bSupportsObject = Texture->Source.GetFormat() == TSF_BGRA8 || Texture->Source.GetFormat() == TSF_RGBA16;
		}
	}
	return bSupportsObject;
}

bool UTextureExporterFastPNG::ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn,
	int32 FileIndex, uint32 PortFlags)
{
	UTexture2D* Texture = CastChecked<UTexture2D>( Object );

	if (!Texture->Source.IsValid() || (Texture->Source.GetFormat() != TSF_BGRA8 && Texture->Source.GetFormat() != TSF_RGBA16))
	{
		return false;
	}

	const bool bIsRGBA16 = Texture->Source.GetFormat() == TSF_RGBA16;

	if (bIsRGBA16)
	{
		FMessageLog ExportWarning("EditorErrors");
		FFormatNamedArguments Arguments;
		ExportWarning.Warning(FText::FromString(FString::Printf(TEXT("%s: Texture is RGBA16 and cannot be represented at such high bit depth in .png. Color will be scaled to RGBA8."), *Texture->GetName())));
		ExportWarning.Open(EMessageSeverity::Warning);
	}

	UPNGExportOption* Option = nullptr;
	if (ExportTask)
	{
		Option = Cast<UPNGExportOption>(ExportTask->Options);
	}

	bool bOptionWithAlpha = true;
	float Gamma = 1;
	if (Option)
	{
		bOptionWithAlpha = Option->bWithAlpha;
		Gamma = Option->gamma;
	}

	auto GammaCorrection = [Gamma](uint8 ColorByte)
	{
		return roundf(powf(ColorByte / 255.0f, 1 / Gamma) * 255.0f);
	};

	const int32 BytesPerPixel = bIsRGBA16 ? 8 : 4;

	int32 SizeX = Texture->Source.GetSizeX();
	int32 SizeY = Texture->Source.GetSizeY();
	TArray64<uint8> RawData;
	Texture->Source.GetMipData(RawData, 0);

	// If we should export the file with no alpha info.  
	// If the texture is compressed with no alpha we should definitely not export an alpha channel
	bool bExportWithAlpha = !Texture->CompressionNoAlpha && bOptionWithAlpha;
	if( bExportWithAlpha )
	{
		// If the texture isn't compressed with no alpha scan the texture to see if the alpha values are all 255 which means we can skip exporting it.
		// This is a relatively slow process but we are just exporting textures 
		bExportWithAlpha = false;
		const int32 AlphaOffset = bIsRGBA16 ? 7 : 3;
		for( int32 Y = SizeY - 1; Y >= 0; --Y )
		{
			uint8* Color = &RawData[Y * SizeX * BytesPerPixel];
			for( int32 X = SizeX; X > 0; --X )
			{
				// Skip color info
				Color += AlphaOffset;
				// Get Alpha value then increment the pointer past it for the next pixel
				uint8 Alpha = *Color++;
				if( Alpha != 255 )
				{
					// When a texture is imported with no alpha, the alpha bits are set to 255
					// So if the texture has non 255 alpha values, the texture is a valid alpha channel
					bExportWithAlpha = true;
					break;
				}
			}
			if( bExportWithAlpha )
			{
				break;
			}
		}
	}

	const int32 OriginalWidth = SizeX;
	const int32 OriginalHeight = SizeY;
	const int32 Channels = bExportWithAlpha ? 4 : 3;
	const int32 TotalSize = OriginalWidth * OriginalHeight * Channels;

	uint8* Data = new uint8[TotalSize];

	for( int32 Y = OriginalHeight - 1; Y >= 0; --Y )
	{
		uint8* Color = &RawData[Y * OriginalWidth * BytesPerPixel];
		uint8* OutData = &Data[Y * OriginalWidth * Channels];
		for( int32 X = OriginalWidth; X > 0; --X )
		{
			if (bIsRGBA16)
			{
				// From RGBA16 To RGBA8
				OutData[0] = GammaCorrection(Color[1]);
				OutData[1] = GammaCorrection(Color[3]);
				OutData[2] = GammaCorrection(Color[5]);
				if (bExportWithAlpha)
				{
					OutData[3] = Color[7];
				}
			}
			else
			{
				// From BGRA8 To RGBA8
				OutData[0] = GammaCorrection(Color[2]);
				OutData[1] = GammaCorrection(Color[1]);
				OutData[2] = GammaCorrection(Color[0]);
				if (bExportWithAlpha)
				{
					OutData[3] = Color[3];
				}
			}
			Color += BytesPerPixel;
			OutData += Channels;
		}
	}
	
	int PngSize;
	unsigned char* PngData = stbi_write_png_to_mem(Data, 0, OriginalWidth, OriginalHeight, Channels, &PngSize);
	delete[] Data;
	if (PngData == nullptr)
		return false;
	Ar.Serialize(PngData, PngSize);
	STBIW_FREE(PngData);
	return true;
}