//-----------------------------------------------------------------------
// Sentech Test Class v1.1.0. (with Sentech Common SDK v1.0.1)
//
// Copyright (c) 2017, FAINSTEC,.CO.LTD. 
//
// web : www.fainstec.com
//
// e-mail : tech@fainstec.com
//
// ※ 해당 코드는 사용자의 이해를 돕기 위해 만든 테스트 코드이므로
//   반드시 충분한 검증과 테스트를 통해 참고하여 사용하길 권장 드립니다.
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
	/// \brief					Device Select Dialog 를 이용한 Device 연결 함수.
	/// \param void				
	void OnConnect();
	//******************************************************************************************************************
	/// \brief					Device 정보를 이용한 Device 연결 함수.
	/// \param [in] strValue	Device Information.
	/// \param [in] eType		UserID, Serial Number, IP, MAC
	/// \param void				
	void OnConnect(CString strValue, EConnectType eType);
	//******************************************************************************************************************
	/// \brief					Parameter Window를 호출하는 함수.
	/// \param void				
	void OnShowParameterWnd();
	//******************************************************************************************************************
	/// \brief					AVI 녹화 시작 함수.
	/// \param [in] strPath		녹화 파일 저장 경로.
	/// \param void				
	void OnStartRecording(CString strPath);
	//******************************************************************************************************************
	/// \brief					AVI 녹화 종료 함수.
	/// \param void				
	void OnStopRecording();
	//******************************************************************************************************************
	/// \brief					Device 해제 함수.
	/// \param void				
	void OnDisconnect();
	//******************************************************************************************************************
	/// \brief					영상 취득 시작 함수.
	/// \param [in] nFrame		영상 취득 프레임 수. 기본값 무한 획득.
	/// \param bool				결과 반환.
	void OnStartAcquisition(uint64_t nFrame = GENTL_INFINITE);
	//******************************************************************************************************************
	/// \brief					영상 취득 종료 함수.
	/// \param bool				결과 반환.
	void OnStopAcquisition();
	//******************************************************************************************************************
	/// \brief					Software Trigger Event 발생 함수.
	/// \param bool				결과 반환.
	void OnTriggerEvent();
	//******************************************************************************************************************
	/// \brief					User Set 저장 함수.
	/// \param bool				결과 반환.
	void OnUserSetSave();
	//******************************************************************************************************************
	/// \brief					User Set 불러오기 함수.
	/// \param bool				결과 반환.
	void OnUserSetLoad();
	//******************************************************************************************************************
	/// \brief					이미지 저장 함수.
	/// \param [in] strPath		저장 경로 입력.
	/// \param void
	void OnSaveImage(CString strPath, EFormat eFmt = eBMP);
	//******************************************************************************************************************
	/// \brief					Auto White Balance 설정 함수.
	/// \param [in] Type		Off / Preset 1~3 / Continuous / Once 선택 가능.
	/// \param bool				결과 반환.
	void OnAutoWhiteBalance(EAWB eMode);
	//******************************************************************************************************************
	/// \brief					AWB Once로 계산된 파라미터를 지정된 Preset 파라미터에 저장하는 함수.
	/// \param [in] nPresetNum	특정 Preset 번호 입력.
	/// \param bool				결과 반환.
	void OnSaveAWBValueOnceToPreset(int nPreset);
