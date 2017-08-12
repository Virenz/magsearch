#include <Windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <commctrl.h>
#include <thread>

#include "resource.h"

wchar_t title[20] = L"磁力搜索器";											// 设置标题 

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void performActions(HWND hwnd);												// 获取单选框和编辑框进行对应的操作
template<class T>
int InitTreeControl(T *uidatas);											// 进行tree布局显示数据

HINSTANCE hgInst;
HWND m_tree;
HWND m_progress;
HTREEITEM Selected;

TV_ITEM tvi = { 0 };
wchar_t buf[256] = { 0 };

int WINAPI WinMain(HINSTANCE hThisApp, HINSTANCE hPrevApp, LPSTR lpCmd, int nShow)
{
	hgInst = hThisApp;
	HWND hdlg = CreateDialog(hThisApp, MAKEINTRESOURCE(IDD_SEARCH), NULL, (DLGPROC)DlgProc);
	
	m_progress = GetDlgItem(hdlg, IDC_PROGRESS);
	if (!hdlg)
	{
		return 0;
	}
	ShowWindow(hdlg, SW_SHOW);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_KEYDOWN && GetKeyState('A') && GetKeyState(VK_CONTROL))
		{
			Edit_SetSel(GetDlgItem(hdlg, IDC_SEARCH), 0, -1);
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);	
	}

	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		// 设置对话框的图标 
		SetWindowText(hDlg, title);
		//SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hgInst, MAKEINTRESOURCE(IDI_ICON1)));
		return 0;
	}
	case WM_SYSCOMMAND:
	{
		if (wParam == SC_CLOSE)
		{
			PostQuitMessage(0);//退出     
		}
		return 0;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
			Dlg_OnCommand(hDlg, IDOK, GetDlgItem(hDlg, IDOK), BN_CLICKED);
			break;
		case ID_COPY:
			Dlg_OnCommand(hDlg, ID_COPY, GetDlgItem(m_tree, ID_COPY), BN_CLICKED);
			break;
		}
		break;
	case WM_NOTIFY:
		{
			switch (LOWORD(wParam))
			{
			case IDC_SHOW:
				// if code == NM_CLICK - Single click on an item
				if (((LPNMHDR)lParam)->code == NM_RCLICK)
				{
					/*DWORD dwPos = GetMessagePos(); 
					POINT pt; 
					pt.x = LOWORD(dwPos); 
					pt.y = HIWORD(dwPos);
					ScreenToClient(m_tree, &pt);
					
					TVHITTESTINFO ht = { 0 }; 
					ht.pt = pt;
					ht.flags = TVHT_ONITEM; 
					HTREEITEM hItem = TreeView_HitTest(m_tree, &ht);

					TVITEM ti = { 0 }; 
					TCHAR buf[256] = { 0 };
					ti.mask = TVIF_HANDLE | TVIF_TEXT; 
					ti.cchTextMax = 256; 
					ti.pszText = buf; 
					ti.hItem = hItem;
					TreeView_GetItem(m_tree, &ti); 
					MessageBox(m_tree, buf, NULL, 0);*/
				
					tvi = { 0 };
					memset(&tvi, 0, sizeof(tvi));
					Selected = (HTREEITEM)SendDlgItemMessage(hDlg,
						IDC_SHOW, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)Selected);

					if (Selected == NULL)
					{
						MessageBox(hDlg, L"No Items in TreeView",
							L"Error", MB_OK | MB_ICONINFORMATION);
						break;
					}
					TreeView_EnsureVisible(hDlg, Selected);
					SendDlgItemMessage(hDlg, IDC_SHOW,
						TVM_SELECTITEM, TVGN_CARET, (LPARAM)Selected);
				
					
					tvi.mask = TVIF_HANDLE | TVIF_TEXT;
					tvi.cchTextMax = 256;
					tvi.pszText = buf;
					tvi.hItem = Selected;

					if (SendDlgItemMessage(hDlg, IDC_SHOW, TVM_GETITEM, TVGN_CARET, (LPARAM)&tvi))
					{
						/*MessageBox(hDlg, tvi.pszText,
						L"Example", MB_OK | MB_ICONINFORMATION);	*/
					}

					RECT rect;
					POINT pt;
					// 获取鼠标右击是的坐标     
					DWORD dwPos = GetMessagePos();
					pt.x = LOWORD(dwPos);
					pt.y = HIWORD(dwPos);
					//获取客户区域大小     
					GetClientRect(m_tree, &rect);
					//把屏幕坐标转为客户区坐标     
					ScreenToClient(m_tree, &pt);
					//判断点是否位于客户区域内     
					if (PtInRect(&rect, pt))
					{
						//加载菜单资源     
						HMENU hroot = LoadMenu((HINSTANCE)GetWindowLongPtr(m_tree, GWLP_HINSTANCE), MAKEINTRESOURCE(IDR_MENU));
						if (hroot)
						{
							// 获取第一个弹出菜单     
							HMENU hpop = GetSubMenu(hroot, 0);
							// 把客户区坐标还原为屏幕坐标     
							ClientToScreen(m_tree, &pt);
							//显示快捷菜单     
							TrackPopupMenu(hpop,
								TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
								pt.x,
								pt.y,
								0,
								hDlg,
								NULL);
							// 用完后要销毁菜单资源     
							DestroyMenu(hroot);
						}
					}
				}
			}
		}
		break;
	}
	//return DefWindowProc(hDlg, msg, wParam, lParam);
	return (INT_PTR)FALSE;
}

