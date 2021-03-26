#pragma once

#include <stdint.h>


namespace vrmotioncompensation
{
	enum class MotionCompensationMode : uint32_t
	{
		Disabled = 0,
		ReferenceTracker = 1,
	};

	enum class MotionCompensationDeviceMode : uint32_t
	{
		Default = 0,
		ReferenceTracker = 1,
		MotionCompensated = 2,
		MotionCompensated1 = 3,
		GoGo = 4,
		GoGo1 = 5,
		GoGo_accel = 6,
		GoGo_accel1 = 7,
		Delay = 8,
		Delay1 = 9,
		Saber = 10,
		Saber1 = 11,
		GamepadBoxing = 12,
		GamepadBoxing1 = 13,
		GamepadSaber = 14,
		GamepadSaber1 = 15
	};

	struct DeviceInfo
	{
		uint32_t OpenVRId;
		vr::ETrackedDeviceClass deviceClass;
		MotionCompensationDeviceMode deviceMode;
	};

	struct MMFstruct_OVRMC_v1
	{
		/*#define FLAG_ENABLE_MC		0
		#define FLAG_RESETZEROPOSE	1*/

		vr::HmdVector3d_t Translation;
		vr::HmdVector3d_t Rotation;
		vr::HmdQuaternion_t QRotation;
		uint32_t Flags_1;
		uint32_t Flags_2;
		double Reserved_double[10];
		int Reserved_int[10];

		MMFstruct_OVRMC_v1()
		{
			Translation = { 0, 0, 0 };
			Rotation = { 0, 0, 0 };
			QRotation = { 0, 0, 0, 0 };
			Flags_1 = 0;
			Flags_2 = 0;
		}
	};

} // end namespace vrmotioncompensation