//-----------------------------------------------------------------------
// Sentech Test Class v1.1.0. (with Sentech Common SDK v1.0.1)
//
// Copyright (c) 2017, FAINSTEC,.CO.LTD. 
//
// web : www.fainstec.com
//
// e-mail : tech@fainstec.com
//
// �� �ش� �ڵ�� ������� ���ظ� ���� ���� ���� �׽�Ʈ �ڵ��̹Ƿ�
//   �ݵ�� ����� ������ �׽�Ʈ�� ���� �����Ͽ� ����ϱ� ���� �帳�ϴ�.
//-----------------------------------------------------------------------
#pragma once

#include <atlstr.h>
#include <cstringt.h>

// Include files for using StApi.
#include <StApi_TL.h>
#include <StApi_IP.h>
#include <StApi_GUI.h>

namespace SENTECH_SDK {

//Namespace for using StApi.
using namespace StApi;

//Namespace for using vector.
using namespace std;

#define ST_EXCEPTION const GenICam::GenericException

typedef	enum { eSentech=0, eEuresys, eKaya } EStSystem ;
typedef	enum { eContinuous=0, eSingleFrame, eMultiFrame } EAcqMode ;
typedef	enum { eSoftware=0, eHardware } ETrgSrc;
typedef	enum { eOff=0, eReadOut, ePreviousFrame } ETrgOvl;
typedef	enum { eTimed=0, eTriggerWidth } EExpMode;
typedef	enum { eDefault=0, eUserSet1 } EUserSet;
typedef	enum { eStRaw=0, eBMP, eJPG, eTIFF, ePNG, eCSV } EFormat;
typedef	enum { eAWOff=0, ePreset1, ePreset2, ePreset3, eAWContinuous, eOnce } EAWB;
typedef	enum { eUserID=0, eSeiralNumber, eIP, eMAC } EConnectType;

typedef struct {
	int nAccSts;
	CString strName;
	CString strSN;
	CString strVendor;
	CString strType;
	CString strMacAddr;
	CString strUserID;
	CString strIPAddr;
	CString strSubMask;
} StDeviceInfo;

typedef struct {
	CString strName;
	CString strType;
	CString strMacAddr;
	CString strIPAddr;
	CString strSubMask;

	std::vector<StDeviceInfo> vDvInfo;
} StInterfaceInfo;

typedef struct {
	CString strName;
	CString strVendor;
	CString strType;

	std::vector<StInterfaceInfo> vIfInfo;
} StSystemInfo;

class CStCamera
{
public:
	CStCamera(void);
	~CStCamera(void);

private :
	// For search interface.
	static std::vector<StSystemInfo> m_vSystemInfo;
	CStApiAutoInit m_ApiAutoInit;

public :
	// Search System.
	static bool OnSearchSystem();
	// Get System Information.
	static bool GetSystemCount(int &nSyCnt);
	static bool GetSystemName(int nIdxSy, CString &strValue);
	static bool GetSystemVendor(int nIdxSy, CString &strValue);
	static bool GetSystemType(int nIdxSy, CString &strValue);
	// Get Interface Information.
	static bool GetInterfaceCount(int nIdxSy, int &nIfCnt);
	static bool GetInterfaceName(int nIdxSy, int nIdxIf, CString &strValue);
	static bool GetInterfaceType(int nIdxSy, int nIdxIf, CString &strValue);
	static bool GetInterfaceMACAddr(int nIdxSy, int nIdxIf, CString &strValue);
	static bool GetInterfaceIPAddr(int nIdxSy, int nIdxIf, CString &strValue);
	static bool GetInterfaceSubMask(int nIdxSy, int nIdxIf, CString &strValue);
	// Get Device Information.
	static bool GetDeviceCount(int nIdxSy, int nIdxIf, int &nDvCnt);
	static bool GetDeviceName(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue);
	static bool GetDeviceSN(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue);
	static bool GetDeviceVendor(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue);
	static bool GetDeviceType(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue);
	static bool GetDeviceAccessStatus(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue);
	static bool GetDeviceMACAddr(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue);
	static bool GetDeviceIPAddr(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue);
	static bool GetDeviceSubMask(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue);
	static void SetDeviceIPAddr(int nIdxIf, int nIdxDv, CString strValue);
	static bool SetDeviceSubMask(int nIdxIf, int nIdxDv, CString strValue);
	static bool GetDeviceUserID(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue);

public :
	// Command Functions
	//******************************************************************************************************************
	/// \brief					Device Select Dialog �� �̿��� Device ���� �Լ�.
	/// \param void				
	void OnConnect();
	//******************************************************************************************************************
	/// \brief					Device ������ �̿��� Device ���� �Լ�.
	/// \param [in] strValue	Device Information.
	/// \param [in] eType		UserID, Serial Number, IP, MAC
	/// \param void				
	void OnConnect(CString strValue, EConnectType eType);
	//******************************************************************************************************************
	/// \brief					Parameter Window�� ȣ���ϴ� �Լ�.
	/// \param void				
	void OnShowParameterWnd();
	//******************************************************************************************************************
	/// \brief					AVI ��ȭ ���� �Լ�.
	/// \param [in] strPath		��ȭ ���� ���� ���.
	/// \param void				
	void OnStartRecording(CString strPath);
	//******************************************************************************************************************
	/// \brief					AVI ��ȭ ���� �Լ�.
	/// \param void				
	void OnStopRecording();
	//******************************************************************************************************************
	/// \brief					Device ���� �Լ�.
	/// \param void				
	void OnDisconnect();
	//******************************************************************************************************************
	/// \brief					���� ��� ���� �Լ�.
	/// \param [in] nFrame		���� ��� ������ ��. �⺻�� ���� ȹ��.
	/// \param bool				��� ��ȯ.
	void OnStartAcquisition(uint64_t nFrame = GENTL_INFINITE);
	//******************************************************************************************************************
	/// \brief					���� ��� ���� �Լ�.
	/// \param bool				��� ��ȯ.
	void OnStopAcquisition();
	//******************************************************************************************************************
	/// \brief					Software Trigger Event �߻� �Լ�.
	/// \param bool				��� ��ȯ.
	void OnTriggerEvent();
	//******************************************************************************************************************
	/// \brief					User Set ���� �Լ�.
	/// \param bool				��� ��ȯ.
	void OnUserSetSave();
	//******************************************************************************************************************
	/// \brief					User Set �ҷ����� �Լ�.
	/// \param bool				��� ��ȯ.
	void OnUserSetLoad();
	//******************************************************************************************************************
	/// \brief					�̹��� ���� �Լ�.
	/// \param [in] strPath		���� ��� �Է�.
	/// \param void
	void OnSaveImage(CString strPath, EFormat eFmt = eBMP);
	//******************************************************************************************************************
	/// \brief					Auto White Balance ���� �Լ�.
	/// \param [in] Type		Off / Preset 1~3 / Continuous / Once ���� ����.
	/// \param bool				��� ��ȯ.
	void OnAutoWhiteBalance(EAWB eMode);
	//******************************************************************************************************************
	/// \brief					AWB Once�� ���� �Ķ���͸� ������ Preset �Ķ���Ϳ� �����ϴ� �Լ�.
	/// \param [in] nPresetNum	Ư�� Preset ��ȣ �Է�.
	/// \param bool				��� ��ȯ.
	void OnSaveAWBValueOnceToPreset(int nPreset);
public :
	// Get Functions.
	//******************************************************************************************************************
	/// \brief					Device ���� ���� Ȯ�� �Լ�.
	/// \param bool				��� ��ȯ.
	bool IsConnected();
	//******************************************************************************************************************
	/// \brief					���� ��� ���� Ȯ�� �Լ�.
	/// \param bool				��� ��ȯ.
	bool IsActived();
	//******************************************************************************************************************
	/// \brief					Device User ID Ȯ�� �Լ�.
	/// \param [out] strValue	Device User ID Ȯ��.
	/// \param void
	void GetUserID(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Device Model Name Ȯ�� �Լ�.
	/// \param [out] strValue	Device Model Name Ȯ��.
	/// \param void
	void GetModelName(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Device Serial Number Ȯ�� �Լ�.
	/// \param [out] strValue	Device Serial Number Ȯ��.
	/// \param void
	void GetSerialNumber(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Device Current IP Address Ȯ�� �Լ�.
	/// \param [out] strValue	IP Address Ȯ��.
	/// \param void
	void GetIPAddress(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Device Current Subnet Mask Ȯ�� �Լ�.
	/// \param [out] strValue	Subnet Mask Ȯ��.
	/// \param void
	void GetSubnetMask(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Width Ȯ�� �Լ�.
	/// \param [out] nValue		Width Ȯ��.
	/// \param void
	void GetWidth(int &nValue);
	//******************************************************************************************************************
	/// \brief					Height Ȯ�� �Լ�.
	/// \param [out] nValue		Height Ȯ��.
	/// \param void				
	void GetHeight(int &nValue);
	//******************************************************************************************************************
	/// \brief					Bit per Pixel Ȯ�� �Լ�.
	/// \param [out] nValue		Bit per Pixel Ȯ��.
	/// \param void				
	void GetBpp(int &nValue);
	//******************************************************************************************************************
	/// \brief					Bit per Pixel Ȯ�� �Լ�.
	/// \param [out] strValue	Bit per Pixel Ȯ��.
	/// \param void				
	void GetBpp(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Grab Tact Time Ȯ�� �Լ�.
	/// \param double			Tact Time ��ȯ(Milliseconds).
	double GetGrabTactTimeMS() { return m_dTime; }
	//******************************************************************************************************************
	/// \brief					Total Image Count Ȯ�� �Լ�.
	/// \param uint64_t			Total Image Count ��ȯ.
	uint64_t GetTotalImageCount() { return m_nReceivedImageCount; }
	//******************************************************************************************************************
	/// \brief					Dropped Image Count Ȯ�� �Լ�.
	/// \param uint64_t			Dropped Image Count ��ȯ.
	uint64_t GetDroppedImageCount() { return m_nDroppedIDCount; }
	//******************************************************************************************************************
	/// \brief					Grab Done Handle ��ȯ �Լ�.
	/// \param HANDLE			Gab Done Handle ��ȯ.
	HANDLE GetHandleGrabDone() { return m_hGrabDone; }
	//******************************************************************************************************************
	/// \brief					Image Pointer ��ȯ �Լ�.
	/// \param byte*			Image Pointer ��ȯ.
	byte* GetImageBuffer() { return m_pBuffer; }

public :
	// Set Functions.
	//******************************************************************************************************************
	/// \brief					Continuous Mode ���� �Լ�.
	/// \param bool				��� ��ȯ.
	void SetContinuousMode();
	//******************************************************************************************************************
	/// \brief					Software Trigger Mode ���� �Լ�.
	/// \param bool				��� ��ȯ.
	void SetSoftTriggerMode();
	//******************************************************************************************************************
	/// \brief					Hardware Trigger Mode ���� �Լ�.
	/// \param bool				��� ��ȯ.
	void SetHardTriggerMode();
	//******************************************************************************************************************
	/// \brief					Device User ID ���� �Լ�.
	/// \param [in] strValue	Device User ID �Է�.
	/// \param void
	void SetDeviceUserID(CString strValue);
	//******************************************************************************************************************
	/// \brief					Persistent IP Address ���� �Լ�.
	/// \param [in] strValue	IP Address ����.
	/// \param void
	void SetIPAddress(CString strValue);
	//******************************************************************************************************************
	/// \brief					Persistent Subnet Mask ���� �Լ�.
	/// \param [out] strValue	Subnet Mask ����.
	/// \param void
	void SetSubnetMask(CString strValue);
	//******************************************************************************************************************
	/// \brief					Width ���� �Լ�.
	/// \param [in] nValue		Width �Է�.
	/// \param void
	void SetWidth(int nValue);
	//******************************************************************************************************************
	/// \brief					Height ���� �Լ�.
	/// \param [in] nValue		Height �Է�.
	/// \param void
	void SetHeight(int nValue);
	//******************************************************************************************************************
	/// \brief					Offset X ���� �Լ�.
	/// \param [in] nValue		Offset X �Է�.
	/// \param void
	void SetOffsetX(int nValue);
	//******************************************************************************************************************
	/// \brief					Offset Y ���� �Լ�.
	/// \param [in] nValue		Offset Y �Է�.
	/// \param void
	void SetOffsetY(int nValue);
	//******************************************************************************************************************
	/// \brief					Frame Rate ���� �Լ�.
	/// \param [in] dValue		Frame Rate �Է�.
	/// \param void
	void SetAcquisitionFrameRate(double dValue);
	//******************************************************************************************************************
	/// \brief					Acquisition Mode ���� �Լ�.
	/// \param [in] strValue	Acquisition Mode �Է�.
	/// \param void
	void SetAcquisitionMode(EAcqMode eMode);
	//******************************************************************************************************************
	/// \brief					Trigger Mode ���� �Լ�.
	/// \param [in] eMode		On / Off �Է�.
	/// \param void
	void SetTriggerMode(bool bEnable);
	//******************************************************************************************************************
	/// \brief					Trigger Source ���� �Լ�.
	/// \param [in] eSrc		Software / Hardware �Է�.
	/// \param void
	void SetTriggerSource(ETrgSrc eSrc);
	//******************************************************************************************************************
	/// \brief					Trigger Overlap ���� �Լ�.
	/// \param [in] eOvl		Off=0 / ReadOut / Previous Frame �Է�.
	/// \param void
	void SetTriggerOverlap(ETrgOvl eOvl);
	//******************************************************************************************************************
	/// \brief					Exposure Mode ���� �Լ�.
	/// \param [in] Mode		Timed / Trigger Width �Է�.
	/// \param void
	void SetExposureMode(EExpMode eMode);
	//******************************************************************************************************************
	/// \brief					Exposure Time ���� �Լ�.
	/// \param [in] dValue		Exposure Time �Է�.
	/// \param void
	void SetExposureTime(double dValue);
	//******************************************************************************************************************
	/// \brief					User Selector ���� �Լ�.
	/// \param [in] Mode		Default / UserSet1 �Է�.
	/// \param void
	void SetUserSetSelector(EUserSet eUser);
	//******************************************************************************************************************
	/// \brief					Default Selector ���� �Լ�.
	/// \param [in] Mode		Default / UserSet1 �Է�.
	/// \param void
	void SetUserSetDefaultSelector(EUserSet eUser);
	//******************************************************************************************************************
	/// \brief					Grab Done Set ���� �Լ�.
	/// \param void
	void OnResetHandleGrabDone() { ResetEvent(m_hGrabDone); }

private :
	// Sentech SDK
	static CIStSystemPtrArray	m_pIStSystemList;			// System Pointer Array.
	IStImageBuffer*				m_pIStImageBuff;			// Image Buffer.
	CIStDevicePtr				m_pIStDevice;				// Device Pointer.
	CIStDataStreamPtr			m_pIStDataStream;			// Data Stream Pointer.
	//CIStSystemPtrArray			m_objIStSystemPtrList;		// System Pointer Array.
	CIStRegisteredCallbackPtr	m_pIStCallbackDeviceLost;	// Callback Pointer.
	CIStNodeMapDisplayWndPtr	m_pIStNodeMapDisplayWnd;	// Nodemap Display Window Pointer.
	CIStVideoFilerPtr			m_pIStVideoFiler;			// Video Filer Pointer.
	GenApi::CLock				m_CLockForAVI;				// Clock for AVI.
	
	// Standard
	bool			m_isRecording;			// Check Recording.
	bool			m_isFirstFrame;			// Check First Frame.
	bool			m_isDeviceLost;			// Check Device Lost.
	bool			m_isBayer;				// Check Bayer Camera.
	double			m_dblCameraFrameRate;	// Frame Rate.
	double			m_dTime;				// Grab Tact Time.
	uint64_t		m_iPreTimestamp;
	uint64_t		m_iTimestampOffset;		// Timestamp Offset.
	uint64_t		m_nReceivedImageCount;	// Received Image Count.
	uint64_t		m_nDroppedIDCount;		// Dropped ID Count.
	uint64_t		m_nLastID;				// Last ID.
	HANDLE			m_hGrabDone;			// Grab Done Handle.
	byte*			m_pBuffer;				// User Buffer.

	void CreateDevice(StApi::IStInterface *pIStInterface, const StApi::IStDeviceInfo *pIStDeviceInfo);
	void OnDeviceLost(GenApi::INode *pINode, void*);	// Device Lost Callback Function.
	void OnStCallback(StApi::IStCallbackParamBase *pIStCallbackParamBase, void *pvContext);	// Image Callback Function.
private :
	//******************************************************************************************************************
	/// \brief					Type���� Get/Set �ϴ� �Լ�.
	/// \param [in] pNodeName	�Ķ������ Node Name �Է�.
	/// \param [in]  Value		�Ķ������ �� �Է�.
	/// \param [out] Value		�Ķ������ �� Ȯ��.
	/// \param bool				��� ��ȯ.
	void GetValueString(char* pNodeName, CString &strValue);
	void SetValueString(char* pNodeName, CString strValue);
	void GetValueInt(char* pNodeName, int64_t &nValue);
	void SetValueInt(char* pNodeName, int64_t nValue);
	void GetValueDouble(char* pNodeName, double &dValue);
	void SetValueDouble(char* pNodeName, double dValue);
	void GetValueEnum(char* pNodeName, CString &strValue);
	void SetValueEnum(char* pNodeName, CString strValue);
	void OnExecuteCommand(char* pNodeName);
};

}