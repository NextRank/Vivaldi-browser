// Copyright (c) 2016 Vivaldi Technologies AS. All rights reserved

#ifndef APP_VIVALDI_COMMAND_CONTROLLER_H_
#define APP_VIVALDI_COMMAND_CONTROLLER_H_

class CommandUpdater;
class Browser;

namespace vivaldi {
enum VivaldiScrollType {
  kVivaldiNoScrollType = 0,
  kVivaldiScrollWheel,
  kVivaldiScrollTrackpad,
  kVivaldiScrollInertial
};

bool NeedsDisabledMacMenuItem(int action);
void SetVivaldiScrollType(int scrollType);
void UpdateCommandsForVivaldi(CommandUpdater*);
bool ExecuteVivaldiCommands(Browser* browser, int id);
}

#endif  // APP_VIVALDI_COMMAND_CONTROLLER_H_
