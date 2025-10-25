#include <ranges>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#if defined(_WIN32)
    #include <io.h>
    #include <conio.h>          // _getch()
    #define NOMINMAX 
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>        // GetConsoleScreenBufferInfo
#else
    #include <unistd.h>         // STDIN_FILENO
    #include <termios.h>        // tcgetattr, tcsetattr
    #include <sys/ioctl.h>      // ioctl
#endif

#include <cxxopts.hpp>


enum class KeyAction { 
    LineDown, 
    PageDown, 
    Quit, 
    None
};


void more_pager(const std::vector<std::string>& lines, int lines_sub = 2) {
    auto get_char = []() -> int {
#if defined(_WIN32)
        // Use _getch() for Windows to read a single character
        return _getch();
#else
        // Set terminal to raw mode
        struct termios t;
        tcgetattr(STDIN_FILENO, &t);
        t.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
        int ch = getchar();
        t.c_lflag |= (ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
        return ch;
#endif
    };

    // Get console height
    auto console_height = []() -> int {
        int height = 24; // Fallback default
#if defined(_WIN32)
        // Use Windows API to get console size
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }
#else
        // Use ioctl to get terminal size
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
            height = w.ws_row;
        }
#endif
        return height;
    };

    // Key action to handle user input
    auto get_key_action = [&]() -> KeyAction {
        int ch = get_char();

#if defined(_WIN32)
        // Handle arrow keys
        if (ch == 224) {
            ch = get_char();
            if (ch == 80) {
                return KeyAction::LineDown; // Down arrow
            }
            return KeyAction::None;
        }
#else
        // Handle arrow keys
        if (ch == 27) {
            int next1 = get_char();
            if (next1 == 91) {
                int next2 = get_char();
                if (next2 == 66) return KeyAction::LineDown; // Down arrow
                return KeyAction::None;
            }
            return KeyAction::Quit; // ESC alone
        }
#endif

        // Handle page down action
        if (ch == 13 || ch == 10) {
            return KeyAction::PageDown;  // Enter
        }

        // Handle quit action
        if (ch == 'q' || ch == 'Q' || ch == 27) {
            return KeyAction::Quit; // 'q', 'Q', or ESC
        }

        // Handle other keys
        return KeyAction::None;
    };

    int current_line = 0;
    int total_lines = static_cast<int>(lines.size());

    while (current_line < total_lines) {
        // Recalculate window size each loop to support resizing
        int window_size = std::max(1, console_height() - lines_sub);

        // If we have fewer lines than the window size, just print them all
        int lines_to_show = std::min(window_size, total_lines - current_line);
        int end_line = current_line + lines_to_show;

        // Print the current set of lines
        std::ranges::for_each(lines | std::views::drop(current_line) | std::views::take(lines_to_show),
            [](const std::string& line) { std::cout << line << '\n'; }
        );
        current_line = end_line;

        // If we reached the end, wait for user input
        while (current_line < total_lines) {
            bool valid_input = false;
            bool shown_prompt = false;

            // Wait for user input
            while (!valid_input) {
                // Show prompt with percentage
                if (!shown_prompt) {
                    int percent = static_cast<int>(100.0 * current_line / total_lines + 0.5);
                    std::cout << "\033[2mMore... [" << percent << "%]\033[22m" << std::endl;
                    shown_prompt = true;
                }

                // Get user input
                switch (get_key_action()) {
                    // Handle line down action
                    case KeyAction::LineDown:
                        std::cout << "\033[F\033[2K" << std::flush;
                        if (current_line < total_lines) {
                            std::cout << lines[current_line++] << '\n';
                        }
                        valid_input = true;
                        break;

                    // Handle page down action
                    case KeyAction::PageDown: {
                        std::cout << "\033[F\033[2K" << std::flush;
                        int lines_to_show = std::min(window_size, total_lines - current_line);
                        // Print the next set of lines
                        std::ranges::for_each(
                            lines | std::views::drop(current_line) | std::views::take(lines_to_show),
                            [](const std::string& line) { std::cout << line << '\n'; }
                        );
                        current_line += lines_to_show;
                        valid_input = true;
                        break;
                    }

                    // Handle quit action
                    case KeyAction::Quit:
                        std::cout << "\033[F\033[2K" << std::flush;
                        return;
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    // Set console to UTF-8 for proper Unicode support
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // Enable ANSI escape sequence processing
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif
    
    try {
        // Parse command line options
        cxxopts::Options options("more", "A modern pager");
        options.add_options()("h,help", "Print usage");
        auto result = options.parse(argc, argv);

        // If help is requested, print usage and exit
        if (result.count("help")) {
            std::cout << "Usage: more [input] (input can be a filename or text)\n";
            std::cout << options.help() << std::endl;
            return 0;
        }

        // Check if stdin is piped
#if defined(_WIN32)
        bool stdin_is_piped = (_isatty(_fileno(stdin)) == 0);
#else
        bool stdin_is_piped = (!isatty(fileno(stdin)));
#endif

        bool has_input = false;
        std::vector<std::string> lines;

        // If stdin is piped, read from it
        if (stdin_is_piped) {
            std::string line;
            while (std::getline(std::cin, line)) {
                lines.push_back(line);
            }
            has_input = !lines.empty();
        }
        // If no input from stdin, check for file or text argument
        else if (argc > 1) {
            std::string input_arg = argv[1];
            std::ifstream file(input_arg);

            if (file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    lines.push_back(line);
                }
                has_input = !lines.empty();
            }
            // Not a file, treat as text
            else {
                std::string line;
                std::istringstream iss(input_arg);
                
                while (std::getline(iss, line)) {
                    lines.push_back(line);
                }
                has_input = !lines.empty();
            }
        }

        // If no input was provided, print usage
        if (!has_input) {
            std::cerr << "Usage: more [input] (input can be a filename or text, or pipe input)\n";
            return 1;
        }

        // If we have input, start the pager
        more_pager(lines);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
