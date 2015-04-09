#include "stdafx.h"
#include "usbfuns.h"
#include "cfgmgr32.h"
int     g_debuglevel     = 0;
static  int     TotalHubs;
static  ULONG   TotalDevicesConnected;
static  BOOL    gDoConfigDesc   = TRUE;
wchar_t *gConnectionStatuses[] =
{
    L"NoDeviceConnected",
    L"DeviceConnected",
    L"DeviceFailedEnumeration",
    L"DeviceGeneralFailure",
    L"DeviceCausedOvercurrent",
    L"DeviceNotEnoughPower"
};

PUSB_ROOT_HUB_NAME GetRootHubName(HANDLE  HCD)
{
    BOOL                success;
    ULONG               nBytes;
    USB_ROOT_HUB_NAME   rootHubName;
    PUSB_ROOT_HUB_NAME  rootHubNameW;

    rootHubNameW = NULL;

    // Get the length of the name of the Root Hub attached to the
    // Host Controller
    //
    success = DeviceIoControl(HCD,
                              IOCTL_USB_GET_ROOT_HUB_NAME,
                              0,
                              0,
                              &rootHubName,
                              sizeof(rootHubName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        goto GetRootHubNameError;
    }

    // Allocate space to hold the Root Hub name
    //
    nBytes = rootHubName.ActualLength;

    rootHubNameW = (PUSB_ROOT_HUB_NAME)ALLOC(nBytes);

    if (rootHubNameW == NULL)
    {
        goto GetRootHubNameError;
    }

    // Get the name of the Root Hub attached to the Host Controller
    //
    success = DeviceIoControl(HCD,
                              IOCTL_USB_GET_ROOT_HUB_NAME,
                              NULL,
                              0,
                              rootHubNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success)
    {
        goto GetRootHubNameError;
    }

    // Convert the Root Hub name
    //
    //rootHubNameA = WideStrToMultiStr(rootHubNameW->RootHubName);

    // All done, free the uncoverted Root Hub name and return the
    // converted Root Hub name
    //
    //FREE(rootHubNameW);

    return rootHubNameW;


GetRootHubNameError:
    // There was an error, free anything that was allocated
    //
    if (rootHubNameW != NULL)
    {
        FREE(rootHubNameW);
        rootHubNameW = NULL;
    }

    return NULL;
}


//*****************************************************************************
//
// DriverNameToDeviceDesc()
//
// Returns the Device Description of the DevNode with the matching DriverName.
// Returns NULL if the matching DevNode is not found.
//
// The caller should copy the returned string buffer instead of just saving
// the pointer value.  XXXXX Dynamically allocate return buffer?
//
//*****************************************************************************

PTCHAR DriverNameToDeviceDesc (PTCHAR DriverName)
{
    DEVINST     devInst;
    DEVINST     devInstNext;
    CONFIGRET   cr;
    ULONG       walkDone = 0;
    ULONG       len;
    PTCHAR      deviceDesc;

    deviceDesc = (PTCHAR)ALLOC(DEVICE_DESC_LEN*sizeof(TCHAR));
    if( NULL == deviceDesc) return NULL;
    // Get Root DevNode
    //
    cr = CM_Locate_DevNode(&devInst,
                           NULL,
                           0);

    if (cr != CR_SUCCESS)
    {
        return NULL;
    }
    // Do a depth first search for the DevNode with a matching
    // DriverName value
    //
    while (!walkDone)
    {
        // Get the DriverName value
        //
        len = DEVICE_DESC_LEN;
        cr = CM_Get_DevNode_Registry_Property(devInst,
                                              CM_DRP_DRIVER,
                                              NULL,
                                              deviceDesc,
                                              &len,
                                              0);
        // If the DriverName value matches, return the DeviceDescription
        //
        if (cr == CR_SUCCESS && _tcscmp(DriverName, deviceDesc) == 0) {
            len = DEVICE_DESC_LEN;
            cr = CM_Get_DevNode_Registry_Property(devInst,
                                                  CM_DRP_DEVICEDESC,
                                                  NULL,
                                                  deviceDesc,
                                                  &len,
                                                  0);
            if (cr == CR_SUCCESS) {
                return deviceDesc;
            } else {
                return NULL;
            }
        }
        // This DevNode didn't match, go down a level to the first child.
        //
        cr = CM_Get_Child(&devInstNext,devInst,0);
        if (cr == CR_SUCCESS) {
            devInst = devInstNext;
            continue;
        }
        // Can't go down any further, go across to the next sibling.  If
        // there are no more siblings, go back up until there is a sibling.
        // If we can't go up any further, we're back at the root and we're
        // done.
        //
        for (;;)
        {
            cr = CM_Get_Sibling(&devInstNext,devInst,0);
            if (cr == CR_SUCCESS) {
                devInst = devInstNext;
                break;
            }
            cr = CM_Get_Parent(&devInstNext, devInst, 0);
            if (cr == CR_SUCCESS) {
                devInst = devInstNext;
            } else {
                walkDone = 1;
                break;
            }
        }
    }
    return NULL;
}



#if 0

BOOL DriverNameToDeviceDesc (TCHAR* DriverName,TCHAR *Desc,DWORD dwCnt)
{
    DEVINST     devInst;
    DEVINST     devInstNext;
    CONFIGRET   cr;
    ULONG       walkDone = 0;
    ULONG       len;

    // Get Root DevNode
    //
    cr = CM_Locate_DevNode(&devInst,
                           NULL,
                           0);

    if (cr != CR_SUCCESS)
    {
        return NULL;
    }

    // Do a depth first search for the DevNode with a matching
    // DriverName value
    //
    while (!walkDone)
    {
        // Get the DriverName value
        //
        len = sizeof(buf);
        cr = CM_Get_DevNode_Registry_Property(devInst,
                                              CM_DRP_DRIVER,
                                              NULL,
                                              buf,
                                              &len,
                                              0);

        // If the DriverName value matches, return the DeviceDescription
        //
        if (cr == CR_SUCCESS && strcmp(DriverName, buf) == 0)
        {
            len = sizeof(buf);
            cr = CM_Get_DevNode_Registry_Property(devInst,
                                                  CM_DRP_DEVICEDESC,
                                                  NULL,
                                                  buf,
                                                  &len,
                                                  0);

            if (cr == CR_SUCCESS)
            {
                return buf;
            }
            else
            {
                return NULL;
            }
        }

        // This DevNode didn't match, go down a level to the first child.
        //
        cr = CM_Get_Child(&devInstNext,
                          devInst,
                          0);

        if (cr == CR_SUCCESS)
        {
            devInst = devInstNext;
            continue;
        }

        // Can't go down any further, go across to the next sibling.  If
        // there are no more siblings, go back up until there is a sibling.
        // If we can't go up any further, we're back at the root and we're
        // done.
        //
        for (;;)
        {
            cr = CM_Get_Sibling(&devInstNext,
                                devInst,
                                0);

            if (cr == CR_SUCCESS)
            {
                devInst = devInstNext;
                break;
            }

            cr = CM_Get_Parent(&devInstNext,
                               devInst,
                               0);


            if (cr == CR_SUCCESS)
            {
                devInst = devInstNext;
            }
            else
            {
                walkDone = 1;
                break;
            }
        }
    }

    return NULL;
}
#endif
//*****************************************************************************
//
// GetDriverKeyName()
//
//*****************************************************************************

PUSB_NODE_CONNECTION_DRIVERKEY_NAME GetDriverKeyName ( HANDLE Hub,ULONG ConnectionIndex)
{
    BOOL                                success;
    ULONG                               nBytes;
    USB_NODE_CONNECTION_DRIVERKEY_NAME  driverKeyName;
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW  = NULL;

    driverKeyName.ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &nBytes,
                              NULL);

    if (!success) 
    {
        goto GetDriverKeyNameError;
    }
    // Allocate space to hold the driver key name
    //
    nBytes = driverKeyName.ActualLength;

    if (nBytes <= sizeof(driverKeyName))
    {
        goto GetDriverKeyNameError;
    }

    driverKeyNameW = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME)ALLOC(nBytes);

    if (driverKeyNameW == NULL)
    {
        goto GetDriverKeyNameError;
    }

    // Get the name of the driver key of the device attached to
    // the specified port.
    //
    driverKeyNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                              driverKeyNameW,
                              nBytes,
                              driverKeyNameW,
                              nBytes,
                              &nBytes,
                              NULL);
    if (!success)
    {
        goto GetDriverKeyNameError;
    }


    // All done, free the uncoverted driver key name and return the
    // converted driver key name
    //
    //driverKeyNameA = WideStrToMultiStr(driverKeyNameW->DriverKeyName);
    return driverKeyNameW;