public :
	// Get Functions.
	//******************************************************************************************************************
	/// \brief					Device 연결 상태 확인 함수.
	/// \param bool				결과 반환.
	bool IsConnected();
	//******************************************************************************************************************
	/// \brief					영상 취득 상태 확인 함수.
	/// \param bool				결과 반환.
	bool IsActived();
	//******************************************************************************************************************
	/// \brief					Device User ID 확인 함수.
	/// \param [out] strValue	Device User ID 확인.
	/// \param void
	void GetUserID(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Device Model Name 확인 함수.
	/// \param [out] strValue	Device Model Name 확인.
	/// \param void
	void GetModelName(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Device Serial Number 확인 함수.
	/// \param [out] strValue	Device Serial Number 확인.
	/// \param void
	void GetSerialNumber(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Device Current IP Address 확인 함수.
	/// \param [out] strValue	IP Address 확인.
	/// \param void
	void GetIPAddress(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Device Current Subnet Mask 확인 함수.
	/// \param [out] strValue	Subnet Mask 확인.
	/// \param void
	void GetSubnetMask(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Width 확인 함수.
	/// \param [out] nValue		Width 확인.
	/// \param void
	void GetWidth(int &nValue);
	//******************************************************************************************************************
	/// \brief					Height 확인 함수.
	/// \param [out] nValue		Height 확인.
	/// \param void				
	void GetHeight(int &nValue);
	//******************************************************************************************************************
	/// \brief					Bit per Pixel 확인 함수.
	/// \param [out] nValue		Bit per Pixel 확인.
	/// \param void				
	void GetBpp(int &nValue);
	//******************************************************************************************************************
	/// \brief					Bit per Pixel 확인 함수.
	/// \param [out] strValue	Bit per Pixel 확인.
	/// \param void				
	void GetBpp(CString &strValue);
	//******************************************************************************************************************
	/// \brief					Grab Tact Time 확인 함수.
	/// \param double			Tact Time 반환(Milliseconds).
	double GetGrabTactTimeMS() { return m_dTime; }
	//******************************************************************************************************************
	/// \brief					Total Image Count 확인 함수.
	/// \param uint64_t			Total Image Count 반환.
	uint64_t GetTotalImageCount() { return m_nReceivedImageCount; }
	//******************************************************************************************************************
	/// \brief					Dropped Image Count 확인 함수.
	/// \param uint64_t			Dropped Image Count 반환.
	uint64_t GetDroppedImageCount() { return m_nDroppedIDCount; }
	//******************************************************************************************************************
	/// \brief					Grab Done Handle 반환 함수.
	/// \param HANDLE			Gab Done Handle 반환.
	HANDLE GetHandleGrabDone() { return m_hGrabDone; }
	//******************************************************************************************************************
	/// \brief					Image Pointer 반환 함수.
	/// \param byte*			Image Pointer 반환.
	byte* GetImageBuffer() { return m_pBuffer; }

public :
	// Set Functions.
	//******************************************************************************************************************
	/// \brief					Continuous Mode 설정 함수.
	/// \param bool				결과 반환.
	void SetContinuousMode();
	//******************************************************************************************************************
	/// \brief					Software Trigger Mode 설정 함수.
	/// \param bool				결과 반환.
	void SetSoftTriggerMode();
	//******************************************************************************************************************
	/// \brief					Hardware Trigger Mode 설정 함수.
	/// \param bool				결과 반환.
	void SetHardTriggerMode();
	//******************************************************************************************************************
	/// \brief					Device User ID 설정 함수.
	/// \param [in] strValue	Device User ID 입력.
	/// \param void
	void SetDeviceUserID(CString strValue);
	//******************************************************************************************************************
	/// \brief					Persistent IP Address 설정 함수.
	/// \param [in] strValue	IP Address 설정.
	/// \param void
	void SetIPAddress(CString strValue);
	//******************************************************************************************************************
	/// \brief					Persistent Subnet Mask 설정 함수.
	/// \param [out] strValue	Subnet Mask 설정.
	/// \param void
	void SetSubnetMask(CString strValue);
	//******************************************************************************************************************
	/// \brief					Width 설정 함수.
	/// \param [in] nValue		Width 입력.
	/// \param void
	void SetWidth(int nValue);
	//******************************************************************************************************************
	/// \brief					Height 설정 함수.
	/// \param [in] nValue		Height 입력.
	/// \param void
	void SetHeight(int nValue);
	//******************************************************************************************************************
	/// \brief					Offset X 설정 함수.
	/// \param [in] nValue		Offset X 입력.
	/// \param void
	void SetOffsetX(int nValue);
	//******************************************************************************************************************
	/// \brief					Offset Y 설정 함수.
	/// \param [in] nValue		Offset Y 입력.
	/// \param void
	void SetOffsetY(int nValue);
	//******************************************************************************************************************
	/// \brief					Frame Rate 설정 함수.
	/// \param [in] dValue		Frame Rate 입력.
	/// \param void
	void SetAcquisitionFrameRate(double dValue);
	//******************************************************************************************************************
	/// \brief					Acquisition Mode 설정 함수.
	/// \param [in] strValue	Acquisition Mode 입력.
	/// \param void
	void SetAcquisitionMode(EAcqMode eMode);
	//******************************************************************************************************************
	/// \brief					Trigger Mode 설정 함수.
	/// \param [in] eMode		On / Off 입력.
	/// \param void
	void SetTriggerMode(bool bEnable);
	//******************************************************************************************************************
	/// \brief					Trigger Source 설정 함수.
	/// \param [in] eSrc		Software / Hardware 입력.
	/// \param void
	void SetTriggerSource(ETrgSrc eSrc);
	//******************************************************************************************************************
	/// \brief					Trigger Overlap 설정 함수.
	/// \param [in] eOvl		Off=0 / ReadOut / Previous Frame 입력.
	/// \param void
	void SetTriggerOverlap(ETrgOvl eOvl);
	//******************************************************************************************************************
	/// \brief					Exposure Mode 설정 함수.
	/// \param [in] Mode		Timed / Trigger Width 입력.
	/// \param void
	void SetExposureMode(EExpMode eMode);
	//******************************************************************************************************************
	/// \brief					Exposure Time 설정 함수.
	/// \param [in] dValue		Exposure Time 입력.
	/// \param void
	void SetExposureTime(double dValue);
	//******************************************************************************************************************
	/// \brief					User Selector 설정 함수.
	/// \param [in] Mode		Default / UserSet1 입력.
	/// \param void
	void SetUserSetSelector(EUserSet eUser);
	//******************************************************************************************************************
	/// \brief					Default Selector 설정 함수.
	/// \param [in] Mode		Default / UserSet1 입력.
	/// \param void
	void SetUserSetDefaultSelector(EUserSet eUser);
	//******************************************************************************************************************
	/// \brief					Grab Done Set 설정 함수.
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
	/// \brief					Type별로 Get/Set 하는 함수.
	/// \param [in] pNodeName	파라미터의 Node Name 입력.
	/// \param [in]  Value		파라미터의 값 입력.
	/// \param [out] Value		파라미터의 값 확인.
	/// \param bool				결과 반환.
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