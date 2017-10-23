#include "StdAfx.h"
#include "StCamera.h"

using namespace SENTECH_SDK;

std::vector<StSystemInfo> CStCamera::m_vSystemInfo;
CIStSystemPtrArray CStCamera::m_pIStSystemList;
CStCamera::CStCamera(void)
	: m_pIStDevice(NULL)
	, m_pIStDataStream(NULL)
	, m_pIStCallbackDeviceLost(NULL)
	, m_pIStImageBuff(NULL)
	, m_pIStNodeMapDisplayWnd(NULL)
	, m_pIStVideoFiler(NULL)
	, m_ApiAutoInit()
{
	m_isDeviceLost = false;
	m_isRecording = false;
	m_isFirstFrame = true;
	m_isBayer = false;
	m_nReceivedImageCount = 0;
	m_nDroppedIDCount = 0;
	m_nLastID = 0;
	m_iTimestampOffset = 0;
	m_iPreTimestamp = 0;
	m_dblCameraFrameRate = 60.0;
	m_dTime = 0;
	m_pBuffer = NULL;

	m_hGrabDone = CreateEvent(NULL,TRUE,FALSE,NULL);
}

CStCamera::~CStCamera(void)
{
	OnDisconnect();
}

bool CStCamera::OnSearchSystem()
{
	try
	{
		m_vSystemInfo.clear();
		for (EStSystemVendor_t eSystemVendor = StSystemVendor_Sentech; eSystemVendor < StSystemVendor_Count; eSystemVendor = (EStSystemVendor_t)(eSystemVendor + 1))
		{
			try
			{
				m_pIStSystemList.Register(CreateIStSystem(eSystemVendor, StInterfaceType_All));
			}
			catch (const GenICam::GenericException &e)
			{
				CString msg = (CString)e.GetDescription();
			}
		}

		// Get System Count.
		int nSyCnt = (int)m_pIStSystemList.GetSize();
		for (int i=0; i<nSyCnt; i++)
		{
			StSystemInfo SyInfo;
			SyInfo.strName		= (CString)m_pIStSystemList[i]->GetIStSystemInfo()->GetDisplayName().c_str();
			SyInfo.strVendor	= (CString)m_pIStSystemList[i]->GetIStSystemInfo()->GetVendor().c_str();
			SyInfo.strType		= (CString)m_pIStSystemList[i]->GetIStSystemInfo()->GetTLType().c_str();

			// Get Interface Count.
			int nIfCnt = m_pIStSystemList[i]->GetInterfaceCount();
			for (int j=0; j<nIfCnt; j++)
			{
				StInterfaceInfo IfInfo;
				m_pIStSystemList[i]->GetIStInterface(j)->UpdateDeviceList();
				IfInfo.strName		= (CString)m_pIStSystemList[i]->GetIStInterface(j)->GetIStInterfaceInfo()->GetDisplayName().c_str();
				IfInfo.strType		= (CString)m_pIStSystemList[i]->GetIStInterface(j)->GetIStInterfaceInfo()->GetTLType().c_str();
				IfInfo.strMacAddr	= (CString)m_pIStSystemList[i]->GetIStInterface(j)->GetIStInterfaceInfo()->GetID().c_str();
				if (IfInfo.strType == _T("GEV"))
				{
					GenApi::CIntegerPtr pGevIP(m_pIStSystemList[i]->GetIStInterface(j)->GetIStPort()->GetINodeMap()->GetNode("GevInterfaceSubnetIPAddress"));
					IfInfo.strIPAddr = (CString)pGevIP->ToString().c_str();
					
					GenApi::CIntegerPtr pGevSubMask(m_pIStSystemList[i]->GetIStInterface(j)->GetIStPort()->GetINodeMap()->GetNode("GevInterfaceSubnetMask"));
					IfInfo.strSubMask = (CString)pGevSubMask->ToString().c_str();
				}
				
				// Get Device Count.
				int nDvCnt = m_pIStSystemList[i]->GetIStInterface(j)->GetDeviceCount();
				for (int k=0; k<nDvCnt; k++)
				{
					StDeviceInfo DvInfo;
					DvInfo.strName		= (CString)m_pIStSystemList[i]->GetIStInterface(j)->GetIStDeviceInfo(k)->GetDisplayName().c_str();
					DvInfo.strVendor	= (CString)m_pIStSystemList[i]->GetIStInterface(j)->GetIStDeviceInfo(k)->GetVendor().c_str();
					DvInfo.nAccSts		= m_pIStSystemList[i]->GetIStInterface(j)->GetIStDeviceInfo(k)->GetAccessStatus();
					DvInfo.strMacAddr	= (CString)m_pIStSystemList[i]->GetIStInterface(j)->GetIStDeviceInfo(k)->GetID().c_str();
					DvInfo.strUserID	= (CString)m_pIStSystemList[i]->GetIStInterface(j)->GetIStDeviceInfo(k)->GetUserDefinedName().c_str();
					DvInfo.strSN		= (CString)m_pIStSystemList[i]->GetIStInterface(j)->GetIStDeviceInfo(k)->GetSerialNumber().c_str();
					DvInfo.strType		= (CString)m_pIStSystemList[i]->GetIStInterface(j)->GetIStDeviceInfo(k)->GetTLType().c_str();
					if (DvInfo.strType == _T("GEV"))
					{
						GenApi::CIntegerPtr pGevIP(m_pIStSystemList[i]->GetIStInterface(j)->GetIStPort()->GetINodeMap()->GetNode("GevDeviceIPAddress"));
						DvInfo.strIPAddr = (CString)pGevIP->ToString().c_str();

						GenApi::CIntegerPtr pGevSubMask(m_pIStSystemList[i]->GetIStInterface(j)->GetIStPort()->GetINodeMap()->GetNode("GevDeviceSubnetMask"));
						DvInfo.strSubMask = (CString)pGevSubMask->ToString().c_str();
					}
					IfInfo.vDvInfo.push_back(DvInfo);
				}
				SyInfo.vIfInfo.push_back(IfInfo);
			}
			m_vSystemInfo.push_back(SyInfo);	
		}
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}

	return true;
}

