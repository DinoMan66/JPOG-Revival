// Toshi::TRenderAdapter / Mode / Device (TRenderInterface.dll) - abstract display adapter database.
// Everything except GetModeList is defined in the class bodies (the original emits those
// members in its inline/COMDAT region, 0x7b10-0x7ce0). GetModeList is TRenderAdapter.cpp's
// only function (0x9b90), so other files call it rather than inline it.
//   TRenderAdapter (0x20): vtable, TNodeList<TRenderAdapter>::TNode (+4), TNodeList<Mode> (+0x10)
//   Mode (0x10), Device (0x10): vtable + their list node
// Pure virtual slots follow OpenToshi's TRenderAdapter.h (names and parameter types are a best
// guess: no JPOG module exports them; only the slot numbers are confirmed - 9, 8 and 7).
#ifndef TRENDERADAPTER_H
#define TRENDERADAPTER_H
#include "TRenderAPI.h"
#include "TNodeList.h"
#include "TCString.h"

namespace Toshi {

class TRENDER_API TRenderAdapter : public TNodeList<TRenderAdapter>::TNode
{
public:
    // a GUID (DumpStats prints Data1, Data2, Data3 and the address of Data4)
    struct DeviceIdentifier
    {
        unsigned int   Data1;
        unsigned short Data2;
        unsigned short Data3;
        unsigned char  Data4[8];
    };
    class TRENDER_API Mode : public TNodeList<Mode>::TNode
    {
    public:
        class TRENDER_API Device : public TNodeList<Device>::TNode
        {
        public:
            enum DEPTHSTENCILFORMAT { };

            Device() {}
            virtual ~Device() {}                                            // slot 0

            virtual Mode* GetMode() const = 0;                              // slot 1
            virtual unsigned int GetDeviceIndex() const = 0;                // slot 2
            virtual const char* GetTypeString() const = 0;                  // slot 3
            virtual bool IsSoftware() const = 0;                            // slot 4
            virtual bool CanRenderWindowed() const = 0;                     // slot 5
            virtual bool SupportsHardwareTransfomations() const = 0;        // slot 6
            virtual bool IsDepthStencilFormatSupported(unsigned int index) const = 0; // slot 7
        };

        Mode() {}
        virtual ~Mode() {}                                                  // slot 0

        virtual TRenderAdapter* GetAdapter() const = 0;                     // slot 1
        virtual unsigned int GetModeIndex() const = 0;                      // slot 2
        virtual unsigned int GetWidth() const = 0;                          // slot 3
        virtual unsigned int GetHeight() const = 0;                         // slot 4
        virtual bool Is32Bit() const = 0;                                   // slot 5
        virtual bool Is16Bit() const = 0;                                   // slot 6
        virtual unsigned int GetRefreshRate() const = 0;                    // slot 7
        virtual Device* GetDevice(unsigned int index) = 0;                  // slot 8
    };

    TRenderAdapter() {}
#ifdef TNODELIST_NO_DTOR
    virtual ~TRenderAdapter() { m_oModes.DeleteAll(); }
#else
    virtual ~TRenderAdapter() {}                                            // slot 0 (deletes the modes)
#endif

    virtual unsigned int GetAdapterIndex() const = 0;                       // slot 1
    virtual const TCString& GetDriver() const = 0;                          // slot 2
    virtual const TCString& GetDriverDescription() const = 0;               // slot 3
    virtual unsigned int GetProductID() const = 0;                          // slot 4
    virtual unsigned int GetVersion() const = 0;                            // slot 5
    virtual unsigned int GetSubVersion() const = 0;                         // slot 6
    virtual unsigned int GetBuild() const = 0;                              // slot 7
    virtual const DeviceIdentifier* GetDeviceIdentifier() const = 0;                    // slot 8
    virtual int GetNumSupportedDevices() const = 0;                         // slot 9 (signed: jl)

    TNodeList<Mode>* GetModeList();                                         // TRenderAdapter.cpp (0x9b90)

    TNodeList<Mode> m_oModes;                                               // +0x10
};

} // namespace Toshi
#endif
