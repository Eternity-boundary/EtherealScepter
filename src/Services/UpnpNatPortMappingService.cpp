//Created by: EternityBoundary on Jan 8, 2025
#include "pch.h"
#include "include/Services/UpnpNatPortMappingService.h"

#include <natupnp.h>     
#include <oaidl.h>       
#include <comdef.h>      

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

using namespace winrt;
using winrt::EtherealScepter::Models::PortMappingInfo;

namespace
{
    struct unique_bstr
    {
        BSTR v{ nullptr };
        unique_bstr() = default;
        explicit unique_bstr(wchar_t const* s) : v(::SysAllocString(s)) {}
        explicit unique_bstr(winrt::hstring const& s)
            : v(::SysAllocStringLen(s.data(), static_cast<UINT>(s.size()))) {
        }
        ~unique_bstr() { if (v) ::SysFreeString(v); }
        unique_bstr(unique_bstr const&) = delete;
        unique_bstr& operator=(unique_bstr const&) = delete;
        operator BSTR() const { return v; }
    };

    com_ptr<IStaticPortMappingCollection> GetCollection()
    {
        com_ptr<IUPnPNAT> nat;
        check_hresult(::CoCreateInstance(__uuidof(UPnPNAT), nullptr, CLSCTX_INPROC_SERVER,
            __uuidof(IUPnPNAT), nat.put_void()));

        com_ptr<IStaticPortMappingCollection> col;
        check_hresult(nat->get_StaticPortMappingCollection(col.put()));
        if (!col) throw hresult_error(E_FAIL, L"UPnP NAT not available (no port mapping collection).");
        return col;
    }
}

namespace EtherealScepter::Services
{
	std::vector<winrt::EtherealScepter::Models::PortMappingInfo> UpnpNatPortMappingService::Enumerate()
    {
        auto col = GetCollection();

        // 1) 取得 enumerator
        winrt::com_ptr<IUnknown> unkEnum;
        winrt::check_hresult(col->get__NewEnum(unkEnum.put()));

        winrt::com_ptr<IEnumVARIANT> e;
        winrt::check_hresult(unkEnum->QueryInterface(__uuidof(IEnumVARIANT), e.put_void()));

        auto bstr_to_h = [](BSTR b) -> winrt::hstring
            {
                return b ? winrt::hstring(b) : winrt::hstring{};
            };

        std::vector<winrt::EtherealScepter::Models::PortMappingInfo> out;

        // 2) 逐項列舉
        for (;;)
        {
            VARIANT item{};
            ::VariantInit(&item);

            ULONG fetched = 0;
            HRESULT hr = e->Next(1, &item, &fetched);
            if (hr != S_OK || fetched != 1)
            {
                ::VariantClear(&item);
                break; 
            }

            // VT_DISPATCH
            winrt::com_ptr<IStaticPortMapping> pm;

            if (item.vt == VT_DISPATCH && item.pdispVal)
            {
                winrt::check_hresult(item.pdispVal->QueryInterface(__uuidof(IStaticPortMapping), pm.put_void()));
            }
            else if (item.vt == VT_UNKNOWN && item.punkVal)
            {
                winrt::check_hresult(item.punkVal->QueryInterface(__uuidof(IStaticPortMapping), pm.put_void()));
            }
            else
            {
                ::VariantClear(&item);
                continue;
            }

            // 3) 讀取欄位
            BSTR desc = nullptr;
            BSTR proto = nullptr;
            BSTR client = nullptr;
            long extPort = 0;
            long intPort = 0;

            // 這些 getter 通常回 S_OK，即使字串可能是 nullptr
            winrt::check_hresult(pm->get_Description(&desc));
            winrt::check_hresult(pm->get_Protocol(&proto));
            winrt::check_hresult(pm->get_InternalClient(&client));
            winrt::check_hresult(pm->get_ExternalPort(&extPort));
            winrt::check_hresult(pm->get_InternalPort(&intPort));

            winrt::EtherealScepter::Models::PortMappingInfo m{};
            m.Description = bstr_to_h(desc);
            m.Protocol = bstr_to_h(proto);
            m.InternalClient = bstr_to_h(client);
            m.ExternalPort = static_cast<uint16_t>(extPort);
            m.InternalPort = static_cast<uint16_t>(intPort);

            if (desc)   ::SysFreeString(desc);
            if (proto)  ::SysFreeString(proto);
            if (client) ::SysFreeString(client);

            out.push_back(std::move(m));

            ::VariantClear(&item);
        }

        return out;
    }


    void UpnpNatPortMappingService::Add(PortMappingInfo const& m)
    {
        auto col = GetCollection();

        unique_bstr proto(m.Protocol);
        unique_bstr client(m.InternalClient);
        unique_bstr desc(m.Description);

        com_ptr<IStaticPortMapping> created;
        check_hresult(col->Add(
            static_cast<long>(m.ExternalPort),
            proto,
            static_cast<long>(m.InternalPort),
            client,
            VARIANT_TRUE,
            desc,
            created.put()
        ));
    }

    void UpnpNatPortMappingService::Remove(PortMappingInfo const& m)
    {
        auto col = GetCollection();
        unique_bstr proto(m.Protocol);

        // UPnP NAT 表通常以 (ExternalPort + Protocol) 作為 key
        check_hresult(col->Remove(static_cast<long>(m.ExternalPort), proto));
    }
}
