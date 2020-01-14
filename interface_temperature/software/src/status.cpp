#define STATUS_CPP

#include "status.hpp"
#include "cmd/cmd.hpp"

void status_init(void)
{
  memset(boardStatus, 0, NB_STATUS);
}
