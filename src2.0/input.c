//
// Created by user on 21. 3. 2026..
//

/*
 * Handles getting input
 *
 *
 */

#include "input.h"
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

static struct termios original_settings;  // Needed to reset the settings at the end

void enable_raw_mode() {
    struct termios t;

    tcgetattr(STDIN_FILENO, &original_settings);    // Save current settings
    t = original_settings;

    t.c_lflag &= (tcflag_t)~(ICANON | ECHO); // no line buffering, no echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);   // Apply new settings
}

void disable_raw_mode() {
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
}

void enable_nonblocking() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);  // Get flags
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

int read_char() {
    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);

    if (n >= 0) return (unsigned char)c;     // If a character was read
    return -1; // no input available
}