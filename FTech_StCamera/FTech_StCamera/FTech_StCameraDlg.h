
// FTech_StCameraDlg.h : header file
//

#pragma once

#include "include/StCamera.h"

using namespace SENTECH_SDK;
// CFTech_StCameraDlg dialog

#define MAX_CAM 2

typedef struct {
	bool bActived;
	bool bConnected;
	bool bRecorded;
	bool bColor;
} StCamInfo;

class CFTech_StCameraDlg : public CDialogEx
{
// Construction
public:
	CFTech_StCameraDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FTECH_STCAMERA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public :
	CStCamera m_Camera[MAX_CAM];
	StCamInfo m_CamInfo[MAX_CAM];
	CWinThread* m_pThDsp[MAX_CAM];
	BITMAPINFO*	m_pBitmapInfo[MAX_CAM];
	int m_nWidth[MAX_CAM];
	int m_nHeight[MAX_CAM];
	int m_rbtnCam;
	bool m_bThDsp[MAX_CAM];
	void OnDisplayImageCam1();
	void OnDisplayImageCam2();
	void CreateBmpInfo(int idxCam, int nWidth, int nHeight, int nBpp);
	
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnConnection();
	afx_msg void OnBnClickedBtnAcq();
	afx_msg void OnBnClickedBtnImgSave();
	afx_msg void OnBnClickedBtnShowparam();
	afx_msg void OnBnClickedBtnRecording();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnAwb();
	afx_msg void OnBnClickedBtnWbsave();
	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedRbtnSelCam(UINT value);
	
};