void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

	switch (id) {

	case IDCANCEL:
		EndDialog(hwnd, id);
		break;

	case ID_COPY:
		{
			//复制内容     
			//打开剪贴板     
			OpenClipboard(NULL);
			//清空剪贴板     
			EmptyClipboard();

			//分配内存     
			HGLOBAL hgl = GlobalAlloc(GMEM_MOVEABLE, 256 * sizeof(WCHAR));
			LPWSTR lpstrcpy = (LPWSTR)GlobalLock(hgl);
			memcpy(lpstrcpy, buf, 256 * sizeof(WCHAR));
			GlobalUnlock(hgl);
			SetClipboardData(CF_UNICODETEXT, lpstrcpy);
			//关闭剪贴板     
			CloseClipboard();
			break;
		}
	case IDOK:
		{
			std::thread action(performActions, hwnd);
			action.detach();
			//performActions(hwnd, fileName);
			break;
		}
	}
}

template<class T> 
int InitTreeControl(T *uidatas)
{
	auto infos = uidatas->getInfos();
	for (auto sp : infos)
	{
		TV_ITEM item;
		item.mask = TVIF_TEXT;
		item.cchTextMax = 10;
		item.pszText = StringToWchar_t(sp->getVirusName());

		TV_INSERTSTRUCT insert;
		insert.hParent = TVI_ROOT;
		insert.hInsertAfter = TVI_LAST;
		insert.item = item;

		Selected = TreeView_InsertItem(m_tree, &insert);
		for (std::multimap<std::string, std::string>::iterator iter = sp->getJsonsInfo().begin(); iter != sp->getJsonsInfo().end(); ++iter) //遍历json成员
		{
			std::string str;
			str.append(iter->first);
			str.append(" : ");
			str.append(iter->second);
			wchar_t * wszUtf8 = StringToWchar_t(str);

			TV_ITEM item1;
			item1.mask = TVIF_TEXT | TVIF_PARAM | TVS_HASLINES | TVS_LINESATROOT;
			item1.cchTextMax = 2;
			item1.pszText = wszUtf8;

			TV_INSERTSTRUCT insert1;
			insert1.hParent = Selected;
			insert1.hInsertAfter = TVI_LAST;
			insert1.item = item1;

			HTREEITEM root2 = TreeView_InsertItem(m_tree, &insert1);

			delete[] wszUtf8;
		}

	}

	return 0;
}

void performActions(HWND hwnd)
{

}