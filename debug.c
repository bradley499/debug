#include "debug.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <linux/limits.h> // PATH_MAX
#elif __APPLE__
#include <sys/syslimits.h> // PATH_MAX
#endif

#define DEBUG_MESSAGE_SIZE 2048
#define DEBUG_ALLOW_DYNAMIC_ALLOCATION true

/**
 * @brief Outputs a debug message to the terminal
 * 
 * @param category Debug category
 * @param message Debug message
 * @return true 
 * @return false 
 */
bool debug_output(debug_t category, char * message);

/**
 * @brief Writes a debug message to file
 * 
 * @param category Debug category
 * @param message Debug message
 * @return true 
 * @return false 
 */
bool debug_write(debug_t category, char * message);

/**
 * @brief Returns "INFO" category declaration with selected coloring
 * 
 * @return char* 
 */
char *debug_category_text_colored_info();

/**
 * @brief Returns "WARNING" category declaration with selected coloring
 * 
 * @return char* 
 */
char *debug_category_text_colored_warn();

/**
 * @brief Returns "ERROR" category declaration with selected coloring
 * 
 * @return char* 
 */
char *debug_category_text_colored_error();

/**
 * @brief Returns "FATAL" category declaration with selected coloring
 * 
 * @return char* 
 */
char *debug_category_text_colored_fatal();

typedef struct
{
	bool enabled;
	char path[PATH_MAX];
	int descriptor;
} debug_file_t;

debug_file_t debug_file = {
	.enabled = false,
	.path = "/var/log/NOT_SPECIFIED.log\0",
	.descriptor = -1,
};

typedef struct
{
	debug_color_t info;
	debug_color_t warn;
	debug_color_t error;
	debug_color_t fatal;
	bool enabled;
} debug_colors_t;

debug_colors_t debug_colors = {
	.info = DEBUG_COLOR_GREEN,
	.warn = DEBUG_COLOR_YELLOW,
	.error = DEBUG_COLOR_RED,
	.fatal = DEBUG_COLOR_CYAN,
	.enabled = true,
};

bool debug(debug_t category, char * message, ...)
{
	va_list args;
	va_start(args, message);
	char message_buffer[DEBUG_MESSAGE_SIZE] = {0};
	int formatted_message_size = (vsnprintf(NULL, 0, message, args) + 1);
	va_end(args);
	va_start(args, message);
	char *message_formatted = NULL;
	if (formatted_message_size > DEBUG_MESSAGE_SIZE &&
		DEBUG_ALLOW_DYNAMIC_ALLOCATION == true)
	{
		message_formatted = (char *)malloc(formatted_message_size);
		if (message_formatted == NULL)
		{
			debug(DEBUG_WARN, "Internal debug error: Failed to allocate memory to " \
							  "format output");
			goto DEBUG_FORMAT_MESSAGE_PREALLOCATED;
		}
		vsnprintf(message_formatted, formatted_message_size, message, args);
	}
	else
	{
DEBUG_FORMAT_MESSAGE_PREALLOCATED:
		vsnprintf(message_buffer, DEBUG_MESSAGE_SIZE, message, args);
		message_formatted = *&message_buffer;
	}
	va_end(args);
	bool outputted = debug_output(category, message_formatted);
	if (debug_file.enabled)
		outputted = debug_write(category, message_formatted);
	if (formatted_message_size > DEBUG_MESSAGE_SIZE)
	{
		if (DEBUG_ALLOW_DYNAMIC_ALLOCATION == false)
			debug(DEBUG_WARN, "Internal debug error: Previous message was truncated");
		else
			free(message_formatted);
	}
	return outputted;
}

bool debug_color(debug_t category, debug_color_t color)
{
	switch (category)
	{
	case DEBUG_INFO:
		debug_colors.info = color;
		break;
	case DEBUG_WARN:
		debug_colors.warn = color;
		break;
	case DEBUG_ERROR:
		debug_colors.error = color;
		break;
	case DEBUG_FATAL:
		debug_colors.fatal = color;
		break;
	default:
		debug(DEBUG_WARN, "Internal debug error: Invalid debug category specified for " \
						  "operation");
		return false;
	}
	return true;
}

bool debug_color_enable()
{
	debug_colors.enabled = true;
	return true;
}

bool debug_color_disable()
{
	debug_colors.enabled = false;
	return true;
}

bool debug_set_file(char * path)
{
	if (strlen(path) >= PATH_MAX)
	{
		debug(DEBUG_ERROR, "Internal debug error: Invalid debug file path length was " \
						   "provided");
		return false;
	}
	strncpy(debug_file.path, path, PATH_MAX);
	return true;
}

bool debug_file_enable()
{
	debug_file.enabled = true;
	return true;
}

bool debug_file_disable()
{
	debug_file.enabled = false;
	if (debug_file.descriptor != -1)
		return (close(debug_file.descriptor) == 0);
	return true;
}

int debug_compiled_version()
{
	return DEBUG_VERSION;
}

bool debug_output(debug_t category, char * message)
{
	switch (category)
	{
	case DEBUG_INFO:
		printf("%s %s\n\r", debug_category_text_colored_info(), message);
		break;
	case DEBUG_WARN:
		printf("%s %s\n\r", debug_category_text_colored_warn(), message);
		break;
	case DEBUG_ERROR:
		printf("%s %s\n\r", debug_category_text_colored_error(), message);
		break;
	case DEBUG_FATAL:
		printf("%s %s\n\r", debug_category_text_colored_fatal(), message);
		break;
	default:
		debug_output(DEBUG_WARN, "Internal debug error: Invalid debug category " \
								 "specified for operation");
		return false;
	}
	return true;
}

