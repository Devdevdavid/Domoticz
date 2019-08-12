#include "status.hpp"
#include "cmd/cmd.hpp"

void status_init(void)
{
  memset(boardStatus, 0, NB_STATUS);
  STATUS_BRIGHTNESS = STATUS_BRIGHTNESS_DEFAULT;
  _set(STATUS_APPLI, STATUS_APPLI_LED_IS_ON);
  cmd_set_led_visu(true);
}
