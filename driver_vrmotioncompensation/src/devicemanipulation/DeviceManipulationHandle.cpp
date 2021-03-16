#include "../driver/ServerDriver.h"
#include "DeviceManipulationHandle.h"


#include "../hooks/IVRServerDriverHost004Hooks.h"
#include "../hooks/IVRServerDriverHost005Hooks.h"

// #define WIN32_LEAN_AND_MEAN
#undef NOSOUND
#include <Windows.h>
// According to windows documentation mmsystem.h should be automatically included with Windows.h when WIN32_LEAN_AND_MEAN and NOSOUND are not defined
// But it doesn't work so I have to include it manually
#include <mmsystem.h>


namespace vrmotioncompensation
{
	namespace driver
	{
		DeviceManipulationHandle::DeviceManipulationHandle(const char* serial, vr::ETrackedDeviceClass eDeviceClass)
			: m_isValid(true), m_parent(ServerDriver::getInstance()), m_motionCompensationManager(m_parent->motionCompensation()), m_eDeviceClass(eDeviceClass), m_serialNumber(serial)
		{
		}

		void DeviceManipulationHandle::setValid(bool isValid)
		{
			m_isValid = isValid;
		}

		bool DeviceManipulationHandle::handlePoseUpdate(uint32_t& unWhichDevice, vr::DriverPose_t& newPose, uint32_t unPoseStructSize)
		{

			if (m_deviceMode == MotionCompensationDeviceMode::ReferenceTracker)
			{ 
				//Check if the pose is valid to prevent unwanted jitter and movement
				if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK)
				{
					//Set the Zero-Point for the reference tracker if not done yet
					if (!m_motionCompensationManager.isZeroPoseValid())
					{						
						m_motionCompensationManager.setZeroPose(newPose);
					}
					else
					{
						//Update reference tracker position
						m_motionCompensationManager.updateRefPose(newPose);
					}
				}
			}
			else if (m_deviceMode == MotionCompensationDeviceMode::MotionCompensated)
			{
				//Check if the pose is valid to prevent unwanted jitter and movement
				if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK)
				{
					m_motionCompensationManager.applyGoGo(newPose);
				}
			}

			return true;
		}

		void DeviceManipulationHandle::setMotionCompensationDeviceMode(MotionCompensationDeviceMode DeviceMode)
		{
			LOG(INFO) << "Jotaro: setting motion compensation device mode as " << (int)DeviceMode;
			m_deviceMode = DeviceMode;
		}
	} // end namespace driver
} // end namespace vrmotioncompensation