GetDriverKeyNameError:
    // There was an error, free anything that was allocated
    //
    if (driverKeyNameW != NULL) {
        GlobalFree(driverKeyNameW);
        driverKeyNameW = NULL;
    }
    return NULL;
}
PUSB_HCD_DRIVERKEY_NAME GetHCDDriverKeyName(HANDLE  HCD)
{
    BOOL                    success;
    ULONG                   nBytes;
    USB_HCD_DRIVERKEY_NAME  driverKeyName;
    PUSB_HCD_DRIVERKEY_NAME driverKeyNameW  = NULL;



    // Get the length of the name of the driver key of the HCD
    //
    success = DeviceIoControl(HCD,
                              IOCTL_GET_HCD_DRIVERKEY_NAME,
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        goto GetHCDDriverKeyNameError;
    }

    // Allocate space to hold the driver key name
    //
    nBytes = driverKeyName.ActualLength;

    if (nBytes <= sizeof(driverKeyName))
    {
        goto GetHCDDriverKeyNameError;
    }

    driverKeyNameW =(PUSB_HCD_DRIVERKEY_NAME)ALLOC(nBytes);

    if (driverKeyNameW == NULL)
    {
        goto GetHCDDriverKeyNameError;
    }

    // Get the name of the driver key of the device attached to
    // the specified port.
    //
    success = DeviceIoControl(HCD,
                              IOCTL_GET_HCD_DRIVERKEY_NAME,
                              driverKeyNameW,
                              nBytes,
                              driverKeyNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success)
    {
        goto GetHCDDriverKeyNameError;
    }

    // Convert the driver key name
    //
    //driverKeyNameA = WideStrToMultiStr(driverKeyNameW->DriverKeyName);

    // All done, free the uncoverted driver key name and return the
    // converted driver key name
    //
    //FREE(driverKeyNameW);

    return driverKeyNameW;


GetHCDDriverKeyNameError:
    // There was an error, free anything that was allocated
    //
    if (driverKeyNameW != NULL)
    {
        FREE(driverKeyNameW);
        driverKeyNameW = NULL;
    }

    return NULL;
}
BOOL EnumerateHub (
    PTCHAR                              HubName,
    PUSB_NODE_CONNECTION_INFORMATION    ConnectionInfo,
    PUSB_DESCRIPTOR_REQUEST             ConfigDesc,
    PSTRING_DESCRIPTOR_NODE             StringDescs,
    PTCHAR                              DeviceDesc
)
{
    HANDLE          hHubDevice;
    PTCHAR          deviceName;
    BOOL            success;
    ULONG           nBytes;
    PUSBDEVICEINFO  info;
    PUSB_NODE_INFORMATION HubInfo;
    TCHAR           leafName[512]; // XXXXX how big does this have to be?

    // Initialize locals to not allocated state so the error cleanup routine
    // only tries to cleanup things that were successfully allocated.
    //
    info        = NULL;
    hHubDevice  = INVALID_HANDLE_VALUE;

    // Allocate some space for a USBDEVICEINFO structure to hold the
    // hub info, hub name, and connection info pointers.  GPTR zero
    // initializes the structure for us.
    //
    info = (PUSBDEVICEINFO) ALLOC(sizeof(USBDEVICEINFO));

    if (info == NULL)
    {
        goto EnumerateHubError;
    }

    // Keep copies of the Hub Name, Connection Info, and Configuration
    // Descriptor pointers
    //
    info->HubName = HubName;

    info->ConnectionInfo = ConnectionInfo;

    info->ConfigDesc = ConfigDesc;

    info->StringDescs = StringDescs;


    // Allocate some space for a USB_NODE_INFORMATION structure for this Hub,
    //
    HubInfo = (PUSB_NODE_INFORMATION)ALLOC(sizeof(USB_NODE_INFORMATION));

    if (HubInfo == NULL)
    {
        goto EnumerateHubError;
    }
    info->HubInfo = HubInfo;

    deviceName = (PTCHAR)ALLOC(_tcslen(HubName)*sizeof(TCHAR) + sizeof(TEXT("\\\\.\\")));

    if (deviceName == NULL)
    {
        goto EnumerateHubError;
    }

    // Create the full hub device name
    //
    wsprintf(deviceName,TEXT("\\\\.\\%s"),HubName);
    // Try to hub the open device
    //
    hHubDevice = CreateFile(deviceName,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

    // Done with temp buffer for full hub device name
    //
    FREE(deviceName);
    deviceName = NULL;
    if (INVALID_HANDLE_VALUE == hHubDevice ) {
        goto EnumerateHubError;
    }
    
    //
    // Now query USBHUB for the USB_NODE_INFORMATION structure for this hub.
    // This will tell us the number of downstream ports to enumerate, among
    // other things.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_NODE_INFORMATION,
                              info->HubInfo,
                              sizeof(USB_NODE_INFORMATION),
                              info->HubInfo,
                              sizeof(USB_NODE_INFORMATION),
                              &nBytes,
                              NULL);

    if (!success) {
        goto EnumerateHubError;
    }


    // Build the leaf name from the port number and the device description
    //
    if (ConnectionInfo)
    {
        wsprintf(leafName, TEXT("[Port%d] "), ConnectionInfo->ConnectionIndex);
        _tcscat_s(leafName,dim(leafName), gConnectionStatuses[ConnectionInfo->ConnectionStatus]);
        _tcscat_s(leafName,dim(leafName), TEXT(" :  "));
    }
    else
    {
        leafName[0] = TEXT('\0');
    }

    if (DeviceDesc)
    {
        _tcscat_s(leafName,dim(leafName),DeviceDesc);
    }
    else
    {
        _tcscat_s(leafName,dim(leafName),HubName);
        LDEBUGMSG(DEBUG_LOG, (TEXT("%s Ports:%d\r\n"),leafName,HubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts))
    }
    //DEBUG_P;
    
    //DEBUG_M;
    // Now add an item to the TreeView with the PUSBDEVICEINFO pointer info
    // as the LPARAM reference value containing everything we know about the
    // hub.
    //
    DEBUG_P;
    EnumerateHubPorts(hHubDevice,HubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts);
    DEBUG_M;
    CloseHandle(hHubDevice);
    hHubDevice = INVALID_HANDLE_VALUE;
    if (info != NULL) {
        FREE(info);
        info = NULL;
    }
    if(NULL != HubInfo) {
        FREE(HubInfo);
        HubInfo = NULL;
    }
    return TRUE;
EnumerateHubError:
    if (INVALID_HANDLE_VALUE != hHubDevice ) {
        CloseHandle(hHubDevice);
        hHubDevice = INVALID_HANDLE_VALUE;
    }
    if (info != NULL) {
        FREE(info);
        info = NULL;
    }
    return FALSE;
}

