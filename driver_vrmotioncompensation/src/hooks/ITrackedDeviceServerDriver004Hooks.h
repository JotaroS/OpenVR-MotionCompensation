#pragma once

#include "common.h"
#include <openvr_driver.h>
#include <memory>
#include <map>


namespace vrmotioncompensation
{
	namespace driver
	{
		// forward declarations
		class ServerDriver;

		class ITrackedDeviceServerDriver004Hooks : public InterfaceHooks
		{
		public:
			typedef vr::EVRInitError(*activate_t)(void*, uint32_t unObjectId);

			static std::shared_ptr<InterfaceHooks> createHooks(void* iptr);
			virtual ~ITrackedDeviceServerDriver004Hooks();

		private:
			HookData<activate_t> activateHook;
			void* activateAddress = nullptr;

			ITrackedDeviceServerDriver004Hooks(void* iptr);

			template<typename T>
			struct _hookedAdressMapEntry
			{
				unsigned useCount = 0;
				HookData<T> hookData;
			};
			static std::map<void*, _hookedAdressMapEntry<activate_t>> _hookedActivateAdressMap;

			static vr::EVRInitError _activate(void*, uint32_t unObjectId);

		};
	}
}