#pragma once
#include <boost/timer/timer.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <openvr_driver.h>
#include <vrmotioncompensation_types.h>
#include <openvr_math.h>
#include "../logging.h"
#include "Debugger.h"



// driver namespace
namespace vrmotioncompensation
{
	namespace driver
	{
		// forward declarations
		class ServerDriver;
		class DeviceManipulationHandle;

		class Spinlock
		{
			std::atomic_flag _Flag = ATOMIC_FLAG_INIT;

		public:

			void lock() noexcept
			{
				while (_Flag.test_and_set(std::memory_order_acquire))
				{
					// pause insn
					YieldProcessor();
				}
			}

			void unlock() noexcept
			{
				_Flag.clear(std::memory_order_release);
			}
			
		};

		class MotionCompensationManager
		{
		public:
			MotionCompensationManager(ServerDriver* parent);/* : m_parent(parent)
			{
			}*/
			
			bool StartDebugData();

			void StopDebugData();

			bool setMotionCompensationMode(MotionCompensationMode Mode, int MCdevice, int RTdevice);

			void setNewMotionCompensatedDevice(int McDevice);

			void setNewReferenceTracker(int RtDevice);

			MotionCompensationMode getMotionCompensationMode()
			{
				return _Mode;
			}

			void setAlpha(uint32_t samples);

			void setLpfBeta(double NewBeta)
			{
				_LpfBeta = NewBeta;
			}

			double getLPFBeta()
			{
				return _LpfBeta;
			}

			int getMCdeviceID()
			{
				return _McDeviceID;
			}

			int getRTdeviceID()
			{
				return _RtDeviceID;
			}

			void setZeroMode(bool setZero);

			void setOffsets(MMFstruct_OVRMC_v1 offsets);

			bool isZeroPoseValid();
			
			void resetZeroPose();

			void setZeroPose(const vr::DriverPose_t& pose);
			
			void updateRefPose(const vr::DriverPose_t& pose);
			
			bool applyMotionCompensation(vr::DriverPose_t& pose);

			bool applyGoGo(vr::DriverPose_t& pose, int idx);

			void runFrame();
			bool applyGoGo(vr::DriverPose_t& pose);
			bool applyGoGo1(vr::DriverPose_t& pose);
			void setPunchDist(float val, int idx);
			void setPunchTriggerOffset(float val,int idx);
			void setRefPos(int idx){
				_GoGoRefPos[idx][0] = _LastPos[idx][0];
				_GoGoRefPos[idx][1] = _LastPos[idx][1];
				_GoGoRefPos[idx][2] = _LastPos[idx][2];
			}

			void setCDRatio(double x, double y, double z, int idx){
				_CDRatio[idx][0] = x;
				_CDRatio[idx][1] = y;
				_CDRatio[idx][2] = z;
			}
			void setOffset(double x, double y, double z, int idx){
				_OffsetPos[idx][0] = x;
				_OffsetPos[idx][1] = y;
				_OffsetPos[idx][2] = z;
			}
			void setRotOffset(double x, double y, double z, int idx){
				_OffsetRot[idx][0] = x;
				_OffsetRot[idx][1] = y;
				_OffsetRot[idx][2] = z;
			}
			void setRotOffsetQuat(double w, double x, double y, double z, int idx) {
				_OffsetQuat[idx].w = w;
				_OffsetQuat[idx].x = x;
				_OffsetQuat[idx].y = y;
				_OffsetQuat[idx].z = z;
			}
			void applyRotByQuat(vr::HmdQuaternion_t* q1, vr::HmdQuaternion_t q2) {
				vr::HmdQuaternion_t ret;
				ret.w = q1->w * q2.w - q1->x * q2.x - q1->y * q2.y - q1->z * q2.z;
				ret.x = q1->w * q2.x + q1->x * q2.w + q1->y * q2.z - q1->z * q2.y;
				ret.y = q1->w * q2.y - q1->x * q2.z + q1->y * q2.w + q1->z * q2.x;
				ret.z = q1->w * q2.z + q1->x * q2.y - q1->y * q2.x + q1->z * q2.w;

				q1->w = ret.w;
				q1->x = ret.x;
				q1->y = ret.y;
				q1->z = ret.z;

			}
			

		private:

			void InitDebugData();
			void WriteDebugData();
			
			double vecVelocity(double time, const double vecPosition, const double Old_vecPosition);

			double vecAcceleration(double time, const double vecVelocity, const double Old_vecVelocity);

			double rotVelocity(double time, const double vecAngle, const double Old_vecAngle);

			double DEMA(const double RawData, int Axis);

			vr::HmdVector3d_t LPF(const double RawData[3], vr::HmdVector3d_t SmoothData);

			vr::HmdVector3d_t LPF(vr::HmdVector3d_t RawData, vr::HmdVector3d_t SmoothData);

			vr::HmdQuaternion_t lowPassFilterQuaternion(vr::HmdQuaternion_t RawData, vr::HmdQuaternion_t SmoothData);

			vr::HmdQuaternion_t slerp(vr::HmdQuaternion_t q1, vr::HmdQuaternion_t q2, double lambda);