//*****************************************************************************
//
// EnumerateHubPorts()
//
// hTreeParent - Handle of the TreeView item under which the hub port should
// be added.
//
// hHubDevice - Handle of the hub device to enumerate.
//
// NumPorts - Number of ports on the hub.
//
//*****************************************************************************

BOOL EnumerateHubPorts (HANDLE hHubDevice,ULONG NumPorts )
{
    ULONG       index;
    BOOL        success;
    PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;
    PUSB_DESCRIPTOR_REQUEST             configDesc           = NULL;
    PSTRING_DESCRIPTOR_NODE             stringDescs          = NULL;
    PUSBDEVICEINFO                      info;
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyName;
    PTCHAR                              deviceDesc;
    TCHAR                               leafName[512]; // XXXXX how big does this have to be?
    //TCHAR                               extHubName[128];
    DWORD                               dwLen;
    for (index=1; index <= NumPorts; index++) {
        ULONG nBytes;

        // Allocate space to hold the connection info for this port.
        // For now, allocate it big enough to hold info for 30 pipes.
        //
        // Endpoint numbers are 0-15.  Endpoint number 0 is the standard
        // control endpoint which is not explicitly listed in the Configuration
        // Descriptor.  There can be an IN endpoint and an OUT endpoint at
        // endpoint numbers 1-15 so there can be a maximum of 30 endpoints
        // per device configuration.
        //
        // Should probably size this dynamically at some point.
        //
        nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) + sizeof(USB_PIPE_INFO) * 30;
        connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)GlobalAlloc(GPTR,nBytes);
        if (connectionInfo == NULL) {
            break;
        }
        //
        // Now query USBHUB for the USB_NODE_CONNECTION_INFORMATION structure
        // for this port.  This will tell us if a device is attached to this
        // port, among other things.
        //
        connectionInfo->ConnectionIndex = index;

        success = DeviceIoControl(hHubDevice,
                                  IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                                  connectionInfo,
                                  nBytes,
                                  connectionInfo,
                                  nBytes,
                                  &nBytes,
                                  NULL);
        if (!success) {
            GlobalFree(connectionInfo);
            connectionInfo = NULL;
            continue;
        }
        // Update the count of connected devices
        //
        if (connectionInfo->ConnectionStatus == DeviceConnected) {
            TotalDevicesConnected++;
        } 
        LDEBUGMSG(DEBUG_LOG, (TEXT("PORT:%d %s "),index , gConnectionStatuses[connectionInfo->ConnectionStatus]));

        if (connectionInfo->DeviceIsHub) {
            TotalHubs++;
        }
        // If there is a device connected, get the Device Description
        //
        if (connectionInfo->ConnectionStatus != NoDeviceConnected)
        {
            driverKeyName = GetDriverKeyName(hHubDevice,
                                             index);

            if (driverKeyName)
            {
                //wprintf(TEXT("    %d %s\r\n"),index,driverKeyName->DriverKeyName);
                deviceDesc = DriverNameToDeviceDesc(driverKeyName->DriverKeyName);
                if(deviceDesc) {
                    wprintf(TEXT(" %s "),deviceDesc);
                    FREE(driverKeyName);
                }
                FREE(driverKeyName);
            }
        }
        wprintf(TEXT("   VID=%04X PID=%04X\r\n"),connectionInfo->DeviceDescriptor.idProduct,connectionInfo->DeviceDescriptor.idVendor);
        // If there is a device connected to the port, try to retrieve the
        // Configuration Descriptor from the device.
        //
        if (gDoConfigDesc &&
            connectionInfo->ConnectionStatus == DeviceConnected)
        {
            configDesc = GetConfigDescriptor(hHubDevice,
                                             index,
                                             0);
        }
        else
        {
            configDesc = NULL;
        }

        if (configDesc != NULL &&
            AreThereStringDescriptors(&connectionInfo->DeviceDescriptor,
                                      (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc+1)))
        {
            DEBUG_P;
            stringDescs = GetAllStringDescriptors(
                              hHubDevice,
                              index,
                              &connectionInfo->DeviceDescriptor,
                              (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc+1));
            DEBUG_M;
        }
        else
        {
            stringDescs = NULL;
        }



        // If the device connected to the port is an external hub, get the
        // name of the external hub and recursively enumerate it.
        //
        if (connectionInfo->DeviceIsHub)
        {
            PUSB_NODE_CONNECTION_NAME extHubName;
            extHubName = GetExternalHubName(hHubDevice,
                                            index);

            if (extHubName != NULL)
            {
                DEBUG_P;
                EnumerateHub(extHubName->NodeName,
                             connectionInfo,
                             configDesc,
                             stringDescs,
                             deviceDesc);
                DEBUG_M;

                // On to the next port
                //
                continue;
            }
        }

        if (connectionInfo)
        {
            FREE(connectionInfo);
            connectionInfo = NULL;
        }

        if (configDesc)
        {
            FREE(configDesc);
            configDesc = NULL;
        }
        if (stringDescs != NULL)
        {
            PSTRING_DESCRIPTOR_NODE Next;
            do {
                Next = stringDescs->Next;
                FREE(stringDescs);
                stringDescs = Next;
            } while (stringDescs != NULL);
        }
    }
    return FALSE;
}



