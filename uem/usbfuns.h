#ifndef _USBFUNS_H_
#define _USBFUNS_H_
#define DEBUG_LOG       TRUE
#define DEBUG_P         g_debuglevel+=4
#define DEBUG_M         g_debuglevel-=4
#define LDEBUGMSG(cond,p) if (cond) {  \
    for(int i = 0 ; i < g_debuglevel; i++ ) {    \
        wprintf(TEXT(" "));               \
    }                                     \
    wprintf p;                            \
}
#define NUM_HCS_TO_CHECK    10
#define DEVICE_DESC_LEN     512
#if DBG
#define ALLOC(dwBytes) MyAlloc(__FILE__, __LINE__, (dwBytes))
#define REALLOC(hMem, dwBytes) MyReAlloc((hMem), (dwBytes))
#define FREE(hMem)  MyFree((hMem))
#define CHECKFORLEAKS() MyCheckForLeaks()
#else
#define ALLOC(dwBytes) GlobalAlloc(GPTR,(dwBytes))
#define REALLOC(hMem, dwBytes) GlobalReAlloc((hMem), (dwBytes), (GMEM_MOVEABLE|GMEM_ZEROINIT))
#define FREE(hMem)  GlobalFree((hMem))
#define CHECKFORLEAKS()
#endif

#pragma pack(push, 1)
// Common Class Interface Descriptor
//
typedef struct _USB_INTERFACE_DESCRIPTOR2 {
    UCHAR  bLength;             // offset 0, size 1
    UCHAR  bDescriptorType;     // offset 1, size 1
    UCHAR  bInterfaceNumber;    // offset 2, size 1
    UCHAR  bAlternateSetting;   // offset 3, size 1
    UCHAR  bNumEndpoints;       // offset 4, size 1
    UCHAR  bInterfaceClass;     // offset 5, size 1
    UCHAR  bInterfaceSubClass;  // offset 6, size 1
    UCHAR  bInterfaceProtocol;  // offset 7, size 1
    UCHAR  iInterface;          // offset 8, size 1
    USHORT wNumClasses;         // offset 9, size 2
} USB_INTERFACE_DESCRIPTOR2, *PUSB_INTERFACE_DESCRIPTOR2;
#pragma pack(pop)
//
// Structure used to build a linked list of String Descriptors
// retrieved from a device.
//
#pragma warning( disable : 4200 )
typedef struct _STRING_DESCRIPTOR_NODE
{
    struct _STRING_DESCRIPTOR_NODE  *Next;
    UCHAR                           DescriptorIndex;
    USHORT                          LanguageID;
    USB_STRING_DESCRIPTOR           StringDescriptor[0];
} STRING_DESCRIPTOR_NODE, *PSTRING_DESCRIPTOR_NODE;
#pragma warning( default : 4200 )

//
// Structures assocated with TreeView items through the lParam.  When an item
// is selected, the lParam is retrieved and the structure it which it points
// is used to display information in the edit control.
//
typedef struct
{
    PUSB_NODE_INFORMATION               HubInfo;        // NULL if not a HUB
    TCHAR                               *HubName;       // NULL if not a HUB
    PUSB_NODE_CONNECTION_INFORMATION    ConnectionInfo; // NULL if root HUB
    PUSB_DESCRIPTOR_REQUEST             ConfigDesc;     // NULL if root HUB
    PSTRING_DESCRIPTOR_NODE             StringDescs;
} USBDEVICEINFO, *PUSBDEVICEINFO;

VOID EnumerateHostControllers   (ULONG *DevicesConnected);
BOOL GetRootHubName             (HANDLE  HCD    ,TCHAR * HubName,DWORD  * dwCnt);
BOOL GetHCDDriverKeyName        (HANDLE  HCD    ,TCHAR *KeyName        ,DWORD  * dwCnt);
BOOL GetDriverKeyName           (HANDLE  Hub    ,ULONG ConnectionIndex ,TCHAR * DriverKeyName  ,DWORD  * dwCnt);

//*****************************************************************************
//
// EnumerateHub()
//
// hTreeParent - Handle of the TreeView item under which this hub should be
// added.
//
// HubName - Name of this hub.  This pointer is kept so the caller can neither
// free nor reuse this memory.
//
// ConnectionInfo - NULL if this is a root hub, else this is the connection
// info for an external hub.  This pointer is kept so the caller can neither
// free nor reuse this memory.
//
// ConfigDesc - NULL if this is a root hub, else this is the Configuration
// Descriptor for an external hub.  This pointer is kept so the caller can
// neither free nor reuse this memory.
//
//*****************************************************************************

BOOL EnumerateHub (
    TCHAR                               *HubName,
    PUSB_NODE_CONNECTION_INFORMATION    ConnectionInfo,
    PUSB_DESCRIPTOR_REQUEST             ConfigDesc,
    PSTRING_DESCRIPTOR_NODE             StringDescs,
    PTCHAR                              DeviceDesc
);
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
BOOL                        EnumerateHubPorts           (HANDLE hHubDevice,ULONG NumPorts );
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
);
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
);
PUSB_DESCRIPTOR_REQUEST     GetConfigDescriptor         (HANDLE  hHubDevice, ULONG   ConnectionIndex, UCHAR   DescriptorIndex);
//*****************************************************************************
//
// GetExternalHubName()
//
//*****************************************************************************
PUSB_NODE_CONNECTION_NAME GetExternalHubName (HANDLE Hub,ULONG ConnectionIndex);
BOOL                        AreThereStringDescriptors   (PUSB_DEVICE_DESCRIPTOR DeviceDesc, PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc);


#endif
