/**
  * @file   prompt.cpp
  * @brief  Prompt management for terminal
  * @author David DEVANT
  * @date   26/03/2021
  */

#include "prompt.hpp"
#include "global.hpp"

#ifdef MODULE_TERM

struct prompt_t {
	char    lineBuffer[PROMPT_LINE_BUFFER_SIZE]; /**< Buffer to store the state of the line */
	uint8_t lineSize;                            /**< Size of the line (without ending '\0') */
	char *  pCurPos;                             /**< Current position of the cursor */
	uint8_t isEscaping : 1;                      /**< Tell if next bytes will be managed as escaped command */
	uint8_t escPos;                              /**< Current position in the escBuffer */
};
struct prompt_t promptHandle;

//========================
//	  STATIC FUNCTIONS
//========================

/**
 * @brief Insert the caracter toInsert into .lineBuffer
 * @note Do nothing if overflow is detected
 * @param toInsert Character to insert
 */
static void prompt_line_insert_at_cursor(char toInsert)
{
	char * pBuffer = promptHandle.pCurPos;
	char * pEnd;
	char   backup;

	// Check size before inserting
	if ((promptHandle.lineSize + 1) >= PROMPT_LINE_BUFFER_SIZE) {
		return;
	}

	// Define the new ending line
	++promptHandle.lineSize;
	pEnd = &promptHandle.lineBuffer[promptHandle.lineSize];

	// Slide caracters
	while (pBuffer <= pEnd) {
		backup   = *pBuffer;
		*pBuffer = toInsert;
		toInsert = backup;
		pBuffer++;
	}

	// Slide cursor position
	++promptHandle.pCurPos;
}

/**
 * @brief Remove the caracter at .pCurPos
 * @note Do nothing if line is empty
 */
static void prompt_line_remove_at_cursor(void)
{
	char * pBuffer = promptHandle.pCurPos;
	char * pEnd;

	// Check size before removing
	if (((int32_t) promptHandle.lineSize - 1) < 0) {
		return;
	}

	--promptHandle.lineSize;
	pEnd = &promptHandle.lineBuffer[promptHandle.lineSize];

	// Slide caracters
	while (pBuffer <= pEnd) {
		*pBuffer = *(pBuffer + 1);
		pBuffer++;
	}
}

/**
 * @brief Execute the actions assciated to escaped codes
 *
 * @param byte The escaped code
 * @return 0: OK, -1: unsupported code
 */
static int32_t prompt_exec_escaped_code(uint8_t byte)
{
	int32_t curPos;

	switch (byte) {
	// case PROMPT_CODE_ARROW_UP:
	// 	break;
	// case PROMPT_CODE_ARROW_DOWN:
	// 	break;
	case PROMPT_CODE_ARROW_RIGHT:
		// Increment cursor
		curPos = promptHandle.pCurPos - promptHandle.lineBuffer;
		if (curPos <= (promptHandle.lineSize - 1)) {
			++promptHandle.pCurPos;
		}
		break;
	case PROMPT_CODE_ARROW_LEFT:
		// Decrement cursor
		if (promptHandle.pCurPos > 0) {
			--promptHandle.pCurPos;
		}
		break;
	default:
		log_error("Unsupported escape code : 0x%02X", byte);
		return -1;
	}
	return 0;
}

/**
 * @brief Handle the escaped codes with a simple state machine
 *
 * @param byte The incomming new character
 * @return [description]
 */
static void prompt_handle_escaped(uint8_t byte)
{
	++promptHandle.escPos;

	// Check '['
	if (promptHandle.escPos == 1) {
		// If 2nd character is ok, wait for the next one
		if (byte == PROMPT_KEY_OPEN_BRACKET) {
			return;
		}
	} else if (promptHandle.escPos == 2) {
		prompt_exec_escaped_code(byte);
	}

	// Reset escape handler
	promptHandle.isEscaping = false;
	promptHandle.escPos     = 0;
}

//========================
//	  PUBLIC FUNCTIONS
//========================

/**
 * @brief Return a String representing the current buffer state
 * @return A String object
 */
String prompt_get_line(void)
{
	return String(promptHandle.lineBuffer);
}

/**
 * @brief Get the current cursor position
 * @details [long description]
 * @return [description]
 */
uint8_t prompt_get_cursor_pos(void)
{
	int32_t curPos = promptHandle.pCurPos - promptHandle.lineBuffer;
	return (uint8_t) curPos;
}

/**
 * @brief Handle character reception
 *
 * @param byte The incomming new character
 */
int32_t prompt_rx(uint8_t byte)
{
	if (promptHandle.isEscaping) {
		prompt_handle_escaped(byte);
	} else if (byte == PROMPT_KEY_ESC) {
		promptHandle.isEscaping = true;
	} else if (byte == PROMPT_KEY_ENTER_WIN) {
		// Ignore this
	} else if (byte == PROMPT_KEY_ENTER_UNIX) {
		return 0;
	} else if (byte == PROMPT_KEY_BACKSPACE) {
		prompt_line_remove_at_cursor();
	} else {
		prompt_line_insert_at_cursor(byte);
	}
	return 1;
}

/**
 * @brief Initialize the module prompt
 */
void prompt_init(void)
{
	memset(promptHandle.lineBuffer, 0, PROMPT_LINE_BUFFER_SIZE);
	promptHandle.lineSize   = 0;
	promptHandle.pCurPos    = promptHandle.lineBuffer;
	promptHandle.isEscaping = false;
	promptHandle.escPos     = 0;
}

#endif /* MODULE_TERM */