//*****************************************************************************
//
// GetStringDescriptors()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptor will be requested.
//
// DescriptorIndex - String Descriptor index.
//
// NumLanguageIDs -  Number of languages in which the string should be
// requested.
//
// LanguageIDs - Languages in which the string should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE
GetStringDescriptors (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    ULONG   NumLanguageIDs,
    USHORT  *LanguageIDs,
    PSTRING_DESCRIPTOR_NODE StringDescNodeTail
)
{
    ULONG i;

    for (i=0; i<NumLanguageIDs; i++)
    {
        StringDescNodeTail->Next = GetStringDescriptor(hHubDevice,
                                                       ConnectionIndex,
                                                       DescriptorIndex,
                                                       *LanguageIDs);

        if (StringDescNodeTail->Next)
        {
            StringDescNodeTail = StringDescNodeTail->Next;
        }

        LanguageIDs++;
    }

    return StringDescNodeTail;
}

//*****************************************************************************
//
// GetAllStringDescriptors()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptors will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptors will be requested.
//
// DeviceDesc - Device Descriptor for which String Descriptors should be
// requested.
//
// ConfigDesc - Configuration Descriptor (also containing Interface Descriptor)
// for which String Descriptors should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE
GetAllStringDescriptors (
    HANDLE                          hHubDevice,
    ULONG                           ConnectionIndex,
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
{
    PSTRING_DESCRIPTOR_NODE supportedLanguagesString;
    PSTRING_DESCRIPTOR_NODE stringDescNodeTail;
    ULONG                   numLanguageIDs;
    USHORT                  *languageIDs;

    PUCHAR                  descEnd;
    PUSB_COMMON_DESCRIPTOR  commonDesc;

    //
    // Get the array of supported Language IDs, which is returned
    // in String Descriptor 0
    //
    supportedLanguagesString = GetStringDescriptor(hHubDevice,
                                                   ConnectionIndex,
                                                   0,
                                                   0);

    if (supportedLanguagesString == NULL)
    {
        return NULL;
    }

    numLanguageIDs = (supportedLanguagesString->StringDescriptor->bLength - 2) / 2;
    //wprintf(TEXT("languageIDs: %d\r\n"),numLanguageIDs);
    languageIDs = (USHORT *)&supportedLanguagesString->StringDescriptor->bString[0];
    
    stringDescNodeTail = supportedLanguagesString;

    //
    // Get the Device Descriptor strings
    //

    if (DeviceDesc->iManufacturer)
    {
        stringDescNodeTail = GetStringDescriptors(hHubDevice,
                                                  ConnectionIndex,
                                                  DeviceDesc->iManufacturer,
                                                  numLanguageIDs,
                                                  languageIDs,
                                                  stringDescNodeTail);
        LDEBUGMSG(DEBUG_LOG,(TEXT("iManufacturer : %s\r\n"),stringDescNodeTail->StringDescriptor->bString));
    }

    if (DeviceDesc->iProduct)
    {
        stringDescNodeTail = GetStringDescriptors(hHubDevice,
                                                  ConnectionIndex,
                                                  DeviceDesc->iProduct,
                                                  numLanguageIDs,
                                                  languageIDs,
                                                  stringDescNodeTail);
        LDEBUGMSG(DEBUG_LOG,(TEXT("iProduct      : %s\r\n"),stringDescNodeTail->StringDescriptor->bString));
    }

    if (DeviceDesc->iSerialNumber)
    {
        stringDescNodeTail = GetStringDescriptors(hHubDevice,
                                                  ConnectionIndex,
                                                  DeviceDesc->iSerialNumber,
                                                  numLanguageIDs,
                                                  languageIDs,
                                                  stringDescNodeTail);
        LDEBUGMSG(DEBUG_LOG,(TEXT("iSerialNumber : %s\r\n"),stringDescNodeTail->StringDescriptor->bString));
    }


    //
    // Get the Configuration and Interface Descriptor strings
    //

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
           (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    //OOPS();
                    break;
                }
                if (((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
                {
                    stringDescNodeTail = GetStringDescriptors(
                                             hHubDevice,
                                             ConnectionIndex,
                                             ((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration,
                                             numLanguageIDs,
                                             languageIDs,
                                             stringDescNodeTail);
                                            //"iProduct      : %s\r\n"
                                            //
                                            //"iSerialNumber : %s\r\n"
                                            //"iInterface    : %s\r\n"
                    LDEBUGMSG(DEBUG_LOG,(TEXT("iConfiguration: %s\r\n"),stringDescNodeTail->StringDescriptor->bString));
                }
                commonDesc = PUSB_COMMON_DESCRIPTOR((PUCHAR)commonDesc + commonDesc->bLength);
                continue;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
                    commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
                {
                    //OOPS();
                    break;
                }
                if (((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
                {
                    stringDescNodeTail = GetStringDescriptors(
                                             hHubDevice,
                                             ConnectionIndex,
                                             ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface,
                                             numLanguageIDs,
                                             languageIDs,
                                             stringDescNodeTail);
                    LDEBUGMSG(DEBUG_LOG,(TEXT("iInterface    : %s\r\n"),stringDescNodeTail->StringDescriptor->bString));
                }
                commonDesc = PUSB_COMMON_DESCRIPTOR((PUCHAR)commonDesc + commonDesc->bLength);
                continue;

            default:
                commonDesc = PUSB_COMMON_DESCRIPTOR((PUCHAR)commonDesc + commonDesc->bLength);
                continue;
        }
        break;
    }

    return supportedLanguagesString;
}
//*****************************************************************************
//
// GetStringDescriptor()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptor will be requested.
//
// DescriptorIndex - String Descriptor index.
//
// LanguageID - Language in which the string should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE
GetStringDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    USHORT  LanguageID
)
{
    BOOL    success;
    ULONG   nBytes;
    ULONG   nBytesReturned;

    UCHAR   stringDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
                             MAXIMUM_USB_STRING_LENGTH];

    PUSB_DESCRIPTOR_REQUEST stringDescReq;
    PUSB_STRING_DESCRIPTOR  stringDesc;
    PSTRING_DESCRIPTOR_NODE stringDescNode;

    nBytes = sizeof(stringDescReqBuf);

    stringDescReq = (PUSB_DESCRIPTOR_REQUEST)stringDescReqBuf;
    stringDesc = (PUSB_STRING_DESCRIPTOR)(stringDescReq+1);

    // Zero fill the entire request structure
    //
    memset(stringDescReq, 0, nBytes);

    // Indicate the port from which the descriptor will be requested
    //
    stringDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    stringDescReq->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8)
                                        | DescriptorIndex;

    stringDescReq->SetupPacket.wIndex = LanguageID;

    stringDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              stringDescReq,
                              nBytes,
                              stringDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);

    //
    // Do some sanity checks on the return from the get descriptor request.
    //

    if (!success)
    {
        //OOPS();
        return NULL;
    }

    if (nBytesReturned < 2)
    {
        //OOPS();
        return NULL;
    }

    if (stringDesc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE)
    {
        //OOPS();
        return NULL;
    }

    if (stringDesc->bLength != nBytesReturned - sizeof(USB_DESCRIPTOR_REQUEST))
    {
        //OOPS();
        return NULL;
    }

    if (stringDesc->bLength % 2 != 0)
    {
        //OOPS();
        return NULL;
    }

    //
    // Looks good, allocate some (zero filled) space for the string descriptor
    // node and copy the string descriptor to it.
    //

    stringDescNode = (PSTRING_DESCRIPTOR_NODE)ALLOC(sizeof(STRING_DESCRIPTOR_NODE) +
                                                    stringDesc->bLength);

    if (stringDescNode == NULL)
    {
        //OOPS();
        return NULL;
    }

    stringDescNode->DescriptorIndex = DescriptorIndex;
    stringDescNode->LanguageID = LanguageID;

    memcpy(stringDescNode->StringDescriptor,
           stringDesc,
           stringDesc->bLength);

    return stringDescNode;
}


//*****************************************************************************
//
// GetConfigDescriptor()
//
// hHubDevice - Handle of the hub device containing the port from which the
// Configuration Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the Configuration Descriptor will be requested.
//
// DescriptorIndex - Configuration Descriptor index, zero based.
//
//*****************************************************************************

PUSB_DESCRIPTOR_REQUEST
GetConfigDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex
)
{
    BOOL    success;
    ULONG   nBytes;
    ULONG   nBytesReturned;

    UCHAR   configDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
                             sizeof(USB_CONFIGURATION_DESCRIPTOR)];

    PUSB_DESCRIPTOR_REQUEST         configDescReq;
    PUSB_CONFIGURATION_DESCRIPTOR   configDesc;


    // Request the Configuration Descriptor the first time using our
    // local buffer, which is just big enough for the Cofiguration
    // Descriptor itself.
    //
    nBytes          = sizeof(configDescReqBuf);
    configDescReq   = (PUSB_DESCRIPTOR_REQUEST)configDescReqBuf;
    configDesc      = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq+1);

    // Zero fill the entire request structure
    //
    memset(configDescReq, 0, nBytes);

    // Indicate the port from which the descriptor will be requested
    //
    configDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)
                                        | DescriptorIndex;

    configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              configDescReq,
                              nBytes,
                              configDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);

    if (!success) {
        return NULL;
    }

    if (nBytes != nBytesReturned) {
        return NULL;
    }

    if (configDesc->wTotalLength < sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
        return NULL;
    }

    // Now request the entire Configuration Descriptor using a dynamically
    // allocated buffer which is sized big enough to hold the entire descriptor
    //
    nBytes = sizeof(USB_DESCRIPTOR_REQUEST) + configDesc->wTotalLength;

    configDescReq = (PUSB_DESCRIPTOR_REQUEST)ALLOC(nBytes);

    if (configDescReq == NULL) {
        return NULL;
    }

    configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq+1);

    // Indicate the port from which the descriptor will be requested
    //
    configDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)
                                        | DescriptorIndex;

    configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              configDescReq,
                              nBytes,
                              configDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);

    if (!success) {
        FREE(configDescReq);
        return NULL;
    }

    if (nBytes != nBytesReturned) {
        FREE(configDescReq);
        return NULL;
    }

    if (configDesc->wTotalLength != (nBytes - sizeof(USB_DESCRIPTOR_REQUEST)))
    {
        FREE(configDescReq);
        return NULL;
    }
    return configDescReq;
}

