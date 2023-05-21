// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PNGExportOption.generated.h"

/**
 * PNGExportOption
 */
UCLASS(BlueprintType)
class BRANEENGINEEXPORTER_API UPNGExportOption : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Exporter)
	uint32 bWithAlpha : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = Exporter)
	float gamma;
};
