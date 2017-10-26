
// FTech_StCameraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FTech_StCamera.h"
#include "FTech_StCameraDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFTech_StCameraDlg dialog


UINT DisplayThreadCam1(LPVOID param)
{
	CFTech_StCameraDlg* pMain = (CFTech_StCameraDlg*)param;

	DWORD dwResult=0;
	DWORD lPrevious = 0;
	while(pMain->m_bThDsp[0])
	{
		if (pMain->m_Camera[0].IsConnected() == true)
		{
			if (pMain->m_Camera[0].IsActived() == true)
			{
				pMain->m_Camera[0].OnResetHandleGrabDone();
				dwResult = WaitForSingleObject(pMain->m_Camera[0].GetHandleGrabDone(),1000);
				if (dwResult == WAIT_OBJECT_0)
				{
					// limit refresh rate to ~ 30 fps max
					DWORD lCurrent = ::GetTickCount();
					long lDelta = ( lCurrent - lPrevious ) - ( 1000 / 30 );
					if ( lDelta > 0 )
					{
						lPrevious = ::GetTickCount();

						pMain->OnDisplayImageCam1();
					}
				}
			}
		}
	}

	return 0;
}

UINT DisplayThreadCam2(LPVOID param)
{
	CFTech_StCameraDlg* pMain = (CFTech_StCameraDlg*)param;

	DWORD dwResult=0;
	DWORD lPrevious = 0;
	while(pMain->m_bThDsp[1])
	{
		if (pMain->m_Camera[1].IsConnected() == true)
		{
			if (pMain->m_Camera[1].IsActived() == true)
			{
				{
					pMain->m_Camera[1].OnResetHandleGrabDone();
					dwResult = WaitForSingleObject(pMain->m_Camera[1].GetHandleGrabDone(),1000);
					if (dwResult == WAIT_OBJECT_0)
					{
						// limit refresh rate to ~ 30 fps max
						DWORD lCurrent = ::GetTickCount();
						long lDelta = ( lCurrent - lPrevious ) - ( 1000 / 30 );
						if ( lDelta > 0 )
						{
							lPrevious = ::GetTickCount();

							pMain->OnDisplayImageCam2();
						}
					}
				}
			}
		}
	}

	return 0;
}

CFTech_StCameraDlg::CFTech_StCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFTech_StCameraDlg::IDD, pParent)
	, m_rbtnCam(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	for (int i=0; i<MAX_CAM; i++)
	{
		m_nWidth[i] = 0;
		m_nHeight[i] = 0;
		m_bThDsp[i] = false;
		m_pThDsp[i] = NULL;
		m_pBitmapInfo[i] = NULL;

		m_CamInfo[i].bActived = false;
		m_CamInfo[i].bConnected = false;
		m_CamInfo[i].bRecorded = false;
		m_CamInfo[i].bColor = false;
	}
}

void CFTech_StCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RBTN_CAM1, (int &)m_rbtnCam);
}

BEGIN_MESSAGE_MAP(CFTech_StCameraDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CONNECTION, &CFTech_StCameraDlg::OnBnClickedBtnConnection)
	ON_BN_CLICKED(IDC_BTN_ACQ, &CFTech_StCameraDlg::OnBnClickedBtnAcq)
	ON_BN_CLICKED(IDC_BTN_IMGSAVE, &CFTech_StCameraDlg::OnBnClickedBtnImgSave)
	ON_BN_CLICKED(IDC_BTN_SHOWPARAM, &CFTech_StCameraDlg::OnBnClickedBtnShowparam)
	ON_BN_CLICKED(IDC_BTN_RECORDING, &CFTech_StCameraDlg::OnBnClickedBtnRecording)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_AWB, &CFTech_StCameraDlg::OnBnClickedBtnAwb)
	ON_BN_CLICKED(IDC_BTN_WBSAVE, &CFTech_StCameraDlg::OnBnClickedBtnWbsave)
	ON_BN_CLICKED(IDC_BTN_REFRESH, &CFTech_StCameraDlg::OnBnClickedBtnRefresh)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RBTN_CAM1, IDC_RBTN_CAM2, &CFTech_StCameraDlg::OnBnClickedRbtnSelCam)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CFTech_StCameraDlg message handlers

