#ifndef _CRERROR_H_
#define _CRERROR_H_

namespace SCRSDK
{
	typedef CrInt32 CrError;
	enum
	{
		// 16bit	always 1
		// 9Å`15bit	category of error
		// 1Å`8bit	error detail in category
		CrError_None								= 0x0000,
		CrError_Generic								= 0x8000,
		CrError_Genric								= 0x8000, /* Do not use. Will be removed in the next release. */
		CrError_File								= 0x8100,
		CrError_Connect								= 0x8200,
		CrError_Memory								= 0x8300,
		CrError_Api									= 0x8400,
		CrError_Init								= 0x8500,
		CrError_Polling								= 0x8600,
		CrError_Adaptor								= 0x8700,
		CrError_Device								= 0x8800,
		CrError_Contents							= 0x8C00,

		CrError_Application							= 0xc000, /* Do not use. Will be removed in the next release. */

		// Uncategorized
		CrError_Generic_Unknown						= CrError_Generic,
		CrError_Generic_Notimpl,
		CrError_Generic_Abort,
		CrError_Generic_NotSupported,
		CrError_Generic_SeriousErrorNotSupported,
		CrError_Generic_InvalidHandle,
		CrError_Generic_InvalidParameter,

		/* [START] Do not use. Will be removed in the next release. */
		CrError_Genric_Unknown						= CrError_Generic,
		CrError_Genric_Notimpl,
		CrError_Genric_Abort,
		CrError_Genric_NotSupported,
		CrError_Genric_SeriousErrorNotSupported,
		CrError_Genric_InvalidHandle,
		CrError_Genric_InvalidParameter,
		/* [END] Do not use. Will be removed in the next release. */

		// File related
		CrError_File_Unknown						= CrError_File,	
		CrError_File_IllegalOperation,
		CrError_File_IllegalParameter,
		CrError_File_EOF,
		CrError_File_OutOfRange,
		CrError_File_NotFound,
		CrError_File_DirNotFound,
		CrError_File_AlreadyOpened,
		CrError_File_PermissionDenied,
		CrError_File_StorageFull,
		CrError_File_AlreadyExists,
		CrError_File_TooManyOpenedFiles,
		CrError_File_ReadOnly,
		CrError_File_CantOpen,
		CrError_File_CantClose,
		CrError_File_CantDelete,
		CrError_File_CantRead,
		CrError_File_CantWrite,
		CrError_File_CantCreateDir,
		CrError_File_OperationAbortedByUser,
		CrError_File_UnsupportedOperation,
		CrError_File_NotYetCompleted,
		CrError_File_Invalid,
		CrError_File_StorageNotExist,
		CrError_File_SharingViolation,
		CrError_File_Rotation,
		CrError_File_SameNameFull,

		// Connection related
		CrError_Connect_Unknown						= CrError_Connect,
		CrError_Connect_Connect,
		CrError_Connect_Reserved1,
		CrError_Connect_Release,
		CrError_Connect_GetProperty,
		CrError_Connect_SendCommand,
		CrError_Connect_HandlePlugin,
		CrError_Connect_Disconnected,
		CrError_Connect_TimeOut,
		CrError_Reconnect_TimeOut,
		CrError_Connect_FailRejected,
		CrError_Connect_FailBusy,
		CrError_Connect_FailUnspecified,
		CrError_Connect_Cancel,
		CrError_Reserved1,
		CrError_Reserved2,
		CrError_Connect_SessionAlreadyOpened,
		CrError_Connect_ContentsTransfer_NotSupported,

		//Memory related
		CrError_Memory_Unknown						= CrError_Memory,
		CrError_Memory_OutOfMemory,
		CrError_Memory_InvalidPointer,
		CrError_Memory_Insufficient,

		//API related
		CrError_Api_Unknown							= CrError_Api,
		CrError_Api_Insufficient,
		CrError_Api_InvalidCalled,

		CrError_Polling_Unknown						= CrError_Polling,
		CrError_Polling_InvalidVal_Intervals,

		//Adaptor related
		CrError_Adaptor_Unknown						= CrError_Adaptor,
		CrError_Adaptor_InvaildProperty,
		CrError_Adaptor_GetInfo,
		CrError_Adaptor_Create,
		CrError_Adaptor_SendCommand,
		CrError_Adaptor_HandlePlugin,
		CrError_Adaptor_CreateDevice,
		CrError_Adaptor_EnumDecvice,
		CrError_Adaptor_Reset,
		CrError_Adaptor_Read,
		CrError_Adaptor_Phase,
		CrError_Adaptor_DataToWiaItem,
		CrError_Adaptor_DeviceBusy,
		CrError_Adaptor_Escape,

		CrError_Device_Unknown						= CrError_Device,

		// [MTP] content transfer
		CrError_Contents_Unknown					= CrError_Contents,
		CrError_Contents_Reserved1,
		CrError_Contents_InvalidHandle,
		CrError_Contents_DateFolderList_NotRetrieved,
		CrError_Contents_ContentsList_NotRetrieved,
		CrError_Contents_Transfer_Unsuccess,
		CrError_Contents_Transfer_Cancel,
		CrError_Contents_RejectRequest,

		CrWarning_Unknown							= 0x00020000,
		CrWarning_Connect_Reconnected,
		CrWarning_Connect_Reconnecting,
		CrWarning_File_StorageFull,
		CrWarning_SetFileName_Failed,
		CrWarning_GetImage_Failed,
		CrWarning_FailedToSetCWB,
		CrWarning_NetworkErrorOccurred,
		CrWarning_NetworkErrorRecovered,
		CrWarning_Format_Failed,
		CrWarning_Format_Invalid,
		CrWarning_Format_Complete,
		CrWarning_Reserved1,
		CrWarning_Reserved2,
		CrWarning_Reserved3,
		CrWarning_Reserved4,
		CrWarning_Reserved5,
		CrWarning_Exposure_Started, /* Do not use. Will be removed in the next release. */
		CrNotify_Captured_Event = CrWarning_Exposure_Started,
		CrWarning_DateTime_Setting_Result_Invalid,
		CrWarning_DateTime_Setting_Result_OK,
		CrWarning_DateTime_Setting_Result_Parameter_Error,
		CrWarning_DateTime_Setting_Result_Exclusion_Error,
		CrWarning_DateTime_Setting_Result_System_Error,
		CrWarning_Frame_NotUpdated,
		CrNotify_All_Download_Complete,
		CrWarning_Connect_Already,
		CrWarning_Connect_OverLimitOfDevice,
		CrWarning_Format_Canceled,
		CrWarning_ZoomAndFocusPosition_Invalid,
		CrWarning_ZoomAndFocusPosition_DifferentLens,
		CrWarning_ZoomAndFocusPosition_InvalidLens,
		CrWarning_ContentsTransferMode_Invalid,
		CrWarning_ContentsTransferMode_DeviceBusy,
		CrWarning_ContentsTransferMode_StatusError,
		CrWarning_ContentsTransferMode_CanceledFromCamera,
		CrWarning_ContentsTransferCancel_Success,
		CrWarning_ContentsTransferCancel_Error,
		CrNotify_ContentsTransfer_Start,
		CrNotify_ContentsTransfer_Complete,
	};

	#define CR_SUCCEEDED(e)	(SCRSDK::CrError_None == (e))
	#define CR_FAILED(e)	(SCRSDK::CrError_None != (e))
}

#endif //_CRERROR_H_
