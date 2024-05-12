#pragma once

#include "TESForm.hpp"
#include "TESImageSpaceModifiableCountForm.hpp"
#include "TESTexture1024.hpp"
#include "TESModel.hpp"

class TESWeather : public TESForm, public TESImageSpaceModifiableCountForm {
public:
	TESWeather();
	virtual ~TESWeather();

	struct WeatherSound {
		UInt32 uiSoundID;
		UInt32 uiType;
	};

	enum FogData {
		FD_DAY_NEAR = 0,
		FD_DAY_FAR = 1,
		FD_NIGHT_NEAR = 2,
		FD_NIGHT_FAR = 3,
		FD_DAY_POWER = 4,
		FD_NIGHT_POWER = 5,
		FD_COUNT
	};

	enum WeatherData {
		WD_WIND_SPEED		= 0,
		WD_CLOUD_SPEEDY		= 1,
		WD_CLOUD_SPEEDX		= 2,
		WD_TRANS_DELTA		= 3,
		WD_SUN_GLARE		= 4,
		WD_SUN_DAMAGE		= 5,
		WD_BEGIN_PRECIP		= 6,
		WD_END_PRECIP		= 7,
		WD_BEGIN_THUNDER	= 8,
		WD_END_THUNDER		= 9,
		WD_THUNDER_FREQ		= 10,
		WD_FLAGS			= 11,
		WD_LIGHTNING_R		= 12,
		WD_LIGHTNING_G		= 13,
		WD_LIGHTNING_B		= 14,
		WD_COUNT,
	};

	enum COLOR_TYPE : UInt32 {
		COLOR_SKY_UPPER		= 0,
		COLOR_FOG			= 1,
		COLOR_CLOUDS_LOWER	= 2,
		COLOR_AMBIENT		= 3,
		COLOR_SUNLIGHT		= 4,
		COLOR_SUN			= 5,
		COLOR_STARS			= 6,
		COLOR_SKY_LOWER		= 7,
		COLOR_HORIZON		= 8,
		COLOR_CLOUDS_UPPER	= 9,
		COLOR_COUNT,
	};

	enum COLOR_TIME : UInt32 {
		TIME_SUNRISE	= 0,
		TIME_DAY		= 1,
		TIME_SUNSET		= 2,
		TIME_NIGHT		= 3,
		TIME_HIGH_NOON	= 4,
		TIME_MIDNIGHT	= 5,
		TIME_COUNT,
	};


	TESTexture1024				kCloudTexture[4];
	UInt8						ucCloudSpeed[4];
	UInt32						uiCloudColorData[4][TIME_COUNT];
	TESModel					kModel;
	UInt8						ucWeatherData[WD_COUNT];
	float						fFogData[FD_COUNT];
	UInt32						uiColorData[COLOR_COUNT][TIME_COUNT];
	BSSimpleList<WeatherSound*> kSoundList;
	float						fHDRInfo[14];
	UInt32						unk238[76]; // Unused
	UInt32						uiNumCloudLayers;

	bool IsRainy() const;
	double GetWeatherData(WeatherData aeData, float afMax, float afMin) const;
	UInt32 GetColor(COLOR_TYPE aeColorType, COLOR_TIME aeColorTime) const;
};

ASSERT_SIZE(TESWeather, 0x36C);