bool CStCamera::GetSystemCount(int &nSyCnt)
{
	nSyCnt = (int)m_vSystemInfo.size();
	return true;
}

bool CStCamera::GetSystemName(int nIdxSy, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	strValue = m_vSystemInfo[nIdxSy].strName;

	return true;
}

bool CStCamera::GetSystemVendor(int nIdxSy, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	strValue = m_vSystemInfo[nIdxSy].strVendor;

	return true;
}

bool CStCamera::GetSystemType(int nIdxSy, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	strValue = m_vSystemInfo[nIdxSy].strType;

	return true;
}

bool CStCamera::GetInterfaceCount(int nIdxSy, int &nIfCnt)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	nIfCnt = (int)m_vSystemInfo[nIdxSy].vIfInfo.size();

	return true;
}

bool CStCamera::GetInterfaceName(int nIdxSy, int nIdxIf, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].strName;

	return true;
}

bool CStCamera::GetInterfaceType(int nIdxSy, int nIdxIf, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].strType;

	return true;
}

bool CStCamera::GetInterfaceMACAddr(int nIdxSy, int nIdxIf, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].strMacAddr;

	return true;
}

bool CStCamera::GetInterfaceIPAddr(int nIdxSy, int nIdxIf, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].strIPAddr;

	return true;
}

bool CStCamera::GetInterfaceSubMask(int nIdxSy, int nIdxIf, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].strSubMask;

	return true;
}

bool CStCamera::GetDeviceCount(int nIdxSy, int nIdxIf, int &nDvCnt)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	nDvCnt = (int)m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size();

	return true;
}

bool CStCamera::GetDeviceName(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() <= nIdxDv)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo[nIdxDv].strName;

	return true;
}

bool CStCamera::GetDeviceSN(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() <= nIdxDv)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo[nIdxDv].strSN;

	return true;
}

bool CStCamera::GetDeviceVendor(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() <= nIdxDv)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo[nIdxDv].strVendor;

	return true;
}

bool CStCamera::GetDeviceType(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() <= nIdxDv)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo[nIdxDv].strType;

	return true;
}

bool CStCamera::GetDeviceAccessStatus(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() <= nIdxDv)
		return false;

	switch (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo[nIdxDv].nAccSts)
	{
	case GenTL::DEVICE_ACCESS_STATUS_UNKNOWN :
		strValue = _T("DEVICE_ACCESS_STATUS_UNKNOWN");
		break;
	case GenTL::DEVICE_ACCESS_STATUS_READWRITE  :
		strValue = _T("DEVICE_ACCESS_STATUS_READWRITE ");
		break;
	case GenTL::DEVICE_ACCESS_STATUS_READONLY :
		strValue = _T("DEVICE_ACCESS_STATUS_READONLY");
		break;
	case GenTL::DEVICE_ACCESS_STATUS_NOACCESS :
		strValue = _T("DEVICE_ACCESS_STATUS_NOACCESS");
		break;
	case GenTL::DEVICE_ACCESS_STATUS_BUSY :
		strValue = _T("DEVICE_ACCESS_STATUS_BUSY");
		break;
	case GenTL::DEVICE_ACCESS_STATUS_OPEN_READWRITE :
		strValue = _T("DEVICE_ACCESS_STATUS_OPEN_READWRITE");
		break;
	case GenTL::DEVICE_ACCESS_STATUS_OPEN_READ :
		strValue = _T("DEVICE_ACCESS_STATUS_OPEN_READ");
		break;
	}

	return true;
}

bool CStCamera::GetDeviceMACAddr(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() <= nIdxDv)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo[nIdxDv].strMacAddr;

	return true;
}

bool CStCamera::GetDeviceIPAddr(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() <= nIdxDv)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo[nIdxDv].strIPAddr;

	return true;
}

bool CStCamera::GetDeviceSubMask(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() <= nIdxDv)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo[nIdxDv].strSubMask;

	return true;
}

