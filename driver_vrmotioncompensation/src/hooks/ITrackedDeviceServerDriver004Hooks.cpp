#include "../driver/ServerDriver.h"

#include "ITrackedDeviceServerDriver004Hooks.h"



namespace vrmotioncompensation
{
	namespace driver
	{
		std::map<void*, ITrackedDeviceServerDriver004Hooks::_hookedAdressMapEntry<ITrackedDeviceServerDriver004Hooks::activate_t>> ITrackedDeviceServerDriver004Hooks::_hookedActivateAdressMap;

		ITrackedDeviceServerDriver004Hooks::ITrackedDeviceServerDriver004Hooks(void* iptr)
		{
			LOG(TRACE) << "ITrackedDeviceServerDriver004Hooks::ctr(" << iptr << ")";
			auto vtable = (*((void***)iptr));
			activateAddress = vtable[0];
			auto it = _hookedActivateAdressMap.find(activateAddress);
			if (it == _hookedActivateAdressMap.end())
			{
				CREATE_MH_HOOK(activateHook, _activate, "ITrackedDeviceServerDriver004::Activate", iptr, 0);
				_hookedActivateAdressMap[activateAddress].useCount = 1;
				_hookedActivateAdressMap[activateAddress].hookData = activateHook;
			}
			else
			{
				activateHook = it->second.hookData;
				it->second.useCount += 1;
			}
		}

		std::shared_ptr<InterfaceHooks> ITrackedDeviceServerDriver004Hooks::createHooks(void* iptr)
		{
			std::shared_ptr<InterfaceHooks> retval = std::shared_ptr<InterfaceHooks>(new ITrackedDeviceServerDriver004Hooks(iptr));
			return retval;
		}

		ITrackedDeviceServerDriver004Hooks::~ITrackedDeviceServerDriver004Hooks()
		{
			auto it = _hookedActivateAdressMap.find(activateAddress);
			if (it != _hookedActivateAdressMap.end())
			{
				if (it->second.useCount <= 1)
				{
					REMOVE_MH_HOOK(activateHook);
					_hookedActivateAdressMap.erase(it);
				}
				else
				{
					it->second.useCount -= 1;
				}
			}
		}

		vr::EVRInitError ITrackedDeviceServerDriver004Hooks::_activate(void* _this, uint32_t unObjectId)
		{
			LOG(TRACE) << "ITrackedDeviceServerDriver004Hooks::_activate(" << _this << ", " << unObjectId << ")";
			auto vtable = (*((void***)_this));
			auto activateAddress = vtable[0];
			auto it = _hookedActivateAdressMap.find(activateAddress);
			if (it != _hookedActivateAdressMap.end())
			{
				serverDriver->hooksTrackedDeviceActivated(_this, 5, unObjectId);
				return it->second.hookData.origFunc(_this, unObjectId);
			}
			else
			{
				LOG(ERROR) << "this pointer not in ITrackedDeviceServerDriver004Hooks::_hookedActivateAdressMap.";
				return vr::VRInitError_Unknown;
			}
		}
	}
}