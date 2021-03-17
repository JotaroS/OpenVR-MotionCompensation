#include "../../MyUDPManager.h"
#include "ServerDriver.h"
#include "../devicemanipulation/DeviceManipulationHandle.h"
#include <nlohmann/json.hpp>
namespace vrmotioncompensation
{
	namespace driver
	{
		ServerDriver* ServerDriver::singleton = nullptr;
		std::string ServerDriver::installDir;

		ServerDriver::ServerDriver() : m_motionCompensation(this)
		{
			singleton = this;
			memset(_openvrIdDeviceManipulationHandle, 0, sizeof(DeviceManipulationHandle*) * vr::k_unMaxTrackedDeviceCount);
			memset(_deviceVersionMap, 0, sizeof(int) * vr::k_unMaxTrackedDeviceCount);
			LOG(INFO) << "jotarodriver::ServerDriver()";
		}

		ServerDriver::~ServerDriver()
		{
			LOG(INFO) << "jotarodriver::~ServerDriver()";
		}

		bool ServerDriver::hooksTrackedDevicePoseUpdated(void* serverDriverHost, int version, uint32_t& unWhichDevice, vr::DriverPose_t& newPose, uint32_t& unPoseStructSize)
		{
			if (_openvrIdDeviceManipulationHandle[unWhichDevice] && _openvrIdDeviceManipulationHandle[unWhichDevice]->isValid())
			{
				if (_deviceVersionMap[unWhichDevice] == 0)
				{
					_deviceVersionMap[unWhichDevice] = version;
				}

				LOG(INFO) << "ServerDriver::hooksTrackedDevicePoseUpdated(version:" << version << ", deviceId:" << unWhichDevice << ", first used version: " << _deviceVersionMap[unWhichDevice] << ")";
				
				if (_deviceVersionMap[unWhichDevice] == version)
				{
					return _openvrIdDeviceManipulationHandle[unWhichDevice]->handlePoseUpdate(unWhichDevice, newPose, unPoseStructSize);
				}

				LOG(INFO) << "ServerDriver::hooksTrackedDevicePoseUpdated called for wrong version, ignoring ";
			}
			return true;
		}

		void ServerDriver::hooksTrackedDeviceAdded(void* serverDriverHost, int version, const char* pchDeviceSerialNumber, vr::ETrackedDeviceClass& eDeviceClass, void* pDriver)
		{
			LOG(INFO) << "ServerDriver::hooksTrackedDeviceAdded(" << serverDriverHost << ", " << version << ", " << pchDeviceSerialNumber << ", " << (int)eDeviceClass << ", " << pDriver << ")";
			LOG(INFO) << "Found device " << pchDeviceSerialNumber << " (deviceClass: " << (int)eDeviceClass << ")";

			// Create ManipulationInfo entry
			auto handle = std::make_shared<DeviceManipulationHandle>(pchDeviceSerialNumber, eDeviceClass);
			_deviceManipulationHandles.insert({ pDriver, handle });

			// Hook into server driver interface
			handle->setServerDriverHooks(InterfaceHooks::hookInterface(pDriver, "ITrackedDeviceServerDriver_005"));
		}

		void ServerDriver::hooksTrackedDeviceActivated(void* serverDriver, int version, uint32_t unObjectId)
		{
			LOG(INFO) << "ServerDriver::hooksTrackedDeviceActivated(" << serverDriver << ", " << version << ", " << unObjectId << ")";

			// Search for the activated device
			auto i = _deviceManipulationHandles.find(serverDriver);

			if (i != _deviceManipulationHandles.end())
			{
				auto handle = i->second;
				handle->setOpenvrId(unObjectId);
				_openvrIdDeviceManipulationHandle[unObjectId] = handle.get();

				//LOG(INFO) << "Successfully added device " << handle->serialNumber() << " (OpenVR Id: " << handle->openvrId() << ")";
				LOG(INFO) << "Successfully added device " << _openvrIdDeviceManipulationHandle[unObjectId]->serialNumber() << " (OpenVR Id: " << _openvrIdDeviceManipulationHandle[unObjectId]->openvrId() << ")";
				deviceActivated[_openvrIdDeviceManipulationHandle[unObjectId]->openvrId()] = true;
			}
		}

		vr::EVRInitError ServerDriver::Init(vr::IVRDriverContext* pDriverContext)
		{
			LOG(INFO) << "CServerDriver::Init()";

			// Initialize Hooking
			InterfaceHooks::setServerDriver(this);
			auto mhError = MH_Initialize();
			if (mhError == MH_OK)
			{
				_driverContextHooks = InterfaceHooks::hookInterface(pDriverContext, "IVRDriverContext");
			}
			else
			{
				LOG(ERROR) << "Error while initializing minHook: " << MH_StatusToString(mhError);
			}

			LOG(DEBUG) << "Initialize driver context.";
			VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

			// Read installation directory
			vr::ETrackedPropertyError tpeError;
			installDir = vr::VRProperties()->GetStringProperty(pDriverContext->GetDriverHandle(), vr::Prop_InstallPath_String, &tpeError);
			if (tpeError == vr::TrackedProp_Success)
			{
				LOG(INFO) << "Install Dir:" << installDir;
			}
			else
			{
				LOG(INFO) << "Could not get Install Dir: " << vr::VRPropertiesRaw()->GetPropErrorNameFromEnum(tpeError);
			}

			// Start IPC thread
			// shmCommunicator.init(this);
			
			// Start UDP Server
			UDPSocket = new MyUDPManager();
			return vr::VRInitError_None;
		}


