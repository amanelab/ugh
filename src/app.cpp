/*-
 * This file is part of Ugh project. For license details, see the file
 * 'LICENSE' in this package.
 */
#define WINVER _WIN32_WINNT_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10

#include <windows.h>

#include <cassert>
#include <stdexcept>

#include <strsafe.h>
#include <shellapi.h>
#include <commctrl.h>

#include "resource.h"

#include "app.h"

#include <windowsx.h>

namespace ugh {

/*!
** \brief Internal Application Class
**
** This class represents an internal substance of the main program. This class
** is an implementation of the App class. So, this is instantiated only by App
** class.
*/

class App::Impl final {
  static constexpr WCHAR NAME[] = L"Ugh";
  static constexpr SIZE WINDOW_SIZE = { 220, 100 };
  static constexpr DWORD INTERVAL_MINUTES_DEFAULT = 5;
  static constexpr DWORD INTERVAL_TIMER_ID = 0xDEAD;
  static constexpr DWORD SYSTRAY_ID = 0xBEEF;
  static constexpr UINT WM_USER_SYSTRAY = WM_APP + SYSTRAY_ID;
  static constexpr UINT ID_USER_EXIT = 0x0;
  static constexpr UINT ID_USER_TOGGLE = 0x1;
public:
  Impl() = delete;
  Impl(HINSTANCE instance);
  ~Impl();
private:
  void init();
private:
  void load_app_icon();
  void destroy_app_icon();
  void load_systray_icon();
  void destroy_systray_icon();
  void add_systray();
  void set_systray(bool is_active);
  void on_systray_right_button_down();
  void on_systray_left_button_down();
  void remove_systray();
private:
  void set_activate_button(bool is_active);
private:
  void toggle_state();
  void set_state(bool is_active);
  void set_state_off();
  void set_interval();
  void get_interval();
  void reset_timer();
  void interrupt();
private:
  LRESULT wndproc(UINT msg, WPARAM wp, LPARAM lp);
  INT_PTR dlgproc(UINT msg, WPARAM wp, LPARAM lp);
public:
  void run();
public:
  static bool is_launched();
  static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
  static INT_PTR CALLBACK dlgproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
private:
  HINSTANCE app_instance_;
  HWND main_window_;
  HWND main_dialog_;
  HICON app_icon_;
  HICON systray_color_icon_;
  HICON systray_gray_icon_;
  UINT interval_minutes_;
  bool is_active_;
  bool is_systray_added_;
};

App::Impl::Impl(HINSTANCE instance)
  : app_instance_{ instance },
    main_window_{ nullptr },
    main_dialog_{ nullptr },
    app_icon_{ nullptr },
    systray_color_icon_{ nullptr },
    systray_gray_icon_{ nullptr },
    interval_minutes_{ App::Impl::INTERVAL_MINUTES_DEFAULT },
    is_active_{ false },
    is_systray_added_{ false }
{ }

App::Impl::~Impl() {
  this->destroy_systray_icon();
  this->destroy_app_icon();
}

static void
init_common_controls_() {
  INITCOMMONCONTROLSEX ctr = {
    .dwSize = sizeof(ctr),
    .dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES,
  };
  ::InitCommonControlsEx(&ctr);
}

void
App::Impl::init() {
  init_common_controls_();
  this->load_app_icon();

  WNDCLASSEX wc = {
    .cbSize = sizeof(wc),
    .style = CS_HREDRAW | CS_VREDRAW,
    .lpfnWndProc = App::Impl::wndproc,
    .cbClsExtra = 0,
    .cbWndExtra = 0,
    .hInstance = this->app_instance_,
    .hIcon = this->app_icon_,
    .hCursor = nullptr,
    .hbrBackground = ::GetSysColorBrush(COLOR_WINDOW),
    .lpszMenuName = nullptr,
    .lpszClassName = App::Impl::NAME,
    .hIconSm = nullptr,
  };

  ::RegisterClassEx(&wc);

  /*
  ** Main Window
  */

  this->main_window_ = ::CreateWindowEx(
    0,
    App::Impl::NAME, App::Impl::NAME,
    WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
    CW_USEDEFAULT, CW_USEDEFAULT,
    App::Impl::WINDOW_SIZE.cx, App::Impl::WINDOW_SIZE.cy,
    nullptr,
    nullptr,
    this->app_instance_,
    this
    );
  if(!this->main_window_) {
    throw std::runtime_error("Failed to create main window");
  }

  this->main_dialog_ = ::CreateDialogParam(
    this->app_instance_,
    MAKEINTRESOURCE(IDD_MAIN),
    this->main_window_,
    App::Impl::dlgproc,
    reinterpret_cast<LPARAM>(this)
    );

  this->set_interval();
  
  /*
  ** Prepare the interval timer
  */ 

  App::Impl::reset_timer();

  /*
  ** Show
  */ 

  ::ShowWindow(this->main_window_, SW_SHOW);
}

void
App::Impl::load_app_icon() {
  auto hInst = this->app_instance_;
  static constexpr auto LI_FLAGS = LR_DEFAULTSIZE | LR_SHARED;

  this->destroy_app_icon();
  this->app_icon_ = reinterpret_cast<HICON>(
    ::LoadImage(hInst, MAKEINTRESOURCE(IDI_APP), IMAGE_ICON, 0, 0, LI_FLAGS));
}

void
App::Impl::destroy_app_icon() {
  if(this->app_icon_) {
    ::DestroyIcon(this->app_icon_);
    this->app_icon_ = nullptr;
  }
}

void
App::Impl::load_systray_icon() {
  auto hInst = this->app_instance_;

  this->destroy_systray_icon();
  this->systray_color_icon_ = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAY_COLOR));
  this->systray_gray_icon_ = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAY_GRAY));
}

