#include <iostream>
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

std::string GetSystemUUID() {
	HRESULT hres;

	// Initialize COM.
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) {
		std::cerr << "Failed to initialize COM library. Error code: " << hres << std::endl;
		return "";
	}

	// Set COM security levels.
	hres = CoInitializeSecurity(
		NULL, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE, NULL);

	if (FAILED(hres)) {
		std::cerr << "Failed to initialize security. Error code: " << hres << std::endl;
		CoUninitialize();
		return "";
	}

	// Obtain the initial locator to WMI.
	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator, 0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres)) {
		std::cerr << "Failed to create IWbemLocator object. Error code: " << hres << std::endl;
		CoUninitialize();
		return "";
	}

	// Connect to WMI.
	IWbemServices *pSvc = NULL;

	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // WMI namespace
		NULL,                    // User name
		NULL,                    // User password
		0,                       // Locale
		NULL,                    // Security flags
		0,                       // Authority
		0,                       // Context object
		&pSvc                    // IWbemServices proxy
	);

	if (FAILED(hres)) {
		std::cerr << "Could not connect. Error code: " << hres << std::endl;
		pLoc->Release();
		CoUninitialize();
		return "";
	}

	// Set security levels on the proxy.
	hres = CoSetProxyBlanket(
		pSvc,                     // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,        // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,         // RPC_C_AUTHZ_xxx
		NULL,                     // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,   // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                     // Client identity
		EOAC_NONE                 // Proxy capabilities 
	);

	if (FAILED(hres)) {
		std::cerr << "Could not set proxy blanket. Error code: " << hres << std::endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "";
	}

	// Query WMI for the system UUID.
	IEnumWbemClassObject* pEnumerator = NULL;

	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT UUID FROM Win32_ComputerSystemProduct"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres)) {
		std::cerr << "Query for system UUID failed. Error code: " << hres << std::endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "";
	}

	// Retrieve the data.
	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;

	std::string uuid = "";

	if (pEnumerator) {
		hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

		if (uReturn) {
			VARIANT vtProp;
			hres = pclsObj->Get(L"UUID", 0, &vtProp, 0, 0);
			if (SUCCEEDED(hres)) {
				uuid = _bstr_t(vtProp.bstrVal);
				VariantClear(&vtProp);
			}
			pclsObj->Release();
		}
		pEnumerator->Release();
	}

	// Cleanup
	pSvc->Release();
	pLoc->Release();
	CoUninitialize();

	return uuid;
}