BOOL CFTech_StCameraDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_WBPRESET);
	pCB->AddString(_T("Preset 1"));
	pCB->AddString(_T("Preset 2"));
	pCB->AddString(_T("Preset 3"));

	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LTC_SYSTEM);
	pList->SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	pList->DeleteAllItems();
	pList->InsertColumn(0, _T(""), LVCFMT_LEFT, 0, -1);
	pList->InsertColumn(1, _T("System"), LVCFMT_CENTER, 100, -1);
	pList->InsertColumn(2, _T("Interface"), LVCFMT_CENTER, 110, -1);
	pList->InsertColumn(3, _T("Model"), LVCFMT_CENTER, 110, -1); 
	pList->InsertColumn(4, _T("S/N"), LVCFMT_CENTER, 80, -1);
	pList->InsertColumn(5, _T("Access"), LVCFMT_CENTER, 80, -1);

	CheckRadioButton(IDC_RBTN_CAM1, IDC_RBTN_CAM2, IDC_RBTN_CAM1);

	SetTimer(100, 300, NULL);

	for (int i=0; i<MAX_CAM; i++)
	{
		m_bThDsp[i] = true;

		switch (i)
		{
		case 0 :
			m_pThDsp[m_rbtnCam] = AfxBeginThread(DisplayThreadCam1, this);
			break;
		case 1 :
			m_pThDsp[m_rbtnCam] = AfxBeginThread(DisplayThreadCam2, this);
			break;
		}
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFTech_StCameraDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFTech_StCameraDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFTech_StCameraDlg::OnBnClickedBtnConnection()
{
	CString strCaption=_T("");
	GetDlgItemText(IDC_BTN_CONNECTION, strCaption);

	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LTC_SYSTEM);
	POSITION pos = pList->GetFirstSelectedItemPosition();

	int nItem=-1;
	bool ret = false;
	if (strCaption == _T("Connect"))
	{
		while (pos)
		{
			nItem = pList->GetNextSelectedItem(pos);
		}

		if (nItem == -1) return;
	}

	CString strSN = pList->GetItemText(nItem, 4);
	try
	{
		if (strCaption == _T("Connect"))
		{
			m_Camera[m_rbtnCam].OnConnect(strSN, eSeiralNumber);

			int w=0, h=0, bpp=0;
			CString strBpp=_T(""), strModel=_T("");

			m_Camera[m_rbtnCam].GetWidth(w);
			m_Camera[m_rbtnCam].GetHeight(h);
			m_Camera[m_rbtnCam].GetBpp(bpp);
			m_Camera[m_rbtnCam].GetBpp(strBpp);
			m_Camera[m_rbtnCam].GetModelName(strModel);
			
			if (m_rbtnCam == 0)
			{
				SetDlgItemInt(IDC_LB_WIDTH1, w);
				SetDlgItemInt(IDC_LB_HEIGHT1, h);
				SetDlgItemText(IDC_LB_BPP1, strBpp);
				SetDlgItemText(IDC_LB_MODEL1, strModel);
			}
			else if (m_rbtnCam == 1)
			{
				SetDlgItemInt(IDC_LB_WIDTH2, w);
				SetDlgItemInt(IDC_LB_HEIGHT2, h);
				SetDlgItemText(IDC_LB_BPP2, strBpp);
				SetDlgItemText(IDC_LB_MODEL2, strModel);
			}

			strBpp.MakeUpper();
			if (strBpp.Find(_T("BAYER")) != -1)
			{
				bpp = 24;
				m_CamInfo[m_rbtnCam].bColor = true;

				GetDlgItem(IDC_BTN_AWB)->EnableWindow(TRUE);
				GetDlgItem(IDC_CB_WBPRESET)->EnableWindow(TRUE);
				GetDlgItem(IDC_BTN_WBSAVE)->EnableWindow(TRUE);
			}

			m_nWidth[m_rbtnCam] = w;
			m_nHeight[m_rbtnCam] = h;
			CreateBmpInfo(m_rbtnCam, w, h, bpp);

			SetDlgItemText(IDC_BTN_CONNECTION, _T("Disconnect"));

			m_CamInfo[m_rbtnCam].bConnected = true;
		}
		else
		{
			if (m_Camera[m_rbtnCam].IsActived() == true)
				m_Camera[m_rbtnCam].OnStopAcquisition();

			m_Camera[m_rbtnCam].OnDisconnect();

			m_CamInfo[m_rbtnCam].bActived = false;
			m_CamInfo[m_rbtnCam].bConnected = false;
			m_CamInfo[m_rbtnCam].bColor = false;

			SetDlgItemText(IDC_LB_WIDTH1, _T(""));
			SetDlgItemText(IDC_LB_HEIGHT1, _T(""));
			SetDlgItemText(IDC_LB_BPP1, _T(""));
			SetDlgItemText(IDC_LB_MODEL1, _T(""));
			SetDlgItemText(IDC_BTN_CONNECTION, _T("Connect"));
		}
	}
	catch (ST_EXCEPTION &e)
	{
		CString strMsg = (CString)e.GetDescription();
	}
}

