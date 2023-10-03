#include <stdbool.h>

#define DEBUG_VERSION 1 // Version 1 - 13:19 3 Oct 2023

typedef enum
{
	DEBUG_INFO = 0,  // Debug information
	DEBUG_WARN = 1,  // Debug warning
	DEBUG_ERROR = 2, // Debug error
	DEBUG_FATAL = 3, // Debug fatal
} debug_t;

typedef enum
{
	DEBUG_COLOR_BLACK,  // Black
	DEBUG_COLOR_RED,    // Red 
	DEBUG_COLOR_GREEN,  // Green
	DEBUG_COLOR_YELLOW, // Yellow
	DEBUG_COLOR_BLUE,   // Blue
	DEBUG_COLOR_PURPLE, // Purple
	DEBUG_COLOR_CYAN,   // Cyan
	DEBUG_COLOR_WHITE,  // White
} debug_color_t;

/**
 * @brief Outputs a debug message
 * 
 * @param category Debug category
 * @param message Debug message
 * @return true 
 * @return false 
 */
extern bool debug(debug_t category, char * message, ...);

/**
 * @brief Set the color of a category of debug
 * 
 * @param category Debug category
 * @param color Debug message color
 * @return true 
 * @return false 
 */
extern bool debug_color(debug_t category, debug_color_t color);

/**
 * @brief Enable rendering of color on debug
 * 
 * @return true 
 * @return false 
 */
extern bool debug_color_enable();

/**
 * @brief Disable rendering of color on debug
 * 
 * @return true 
 * @return false 
 */
extern bool debug_color_disable();

/**
 * @brief Sets the file path of the debug file
 * 
 * @param path Debug file path
 * @return true 
 * @return false 
 */
extern bool debug_set_file(char * path);

/**
 * @brief Enables the ability to write to a debug file
 * 
 * @return true 
 * @return false 
 */
extern bool debug_file_enable();

/**
 * @brief Disables the ability to write to a debug file
 * 
 * @return true 
 * @return false 
 */
extern bool debug_file_disable();

/**
 * @brief Returns the compiled version of the debug library
 * 
 * @return int 
 */
extern int debug_compiled_version();