bool debug_write(debug_t category, char * message)
{
	if (debug_file.descriptor == -1)
	{
		debug_file.descriptor = open(debug_file.path,
									 O_CREAT | O_WRONLY | O_APPEND, 0644);
		if (debug_file.descriptor == -1)
		{
			debug_output(DEBUG_ERROR, "Internal debug error: Failed to open debug " \
									  "file for writing");
			return false;
		}
	}
	switch (category)
	{
	case DEBUG_INFO:
		(void)!write(debug_file.descriptor, "[INFO] \0", 7);
		break;
	case DEBUG_WARN:
		(void)!write(debug_file.descriptor, "[WARNING] \0", 10);
		break;
	case DEBUG_ERROR:
		(void)!write(debug_file.descriptor, "[ERROR] \0", 8);
		break;
	case DEBUG_FATAL:
		(void)!write(debug_file.descriptor, "[FATAL] \0", 8);
		break;
	default:
		debug(DEBUG_WARN, "Internal debug error: Invalid debug category specified for " \
						  "operation");
		return false;
	}
	(void)!write(debug_file.descriptor, message, strlen(message));
	(void)!write(debug_file.descriptor, "\n\r", 2);
	return true;
}

char *debug_category_text_colored_info()
{
	if (debug_colors.enabled)
	{
		switch (debug_colors.info)
		{
			case DEBUG_COLOR_BLACK:
				return "[\e[0;30mINFO\e[0m]";
			case DEBUG_COLOR_RED:
				return "[\e[0;31mINFO\e[0m]";
			case DEBUG_COLOR_GREEN:
				return "[\e[0;32mINFO\e[0m]";
			case DEBUG_COLOR_YELLOW:
				return "[\e[0;33mINFO\e[0m]";
			case DEBUG_COLOR_BLUE:
				return "[\e[0;34mINFO\e[0m]";
			case DEBUG_COLOR_PURPLE:
				return "[\e[0;35mINFO\e[0m]";
			case DEBUG_COLOR_CYAN:
				return "[\e[0;36mINFO\e[0m]";
			case DEBUG_COLOR_WHITE:
				return "[\e[0;37mINFO\e[0m]";
		default:
			break;
		}
	}
	return "[INFO]";
}

char *debug_category_text_colored_warn()
{
	if (debug_colors.enabled)
	{
		switch (debug_colors.warn)
		{
			case DEBUG_COLOR_BLACK:
				return "[\e[0;30mWARNING\e[0m]";
			case DEBUG_COLOR_RED:
				return "[\e[0;31mWARNING\e[0m]";
			case DEBUG_COLOR_GREEN:
				return "[\e[0;32mWARNING\e[0m]";
			case DEBUG_COLOR_YELLOW:
				return "[\e[0;33mWARNING\e[0m]";
			case DEBUG_COLOR_BLUE:
				return "[\e[0;34mWARNING\e[0m]";
			case DEBUG_COLOR_PURPLE:
				return "[\e[0;35mWARNING\e[0m]";
			case DEBUG_COLOR_CYAN:
				return "[\e[0;36mWARNING\e[0m]";
			case DEBUG_COLOR_WHITE:
				return "[\e[0;37mWARNING\e[0m]";
		default:
			break;
		}
	}
	return "[WARNING]";
}

char *debug_category_text_colored_error()
{
	if (debug_colors.enabled)
	{
		switch (debug_colors.error)
		{
			case DEBUG_COLOR_BLACK:
				return "[\e[0;30mERROR\e[0m]";
			case DEBUG_COLOR_RED:
				return "[\e[0;31mERROR\e[0m]";
			case DEBUG_COLOR_GREEN:
				return "[\e[0;32mERROR\e[0m]";
			case DEBUG_COLOR_YELLOW:
				return "[\e[0;33mERROR\e[0m]";
			case DEBUG_COLOR_BLUE:
				return "[\e[0;34mERROR\e[0m]";
			case DEBUG_COLOR_PURPLE:
				return "[\e[0;35mERROR\e[0m]";
			case DEBUG_COLOR_CYAN:
				return "[\e[0;36mERROR\e[0m]";
			case DEBUG_COLOR_WHITE:
				return "[\e[0;37mERROR\e[0m]";
		default:
			break;
		}
	}
	return "[ERROR]";
}

char *debug_category_text_colored_fatal()
{
	if (debug_colors.enabled)
	{
		switch (debug_colors.fatal)
		{
			case DEBUG_COLOR_BLACK:
				return "[\e[0;30mFATAL\e[0m]";
			case DEBUG_COLOR_RED:
				return "[\e[0;31mFATAL\e[0m]";
			case DEBUG_COLOR_GREEN:
				return "[\e[0;32mFATAL\e[0m]";
			case DEBUG_COLOR_YELLOW:
				return "[\e[0;33mFATAL\e[0m]";
			case DEBUG_COLOR_BLUE:
				return "[\e[0;34mFATAL\e[0m]";
			case DEBUG_COLOR_PURPLE:
				return "[\e[0;35mFATAL\e[0m]";
			case DEBUG_COLOR_CYAN:
				return "[\e[0;36mFATAL\e[0m]";
			case DEBUG_COLOR_WHITE:
				return "[\e[0;37mFATAL\e[0m]";
		default:
			break;
		}
	}
	return "[FATAL]";
}
