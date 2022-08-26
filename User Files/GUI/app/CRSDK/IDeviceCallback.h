#ifndef IDEVICECALLBACK_H
#define IDEVICECALLBACK_H

#include	"CrDefines.h"

namespace SCRSDK
{

class IDeviceCallback
{
public:
	virtual void OnConnected(DeviceConnectionVersioin version) = 0;

	virtual void OnDisconnected(CrInt32u error) = 0;

	virtual void OnPropertyChanged() = 0;

	virtual void OnPropertyChangedCodes(CrInt32u num, CrInt32u* codes) = 0;

	virtual void OnLvPropertyChanged() = 0;

	virtual void OnLvPropertyChangedCodes(CrInt32u num, CrInt32u* codes) = 0;

	virtual void OnCompleteDownload(CrChar *filename) = 0;

	virtual void OnNotifyContentsTransfer(CrInt32u notify, CrContentHandle handle, CrChar* filename = 0) = 0;

	virtual void OnWarning(CrInt32u warning) = 0;

	virtual void OnError(CrInt32u error) = 0;
};

}// namespace SCRSDK

#endif // IDEVICECALLBACK_H
