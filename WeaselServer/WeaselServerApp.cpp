﻿#include "stdafx.h"
#include "WeaselServerApp.h"
#include <filesystem>

WeaselServerApp::WeaselServerApp()
    : m_handler(std::make_unique<RimeWithWeaselHandler>(&m_ui)),
      tray_icon(m_ui) {
  // m_handler.reset(new RimeWithWeaselHandler(&m_ui));
  m_server.SetRequestHandler(m_handler.get());
  SetupMenuHandlers();
}

WeaselServerApp::~WeaselServerApp() {}

int WeaselServerApp::Run() {
  if (!m_server.Start())
    return -1;

  // win_sparkle_set_appcast_url("https://www.typeduck.hk/updates/windows.xml");
  win_sparkle_set_registry_path("Software\\Rime\\TypeDuck\\Updates");
  win_sparkle_set_lang(isChinese() ? "zh-TW" : "en");
  win_sparkle_init();
  m_ui.Create(m_server.GetHWnd());

  m_handler->Initialize();
  m_handler->OnUpdateUI([this]() { tray_icon.Refresh(); });

  tray_icon.Create(m_server.GetHWnd());
  tray_icon.Refresh();

  int ret = m_server.Run();

  m_handler->Finalize();
  m_ui.Destroy();
  tray_icon.RemoveIcon();
  win_sparkle_cleanup();

  return ret;
}

void WeaselServerApp::SetupMenuHandlers() {
  std::filesystem::path dir = install_dir();
  m_server.AddMenuHandler(ID_WEASELTRAY_QUIT,
                          [this] { return m_server.Stop() == 0; });
  m_server.AddMenuHandler(ID_WEASELTRAY_DEPLOY,
                          std::bind(execute, dir / L"TypeDuckDeployer.exe",
                                    std::wstring(L"/deploy")));
  m_server.AddMenuHandler(
      ID_WEASELTRAY_SETTINGS,
      std::bind(execute, dir / L"TypeDuckDeployer.exe", std::wstring()));
  m_server.AddMenuHandler(ID_WEASELTRAY_DICT_MANAGEMENT,
                          std::bind(execute, dir / L"TypeDuckDeployer.exe",
                                    std::wstring(L"/dict")));
  m_server.AddMenuHandler(ID_WEASELTRAY_SYNC,
                          std::bind(execute, dir / L"TypeDuckDeployer.exe",
                                    std::wstring(L"/sync")));
  m_server.AddMenuHandler(ID_WEASELTRAY_WEBSITE,
                          std::bind(open, L"https://www.typeduck.hk"));
  // TODO
  m_server.AddMenuHandler(ID_WEASELTRAY_ABOUT,
                          std::bind(open, L"https://www.typeduck.hk/about/"));
  m_server.AddMenuHandler(ID_WEASELTRAY_CHECKUPDATE, check_update);
  m_server.AddMenuHandler(ID_WEASELTRAY_INSTALLDIR, std::bind(explore, dir));
  m_server.AddMenuHandler(ID_WEASELTRAY_USERDIR,
                          std::bind(explore, WeaselUserDataPath()));
  m_server.AddMenuHandler(ID_WEASELTRAY_LOGDIR,
                          std::bind(explore, WeaselLogPath()));
}