void CFTech_StCameraDlg::OnBnClickedBtnAcq()
{
	try
	{
		if (m_Camera[m_rbtnCam].IsConnected() == false) return;

		CString strCaption=_T("");
		GetDlgItemText(IDC_BTN_ACQ, strCaption);

		if (strCaption == _T("Start"))
		{
			m_Camera[m_rbtnCam].OnStartAcquisition();

			SetDlgItemText(IDC_BTN_ACQ, _T("Stop"));

			m_CamInfo[m_rbtnCam].bActived = true;
		}
		else
		{
			m_Camera[m_rbtnCam].OnStopAcquisition();
		
			SetDlgItemText(IDC_BTN_ACQ, _T("Start"));

			m_CamInfo[m_rbtnCam].bActived = false;
		}
	}
	catch (ST_EXCEPTION &e)
	{
		CString strMsg = (CString)e.GetDescription();
	}
}

void CFTech_StCameraDlg::OnBnClickedBtnImgSave()
{
	try
	{
		CFileDialog dlg(FALSE, _T("bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("BMP File(*.bmp)|*.bmp|All Files(*.*)|*.*||"));
		if(dlg.DoModal() !=  IDOK) return;

		CString path = dlg.m_ofn.lpstrFile;

		m_Camera[m_rbtnCam].OnSaveImage(path, eBMP);
	}
	catch (ST_EXCEPTION &e)
	{
		CString msg = (CString)e.GetDescription();
	}
}

void CFTech_StCameraDlg::OnBnClickedBtnShowparam()
{
	try
	{
		m_Camera[m_rbtnCam].OnShowParameterWnd();
	}
	catch (ST_EXCEPTION &e)
	{
		CString msg = (CString)e.GetDescription();
	}
}

void CFTech_StCameraDlg::OnBnClickedBtnRecording()
{
	try
	{
		if (m_Camera[m_rbtnCam].IsConnected() == false) return;

		CString strCaption=_T("");
		GetDlgItemText(IDC_BTN_RECORDING, strCaption);

		if (strCaption == _T("Start"))
		{
			CFileDialog dlg(FALSE, _T("avi"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("AVI File(*.avi)|*.avi|All Files(*.*)|*.*||"));
			if(dlg.DoModal() !=  IDOK) return;

			CString path = dlg.m_ofn.lpstrFile;

			m_Camera[m_rbtnCam].OnStartRecording(path);

			SetDlgItemText(IDC_BTN_RECORDING, _T("Stop"));

			m_CamInfo[m_rbtnCam].bRecorded = true;
		}
		else
		{
			m_Camera[m_rbtnCam].OnStopRecording();

			SetDlgItemText(IDC_BTN_RECORDING, _T("Start"));

			m_CamInfo[m_rbtnCam].bRecorded = false;
		}
	}
	catch (ST_EXCEPTION &e)
	{
		CString strMsg = (CString)e.GetDescription();
	}
}

void CFTech_StCameraDlg::CreateBmpInfo(int idxCam, int nWidth, int nHeight, int nBpp)
{
	if (m_pBitmapInfo[idxCam] != NULL) 
	{
		delete []m_pBitmapInfo[idxCam];
		m_pBitmapInfo[idxCam] = NULL;
	}

	if (nBpp == 8)
		m_pBitmapInfo[idxCam] = (BITMAPINFO *) new BYTE[sizeof(BITMAPINFO) + 255*sizeof(RGBQUAD)];
	else if (nBpp == 24)
		m_pBitmapInfo[idxCam] = (BITMAPINFO *) new BYTE[sizeof(BITMAPINFO)];

	m_pBitmapInfo[idxCam]->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBitmapInfo[idxCam]->bmiHeader.biPlanes = 1;
	m_pBitmapInfo[idxCam]->bmiHeader.biBitCount = nBpp;
	m_pBitmapInfo[idxCam]->bmiHeader.biCompression = BI_RGB;

	if (nBpp == 8)
		m_pBitmapInfo[idxCam]->bmiHeader.biSizeImage = 0;
	else if (nBpp == 24)
		m_pBitmapInfo[idxCam]->bmiHeader.biSizeImage = (((nWidth * 24 + 31) & ~31) >> 3) * nHeight;

	m_pBitmapInfo[idxCam]->bmiHeader.biXPelsPerMeter = 0;
	m_pBitmapInfo[idxCam]->bmiHeader.biYPelsPerMeter = 0;
	m_pBitmapInfo[idxCam]->bmiHeader.biClrUsed = 0;
	m_pBitmapInfo[idxCam]->bmiHeader.biClrImportant = 0;

	if (nBpp == 8)
	{
		for (int i = 0 ; i < 256 ; i++)
		{
			m_pBitmapInfo[idxCam]->bmiColors[i].rgbBlue = (BYTE)i;
			m_pBitmapInfo[idxCam]->bmiColors[i].rgbGreen = (BYTE)i;
			m_pBitmapInfo[idxCam]->bmiColors[i].rgbRed = (BYTE)i;
			m_pBitmapInfo[idxCam]->bmiColors[i].rgbReserved = 0;
		}
	}

	m_pBitmapInfo[idxCam]->bmiHeader.biWidth = nWidth;
	m_pBitmapInfo[idxCam]->bmiHeader.biHeight = -nHeight;
}

// Double Buffering. It has a some delay.
void CFTech_StCameraDlg::OnDisplayImageCam1()
{
	CClientDC dc(GetDlgItem(IDC_PC_CAMERA1));
	CRect rect;
	GetDlgItem(IDC_PC_CAMERA1)->GetClientRect(&rect);

	
	CDC memDC;
	CBitmap *pOldBitmap, bitmap;

	memDC.CreateCompatibleDC(&dc);

	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());

	pOldBitmap = memDC.SelectObject(&bitmap);
	memDC.PatBlt(0, 0, rect.Width(), rect.Height(), SRCCOPY);

	memDC.SetStretchBltMode(COLORONCOLOR); 

	BYTE *pBuffer = m_Camera[0].GetImageBuffer();
	StretchDIBits(memDC.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, m_nWidth[0], m_nHeight[0], pBuffer, m_pBitmapInfo[0], DIB_RGB_COLORS, SRCCOPY);

	if (IsDlgButtonChecked(IDC_CHK_CROSS1) == 1)
	{
		CPen pen, *pOldPen;
		pen.CreatePen(PS_SOLID,1,RGB(255,0,0));
		pOldPen = (CPen*)memDC.SelectObject(&pen);

		memDC.SetBkMode(TRANSPARENT);
		memDC.MoveTo(rect.Width()/2, 0);
		memDC.LineTo(rect.Width()/2, rect.Height());

		memDC.MoveTo(0, rect.Height()/2);
		memDC.LineTo(rect.Width(), rect.Height()/2);

		memDC.SelectObject(pOldPen);
		pen.DeleteObject();
	}

	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
	bitmap.DeleteObject();
}

// Non-Double Buffering.
void CFTech_StCameraDlg::OnDisplayImageCam2()
{
	CClientDC dc(GetDlgItem(IDC_PC_CAMERA2));
	CRect rect;
	GetDlgItem(IDC_PC_CAMERA2)->GetClientRect(&rect);

	dc.SetStretchBltMode(COLORONCOLOR); 

	BYTE *pBuffer = m_Camera[1].GetImageBuffer();
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, m_nWidth[1], m_nHeight[1], pBuffer, m_pBitmapInfo[1], DIB_RGB_COLORS, SRCCOPY);

	if (IsDlgButtonChecked(IDC_CHK_CROSS2) == 1)
	{
		CPen pen, *pOldPen;
		pen.CreatePen(PS_SOLID,1,RGB(255,0,0));
		pOldPen = (CPen*)dc.SelectObject(&pen);

		dc.SetBkMode(TRANSPARENT);
		dc.MoveTo(rect.Width()/2, 0);
		dc.LineTo(rect.Width()/2, rect.Height());

		dc.MoveTo(0, rect.Height()/2);
		dc.LineTo(rect.Width(), rect.Height()/2);

		dc.SelectObject(pOldPen);
		pen.DeleteObject();
	}
}

void CFTech_StCameraDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	KillTimer(100);

	for (int i=0; i<MAX_CAM; i++)
	{
		m_bThDsp[i] = false;

		if (m_pThDsp[i] != NULL)
			WaitForSingleObject(m_pThDsp[i]->m_hThread, 5000);

		if (m_Camera[i].IsConnected() == true)
		{
			if (m_Camera[i].IsActived() == true)
				m_Camera[i].OnStopAcquisition();

			m_Camera[i].OnDisconnect();
		}

		if (m_pBitmapInfo[i] != NULL)
		{
			delete []m_pBitmapInfo[i];
			m_pBitmapInfo[i] = NULL;
		}
	}
}

