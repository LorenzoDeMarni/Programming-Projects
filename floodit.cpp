#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/fl_draw.H>
#include <FL/fl_message.H> // For displaying messages
#include <vector>
#include <random>
#include <set>
#include <iostream>

class Flood : public Fl_Window {
public:
    Flood(int x = 100, int y = 0, int w = 780, int h = 640);
    static void button_cb(Fl_Widget* widget, void* data);
    static void color_button_cb(Fl_Widget* widget, void* data);

private:
    std::vector<Fl_Button*> butlist;
    std::vector<int> colors;
    std::set<int> floodlist;
    std::set<int> bordlist, notbordlist;
    int floodcolor;
    int move_count; // Counter for moves
    Fl_Output* move_display; // Output widget for move count

    void initialize_colors();
    void initialize_buttons();
    void initialize_board();
    void recursive_flooder(int idx, Fl_Color target_color, Fl_Color replacement_color); // Recursive flood-fill method
    bool check_win_condition(); // Check if all non-border buttons are the same color
    void update_move_display(); // Update the move count display
};

Flood::Flood(int x, int y, int w, int h) : Fl_Window(x, y, w, h), move_count(0) {
    begin();
    initialize_colors();
    initialize_buttons();
    initialize_board();

    // Initialize move display
    move_display = new Fl_Output(680, 550, 80, 30);
    move_display->value("0");

    floodcolor = butlist[17]->color();
    floodlist.insert(17);
    recursive_flooder(17, floodcolor, floodcolor);
    end();
}

void Flood::initialize_colors() {
    colors = { FL_RED, FL_BLUE, FL_GREEN, FL_YELLOW, FL_CYAN, FL_MAGENTA };
}

void Flood::initialize_buttons() {
    int button_y = 25;
    for (int i = 0; i < colors.size(); i++) {
        Fl_Button* color_button = new Fl_Button(680, button_y, 60, 40);
        color_button->color(colors[i]);
        color_button->callback(color_button_cb, this);
        button_y += 75;
    }
}

void Flood::initialize_board() {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int index = i * 16 + j;
            Fl_Button* btn = new Fl_Button(j * 40, i * 40, 40, 40);
            btn->box(FL_FLAT_BOX);
            btn->color(colors[rand() % colors.size()]);
            btn->callback(button_cb, this);
            butlist.push_back(btn);

            if (i == 0 || i == 15 || j == 0 || j == 15) {
                bordlist.insert(index);
            }
            else {
                notbordlist.insert(index);
            }
        }
    }

    for (int index : bordlist) {
        butlist[index]->color(FL_BLACK);
        butlist[index]->redraw();
    }
}

void Flood::button_cb(Fl_Widget* widget, void* data) {
    Flood* self = static_cast<Flood*>(data);
    Fl_Button* btn = static_cast<Fl_Button*>(widget);

    for (size_t i = 0; i < self->butlist.size(); i++) {
        if (self->butlist[i] == btn) {
            std::cout << "Button index: " << i << std::endl;
            break;
        }
    }
}

void Flood::color_button_cb(Fl_Widget* widget, void* data) {
    Flood* self = static_cast<Flood*>(data);
    Fl_Button* btn = static_cast<Fl_Button*>(widget);
    Fl_Color replacement_color = btn->color();
    Fl_Color target_color = self->butlist[17]->color();

    if (target_color == replacement_color) {
        return; // No need to flood-fill if the color is the same
    }

    // Clear the floodlist before starting the new flood
    self->floodlist.clear();

    // Start the recursive flood-fill
    self->recursive_flooder(17, target_color, replacement_color);

    // Increment move count and update display
    self->move_count++;
    self->update_move_display();

    // Check if the player has won
    if (self->check_win_condition()) {
        std::string win_message = "Congratulations! You've won in " + std::to_string(self->move_count) + " moves!";
        fl_message(win_message.c_str());
    }
}

void Flood::recursive_flooder(int idx, Fl_Color target_color, Fl_Color replacement_color) {
    // Base case: Out of bounds, color mismatch, or already processed
    if (notbordlist.find(idx) == notbordlist.end() || butlist[idx]->color() != target_color || floodlist.find(idx) != floodlist.end()) {
        return;
    }

    // Add the current index to the flooded list
    floodlist.insert(idx);

    // Change the button color and redraw
    butlist[idx]->color(replacement_color);
    butlist[idx]->redraw();

    // Debug output
    std::cout << "Flooding index: " << idx << ", Color: " << replacement_color << std::endl;

    // Recursively flood in all directions
    recursive_flooder(idx + 1, target_color, replacement_color);  // Right
    recursive_flooder(idx - 1, target_color, replacement_color);  // Left
    recursive_flooder(idx + 16, target_color, replacement_color); // Down
    recursive_flooder(idx - 16, target_color, replacement_color); // Up
}

bool Flood::check_win_condition() {
    // Get the color of the first non-border button
    Fl_Color first_color = butlist[*notbordlist.begin()]->color();

    // Check if all non-border buttons have the same color
    for (int idx : notbordlist) {
        if (butlist[idx]->color() != first_color) {
            return false; // Found a button with a different color
        }
    }
    return true; // All non-border buttons have the same color
}

void Flood::update_move_display() {
    // Update the move count in the display
    move_display->value(std::to_string(move_count).c_str());
}

int main(int argc, char** argv) {
    srand(static_cast<unsigned int>(time(nullptr)));
    Flood app;
    app.show();
    return Fl::run();
}