void
App::Impl::destroy_systray_icon() {
  if(this->systray_color_icon_) {
    ::DestroyIcon(this->systray_color_icon_);
    this->systray_color_icon_ = nullptr;
  }
  if(this->systray_gray_icon_) {
    ::DestroyIcon(this->systray_gray_icon_);
    this->systray_gray_icon_ = nullptr;
  }
}

void
App::Impl::add_systray() {
  this->load_systray_icon();

  NOTIFYICONDATA data = {
    .cbSize = sizeof(data),
    .hWnd = this->main_window_,
    .uID = SYSTRAY_ID,
    .uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP,
    .uCallbackMessage = App::Impl::WM_USER_SYSTRAY,
    .hIcon = nullptr,
    .uVersion = NOTIFYICON_VERSION_4,
  };
  data.hIcon = this->is_active_
             ? this->systray_color_icon_
             : this->systray_gray_icon_;

  ::StringCchCopy(data.szTip, ARRAYSIZE(data.szTip), App::Impl::NAME);

  if (!::Shell_NotifyIcon(NIM_ADD, &data)) {
    throw std::runtime_error("Failed to add notify icon.");
  }
  if (!::Shell_NotifyIcon(NIM_SETVERSION, &data)) {
    throw std::runtime_error("Failed to set notify icon version.");
  }
  this->is_systray_added_ = true;
}

void
App::Impl::set_systray(bool is_active) {
  NOTIFYICONDATA data = {
    .cbSize = sizeof(data),
    .hWnd = this->main_window_,
    .uID = SYSTRAY_ID,
    .uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP,
    .uCallbackMessage = App::Impl::WM_USER_SYSTRAY,
    .hIcon = nullptr,
    .uVersion = NOTIFYICON_VERSION_4,
  };

  if (this->is_systray_added_) {
    if (is_active) {
      data.hIcon = this->systray_color_icon_;
    } else {
      data.hIcon = this->systray_gray_icon_;
    }
    if (!::Shell_NotifyIcon(NIM_MODIFY, &data)) {
      if (!this->main_window_) {
      }
      throw std::runtime_error("Failed to modify notify icon.");
    }
  }
}

