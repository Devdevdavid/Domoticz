/**
  * @file   status.cpp
  * @brief  Handle statuses of the board
  * @author David DEVANT
  * @date   12/08/2019
  */

#define STATUS_CPP

#include "status/status.hpp"
#include "cmd/cmd.hpp"

void status_init(void)
{
	memset(boardStatus, 0, NB_STATUS);
}