//*****************************************************************************
//
// GetExternalHubName()
//
//*****************************************************************************
PUSB_NODE_CONNECTION_NAME GetExternalHubName (HANDLE Hub,ULONG ConnectionIndex)
{
    BOOL                        success;
    ULONG                       nBytes;
    USB_NODE_CONNECTION_NAME    extHubName;
    PUSB_NODE_CONNECTION_NAME   extHubNameW;


    extHubNameW = NULL;

    // Get the length of the name of the external hub attached to the
    // specified port.
    //
    extHubName.ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              &extHubName,
                              sizeof(extHubName),
                              &extHubName,
                              sizeof(extHubName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        //OOPS();
        goto GetExternalHubNameError;
    }

    // Allocate space to hold the external hub name
    //
    nBytes = extHubName.ActualLength;

    if (nBytes <= sizeof(extHubName))
    {
        //OOPS();
        goto GetExternalHubNameError;
    }

    extHubNameW = (PUSB_NODE_CONNECTION_NAME)ALLOC(nBytes);

    if (extHubNameW == NULL)
    {
        //OOPS();
        goto GetExternalHubNameError;
    }

    // Get the name of the external hub attached to the specified port
    //
    extHubNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              extHubNameW,
                              nBytes,
                              extHubNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success)
    {
        //OOPS();
        goto GetExternalHubNameError;
    }

    // Convert the External Hub name
    //
    //extHubNameA = WideStrToMultiStr(extHubNameW->NodeName);

    // All done, free the uncoverted external hub name and return the
    // converted external hub name
    //
    //FREE(extHubNameW);

    return extHubNameW;