void CFTech_StCameraDlg::OnBnClickedBtnAwb()
{
	try
	{
		m_Camera[m_rbtnCam].OnAutoWhiteBalance(eOnce);
	}
	catch (ST_EXCEPTION &e)
	{
		CString msg = (CString)e.GetDescription();
	}
}


void CFTech_StCameraDlg::OnBnClickedBtnWbsave()
{
	CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_CB_WBPRESET);
	CString strPreset=_T("");
	pCB->GetLBText(pCB->GetCurSel(),strPreset);
	
	int nPreset = _ttoi(strPreset.Right(1));
	try
	{
		m_Camera[m_rbtnCam].OnSaveAWBValueOnceToPreset(nPreset);
	}
	catch (ST_EXCEPTION &e)
	{
		CString msg = (CString)e.GetDescription();
	}
}


void CFTech_StCameraDlg::OnBnClickedBtnRefresh()
{
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LTC_SYSTEM);
	pList->DeleteAllItems();

	BeginWaitCursor();

	bool bRet = CStCamera::OnSearchSystem();
	if (bRet == true)
	{
		int nSyCnt=0;
		bRet = CStCamera::GetSystemCount(nSyCnt);
		if (bRet = true)
		{
			for (int i=0; i<nSyCnt; i++)
			{
				int nIfCnt=0;
				bRet = CStCamera::GetInterfaceCount(i, nIfCnt);
				if (bRet == true)
				{
					for (int j=0; j<nIfCnt; j++)
					{
						int nDvCnt=0;
						bRet = CStCamera::GetDeviceCount(i, j, nDvCnt);
						if (bRet == true)
						{
							for (int k=0; k<nDvCnt; k++)
							{
								CString strSy=_T(""), strIf=_T(""),strMd=_T(""), strSN=_T(""), strAc=_T("");

								CStCamera::GetSystemName(i, strSy);
								CStCamera::GetInterfaceName(i, j, strIf);
								CStCamera::GetDeviceName(i, j, k, strMd);
								CStCamera::GetDeviceSN(i, j, k, strSN);
								CStCamera::GetDeviceAccessStatus(i, j, k, strAc);

								strAc = strAc.Right(strAc.GetLength() - 21);

								pList->InsertItem(i, _T(""));
								pList->SetItem(i, 1, LVIF_TEXT, strSy, 0, 0, 0, NULL );
								pList->SetItem(i, 2, LVIF_TEXT, strIf, 0, 0, 0, NULL );
								pList->SetItem(i, 3, LVIF_TEXT, strMd, 0, 0, 0, NULL );
								pList->SetItem(i, 4, LVIF_TEXT, strSN, 0, 0, 0, NULL );
								pList->SetItem(i, 5, LVIF_TEXT, strAc, 0, 0, 0, NULL );
							}
						}
					}
				}
			}
		}
	}
	EndWaitCursor();
}


