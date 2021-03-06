//
// Copyright (c) 2019 Vivaldi Technologies AS. All rights reserved.
//
#include "browser/menus/vivaldi_pwa_menu_controller.h"

#include "browser/vivaldi_browser_finder.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/banners/app_banner_manager.h"
#include "chrome/browser/ui/web_applications/web_app_launch_utils.h"
#include "chrome/browser/web_applications/web_app_provider.h"
#include "content/public/browser/web_contents.h"
#include "extensions/api/runtime/runtime_api.h"
#include "extensions/api/vivaldi_utilities/vivaldi_utilities_api.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/text_elider.h"

constexpr size_t kMaxAppNameLength = 30;

namespace vivaldi {

// Returns the appropriate menu label for the IDC_INSTALL_PWA command if
// available.
// Copied from app_menu_model.cc.
base::Optional<base::string16> GetInstallPWAAppMenuItemName(Browser* browser) {
  WebContents* web_contents =
    browser->tab_strip_model()->GetActiveWebContents();
  if (!web_contents)
    return base::nullopt;
  base::string16 app_name =
    banners::AppBannerManager::GetInstallableWebAppName(web_contents);
  if (app_name.empty())
    return base::nullopt;
  return l10n_util::GetStringFUTF16(IDS_INSTALL_TO_OS_LAUNCH_SURFACE, app_name);
}

PWAMenuController::PWAMenuController(content::WebContents* web_contents)
  :web_contents_(web_contents),
   browser_(FindBrowserForEmbedderWebContents(web_contents)),
   enabled_(HasFeature()) {
}

const extensions::Extension* PWAMenuController::GetExtension() const {
  extensions::ProcessManager* process_manager =
      extensions::ProcessManager::Get(browser_->profile());
  return process_manager->GetExtensionForWebContents(web_contents_);
}

bool PWAMenuController::HasFeature() {
  return extensions::VivaldiRuntimeFeatures::IsEnabled(browser_->profile(),
      "install_pwa");
}

void PWAMenuController::PopulateModel(ui::SimpleMenuModel* menu_model) {
  if (enabled_) {
    base::Optional <web_app::AppId> pwa =
        web_app::GetPwaForSecureActiveTab(browser_);
    if (pwa) {
      auto* provider = web_app::WebAppProvider::Get(browser_->profile());
      menu_model->AddSeparator(ui::NORMAL_SEPARATOR);
      menu_model->AddItem(IDC_OPEN_IN_PWA_WINDOW, l10n_util::GetStringFUTF16(
          IDS_OPEN_IN_APP_WINDOW,
          gfx::TruncateString(
              base::UTF8ToUTF16(provider->registrar().GetAppShortName(*pwa)),
                                kMaxAppNameLength,
                                gfx::CHARACTER_BREAK)));
    } else {
      base::Optional<base::string16> install_pwa_item_name =
          GetInstallPWAAppMenuItemName(browser_);
      if (install_pwa_item_name) {
        menu_model->AddSeparator(ui::NORMAL_SEPARATOR);
        menu_model->AddItem(IDC_INSTALL_PWA, *install_pwa_item_name);
        menu_model->AddItemWithStringId(IDC_CREATE_SHORTCUT,
                                        IDS_ADD_TO_OS_LAUNCH_SURFACE);
      }
    }
  }
}

bool PWAMenuController::IsItemForCommandIdDynamic(int command_id) const {
  return command_id == IDC_INSTALL_PWA;
}

base::string16 PWAMenuController::GetLabelForCommandId(int command_id) const {
  if (command_id == IDC_INSTALL_PWA) {
    return GetInstallPWAAppMenuItemName(browser_).value();
  } else {
    return base::string16();
  }
}

bool PWAMenuController::IsCommand(int command_id) const {
  return command_id == IDC_OPEN_IN_PWA_WINDOW ||
         command_id == IDC_INSTALL_PWA ||
         command_id == IDC_CREATE_SHORTCUT;
}


bool PWAMenuController::HandleCommand(int command_id) {
  if (enabled_) {
    switch (command_id) {
      case IDC_CREATE_SHORTCUT:
        chrome::CreateBookmarkAppFromCurrentWebContents(browser_,
          true /* force_shortcut_app */);
        return true;

      case IDC_INSTALL_PWA:
        chrome::CreateBookmarkAppFromCurrentWebContents(browser_,
          false /* force_shortcut_app */);
        return true;

      case IDC_OPEN_IN_PWA_WINDOW:
        web_app::ReparentWebAppForSecureActiveTab(browser_);
        return true;
    }
  }
  return false;
}

}  // namespace vivaldi