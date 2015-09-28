// prefs.h

class CPrefs {
public:
	// hr is set to S_OK if everything works
	// hr is set to S_FALSE if this is a usage statement
	// hr is set to an error code if something bad happens
	CPrefs(int argc, LPCTSTR argv[], HRESULT &hr);
	~CPrefs();

	IMMDevice *pMMDevice;
	
	bool bCopyOK;
	bool bDigitalOutputDisable;
	bool bTestCertificateEnable;
	DWORD dwDrmLevel;
};
