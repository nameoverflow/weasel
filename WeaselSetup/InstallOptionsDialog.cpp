#include "stdafx.h"
#include "InstallOptionsDialog.h"

int uninstall(bool silent);

std::map<InstallRegion, int> InstallOptionsDialog::region2radio = {
	{ InstallRegion::CN, IDC_RADIO_CN },
	{ InstallRegion::TW, IDC_RADIO_TW },
	{ InstallRegion::SG, IDC_RADIO_SG },
	{ InstallRegion::MO, IDC_RADIO_MO },
	{ InstallRegion::HK, IDC_RADIO_HK },
};
std::map<int, InstallRegion> InstallOptionsDialog::radio2region = {
	{ IDC_RADIO_CN, InstallRegion::CN },
	{ IDC_RADIO_TW, InstallRegion::TW },
	{ IDC_RADIO_SG, InstallRegion::SG },
	{ IDC_RADIO_MO, InstallRegion::MO },
	{ IDC_RADIO_HK, InstallRegion::HK },
};

InstallOptionsDialog::InstallOptionsDialog()
	: installed(false), region(InstallRegion::CN), user_dir()
{
}

InstallOptionsDialog::~InstallOptionsDialog()
{
}

LRESULT InstallOptionsDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	for (auto r : region2radio) {
		region_btns[r.first].Attach(GetDlgItem(r.second));
		region_btns[r.first].EnableWindow(!installed);
	}
	//cn_.Attach(GetDlgItem(IDC_RADIO_CN));
	//tw_.Attach(GetDlgItem(IDC_RADIO_TW));
	remove_.Attach(GetDlgItem(IDC_REMOVE));
	default_dir_.Attach(GetDlgItem(IDC_RADIO_DEFAULT_DIR));
	custom_dir_.Attach(GetDlgItem(IDC_RADIO_CUSTOM_DIR));
	dir_.Attach(GetDlgItem(IDC_EDIT_DIR));

	CheckRadioButton(IDC_RADIO_CN, IDC_RADIO_SG, region2radio[region]);
	CheckRadioButton(IDC_RADIO_DEFAULT_DIR, IDC_RADIO_CUSTOM_DIR,
		(user_dir.empty() ? IDC_RADIO_DEFAULT_DIR : IDC_RADIO_CUSTOM_DIR));
	dir_.SetWindowTextW(user_dir.c_str());

	//cn_.EnableWindow(!installed);
	//tw_.EnableWindow(!installed);
	remove_.EnableWindow(installed);
	dir_.EnableWindow(user_dir.empty() ? FALSE : TRUE);

	CenterWindow();
	return 0;
}

LRESULT InstallOptionsDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&) {
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT InstallOptionsDialog::OnOK(WORD, WORD code, HWND, BOOL&) {
	//hant = (IsDlgButtonChecked(IDC_RADIO_TW) == BST_CHECKED);
	if (IsDlgButtonChecked(IDC_RADIO_CUSTOM_DIR) == BST_CHECKED) {
		CStringW text;
		dir_.GetWindowTextW(text);
		user_dir = text;
	}
	else {
		user_dir.clear();
	}
	EndDialog(IDOK);
	return 0;
}

LRESULT InstallOptionsDialog::OnRemove(WORD, WORD code, HWND, BOOL&) {
  const bool non_silent = false;
	uninstall(non_silent);
	installed = false;
	for (auto r : region2radio) {
		region_btns[r.first].EnableWindow(!installed);
	}
	//cn_.EnableWindow(!installed);
	//tw_.EnableWindow(!installed);
	remove_.EnableWindow(installed);
	return 0;
}

LRESULT InstallOptionsDialog::OnUseDefaultDir(WORD, WORD code, HWND, BOOL&) {
	dir_.EnableWindow(FALSE);
	return 0;
}

LRESULT InstallOptionsDialog::OnUseCustomDir(WORD, WORD code, HWND, BOOL&) {
	dir_.EnableWindow(TRUE);
	dir_.SetFocus();
	return 0;
}
