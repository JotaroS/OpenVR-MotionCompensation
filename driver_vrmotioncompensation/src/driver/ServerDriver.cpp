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
			if (!firstTrackerActivated) {
				handle->setServerDriverHooks(InterfaceHooks::hookInterface(pDriver, "ITrackedDeviceServerDriver_005"));
				firstTrackerActivated = true;
			}
			else {
				handle->setServerDriverHooks(InterfaceHooks::hookInterface(pDriver, "ITrackedDeviceServerDriver_004"));
			}
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
			// handle GamePad input
			
			if (!gamepad.Refresh())
			{
				if (wasConnected)
				{
					wasConnected = false;
					LOG(INFO) << "Please connect an Xbox 360 controller.";
				}
			}
			else
			{
				if (!wasConnected)
				{
					wasConnected = true;
					LOG(INFO) << "Controller connected on port " << gamepad.GetPort();
				}
				//LOG(INFO) << "Left thumb stick: (" << gamepad.leftStickX << ", " << gamepad.leftStickY << ")   Right thumb stick : (" << gamepad.rightStickX << ", " << gamepad.rightStickY << ")";
				//LOG(INFO) << "Left analog trigger: " << gamepad.leftTrigger << "   Right analog trigger: " << gamepad.rightTrigger;

				//if (gamepad.IsPressed(XINPUT_GAMEPAD_A))
				//	LOG(INFO) << "(A) button pressed";
			}

			// handle UDP message from GUIController.
			std::string msg = UDPSocket->getLastMessage();
			//LOG(INFO) << "Jotaro: last message = " << msg;
			uint32_t hmdDeviceManipulationHandle = 0;

			if (deviceActivated[hmdDeviceManipulationHandle]) {
				auto m_handle = this->getDeviceManipulationHandleById(hmdDeviceManipulationHandle);
				m_handle->setMotionCompensationDeviceMode(MotionCompensationDeviceMode::HMDTracking);
			}

			uint32_t leftDeviceManipulationHandle = 1;
			uint32_t rightDeviceManipulationHandle = 2;
			if (msg == "SetRefPos") {
				if (deviceActivated[leftDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(leftDeviceManipulationHandle); m_handle->setRefPos(0);
				}
				if (deviceActivated[rightDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(rightDeviceManipulationHandle); m_handle->setRefPos(1);
				}
			}
			else if (msg == "DeactivateGoGo") {
				if (deviceActivated[leftDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(leftDeviceManipulationHandle); m_handle->setRefPos(0);
					m_handle->setMotionCompensationDeviceMode(MotionCompensationDeviceMode::Default);
					m_handle->isGoGoActive = false;
				}
				if (deviceActivated[rightDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(rightDeviceManipulationHandle); m_handle->setRefPos(1);
					m_handle->setMotionCompensationDeviceMode(MotionCompensationDeviceMode::Default);
					m_handle->isGoGoActive = false;
				}
			}
			else if (msg == "ActivateGoGo") {
				if (deviceActivated[leftDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(leftDeviceManipulationHandle);  m_handle->setRefPos(0);
					m_handle->setMotionCompensationDeviceMode(MotionCompensationDeviceMode::MotionCompensated1);
					m_handle->isGoGoActive = true;
				}
				if (deviceActivated[rightDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(rightDeviceManipulationHandle); m_handle->setRefPos(1);
					m_handle->setMotionCompensationDeviceMode(MotionCompensationDeviceMode::MotionCompensated);
					m_handle->isGoGoActive = true;
				}
			}
			// this runs every 50ms.
			else if (msg != "") {
				auto j = nlohmann::json::parse(msg);
				MotionCompensationDeviceMode mode; //left
				MotionCompensationDeviceMode mode1;//right
				std::string type_interaction = j["type_interaction"];
				if (type_interaction == "go-go") {
					mode = MotionCompensationDeviceMode::MotionCompensated;
					mode1 = MotionCompensationDeviceMode::MotionCompensated1;
				}
				if (type_interaction == "go-go-with-accel") {
					mode = MotionCompensationDeviceMode::GoGo_accel;
					mode1 = MotionCompensationDeviceMode::GoGo_accel1;
				}
				if (type_interaction == "deactivate") {
					mode = MotionCompensationDeviceMode::Default;
					mode1 = MotionCompensationDeviceMode::Default;
				}
				if (type_interaction == "delay") {
					mode = MotionCompensationDeviceMode::Delay;
					mode1 = MotionCompensationDeviceMode::Delay1;
				}
				if (type_interaction == "saber") {
					mode = MotionCompensationDeviceMode::Saber;
					mode1 = MotionCompensationDeviceMode::Saber1;
				}
				if (type_interaction == "gamepad-boxing") {
					mode = MotionCompensationDeviceMode::GamepadBoxing;
					mode1 = MotionCompensationDeviceMode::GamepadBoxing1;
				}
				if (type_interaction == "gamepad-saber") {
					mode = MotionCompensationDeviceMode::GamepadSaber;
					mode1 = MotionCompensationDeviceMode::GamepadSaber1;
				}
				if (deviceActivated[leftDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(leftDeviceManipulationHandle);
					m_handle->setMotionCompensationDeviceMode(mode1);
					m_handle->setCDRatio(j["x-CD-r"], j["y-CD-r"], j["z-CD-r"],0);
					m_handle->setOffset(j["x-ofs-r"], j["y-ofs-r"], j["z-ofs-r"],0);
					m_handle->setRotOffset(j["rotx-ofs-r"], j["roty-ofs-r"], j["rotz-ofs-r"],0);
					m_handle->setRotOffsetQuat(j["qw-ofs-r"], j["qx-ofs-r"], j["qy-ofs-r"], j["qz-ofs-r"],0);
					m_handle->setPunchDist(j["punch_dist"],0);
					m_handle->setSaberRot(j["saber_rot"]);
				}
				if (deviceActivated[rightDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(rightDeviceManipulationHandle);
					m_handle->setMotionCompensationDeviceMode(mode);
					m_handle->setCDRatio(j["x-CD-l"], j["y-CD-l"], j["z-CD-l"], 1);
					m_handle->setOffset(j["x-ofs-l"], j["y-ofs-l"], j["z-ofs-l"], 1);
					m_handle->setRotOffset(j["rotx-ofs-l"], j["roty-ofs-l"], j["rotz-ofs-l"], 1);
					m_handle->setRotOffsetQuat(j["qw-ofs-l"], j["qx-ofs-l"], j["qy-ofs-l"], j["qz-ofs-l"], 1);
					m_handle->setPunchDist(j["punch_dist"], 1);
					m_handle->setSaberRot(j["saber_rot"]);
				}
			}
			//punching / reaching 'controller' condition
			//this runs every frame.
			{
				if (deviceActivated[leftDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(leftDeviceManipulationHandle);
					m_handle->setPunchTriggerOffset(gamepad.rightTrigger, 0);
					m_handle->setGamepadStickOffset(gamepad.rightStickX, gamepad.rightStickY, 0);
					m_handle->setAButtonPressed(gamepad.IsPressed(XINPUT_GAMEPAD_A));
					m_handle->setBButtonPressed(gamepad.IsPressed(XINPUT_GAMEPAD_B));
				}
				if (deviceActivated[rightDeviceManipulationHandle]) {
					auto m_handle = this->getDeviceManipulationHandleById(rightDeviceManipulationHandle);
					m_handle->setGamepadStickOffset(gamepad.leftStickX, gamepad.leftStickY, 1);
					m_handle->setPunchTriggerOffset(gamepad.leftTrigger, 1);
				
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