GetExternalHubNameError:
    // There was an error, free anything that was allocated
    //
    if (extHubNameW != NULL)
    {
        FREE(extHubNameW);
        extHubNameW = NULL;
    }

    return NULL;
}

//*****************************************************************************
//
// AreThereStringDescriptors()
//
// DeviceDesc - Device Descriptor for which String Descriptors should be
// checked.
//
// ConfigDesc - Configuration Descriptor (also containing Interface Descriptor)
// for which String Descriptors should be checked.
//
//*****************************************************************************

BOOL
AreThereStringDescriptors (
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
{
    PUCHAR                  descEnd;
    PUSB_COMMON_DESCRIPTOR  commonDesc;

    //
    // Check Device Descriptor strings
    //

    if (DeviceDesc->iManufacturer ||DeviceDesc->iProduct      ||DeviceDesc->iSerialNumber) {
        return TRUE;
    }


    //
    // Check the Configuration and Interface Descriptor strings
    //

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&(PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
                    break;
                }
                if (((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration) {
                    return TRUE;
                }
                commonDesc  = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
                continue;
            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) && commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2)) {
                    break;
                }
                if (((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface) {
                    return TRUE;
                }
                commonDesc  = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
                continue;
            default:
                commonDesc  = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
                continue;
        }
        break;
    }

    return FALSE;
}

