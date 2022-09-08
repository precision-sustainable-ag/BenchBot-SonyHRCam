#ifndef MESSAGEDEFINE_H
#define MESSAGEDEFINE_H

#include "CRSDK/CrTypes.h"
#include "CRSDK/CrError.h"
#include "Text.h"
#include <unordered_map>

namespace cli
{
    text get_message_desc(CrInt32u code);

    // Error code category
    const std::unordered_map<CrInt32u, text> map_ERR_CAT
    {
        {SCRSDK::CrError_Generic, TEXT("Generic")},
        {SCRSDK::CrError_File,    TEXT("File   ")},
        {SCRSDK::CrError_Connect, TEXT("Connect")},
        {SCRSDK::CrError_Memory,  TEXT("Memory ")},
        {SCRSDK::CrError_Api,     TEXT("Api    ")},
        {SCRSDK::CrError_Polling, TEXT("Polling")},
        {SCRSDK::CrError_Adaptor, TEXT("Adaptor")},
        {SCRSDK::CrError_Device,  TEXT("Device ")},
        {SCRSDK::CrError_Contents,TEXT("Content")},
    };

    // Error code detail
    const std::unordered_map<CrInt32u, text> map_ERR_DETAIL
    {
        {SCRSDK::CrError_Generic_Unknown, TEXT("Uncategorized errors")},
        {SCRSDK::CrError_Generic_Notimpl, TEXT("Not implemented")},
        {SCRSDK::CrError_Generic_Abort, TEXT("Processing was aborted")},
        {SCRSDK::CrError_Generic_NotSupported, TEXT("Not supported")},
        {SCRSDK::CrError_Generic_SeriousErrorNotSupported, TEXT("Serious error not supported")},
        {SCRSDK::CrError_Generic_InvalidHandle, TEXT("Not valid handle")},
        {SCRSDK::CrError_Generic_InvalidParameter, TEXT("Invalid parameter")},

        {SCRSDK::CrError_File_Unknown, TEXT("Unknown file errors")},
        {SCRSDK::CrError_File_IllegalOperation, TEXT("Illegal operation (e.g., loading without opening)")},
        {SCRSDK::CrError_File_IllegalParameter, TEXT("Illegal parameter")},
        {SCRSDK::CrError_File_EOF, TEXT("EOF")},
        {SCRSDK::CrError_File_OutOfRange, TEXT("Operation, such as seek, is out of range")},
        {SCRSDK::CrError_File_NotFound, TEXT("File not found")},
        {SCRSDK::CrError_File_DirNotFound, TEXT("Directory not found")},
        {SCRSDK::CrError_File_AlreadyOpened, TEXT("Already opened")},
        {SCRSDK::CrError_File_PermissionDenied, TEXT("No access permission")},
        {SCRSDK::CrError_File_StorageFull, TEXT("Host storage is full")},
        {SCRSDK::CrError_File_AlreadyExists, TEXT("Already exists")},
        {SCRSDK::CrError_File_TooManyOpenedFiles, TEXT("Too many open files")},
        {SCRSDK::CrError_File_ReadOnly, TEXT("Read-Only file")},
        {SCRSDK::CrError_File_CantOpen, TEXT("Cannot open")},
        {SCRSDK::CrError_File_CantClose, TEXT("Cannot close")},
        {SCRSDK::CrError_File_CantDelete, TEXT("Cannot delete")},
        {SCRSDK::CrError_File_CantRead, TEXT("Cannot read")},
        {SCRSDK::CrError_File_CantWrite, TEXT("Cannot write")},
        {SCRSDK::CrError_File_CantCreateDir, TEXT("Cannot create a directory")},
        {SCRSDK::CrError_File_OperationAbortedByUser, TEXT("Processing was aborted by user")},
        {SCRSDK::CrError_File_UnsupportedOperation, TEXT("API not supported for the platform was called")},
        {SCRSDK::CrError_File_NotYetCompleted, TEXT("Operation is not completed")},
        {SCRSDK::CrError_File_Invalid, TEXT("The file is no longer valid because the volume for the file was altered")},
        {SCRSDK::CrError_File_StorageNotExist, TEXT("The specified network resource or device is no longer available")},
        {SCRSDK::CrError_File_SharingViolation, TEXT("Sharing violation")},
        {SCRSDK::CrError_File_Rotation, TEXT("Invalid file orientation")},
        {SCRSDK::CrError_File_SameNameFull, TEXT("Too many same-name files")},

        {SCRSDK::CrError_Connect_Unknown, TEXT("Other errors classified as connection except below")},
        {SCRSDK::CrError_Connect_Connect, TEXT("A connection request failed through the USB")},
        {SCRSDK::CrError_Connect_Release, TEXT("Release failed")},
        {SCRSDK::CrError_Connect_GetProperty, TEXT("Getting property failed")},
        {SCRSDK::CrError_Connect_SendCommand, TEXT("Sending command failed")},
        {SCRSDK::CrError_Connect_HandlePlugin, TEXT("Illegal handle plug-in")},
        {SCRSDK::CrError_Connect_Disconnected, TEXT("A connection disconnected")},
        {SCRSDK::CrError_Connect_TimeOut, TEXT("A connection operation timed out")},
        {SCRSDK::CrError_Reconnect_TimeOut, TEXT("Reconnection operations timed out")},
        {SCRSDK::CrError_Connect_FailRejected, TEXT("Connection rejected and failed")},
        {SCRSDK::CrError_Connect_FailBusy, TEXT("Connection failed due to processing in progress")},
        {SCRSDK::CrError_Connect_FailUnspecified, TEXT("Unspecified connection failure")},
        {SCRSDK::CrError_Connect_Cancel, TEXT("Connection canceled")},
        {SCRSDK::CrError_Connect_SessionAlreadyOpened, TEXT("A connection failed because camera was not ready")},
        {SCRSDK::CrError_Connect_ContentsTransfer_NotSupported, TEXT("This camera does not support content transfer")},

        {SCRSDK::CrError_Memory_Unknown, TEXT("Unknown memory error")},
        {SCRSDK::CrError_Memory_OutOfMemory, TEXT("Cannot allocate memory")},
        {SCRSDK::CrError_Memory_InvalidPointer, TEXT("Invalid pointer")},
        {SCRSDK::CrError_Memory_Insufficient, TEXT("Allocate memory insufficient")},

        {SCRSDK::CrError_Api_Unknown, TEXT("Unknown API error")},
        {SCRSDK::CrError_Api_Insufficient, TEXT("Incorrect parameter")},
        {SCRSDK::CrError_Api_InvalidCalled, TEXT("Invalid API call")},

        {SCRSDK::CrError_Polling_Unknown, TEXT("Unknown polling error")},
        {SCRSDK::CrError_Polling_InvalidVal_Intervals, TEXT("Invalid polling interval setting value")},

        {SCRSDK::CrError_Adaptor_Unknown, TEXT("Unknown adapter error")},
        {SCRSDK::CrError_Adaptor_InvaildProperty, TEXT("A property that doesn't exist was used")},
        {SCRSDK::CrError_Adaptor_GetInfo, TEXT("Getting information failed")},
        {SCRSDK::CrError_Adaptor_Create, TEXT("Creation failed")},
        {SCRSDK::CrError_Adaptor_SendCommand, TEXT("Sending command failed")},
        {SCRSDK::CrError_Adaptor_HandlePlugin, TEXT("Illegal handle plug-in")},
        {SCRSDK::CrError_Adaptor_CreateDevice, TEXT("Device creation failed")},
        {SCRSDK::CrError_Adaptor_EnumDecvice, TEXT("Enumeration of device information failed")},
        {SCRSDK::CrError_Adaptor_Reset, TEXT("Reset failed")},
        {SCRSDK::CrError_Adaptor_Read, TEXT("Read failed")},
        {SCRSDK::CrError_Adaptor_Phase, TEXT("Parse failed")},
        {SCRSDK::CrError_Adaptor_DataToWiaItem, TEXT("Failed to set data as WIA item")},
        {SCRSDK::CrError_Adaptor_DeviceBusy, TEXT("The setting side is busy")},
        {SCRSDK::CrError_Adaptor_Escape, TEXT("Escape failed")},

        {SCRSDK::CrError_Contents_Unknown, TEXT("Unknown content transfer error")},
        {SCRSDK::CrError_Contents_InvalidHandle, TEXT("Not valid content handle")},
        {SCRSDK::CrError_Contents_DateFolderList_NotRetrieved, TEXT("Date folder list not acquired")},
        {SCRSDK::CrError_Contents_ContentsList_NotRetrieved, TEXT("Content handle list not acquired")},
        {SCRSDK::CrError_Contents_Transfer_Unsuccess, TEXT("Content transfer failed")},
        {SCRSDK::CrError_Contents_Transfer_Cancel,TEXT("Not transferred due to successful content transfer cancel")},
        {SCRSDK::CrError_Contents_RejectRequest, TEXT("Rejected request")},

        {SCRSDK::CrError_Device_Unknown, TEXT("Unknown device error")},
    }; 

} // namespace cli

#endif // !MESSAGEDEFINE_H