		void ServerDriver::Cleanup()
		{
			LOG(INFO) << "ServerDriver::Cleanup()";
			_driverContextHooks.reset();
			MH_Uninitialize();
			m_motionCompensation.StopDebugData();
			//shmCommunicator.shutdown();
			VR_CLEANUP_SERVER_DRIVER_CONTEXT();
		}

		// Call frequency: ~93Hz
		void ServerDriver::RunFrame() {
			//below:jotaro code
			std::string msg = UDPSocket->getLastMessage();
			LOG(INFO) << "Jotaro: last message = " << msg;
			if (msg == "SetRefPos") {
				if (deviceActivated[1]) {
					auto m_handle = this->getDeviceManipulationHandleById(1); m_handle->setRefPos();
				}
				if (deviceActivated[2]) {
					auto m_handle = this->getDeviceManipulationHandleById(2); m_handle->setRefPos();
				}
			}
			else if (msg == "DeactivateGoGo") {
				if (deviceActivated[1]) {
					auto m_handle = this->getDeviceManipulationHandleById(1); m_handle->setRefPos();
					m_handle->setMotionCompensationDeviceMode(MotionCompensationDeviceMode::Default);
					m_handle->isGoGoActive = false;
				}
				if (deviceActivated[2]) {
					auto m_handle = this->getDeviceManipulationHandleById(2); m_handle->setRefPos();
					m_handle->setMotionCompensationDeviceMode(MotionCompensationDeviceMode::Default);
					m_handle->isGoGoActive = false;
				}
			}
			else if (msg == "ActivateGoGo") {
				if (deviceActivated[1]) {
					auto m_handle = this->getDeviceManipulationHandleById(1);
					m_handle->setMotionCompensationDeviceMode(MotionCompensationDeviceMode::MotionCompensated);
					m_handle->isGoGoActive = true;
				}
				if (deviceActivated[2]) {
					auto m_handle = this->getDeviceManipulationHandleById(2);
					m_handle->setMotionCompensationDeviceMode(MotionCompensationDeviceMode::MotionCompensated);
					m_handle->isGoGoActive = true;
				}
			}
			else if (msg != "") {
				auto j = nlohmann::json::parse(msg);
				if (deviceActivated[1]) {
					auto m_handle = this->getDeviceManipulationHandleById(1); m_handle->setRefPos();
					m_handle->setCDRatio(j["x-CD-r"], j["y-CD-r"], j["z-CD-r"]);
					m_handle->setOffset(j["x-ofs-r"], j["y-ofs-r"], j["z-ofs-r"]);
					m_handle->setRotOffset(j["rotx-ofs-r"], j["roty-ofs-r"], j["rotz-ofs-r"]);
				}
				if (deviceActivated[2]) {
					auto m_handle = this->getDeviceManipulationHandleById(2); m_handle->setRefPos();
					m_handle->setCDRatio(j["x-CD-l"], j["y-CD-l"], j["z-CD-l"]);
					m_handle->setOffset(j["x-ofs-l"], j["y-ofs-l"], j["z-ofs-l"]);
					m_handle->setRotOffset(j["rotx-ofs-l"], j["roty-ofs-l"], j["rotz-ofs-l"]);
				}
			}
		}

		DeviceManipulationHandle* ServerDriver::getDeviceManipulationHandleById(uint32_t unWhichDevice)
		{
			LOG(INFO) << "getDeviceByID: unWhichDevice: " << unWhichDevice;

			std::lock_guard<std::recursive_mutex> lock(_deviceManipulationHandlesMutex);

			if (_openvrIdDeviceManipulationHandle[unWhichDevice]->isValid())
			{
				if (_openvrIdDeviceManipulationHandle[unWhichDevice])
				{
					return _openvrIdDeviceManipulationHandle[unWhichDevice];
				}
				else
				{
					LOG(INFO) << "_openvrIdDeviceManipulationHandle[unWhichDevice] is NULL. unWhichDevice: " << unWhichDevice;
				}
			}
			else
			{
				LOG(INFO) << "_openvrIdDeviceManipulationHandle[unWhichDevice] is not valid. unWhichDevice: " << unWhichDevice;
			}

			return nullptr;
		}

	} // end namespace driver
} // end namespace vrmotioncompensation