void
App::Impl::remove_systray() {
  NOTIFYICONDATA data = {
    .cbSize = sizeof(data),
    .hWnd = this->main_window_,
    .uID = SYSTRAY_ID,
    .hBalloonIcon = nullptr,
  };

  if (this->is_systray_added_) {
    if (!::Shell_NotifyIcon(NIM_DELETE, &data)) {
      throw std::runtime_error("Failed to delete notify icon.");
    }
    this->destroy_systray_icon();
  }
}

void
App::Impl::reset_timer() {
  constexpr DWORD ID = App::Impl::INTERVAL_TIMER_ID;
  const DWORD INTERVAL = this->interval_minutes_ * 1000 * 60;

  ::SetTimer(this->main_window_, ID, INTERVAL, nullptr);
}

void
App::Impl::set_activate_button(bool is_active) {
  static const WCHAR* TURN_ON = L"Turn ON";
  static const WCHAR* TURN_OFF = L"Turn OFF";

  HWND hwnd = ::GetDlgItem(this->main_dialog_, IDC_BUTTON_ACTIVATE);

  if (is_active) {
    ::Button_SetText(hwnd, TURN_OFF);
  } else {
    ::Button_SetText(hwnd, TURN_ON);
  }
}

void
App::Impl::toggle_state() {
  this->set_state(this->is_active_ ? false : true);
}

void
App::Impl::set_state(bool is_active) {
  this->is_active_ = is_active;
  this->set_systray(is_active);
  this->set_activate_button(is_active);
  this->get_interval();
  this->reset_timer();
}

void
App::Impl::set_state_off() {
  this->set_state(false);
}

void
App::Impl::set_interval() {
  WCHAR str[64] = { 0 };
  HWND hwnd = ::GetDlgItem(this->main_dialog_, IDC_EDIT_INTERVAL);

  ::StringCchPrintf(str, ARRAYSIZE(str), L"%d", this->interval_minutes_);
  ::Edit_SetText(hwnd, str);
}

void
App::Impl::get_interval() {
  WCHAR str[64] = { 0 };
  DWORD min = 0;
  WCHAR* e = nullptr;
  HWND hwnd = ::GetDlgItem(this->main_dialog_, IDC_EDIT_INTERVAL);

  Edit_GetText(hwnd, str, ARRAYSIZE(str));
  min = wcstoul(str, &e, 10);
  if (!e || *e || min <= 0) {
    this->set_interval();
    Edit_SetSel(hwnd, 0, -1);
  } else {
    this->interval_minutes_ = min;
  }
}

void
App::Impl::interrupt() {
  static constexpr auto TYPE = INPUT_KEYBOARD;
  static constexpr auto VK = VK_RCONTROL;
  static constexpr auto FLG_KEYUP = KEYEVENTF_KEYUP;
  
  INPUT input[] ={
    { .type = TYPE, .ki = { .wVk = VK } },
    { .type = TYPE, .ki = { .wVk = VK, .dwFlags = FLG_KEYUP } },
  };
  if (this->is_active_) {
    ::SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
  }
}

void
App::Impl::on_systray_left_button_down() {
  ::ShowWindow(this->main_window_, SW_SHOW);
}

