#include <memory>
#include <stdio.h>
#include <tchar.h>
#include <atlbase.h>
#include <wrl\client.h>
#include <comip.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

class UnknownTest : public IUnknown {
public:
    UnknownTest(LPCWSTR p) : prefix(p), ref(1) {
        LOG(L"%s constructed, ref %u", prefix, ref);
    }

    ~UnknownTest() {
        Release();
        LOG(L"%s destructed, ref %u", prefix, ref);
    }

    STDMETHOD_(ULONG, AddRef)() {
        ULONG newRef = InterlockedIncrement(&ref);
        LOG(L"%s AddRef: ref %u", prefix, newRef);
        return newRef;
    }

    STDMETHOD_(ULONG, Release)() {
        ULONG newRef = InterlockedDecrement(&ref);
        LOG(L"%s Release: ref %u", prefix, newRef);
        return newRef;
    }

    STDMETHOD(QueryInterface)(REFIID iid, void **out) {
        if (nullptr == out) {
            return E_POINTER;
        }

        if (iid == __uuidof(IUnknown)) {
            *out = static_cast<IUnknown *>(this);
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

private:
    LPCWSTR prefix;
    ULONG ref;
};

int _cdecl wmain()
{
    // Microsoft::WRL::ComPtr
    {
        UnknownTest u(L"Microsoft::WRL::ComPtr");
        Microsoft::WRL::ComPtr<IUnknown> comptr = &u;

        // comptr.AddRef(); compile error, Microsoft::WRL::ComPtr<T> has no such member

        // comptr->AddRef(); compile error, Microsoft::WRL::RemoveIUnknownBase<T>::AddRef is inaccessible

        // static_cast<IUnknown *>(comptr)->AddRef(); compile error, no suitable conversion
        // static_cast<IUnknown *>(comptr)->Release(); compile error, no suitable conversion

        comptr.Get()->AddRef();
        comptr.Get()->Release();
    }

    // ATL::CComPtr
    {
        UnknownTest u(L"ATL::CComPtr");
        ATL::CComPtr<IUnknown> ccomptr = &u;

        // ccomptr.AddRef(); compile error, ATL::CComPtr<T> has no such member

        // ccomptr->AddRef(); compile error, ATL::_NoAddRefReleaseOnCComPtr<T>::AddRef is inaccessible

        static_cast<IUnknown *>(ccomptr)->AddRef();
        static_cast<IUnknown *>(ccomptr)->Release();

        ccomptr.p->AddRef();
        ccomptr.p->Release();
    }

    // _com_ptr_t
    {
        UnknownTest u(L"_com_ptr_t");
        // or _COM_SMARTPTR_TYPEDEF(IUnknown, __uuidof(IUnknown)) and use IUnknownPtr
        _com_ptr_t< _com_IIID<IUnknown, &__uuidof(IUnknown)> > com_ptr_t(&u);

        com_ptr_t.AddRef();
        com_ptr_t.Release(); com_ptr_t.Attach(&u); // .Release() cleared the internal pointer

        com_ptr_t->AddRef();
        com_ptr_t->Release();

        static_cast<IUnknown *>(com_ptr_t)->AddRef();
        static_cast<IUnknown *>(com_ptr_t)->Release();

        com_ptr_t.GetInterfacePtr()->AddRef();
        com_ptr_t.GetInterfacePtr()->Release();
    }

    {
        struct Releaser
        {
            void operator()(IUnknown* p) { if (p) { p->Release(); } }
        };

        UnknownTest u(L"std::unique_ptr");

        std::unique_ptr<IUnknown, Releaser> unique_ptr(&u);
        u.AddRef(); // unique_ptr's reference

        // unique_ptr.AddRef(); // compile error, std::unique_ptr<T, U> has no such member

        unique_ptr->AddRef();
        unique_ptr->Release();

        // static_cast<IUnknown *>(unique_ptr)->AddRef(); compile error, no suitable conversion
        // static_cast<IUnknown *>(unique_ptr)->Release(); compile error, no suitable conversion

        unique_ptr.get()->AddRef();
        unique_ptr.get()->Release();
    }

    return 0;
}