void CStCamera::SetDeviceIPAddr(int nIdxIf, int nIdxDv, CString strValue)
{
	try
	{
		// Create a system object for device scan and connection.
		CIStSystemPtr pIStSystem(CreateIStSystem(StSystemVendor_Sentech, StInterfaceType_GigEVision));

		// Check GigE interface for devices.
		// If there are no any cameras, throw exception for exit.
		IStInterface *pIStInterface = NULL;
		int nIfCnt = pIStSystem->GetInterfaceCount();
		if (nIdxIf < 0 || nIdxIf >= nIfCnt) return;

		int nDvCnt = pIStSystem->GetIStInterface(nIdxIf)->GetDeviceCount();
		if (nIdxDv < 0 || nIdxDv >= nDvCnt) return;

		pIStInterface = pIStSystem->GetIStInterface(nIdxIf);

		GenApi::CIntegerPtr pGevIP = pIStInterface->GetIStPort()->GetINodeMap()->GetNode("GevDeviceForceIPAddress");

		gcstring strIP(strValue);
		const uint32_t nNewIP = ntohl(inet_addr(strIP.c_str()));

		pGevIP->SetValue(nNewIP);

		GenApi::CCommandPtr pForceIP(pIStInterface->GetIStPort()->GetINodeMap()->GetNode("GevDeviceForceIP"));
		pForceIP->Execute();
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

bool CStCamera::SetDeviceSubMask(int nIdxIf, int nIdxDv, CString strValue)
{
	try
	{
		// Create a system object for device scan and connection.
		CIStSystemPtr pIStSystem(CreateIStSystem(StSystemVendor_Sentech, StInterfaceType_GigEVision));

		// Check GigE interface for devices.
		// If there are no any cameras, throw exception for exit.
		IStInterface *pIStInterface = NULL;
		int nIfCnt = pIStSystem->GetInterfaceCount();
		if (nIdxIf < 0 || nIdxIf >= nIfCnt) return false;

		int nDvCnt = pIStSystem->GetIStInterface(nIdxIf)->GetDeviceCount();
		if (nIdxDv < 0 || nIdxDv >= nDvCnt) return false;

		pIStInterface = pIStSystem->GetIStInterface(nIdxIf);

		GenApi::CIntegerPtr pGevIP(pIStInterface->GetIStPort()->GetINodeMap()->GetNode("GevDeviceForceSubnetMask"));

		gcstring strIP(strValue);
		const uint32_t nNewIP = ntohl(inet_addr(strIP.c_str()));

		pGevIP->SetValue(nNewIP);

		GenApi::CCommandPtr pForceIP(pIStInterface->GetIStPort()->GetINodeMap()->GetNode("GevDeviceForceIP"));
		pForceIP->Execute();
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

bool CStCamera::GetDeviceUserID(int nIdxSy, int nIdxIf, int nIdxDv, CString &strValue)
{
	if (m_vSystemInfo.size() == 0 || m_vSystemInfo.size() <= nIdxSy)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo.size() <= nIdxIf)
		return false;

	if (m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() == 0 || m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo.size() <= nIdxDv)
		return false;

	strValue = m_vSystemInfo[nIdxSy].vIfInfo[nIdxIf].vDvInfo[nIdxDv].strUserID;

	return true;
}

void CStCamera::CreateDevice(StApi::IStInterface *pIStInterface, const StApi::IStDeviceInfo *pIStDeviceInfo)
{
	try
	{
		IStDeviceReleasable *pIStDeviceReleasable = NULL;

		//Create object and get IStDeviceReleasable pointer.
		GenICam::gcstring strDeviceID = pIStDeviceInfo->GetID();
		pIStDeviceReleasable = pIStInterface->CreateIStDevice(strDeviceID);

		m_pIStDevice.Reset(pIStDeviceReleasable);

		//Register device lost event.
		GenApi::CNodeMapPtr pINodeMapLocalDevice(m_pIStDevice->GetLocalIStPort()->GetINodeMap());
		if (pINodeMapLocalDevice)
		{
			GenApi::CNodePtr pINodeEventDeviceLost(pINodeMapLocalDevice->GetNode("EventDeviceLost"));
			if (pINodeEventDeviceLost != NULL)
			{
				m_pIStCallbackDeviceLost.Reset(StApi::RegisterCallback(pINodeEventDeviceLost, *this, &CStCamera::OnDeviceLost, (void*)NULL, GenApi::cbPostOutsideLock));
			}
		}

		//Start Event Acquisition Thread
		m_pIStDevice->StartEventAcquisitionThread();

		//Create object and get IStDataStream pointer.
		m_pIStDataStream.Reset(m_pIStDevice->CreateIStDataStream(0));
		if (m_pIStDataStream != NULL)
		{
			//Register callback function to receive images.
			RegisterCallback(m_pIStDataStream, *this, &CStCamera::OnStCallback, (void*)NULL);
		}

		GenApi::CIntegerPtr pInt = NULL;
		pInt = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("PayloadSize");
		int nPayload = (int)pInt->GetValue();

		GenApi::CNodeMapPtr pINodeMapRemote(m_pIStDevice->GetRemoteIStPort()->GetINodeMap());
		EStPixelFormatNamingConverntion_t nPFNC;
		IStPixelFormatInfo *pIStPixelFormatInfo = NULL;

		int nBpp=8;
		m_isBayer = false;
		// GigE / U3V Interface.
		try
		{
			nPFNC = (EStPixelFormatNamingConverntion_t)dynamic_cast<GenApi::IEnumeration*>(pINodeMapRemote->GetNode("PixelFormat"))->GetIntValue();
			pIStPixelFormatInfo = GetIStPixelFormatInfo(nPFNC);
			
			if (pIStPixelFormatInfo->IsBayer() == true)
			{
				m_isBayer = true;
				nBpp = 24;
			}
		}
		// CXP Interface.
		// CXP Interface 는 PFNC(Pixel Format Naming Conversion)가 대응되지 않으므로 catch 에서 따로 처리해야 한다.
		catch (const GenICam::GenericException &e)
		{
			CString strPxlFmt=_T("");
			GetValueEnum("PixelFormat", strPxlFmt);
			
			strPxlFmt.MakeUpper();
			if (strPxlFmt.Find(_T("24")) != -1)
			{
				m_isBayer = true;
				nBpp = 24;
			}
		}
		
		if (m_pBuffer != NULL)
		{
			delete []m_pBuffer;
			m_pBuffer = NULL;
		}

		m_pBuffer = new byte[nPayload*nBpp/8];
		ZeroMemory(m_pBuffer, nPayload*nBpp/8);

		m_pIStImageBuff = CreateIStImageBuffer();
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::OnConnect()
{
	try
	{
		const uint32_t nCount = StSystemVendor_Count;
		//const uint32_t nCount = 1;	//Sentech only
		//const uint32_t nCount = 2;	//Sentech + Euresys
		for (uint32_t i = StSystemVendor_Sentech; i < nCount; ++i)
		{
			EStSystemVendor_t eStSystemVendor = (EStSystemVendor_t)i;
			try
			{
				// Create a system object, to get the IStSystemReleasable interface pointer.
				// After the system object is no longer needed, call the IStSystemReleasable::Release(), please discard the system object.
				//m_objIStSystemPtrList.Register(CreateIStSystem(eStSystemVendor, StInterfaceType_All));
				m_pIStSystemList.Register(CreateIStSystem(eStSystemVendor, StInterfaceType_All));
			}
			catch (const GenICam::GenericException &e)
			{
				CString msg = (CString)e.GetDescription();
			}
		}

		//Create "DeviceSelectionWnd".
		CIStDeviceSelectionWndPtr pIStDeviceSelectionWnd(CreateIStWnd(StWindowType_DeviceSelection));

		//Move the "DeviceSelectionWnd" to the center of the main window.
		int nWidth = 1280;
		int nHeight = 720;
		pIStDeviceSelectionWnd->SetPosition(0, 0, nWidth, nHeight);

		//Specify the "IStSystem" to use.
		//pIStDeviceSelectionWnd->RegisterTargetIStSystemList(m_objIStSystemPtrList);
		pIStDeviceSelectionWnd->RegisterTargetIStSystemList(m_pIStSystemList);

		//Show the "DeviceSelectionWnd".
		//pIStDeviceSelectionWnd->Show(this->GetMainWnd()->GetSafeHwnd(), StWindowMode_Modal);
		pIStDeviceSelectionWnd->Show(NULL, StWindowMode_Modal);

		//Get selected device information.
		StApi::IStInterface *pIStInterface = NULL;
		const StApi::IStDeviceInfo *pIStDeviceInfo = NULL;
		pIStDeviceSelectionWnd->GetSelectedDeviceInfo(&pIStInterface, &pIStDeviceInfo);

		if(pIStDeviceInfo !=  NULL)
		{
			CreateDevice(pIStInterface, pIStDeviceInfo);
		}
	}
	catch (const GenICam::GenericException &e)
	{
		m_pIStCallbackDeviceLost.Reset(NULL);
		m_pIStDataStream.Reset(NULL);
		m_pIStDevice.Reset(NULL);

		throw e;
	}
}

void CStCamera::OnConnect(CString strValue, EConnectType eType)
{
	try
	{
		int idxSy=0, idxIf=0, idxDv=0;
		bool bFound=false;
		for (std::vector<StSystemInfo>::iterator it1 = m_vSystemInfo.begin(); it1 != m_vSystemInfo.end(); it1++)
		{
			idxIf = 0;
			for (std::vector<StInterfaceInfo>::iterator it2 = it1->vIfInfo.begin(); it2 != it1->vIfInfo.end(); it2++)
			{
				idxDv = 0;
				for (std::vector<StDeviceInfo>::iterator it3 = it2->vDvInfo.begin(); it3 != it2->vDvInfo.end(); it3++)
				{
					switch (eType)
					{
					case eUserID :
						if (it3->strUserID == strValue)
						{
							bFound = true;
							goto endloop;
						}
						break;
					case eSeiralNumber :
						if (it3->strSN == strValue)
						{
							bFound = true;
							goto endloop;
						}
						break;
					case eIP :
						if (it3->strIPAddr == strValue)
						{
							bFound = true;
							goto endloop;
						}
						break;
					case eMAC :
						if (it3->strMacAddr == strValue)
						{
							bFound = true;
							goto endloop;
						}
						break;
					}
						
					idxDv++;
				}
				idxIf++;
			}
			idxSy++;
		}
		
endloop :

		if (bFound == false) return;
		
		StApi::IStInterface *pIStInterface = NULL;
		IStDeviceReleasable *pIStDeviceReleasable = NULL;

		const StApi::IStDeviceInfo *pIStDeviceInfo = NULL;
		pIStInterface = m_pIStSystemList[idxSy]->GetIStInterface(idxIf);
		
		pIStDeviceInfo = pIStInterface->GetIStDeviceInfo(idxDv);
		CString model = (CString)pIStDeviceInfo->GetModel().c_str();
		GenICam::gcstring strDeviceID = pIStDeviceInfo->GetID();

		CreateDevice(pIStInterface, pIStDeviceInfo);
	}
	catch (const GenICam::GenericException &e)
	{
		m_pIStCallbackDeviceLost.Reset(NULL);
		m_pIStDataStream.Reset(NULL);
		m_pIStDevice.Reset(NULL);
		throw e;
	}
}

void CStCamera::OnDisconnect()
{
	try
	{
		OnStopAcquisition();

		if (m_pIStVideoFiler != NULL)
			m_pIStVideoFiler.Reset(NULL);

		if (m_pIStNodeMapDisplayWnd != NULL)
		{
			m_pIStNodeMapDisplayWnd->Close();
			m_pIStNodeMapDisplayWnd.Reset(NULL);
		}

		if (m_pIStDevice != NULL)
			m_pIStDevice->StopEventAcquisitionThread();

		if (m_pIStDataStream != NULL)
			m_pIStDataStream.Reset(NULL);

		if (m_pIStCallbackDeviceLost != NULL)
			m_pIStCallbackDeviceLost.Reset(NULL);

		if (m_pIStDevice != NULL)
			m_pIStDevice.Reset(NULL);

		if (m_pBuffer != NULL)
		{
			delete []m_pBuffer;
			m_pBuffer = NULL;
		}
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::OnStartRecording(CString strPath)
{
	try
	{
		m_pIStVideoFiler.Reset(StApi::CreateIStFiler(StApi::StFilerType_Video));

		// Get the INodeMap interface pointer for the camera settings.
		GenApi::CNodeMapPtr pINodeMapRemote(m_pIStDevice->GetRemoteIStPort()->GetINodeMap());

		//Get "AcquisitionFrameRate" to estimate frame number from timestamp value.
		GenApi::CFloatPtr pIFloat_AcquisitionFrameRate(pINodeMapRemote->GetNode("AcquisitionFrameRate"));
		
		if(pIFloat_AcquisitionFrameRate)
		{
			m_dblCameraFrameRate = pIFloat_AcquisitionFrameRate->GetValue();
		}
		m_pIStVideoFiler->SetFPS(m_dblCameraFrameRate);
		
		gcstring path(strPath);

		m_pIStVideoFiler->RegisterFileName(path);

		CIStNodeMapDisplayWndPtr pIStNodeMapDisplayWnd;
		// Create a data NodeMapDisplayWnd object to get the IStWndReleasable interface pointer.
		// After the NodeMapDisplayWnd object is no longer needed, call the IStWndReleasable::Release(), please discard the NodeMapDisplayWnd object.
		// In the destructor of CIStNodeMapDisplayWndPtr, IStWndReleasable::Release() is called.
		pIStNodeMapDisplayWnd.Reset(StApi::CreateIStWnd(StApi::StWindowType_NodeMapDisplay));

		//Register the Root node of the NodeMap of IStVideoFiler.
		pIStNodeMapDisplayWnd->RegisterINode(m_pIStVideoFiler->GetINodeMap()->GetNode("Root"), "");

		//Hide unneeded controls.
		pIStNodeMapDisplayWnd->SetVisibleAlphabeticMode(false);
		pIStNodeMapDisplayWnd->SetVisibleCollapse(false);
		pIStNodeMapDisplayWnd->SetVisibleExpand(false);
		pIStNodeMapDisplayWnd->SetVisiblePolling(false);
		pIStNodeMapDisplayWnd->SetVisibleRefresh(false);
		pIStNodeMapDisplayWnd->SetVisibleVisibility(false);
		pIStNodeMapDisplayWnd->SetVisibleDescription(false);

		// Sets the position and size of the window.
		pIStNodeMapDisplayWnd->SetPosition(0, 0, 400, 400);

		//Display the window.
		pIStNodeMapDisplayWnd->Show(NULL, StApi::StWindowMode_Modal);
		
		GenApi::AutoLock autoLock(m_CLockForAVI);

		m_isRecording = true;
	}
	catch (const GenICam::GenericException &e)
	{
		m_isRecording = false;
		throw e;
	}
}

void CStCamera::OnStopRecording()
{
	try
	{
		GenApi::AutoLock autoLock(m_CLockForAVI);
		m_isRecording = false;

		m_pIStVideoFiler.Reset(NULL);
	}
	catch (const GenICam::GenericException &e)
	{
		m_isRecording = false;
		throw e;
	}
}

void CStCamera::OnStCallback(StApi::IStCallbackParamBase *pIStCallbackParamBase, void *pvContext)
{
	try
	{
		StApi::EStCallbackType_t eStCallbackType = pIStCallbackParamBase->GetCallbackType();
		if(eStCallbackType  ==  StApi::StCallbackType_GenTLEvent_DataStreamNewBuffer)
		{
			++m_nReceivedImageCount;

			StApi::IStCallbackParamGenTLEventNewBuffer *pIStCallbackParamGenTLEventNewBuffer = dynamic_cast<StApi::IStCallbackParamGenTLEventNewBuffer*>(pIStCallbackParamBase);
			StApi::IStDataStream *pIStDataStream = pIStCallbackParamGenTLEventNewBuffer->GetIStDataStream();

			// Wait until the data is acquired.
			// If the data has been received, IStStreamBufferReleasable interface pointer is retrieved.
			// When the received data is no longer needed, immediately call the IStStreamBufferReleasable::Release(), please return the buffer to the streaming queue.
			// In the destructor of CIStStreamBufferPtr, IStStreamBufferReleasable::Release() is called.
			StApi::CIStStreamBufferPtr pIStStreamBuffer(pIStDataStream->RetrieveBuffer(0));

			uint64_t nCurrentTimestampNs = 0;
			nCurrentTimestampNs = pIStStreamBuffer->GetIStStreamBufferInfo()->GetTimestampNS();

			m_pIStImageBuff->CopyImage(pIStStreamBuffer->GetIStImage());
			
			// Calculating the frame number in consideration of the frame drop.
			uint32_t nEstimatedFrameNo = 0;
			if(m_isFirstFrame == true)
			{
				//Initialize timestamp offset value
				m_iTimestampOffset = nCurrentTimestampNs;
				m_iPreTimestamp = nCurrentTimestampNs;
				m_isFirstFrame = false;
			}

			// Calculate Tact Time.
			m_dTime = (double)(nCurrentTimestampNs - m_iPreTimestamp)/1000000;

			m_iPreTimestamp = nCurrentTimestampNs;
			if (m_isBayer == true)
			{
				CIStImageBufferPtr buffer = CreateIStImageBuffer();;
				
				// Create a data converter object for pixel format conversion.
				CIStPixelFormatConverterPtr pIStPixelFormatConverter = CreateIStConverter(StConverterType_PixelFormat);

				// Convert the image data to StPFNC_BGR8 format
				pIStPixelFormatConverter->SetDestinationPixelFormat(StPFNC_BGR8);
				pIStPixelFormatConverter->Convert(pIStStreamBuffer->GetIStImage(), buffer);
				
				memcpy(m_pBuffer, (byte*)buffer->GetIStImage()->GetImageBuffer(), pIStStreamBuffer->GetIStStreamBufferInfo()->GetBufferSize()*3);
			}
			else
			{
				memcpy(m_pBuffer, (byte*)pIStStreamBuffer->GetIStImage()->GetImageBuffer(), pIStStreamBuffer->GetIStStreamBufferInfo()->GetBufferSize());
			}

			SetEvent(m_hGrabDone);

			if(m_isRecording == true)
			{
				GenApi::AutoLock autoLock(m_CLockForAVI);
				if(m_pIStVideoFiler != NULL)
				{
					//Get timestamp value
					uint64_t nCurrentTimestampNs = 0;
					try
					{
						nCurrentTimestampNs = pIStStreamBuffer->GetIStStreamBufferInfo()->GetTimestampNS();
					}
					catch (...)
					{
						const uint64_t iCurrentTimestamp = pIStStreamBuffer->GetIStStreamBufferInfo()->GetTimestamp();

						//TODO:Timestamp
						nCurrentTimestampNs = iCurrentTimestamp;
					}

					if (m_isFirstFrame == false)
					{
						//estimate frame number from timestamp value.
						uint64_t nDelta = nCurrentTimestampNs - m_iTimestampOffset;
						double dblTmp = nDelta * m_dblCameraFrameRate;
						dblTmp /= 1000000000;

						nEstimatedFrameNo = (uint32_t)(dblTmp + 0.5);
					}

					//Add new frame to the avi file.
					m_pIStVideoFiler->RegisterIStImage(pIStStreamBuffer->GetIStImage(), nEstimatedFrameNo);

					//Returned true if AVI file already closed.
					m_pIStVideoFiler->IsStopped();
				}
			}

			const StApi::IStStreamBufferInfo *pIStStreamBufferInfo = pIStStreamBuffer->GetIStStreamBufferInfo();
			const uint64_t nFrameID = pIStStreamBufferInfo->GetFrameID();

			if (!m_isFirstFrame)
			{
				if (m_nLastID < nFrameID)
				{
					m_nDroppedIDCount += (nFrameID - m_nLastID - 1);
				}
			}
			m_nLastID = nFrameID;
		}
		else if(eStCallbackType  ==  StApi::StCallbackType_GenTLEvent_DataStreamError)
		{
			StApi::IStCallbackParamGenTLEventErrorDS *pIStCallbackParamGenTLEventErrorDS = dynamic_cast<StApi::IStCallbackParamGenTLEventErrorDS*>(pIStCallbackParamBase);
			OutputDebugString(GCSTRING_2_LPCTSTR(pIStCallbackParamGenTLEventErrorDS->GetDescription()));
		}
	}
	catch (const GenICam::GenericException &e)
	{
		CString msg = (CString)e.GetDescription();
	}

	m_isFirstFrame = false;
}

void CStCamera::OnDeviceLost(GenApi::INode *pINode, void*)
{

	if (GenApi::IsAvailable(pINode))
	{
		if (m_pIStDevice->IsDeviceLost())
		{
			m_isDeviceLost = true;
		}
	}
}

void CStCamera::OnStartAcquisition(uint64_t nFrame)
{
	try
	{
		m_nReceivedImageCount = 0;
		m_nDroppedIDCount = 0;
		m_nLastID = 0;
		m_isFirstFrame = true;

		m_pIStDataStream->StartAcquisition(nFrame);

		m_pIStDevice->AcquisitionStart();
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::OnStopAcquisition()
{
	try
	{
		if (m_pIStDevice != NULL)
			m_pIStDevice->AcquisitionStop();

		if (m_pIStDataStream != NULL)
			m_pIStDataStream->StopAcquisition();
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::OnAutoWhiteBalance(EAWB eMode)
{
	try
	{
		CString Name=_T("");
		switch (eMode)
		{
		case eAWOff :
			SetValueEnum("BalanceWhiteAuto", _T("Off"));
			break;
		case ePreset1 :
			SetValueEnum("BalanceWhiteAuto", _T("Preset1"));
			break;
		case ePreset2 :
			SetValueEnum("BalanceWhiteAuto", _T("Preset2"));
			break;
		case ePreset3 :
			SetValueEnum("BalanceWhiteAuto", _T("Preset3"));
			break;
		case eAWContinuous :
			SetValueEnum("BalanceWhiteAuto", _T("Continuous"));
			break;
		case eOnce :
			SetValueEnum("BalanceWhiteAuto", _T("Once"));
			break;
		}
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::OnSaveAWBValueOnceToPreset(int nPresetNum)
{
	CString strPreset=_T("");
	strPreset.Format(_T("Preset%d"),nPresetNum);

	char* char_str = NULL;
	try
	{
		for (int i=0; i<4; i++)
		{
			CString Parameter=_T("");
			char szNode[MAX_PATH] = {0, };
			int64_t value=0;

			switch (i)
			{
			case 0 : Parameter = _T("BalanceRatio_R_"); break;
			case 1 : Parameter = _T("BalanceRatio_Gr_"); break;
			case 2 : Parameter = _T("BalanceRatio_B_"); break;
			case 3 : Parameter = _T("BalanceRatio_Gb_"); break;
			}

			CString strNode = Parameter + _T("Once");
			//CString to char* on Unicode
			wchar_t* wchar_str = NULL;
			int char_str_len=0;

			wchar_str = strNode.GetBuffer(strNode.GetLength());
			char_str_len = WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, NULL, 0, NULL, NULL);
			char_str = new char[char_str_len];
			WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, char_str, char_str_len, 0,0);  

			GetValueInt(char_str, value);
			delete []char_str;
			char_str = NULL;

			strNode = Parameter+strPreset;

			wchar_str = strNode.GetBuffer(strNode.GetLength());
			char_str_len = WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, NULL, 0, NULL, NULL);
			char_str = new char[char_str_len];
			WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, char_str, char_str_len, 0,0);  

			SetValueInt(char_str, value);
			delete []char_str;
			char_str = NULL;
		}
	}
	catch (const GenICam::GenericException &e)
	{
		if (char_str != NULL)
		{
			delete []char_str;
			char_str = NULL;
		}
		throw e;
	}
}

void CStCamera::SetContinuousMode()
{
	try
	{
		SetTriggerMode(false);
		SetExposureMode(eTimed);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::SetSoftTriggerMode()
{
	try
	{
		SetTriggerMode(true);
		SetTriggerSource(eSoftware);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::SetHardTriggerMode()
{
	try
	{
		SetTriggerMode(true);
		SetTriggerSource(eHardware);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::OnTriggerEvent()
{
	OnExecuteCommand("TriggerSoftware");
}

void CStCamera::OnUserSetLoad()
{
	OnExecuteCommand("UserSetLoad");
}

void CStCamera::OnUserSetSave()
{
	OnExecuteCommand("UserSetSave");
}

void CStCamera::OnSaveImage(CString strPath, EFormat eFmt)
{
	try
	{
		// Create a still image file handling class object (filer) for still image processing.
		CIStStillImageFilerPtr pIStStillImageFiler = CreateIStFiler(StFilerType_StillImage);

		gcstring value(strPath);

		pIStStillImageFiler->Save(m_pIStImageBuff->GetIStImage(), StStillImageFileFormat_Bitmap, value);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::OnShowParameterWnd()
{
	try
	{
		// Create an NodeMap display window object.
		//CIStNodeMapDisplayWndPtr pIStNodeMapDisplayWnd(CreateIStWnd(StWindowType_NodeMapDisplay));
		m_pIStNodeMapDisplayWnd = CreateIStWnd(StWindowType_NodeMapDisplay);
		// Register the node to NodeMap window.
		m_pIStNodeMapDisplayWnd->RegisterINode(m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("Root"), "Root");

		// Set the position and size of the window.
		m_pIStNodeMapDisplayWnd->SetPosition(0, 0, 480, 640);

		// Display the window.
		m_pIStNodeMapDisplayWnd->Show(NULL, StWindowMode_ModalessOnNewThread);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

bool CStCamera::IsConnected()
{
	if (m_pIStDevice == NULL) 
		return false;

	return true;
}

bool CStCamera::IsActived()
{
	if (m_pIStDevice == NULL) 
		return false;

	if (m_pIStDataStream == NULL)
		return false;

	return m_pIStDataStream->IsGrabbing();
}

void CStCamera::GetUserID(CString &strValue)
{
	CString value=_T("");
	GetValueString("DeviceUserID", value);

	strValue = value;
}

void CStCamera::GetModelName(CString &strValue)
{
	CString value=_T("");
	GetValueString("DeviceModelName", value);

	strValue = value;
}

void CStCamera::GetSerialNumber(CString &strValue)
{
	CString value=_T("");
	GetValueString("DeviceSerialNumber", value);

	strValue = value;
}

void CStCamera::GetIPAddress(CString &strValue)
{
	int64_t value=0;
	GetValueInt("GevCurrentIPAddress", value);

	struct in_addr addr;
	addr.s_addr = htonl(value);
	char *pCh = inet_ntoa(addr);

	strValue = (CString)pCh;
}

void CStCamera::GetSubnetMask(CString &strValue)
{
	int64_t value=0;
	GetValueInt("GevCurrentSubnetMask", value);

	struct in_addr addr;
	addr.s_addr = htonl(value);
	char *pCh = inet_ntoa(addr);

	strValue = (CString)pCh;
}

void CStCamera::GetWidth(int &nValue)
{
	int64_t value=0;
	GetValueInt("Width", value);

	nValue = (int)value;
}

void CStCamera::GetHeight(int &nValue)
{
	int64_t value=0;
	GetValueInt("Height", value);
	
	nValue = (int)value;
}

void CStCamera::GetBpp(int &nValue)
{
	try
	{
		GenApi::CNodeMapPtr pINodeMapRemote(m_pIStDevice->GetRemoteIStPort()->GetINodeMap());
		EStPixelFormatNamingConverntion_t nPFNC;
		IStPixelFormatInfo *pIStPixelFormatInfo = NULL;

		// GigE / U3V Interface.
		try
		{
			nPFNC = (EStPixelFormatNamingConverntion_t)dynamic_cast<GenApi::IEnumeration*>(pINodeMapRemote->GetNode("PixelFormat"))->GetIntValue();
			pIStPixelFormatInfo = GetIStPixelFormatInfo(nPFNC);

			size_t value = pIStPixelFormatInfo->GetEachComponentTotalBitCount();

			nValue = (int)value;
		}
		// CXP Interface.
		// CXP Interface 는 PFNC(Pixel Format Naming Conversion)가 대응되지 않으므로 catch 에서 따로 처리해야 한다.
		catch (const GenICam::GenericException &e)
		{
			CString strPxlFmt=_T("");
			GetValueEnum("PixelFormat", strPxlFmt);

			strPxlFmt.MakeUpper();
			if (strPxlFmt.Find(_T("8")) != -1)
				nValue = 8;
			else if (strPxlFmt.Find(_T("24")) != -1)
				nValue = 24;
		}
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::GetBpp(CString &strValue)
{
	CString value=_T("");
	GetValueEnum("PixelFormat", value);

	strValue = value;
}

void CStCamera::SetDeviceUserID(CString strValue)
{
	SetValueString("DeviceUserID", strValue);
}

void CStCamera::SetIPAddress(CString strValue)
{
	try
	{
		//Retrieves the node from the central map by Name
		GenApi::CNodePtr pINode(m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("GevCurrentIPConfigurationPersistentIP"));

		//SmartPointer for IBoolean interface pointer
		GenApi::CBooleanPtr pIBoolean(pINode);

		//Set node value
		pIBoolean->SetValue(true);

		//CString to char* //Unicode
		wchar_t* wchar_str;     
		char*    char_str;      
		int      char_str_len;  
		wchar_str = strValue.GetBuffer(strValue.GetLength());

		char_str_len = WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, NULL, 0, NULL, NULL);
		char_str = new char[char_str_len];
		WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, char_str, char_str_len, 0,0);  

		int64_t nAddr = ntohl(inet_addr(char_str));

		delete []char_str;
		char_str = NULL;

		GenApi::CIntegerPtr pInt = NULL;
		pInt = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("GevPersistentIPAddress");

		pInt->SetValue(nAddr);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::SetSubnetMask(CString strValue)
{
	try
	{
		//Retrieves the node from the central map by Name
		GenApi::CNodePtr pINode(m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("GevCurrentIPConfigurationPersistentIP"));

		//SmartPointer for IBoolean interface pointer
		GenApi::CBooleanPtr pIBoolean(pINode);

		//Set node value
		pIBoolean->SetValue(true);

		//CString to char* //Unicode
		wchar_t* wchar_str;     
		char*    char_str;      
		int      char_str_len;  
		wchar_str = strValue.GetBuffer(strValue.GetLength());

		char_str_len = WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, NULL, 0, NULL, NULL);
		char_str = new char[char_str_len];
		WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, char_str, char_str_len, 0,0);  

		int64_t nAddr = ntohl(inet_addr(char_str));

		delete []char_str;
		char_str = NULL;

		GenApi::CIntegerPtr pInt = NULL;
		pInt = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode("GevPersistentSubnetMask");

		pInt->SetValue(nAddr);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::SetWidth(int nValue)
{
	SetValueInt("Width", nValue);
}

void CStCamera::SetHeight(int nValue)
{
	SetValueInt("Height", nValue);
}

void CStCamera::SetOffsetX(int nValue)
{
	SetValueInt("OffsetX", nValue);
}

void CStCamera::SetOffsetY(int nValue)
{
	SetValueInt("OffsetY", nValue);
}

void CStCamera::SetAcquisitionFrameRate(double dValue)
{
	SetValueDouble("AcquisitionFrameRate", dValue);
}

void CStCamera::SetAcquisitionMode(EAcqMode eMode)
{
	CString value=_T("");
	switch (eMode)
	{
		case eContinuous : value = _T("Continuous"); break;
		case eSingleFrame : value = _T("SingleFrame"); break;
		case eMultiFrame : value = _T("MultiFrame"); break;
	}
	
	SetValueEnum("AcquisitionMode", value);
}

void CStCamera::SetTriggerMode(bool bEnable)
{
	CString value=_T("");
	
	if (bEnable == true)
		value = _T("On");
	else
		value = _T("Off");

	SetValueEnum("TriggerMode", value);
}

void CStCamera::SetTriggerSource(ETrgSrc eSrc)
{
	CString value=_T("");
	switch (eSrc)
	{
		case eSoftware : value = _T("Software"); break;
		case eHardware : value = _T("Hardware"); break;
	}

	SetValueEnum("TriggerSource", value);
}

void CStCamera::SetTriggerOverlap(ETrgOvl eOvl)
{
	CString value=_T("");
	switch (eOvl)
	{
		case eOff : value = _T("Off"); break;
		case eReadOut : value = _T("ReadOut"); break;
		case ePreviousFrame : value = _T("PreviousFrame"); break;
	}

	SetValueEnum("TriggerOverlap", value);
}

void CStCamera::SetExposureMode(EExpMode eMode)
{
	CString value=_T("");
	switch (eMode)
	{
		case eTimed : value = _T("Timed"); break;
		case eTriggerWidth : value = _T("TriggerWidth"); break;
	}

	SetValueEnum("ExposureMode", value);
}

void CStCamera::SetExposureTime(double dValue)
{
	SetValueDouble("ExposureTime", dValue);
}

void CStCamera::SetUserSetSelector(EUserSet eUser)
{
	CString value=_T("");
	switch (eUser)
	{
		case eDefault : value = _T("Default"); break;
		case eUserSet1 : value = _T("UserSet1"); break;
	}

	SetValueEnum("UserSetSelector", value);
}

void CStCamera::SetUserSetDefaultSelector(EUserSet eUser)
{
	CString value=_T("");
	switch (eUser)
	{
	case eDefault : value = _T("Default"); break;
	case eUserSet1 : value = _T("UserSet1"); break;
	}

	SetValueEnum("UserSetDefaultSelector", value);
}

void CStCamera::GetValueString(char* pNodeName, CString &strValue)
{
	try
	{
		std::string Value;
		GenApi::CStringPtr pNode = NULL;
		pNode = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode(pNodeName);
		Value = pNode->GetValue();

		strValue = (CString)Value.c_str();
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::SetValueString(char* pNodeName, CString strValue)
{
	try
	{
		gcstring Value(strValue);

		GenApi::CStringPtr pNode = NULL;
		pNode = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode(pNodeName);
		pNode->SetValue(Value);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::GetValueInt(char* pNodeName, int64_t &nValue)
{
	try
	{
		GenApi::CIntegerPtr pNode = NULL;
		pNode = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode(pNodeName);
		
		nValue = pNode->GetValue();
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::SetValueInt(char* pNodeName, int64_t nValue)
{
	try
	{
		GenApi::CIntegerPtr pNode = NULL;
		pNode = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode(pNodeName);

		pNode->SetValue(nValue);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::GetValueDouble(char* pNodeName, double &dValue)
{
	try
	{
		GenApi::CFloatPtr pNode = NULL;
		pNode = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode(pNodeName);

		dValue = pNode->GetValue();
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::SetValueDouble(char* pNodeName, double dValue)
{
	try
	{
		GenApi::CFloatPtr pNode = NULL;
		pNode = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode(pNodeName);

		pNode->SetValue(dValue);
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::GetValueEnum(char* pNodeName, CString &strValue)
{
	try
	{
		GenApi::CEnumerationPtr pNode = NULL;
		pNode = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode(pNodeName);
		
		// Get the setting list.
		GenApi::NodeList_t sNodeList;
		pNode->GetEntries(sNodeList);
		for (size_t i = 0; i < sNodeList.size(); i++)
		{
			if (IsAvailable(sNodeList[i]))
			{
				GenApi::CEnumEntryPtr pIEnumEntry(sNodeList[i]);
				if (pNode->GetIntValue() == pIEnumEntry->GetValue())
				{
					strValue = (CString)pIEnumEntry->GetSymbolic().c_str();
					break;
				}
			}
		}
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::SetValueEnum(char* pNodeName, CString strValue)
{
	try
	{
		GenApi::CEnumerationPtr pNode = NULL;
		pNode = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode(pNodeName);
		if (IsWritable(pNode) == false) return;

		// Get the setting list.
		GenApi::NodeList_t sNodeList;
		pNode->GetEntries(sNodeList);
		for (size_t i = 0; i < sNodeList.size(); i++)
		{
			if (IsAvailable(sNodeList[i]))
			{
				GenApi::CEnumEntryPtr pIEnumEntry(sNodeList[i]);
				if (strValue == (CString)pIEnumEntry->GetSymbolic().c_str())
				{
					pNode->SetIntValue(pIEnumEntry->GetValue());
					break;
				}
			}
		}
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}

void CStCamera::OnExecuteCommand(char* pNodeName)
{
	try
	{
		GenApi::CCommandPtr pNode = NULL;
		pNode = m_pIStDevice->GetRemoteIStPort()->GetINodeMap()->GetNode(pNodeName);
		pNode->Execute();
	}
	catch (const GenICam::GenericException &e)
	{
		throw e;
	}
}