void
App::Impl::on_systray_right_button_down() {
  HMENU popup = CreatePopupMenu();
  POINT pos = { .x = 0, .y = 0 };

  auto hWnd = this->main_window_;

  static constexpr auto IM_FLAGS = MF_BYPOSITION | MF_STRING;
  static constexpr auto IM_EXIT_ID = App::Impl::ID_USER_EXIT;
  static constexpr auto IM_EXIT_STR = L"Exit";
  static constexpr auto IM_TOGGLE_ID = App::Impl::ID_USER_TOGGLE;
  static constexpr auto TPM_FLAGS = TPM_BOTTOMALIGN | TPM_LEFTALIGN;

  const auto IM_TOGGLE_STR = this->is_active_ ? L"Turn OFF" : L"Turn ON";
  
  ::GetCursorPos(&pos);

  ::InsertMenu(popup, 0, IM_FLAGS, IM_TOGGLE_ID, IM_TOGGLE_STR);
  ::InsertMenu(popup, 1, IM_FLAGS, IM_EXIT_ID, IM_EXIT_STR);
  ::SetForegroundWindow(hWnd);
  ::TrackPopupMenu(popup, TPM_FLAGS, pos.x, pos.y, 0, hWnd, nullptr);
}

LRESULT
App::Impl::wndproc(UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_TIMER:
    if (wp == App::Impl::INTERVAL_TIMER_ID) {
      this->interrupt();
    }
    return 0;
  case WM_USER_SYSTRAY:
    switch (LOWORD(lp)) {
    case NIN_SELECT:
      this->on_systray_left_button_down();
      break;
    case  WM_CONTEXTMENU:
      this->on_systray_right_button_down();
      break;
    default:
      break;
    }
    break;
  case WM_COMMAND:
    switch (wp) {
    case App::Impl::ID_USER_TOGGLE:
      this->toggle_state();
      break;
    case App::Impl::ID_USER_EXIT:
      ::PostQuitMessage(0);
      break;
    default:
      break;
    }
    break;
  case WM_CLOSE:
    ::ShowWindow(this->main_window_, SW_HIDE);
    return 0;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  default:
    break;
  }
  return ::DefWindowProc(this->main_window_, msg, wp, lp);
}

INT_PTR
App::Impl::dlgproc(UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_COMMAND:
    switch (LOWORD(wp)) {
    case IDC_BUTTON_ACTIVATE:
      if (HIWORD(wp) == BN_CLICKED) {
        this->toggle_state();
      }
      break;
    case IDC_EDIT_INTERVAL:
      if (HIWORD(wp) == EN_CHANGE) {
          this->set_state_off();
      }
      break;
    default:
      break;
    }
  default:
    break;
  }
  return static_cast<INT_PTR>(FALSE);
}

void
App::Impl::run() {
  MSG msg = { 0 };
  BOOL ret = FALSE;

  this->init();

  this->add_systray();

  while((ret = ::GetMessage(&msg, NULL, 0, 0)) != 0) {
    if(ret == -1) {
      throw std::runtime_error("Failed to get window message");
    } else {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }
  }

  this->remove_systray();
}

bool
App::Impl::is_launched() {
  return ::FindWindow(App::Impl::NAME, App::Impl::NAME) ? true : false;
}

LRESULT CALLBACK
App::Impl::wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  LRESULT ret = 0;
  static App::Impl* app = nullptr;

  if (msg == WM_CREATE) {
    CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lp);
    app = static_cast<App::Impl*>(cs->lpCreateParams);
    ret = 0;
  } else {
    if (app) {
      ret = app->wndproc(msg, wp, lp);
    } else {
      ret = ::DefWindowProc(hwnd, msg, wp, lp);
    }
  }

  return ret;
}

INT_PTR CALLBACK
App::Impl::dlgproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  static App::Impl* app = nullptr;

  if (msg == WM_INITDIALOG) {
    app = reinterpret_cast<App::Impl*>(lp);
    assert(app);
    /* It's dirty. But we process WM_INITDIALOG message here. */
    ::ShowWindow(hwnd, SW_SHOW);
  }

  return app ? app->dlgproc(msg, wp, lp) : static_cast<INT_PTR>(FALSE);
}

///////////////////////////////////////////////////////////////////////////////

App::App(HINSTANCE instance)
  : impl_(new Impl(instance)) { }

App::~App() {
  delete this->impl_;
}

void
App::run() {
  if (!this->impl_->is_launched()) {
    this->impl_->run();
  }
}

}
