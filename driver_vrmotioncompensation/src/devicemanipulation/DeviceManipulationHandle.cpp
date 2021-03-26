#include "../driver/ServerDriver.h"
#include "DeviceManipulationHandle.h"


#include "../hooks/IVRServerDriverHost004Hooks.h"
#include "../hooks/IVRServerDriverHost005Hooks.h"
#include "../hooks/IVRServerDriverHost006Hooks.h"

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
					m_motionCompensationManager.applyGoGo(newPose,0); //left hand
				}
			}
			else if (m_deviceMode == MotionCompensationDeviceMode::MotionCompensated1)
			{
				//Check if the pose is valid to prevent unwanted jitter and movement
				if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK)
				{
					m_motionCompensationManager.applyGoGo(newPose,1); //right hand (usually left hand controller is recognizied first. I guess differnet in VIVE/oculus(virtual desktop) envirnonment)
				}
			}

			else if (m_deviceMode == MotionCompensationDeviceMode::GoGo_accel)
			{
				//Check if the pose is valid to prevent unwanted jitter and movement
				if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK)
				{
					m_motionCompensationManager.applyGoGoAccel(newPose,0); //left hand
				}
			}
			else if (m_deviceMode == MotionCompensationDeviceMode::GoGo_accel1)
			{
				//Check if the pose is valid to prevent unwanted jitter and movement
				if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK)
				{
					m_motionCompensationManager.applyGoGoAccel(newPose,1); //right hand (usually left hand controller is recognizied first. I guess differnet in VIVE/oculus(virtual desktop) envirnonment)
				}
			}
			else if (m_deviceMode == MotionCompensationDeviceMode::Delay)
			{
				//Check if the pose is valid to prevent unwanted jitter and movement
				if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK)
				{
					m_motionCompensationManager.applyGoGoDelay(newPose, 0); //right hand (usually left hand controller is recognizied first. I guess differnet in VIVE/oculus(virtual desktop) envirnonment)
				}
			}

			else if (m_deviceMode == MotionCompensationDeviceMode::Delay1)
			{
				//Check if the pose is valid to prevent unwanted jitter and movement
				if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK)
				{
					m_motionCompensationManager.applyGoGoDelay(newPose, 1); //right hand (usually left hand controller is recognizied first. I guess differnet in VIVE/oculus(virtual desktop) envirnonment)
				}
			}

			else if (m_deviceMode == MotionCompensationDeviceMode::Saber)
			{
				//Check if the pose is valid to prevent unwanted jitter and movement
				if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK)
				{
					m_motionCompensationManager.applyGoGoSaber(newPose, 0); //right hand (usually left hand controller is recognizied first. I guess differnet in VIVE/oculus(virtual desktop) envirnonment)
				}
			}

			else if (m_deviceMode == MotionCompensationDeviceMode::Saber1)
			{
				//Check if the pose is valid to prevent unwanted jitter and movement
				if (newPose.poseIsValid && newPose.result == vr::TrackingResult_Running_OK)
				{
					m_motionCompensationManager.applyGoGoSaber(newPose, 1); //right hand (usually left hand controller is recognizied first. I guess differnet in VIVE/oculus(virtual desktop) envirnonment)
				}
			}



			return true;
		}

		void DeviceManipulationHandle::setMotionCompensationDeviceMode(MotionCompensationDeviceMode DeviceMode)
		{
			LOG(INFO) << "Jotaro: setting motion compensation device mode as " << (int)DeviceMode <<" for device" << m_openvrId <<":"<<m_serialNumber;
			m_deviceMode = DeviceMode;
		}

		void DeviceManipulationHandle::setRefPos(int idx) {
			m_motionCompensationManager.setRefPos(idx);
		}
		void DeviceManipulationHandle::setCDRatio(float x, float y, float z, int idx) {
			m_motionCompensationManager.setCDRatio(x, y, z, idx);
		}

		void DeviceManipulationHandle::setOffset(float x, float y, float z, int idx) {
			m_motionCompensationManager.setOffset(x, y, z, idx);
		}

		void DeviceManipulationHandle::setRotOffset(float x, float y, float z, int idx) {
			m_motionCompensationManager.setRotOffset(x, y, z, idx);
		}
		void DeviceManipulationHandle::setRotOffsetQuat(float w, float x, float y, float z, int idx) {
			m_motionCompensationManager.setRotOffsetQuat(w, x, y, z, idx);
		}
		void DeviceManipulationHandle::setPunchTriggerOffset(double val, int idx) {
			m_motionCompensationManager.setPunchTriggerOffset(val, idx);
		}
		void DeviceManipulationHandle::setPunchDist(double val, int idx) {
			m_motionCompensationManager.setPunchDist(val, idx);
		}

		void DeviceManipulationHandle::setSaberRot(double val) {
			m_motionCompensationManager.setSaberRot(val);
		}
	} // end namespace driver
} // end namespace vrmotioncompensation