void CFTech_StCameraDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 100)
	{
		for (int i=0; i<MAX_CAM; i++)
		{
			if (m_Camera[i].IsConnected() == false)
				return;

			double time = m_Camera[i].GetGrabTactTimeMS();
			uint64_t total = m_Camera[i].GetTotalImageCount();
			uint64_t drop = m_Camera[i].GetDroppedImageCount();

			CString strInfo=_T("");
			double dFps = (1/time)*1000;
			strInfo.Format(_T("%.3f fps / %d / %d (drop / total)"), dFps, drop, total);

			switch (i)
			{
			case 0 :
				SetDlgItemText(IDC_LB_INFO1, strInfo);
				break;
			case 1 :
				SetDlgItemText(IDC_LB_INFO2, strInfo);
				break;
			}
			
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CFTech_StCameraDlg::OnBnClickedRbtnSelCam(UINT value)
{ 
	UpdateData(TRUE);

	m_CamInfo[m_rbtnCam].bConnected ? SetDlgItemText(IDC_BTN_CONNECTION, _T("Disconnect")) : SetDlgItemText(IDC_BTN_CONNECTION, _T("Connect"));
	m_CamInfo[m_rbtnCam].bRecorded ? SetDlgItemText(IDC_BTN_RECORDING, _T("Stop")) : SetDlgItemText(IDC_BTN_RECORDING, _T("Start"));
	m_CamInfo[m_rbtnCam].bActived ? SetDlgItemText(IDC_BTN_ACQ, _T("Stop")) : SetDlgItemText(IDC_BTN_ACQ, _T("Start"));
	
	GetDlgItem(IDC_BTN_AWB)->EnableWindow(m_CamInfo[m_rbtnCam].bColor);
	GetDlgItem(IDC_CB_WBPRESET)->EnableWindow(m_CamInfo[m_rbtnCam].bColor);
	GetDlgItem(IDC_BTN_WBSAVE)->EnableWindow(m_CamInfo[m_rbtnCam].bColor);
}
