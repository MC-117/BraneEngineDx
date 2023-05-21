// Fill out your copyright notice in the Description page of Project Settings.


#include "PNGExportOption.h"

UPNGExportOption::UPNGExportOption(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWithAlpha = true;
	gamma = 1;
}