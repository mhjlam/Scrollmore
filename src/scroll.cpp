#include <ranges>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#if defined(_WIN32)
#include <io.h>
#include <stdio.h>
#endif

#include <cxxopts.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>


class ScrollFrame : public ftxui::ComponentBase {
public:
    // Set lines_sub default to 1 (tooltip is one line)
    ScrollFrame(ftxui::ScreenInteractive* screen, const std::vector<std::string>& lines, int lines_sub = 1)
    : screen_(screen)
    , lines_(lines)
    , lines_sub_(lines_sub)
    {
        // Initialize scroll position
        scroll_ = 0;
        Add(ftxui::CatchEvent(ftxui::Renderer([&] { return Render(); }), [&](ftxui::Event event) {
            int window_size = std::max(1, screen_->dimy() - lines_sub_);
            int max_scroll = std::max(0, (int)lines_.size() - window_size);

            if (event == ftxui::Event::ArrowDown) { 
                scroll_ = std::min(scroll_ + 1, max_scroll); 
                return true;
            }
            
            if (event == ftxui::Event::ArrowUp) { 
                scroll_ = std::max(scroll_ - 1, 0); 
                return true;
            }

            if (event == ftxui::Event::PageDown) { 
                scroll_ = std::min(scroll_ + window_size, max_scroll); 
                return true;
            }

            if (event == ftxui::Event::PageUp) { 
                scroll_ = std::max(scroll_ - window_size, 0); 
                return true;
            }

            if (event == ftxui::Event::Home) { 
                scroll_ = 0;
                return true;
            }

            if (event == ftxui::Event::End) { 
                scroll_ = max_scroll; 
                return true;
            }

            if (event == ftxui::Event::Character('q') 
            or  event == ftxui::Event::Character('Q') 
            or  event == ftxui::Event::Escape) {
                if (on_quit_) { 
                    on_quit_();
                }
                return true;
            }
            return false;
        }));
    }

    void OnQuit(std::function<void()> cb) { 
        on_quit_ = std::move(cb);
    }

private:
    ftxui::Element Render() {
        // Tooltip is always one line, so subtract 1 from terminal height
        int window_size = std::max(1, screen_->dimy() - 1);
        int max_scroll = std::max(0, (int)lines_.size() - window_size);
        scroll_ = std::clamp(scroll_, 0, max_scroll);

        // Show as many lines as fit in the terminal (except bottom controls)
        auto visible_view = lines_ | std::views::drop(scroll_) | std::views::take(window_size) | 
            std::views::transform([](const std::string& line) { 
                return ftxui::text(line);
            });
        ftxui::Elements visible_lines{visible_view.begin(), visible_view.end()};

        bool show_up_arrow = scroll_ > 0;
        bool show_down_arrow = (scroll_ + window_size) < (int)lines_.size();

        // If there are more lines above, show (more...) at the top
        if (show_up_arrow && visible_lines.size() > 1) { 
            visible_lines.front() = ftxui::text("(more...)");
        }
        // If there are more lines below, show (more...) at the bottom
        if (show_down_arrow && visible_lines.size() > 1) { 
            visible_lines.back() = ftxui::text("(more...)");
        }

        // Scrollbar calculation
        int bar_height = window_size;
        int total_height = (int)lines_.size();
        int bar_size = std::max(1, window_size * window_size / (3 * std::max(window_size, total_height)));
        int bar_position = (total_height - window_size == 0) ? 0 : scroll_ * (window_size - bar_size) / (total_height - window_size);

        // Create scrollbar elements
        ftxui::Elements scrollbar_elements;
        scrollbar_elements.resize(bar_height, ftxui::text(" "));
        for (int i = bar_position; i < bar_position + bar_size && i < bar_height; ++i) {
            scrollbar_elements[i] = ftxui::text("|");
        }

        // Create the scrollbar element
        auto scrollbar = ftxui::vbox(std::move(scrollbar_elements)) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1);

        // Create highlighted elements for controls
        auto create_highlighted_element = [](const std::string& text, bool highlight) {
            return highlight ? ftxui::text(text) | ftxui::bold | ftxui::color(ftxui::Color::White) 
                             : ftxui::text(text) | ftxui::dim;
        };

        // Create tooltip with controls
        auto tooltip = ftxui::hbox({
            create_highlighted_element("↑", show_up_arrow), ftxui::text("/"),
            create_highlighted_element("↓", show_down_arrow), ftxui::text(": Scroll | ") | ftxui::dim,
            create_highlighted_element("PgUp", show_up_arrow), ftxui::text("/"),
            create_highlighted_element("PgDn", show_down_arrow), ftxui::text(": Scroll Page | ") | ftxui::dim,
            create_highlighted_element("Home", show_up_arrow), ftxui::text("/"),
            create_highlighted_element("End", show_down_arrow), ftxui::text(": Top/Bottom | ") | ftxui::dim,
            create_highlighted_element("Q", true), ftxui::text("/"),
            create_highlighted_element("Esc", true), ftxui::text(": Quit") | ftxui::dim
        });

        // Use yflex and explicit height to fill vertical space and show all lines that fit
        auto lines_box = ftxui::vbox(std::move(visible_lines)) | 
            ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, window_size) | 
            ftxui::flex;

        // Combine lines and scrollbar into a horizontal box
        auto scrollbar_box = scrollbar | 
            ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, window_size) | 
            ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1);

        // Return the final layout with lines and scrollbar, plus tooltip
        return ftxui::vbox({ftxui::hbox({ lines_box, scrollbar_box }), tooltip});
    }

private:
    int scroll_ = 0;                    // Current scroll position
    int lines_sub_;                     // Number of lines reserved for the tooltip
    ftxui::ScreenInteractive* screen_;  // Pointer to the screen for rendering
    std::vector<std::string> lines_;    // Lines to display in the scroller
    std::function<void()> on_quit_;     // Callback for quit event
};


int main(int argc, char* argv[]) {
    try {
        // Parse command line options
        cxxopts::Options options("scroll", "A modern scroller");
        options.add_options()("h,help", "Print usage");
        auto result = options.parse(argc, argv);

        // If help is requested, print usage and exit
        if (result.count("help")) {
            std::cout << "Usage: scroll [input] (input can be a filename or text, or pipe input)\n";
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

        // If stdin is piped, read lines from it
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
            std::cerr << "Usage: scroll [input] (input can be a filename or text, or pipe input)\n";
            return 1;
        }

        // Create the screen and set it to fullscreen
        auto term = ftxui::ScreenInteractive::Fullscreen();

        // Create the ScrollFrame with the lines and a single line for tooltip
        auto list = std::make_shared<ScrollFrame>(&term, lines, 1);

        // Set up quit handler
        list->OnQuit([&] { term.Exit(); });

        // Start the main loop
        term.Loop(list);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