			vr::HmdQuaternion_t qlerp(vr::HmdQuaternion_t q1, vr::HmdQuaternion_t q2, double lambda);

			vr::HmdVector3d_t toEulerAngles(vr::HmdQuaternion_t q);

			const double angleDifference(double angle1, double angle2);

			vr::HmdVector3d_t transform(vr::HmdVector3d_t VecRotation, vr::HmdVector3d_t VecPosition, vr::HmdVector3d_t point);

			vr::HmdVector3d_t transform(vr::HmdQuaternion_t quat, vr::HmdVector3d_t VecPosition, vr::HmdVector3d_t point);

			vr::HmdVector3d_t transform(vr::HmdVector3d_t VecRotation, vr::HmdVector3d_t VecPosition, vr::HmdVector3d_t centerOfRotation, vr::HmdVector3d_t point);

			

			vr::HmdQuaternion_t _OffsetQuat[2] = { { 1,0,0,0 },{ 1, 0, 0, 0 } }; //q(w,x,y,z)
			bool _RefPoseValid = false;
			int _RefPoseValidCounter = 0;

			std::string _vecToStr(char* name, vr::HmdVector3d_t & v)
			{
				std::stringstream ss;
				ss << name << ": [" << v.v[0] << ", " << v.v[1] << ", " << v.v[2] << "]";
				return ss.str();
			}

			std::string _vecToStr(char* name, double v[3])
			{
				std::stringstream ss;
				ss << name << ": [" << v[0] << ", " << v[1] << ", " << v[2] << "]";
				return ss.str();
			}

			std::string _quatToStr(const char* name, const vr::HmdQuaternion_t & q)
			{
				std::stringstream ss;
				ss << name << ": [" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << "]";
				return ss.str();
			}

			inline void _copyVec(double(&d)[3], const double(&s)[3])
			{
				d[0] = s[0];
				d[1] = s[1];
				d[2] = s[2];
			}

			inline void _copyVec(vr::HmdVector3d_t & d, const double(&s)[3])
			{
				d.v[0] = s[0];
				d.v[1] = s[1];
				d.v[2] = s[2];
			}

			inline void _zeroVec(double(&d)[3])
			{
				d[0] = d[1] = d[2] = 0.0;
			}

			inline void _zeroVec(vr::HmdVector3d_t & d)
			{
				d.v[0] = d.v[1] = d.v[2] = 0.0;
			}

			ServerDriver* m_parent;

			boost::interprocess::windows_shared_memory _shdmem;
			boost::interprocess::mapped_region _region;

			int _McDeviceID = -1;
			int _RtDeviceID = -1;
			long long _RefTrackerLastTime = -1;
			vr::DriverPose_t _RefTrackerLastPose;
			vr::HmdVector3d_t _RotEulerFilterOld = {0, 0, 0};

			Debugger _DebugLogger;

			double _LpfBeta = 0.2;
			double _Alpha = -1.0;
			uint32_t _Samples = 100;
			bool _SetZeroMode = false;

			Spinlock _ZeroLock, _RefLock, _RefVelLock;

			bool _Enabled = false;
			MotionCompensationMode _Mode = MotionCompensationMode::Disabled;			
			
			// Offset data
			MMFstruct_OVRMC_v1 _Offset;
			MMFstruct_OVRMC_v1* _Poffset = nullptr;

			// Zero position
			vr::HmdVector3d_t _ZeroPos = { 0, 0, 0 };
			vr::HmdQuaternion_t _ZeroRot = { 1, 0, 0, 0 };
			bool _ZeroPoseValid = false;
			
			// Reference position
			vr::HmdVector3d_t _RefPos = { 0, 0, 0 };
			vr::HmdVector3d_t _Filter_vecPosition[2] = { 0, 0, 0 };

			vr::HmdVector3d_t _RefVel = { 0, 0, 0 };
			vr::HmdVector3d_t _RefAcc = { 0, 0, 0 };

			vr::HmdQuaternion_t _RefRot = { 1, 0, 0, 0 };
			vr::HmdQuaternion_t _RefRotInv = { 1, 0, 0, 0 };
			vr::HmdQuaternion_t _Filter_rotPosition[2] = { 1, 0, 0, 0 };

			vr::HmdVector3d_t _RefRotVel = { 0, 0, 0 };
			vr::HmdVector3d_t _RefRotAcc = { 0, 0, 0 };

			double _GoGoRefPos[2][3] = { { 0,0,0 },{0,0,0} };
			double _CDRatio[2][3] = { { 5.0,5.0,5.0 },{5.0,5.0,5.0} };
			double _LastPos[2][3] = {{ 0,0,0 },{0,0,0}};
			double _OffsetPos[2][3] = {{ 0,0,0 },{0,0,0}};
			double _OffsetRot[2][3] = {{ 0,0,0 },{0,0,0}};
			double _triggerPunchOffset[2] = { 0.0f,0.0 };
			double _punchDist[2] = { 0.1f,0.1f };
			
		};
	}
}