VOID EnumerateHostControllers( ULONG *DevicesConnected)
{
    int                         HCNum;
    TCHAR                       HCName[260];
    TCHAR                       *leafName;
    PUSB_HCD_DRIVERKEY_NAME     DrvKeyName;
    PUSB_ROOT_HUB_NAME          rootHubName;
    PTCHAR                      deviceDesc;
    DWORD                       dwLen;
    HANDLE                      hHCDev;
    for (HCNum = 0; HCNum < NUM_HCS_TO_CHECK; HCNum++) {
        wsprintf(HCName, TEXT("\\\\.\\HCD%d"), HCNum);
        hHCDev = CreateFile(HCName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
        if (INVALID_HANDLE_VALUE  !=  hHCDev) {
            leafName = HCName + sizeof("\\\\.\\") - sizeof("");
            wprintf(TEXT("\r\n%s\r\n"),leafName);
            DrvKeyName = GetHCDDriverKeyName(hHCDev);
            if(DrvKeyName){
                //wprintf(TEXT("DrvrKeyName:%s\r\n"),DrvKeyName->DriverKeyName);
                deviceDesc = DriverNameToDeviceDesc(DrvKeyName->DriverKeyName);
                if(deviceDesc) {
                    wprintf(TEXT("DrvrDescrip:%s\r\n"),deviceDesc);
                    FREE(DrvKeyName);
                    DrvKeyName = NULL;
                }
                FREE(DrvKeyName);
                DrvKeyName = NULL;
            }
            rootHubName =  GetRootHubName(hHCDev);
            if( rootHubName ) {
                //wprintf(TEXT("RootHubName:%s\r\n"),rootHubName->RootHubName);
#if 1
                DEBUG_P;
                EnumerateHub( rootHubName->RootHubName,
                NULL,      // ConnectionInfo
                NULL,      // ConfigDesc
                NULL,      // StringDescs
                TEXT("RootHub")  // DeviceDesc
                );
                DEBUG_M;
#endif
                FREE(rootHubName);
                rootHubName = NULL;
            }
            CloseHandle(hHCDev);
        }
    }
    return ;

}
