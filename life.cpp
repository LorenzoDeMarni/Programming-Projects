#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>

class Life : public Fl_Double_Window {
private:
    std::vector<Fl_Button*> buttons;
    std::vector<int> init_list;
    std::vector<int> new_list;
    Fl_Light_Button* start_button;
    Fl_Button* reset_button;
    Fl_Button* glider_button;
    Fl_Button* random_button;
    std::vector<int> not_border_list;
    int generation;

    // Predefined glider pattern centered
    const std::vector<int> glider = { 162, 243, 244, 164, 84 };

public:
    Life(int width, int height) : Fl_Double_Window(width, height, "Conway's Game of Life") {
        generation = 0;
        std::srand(std::time(0)); // Initialize random seed
        begin();

        // Create grid buttons
        for (int row = 0; row < 80; ++row) {
            for (int col = 0; col < 80; ++col) {
                auto* btn = new Fl_Button(col * 10, row * 10, 10, 10);
                btn->box(FL_FLAT_BOX);
                btn->color(FL_WHITE);
                btn->callback(button_callback, this);
                buttons.push_back(btn);
                init_list.push_back(0);
            }
        }

        // Create control buttons
        start_button = new Fl_Light_Button(850, 50, 70, 40, "Start");
        start_button->color(FL_GREEN);
        start_button->callback(start_callback, this);

        reset_button = new Fl_Button(850, 100, 70, 40, "Reset");
        reset_button->color(FL_RED);
        reset_button->callback(reset_callback, this);

        glider_button = new Fl_Button(850, 150, 70, 40, "Glider");
        glider_button->color(FL_CYAN);
        glider_button->callback(glider_callback, this);

        random_button = new Fl_Button(850, 200, 70, 40, "Random");
        random_button->color(FL_YELLOW);
        random_button->callback(random_callback, this);

        // Define non-border cells
        for (int i = 0; i < 6400; ++i) {
            if (i > 80 && i < 6319 && i % 80 != 79 && i % 80 != 0) {
                not_border_list.push_back(i);
            }
        }

        end();
    }

    static void button_callback(Fl_Widget* w, void* data) {
        auto* self = (Life*)data;
        auto* btn = (Fl_Button*)w;
        int index = std::distance(self->buttons.begin(), std::find(self->buttons.begin(), self->buttons.end(), btn));

        if (self->not_border_list.end() != std::find(self->not_border_list.begin(), self->not_border_list.end(), index)) {
            if (self->init_list[index] == 1) {
                btn->color(FL_WHITE);
                self->init_list[index] = 0;
            }
            else {
                btn->color(FL_BLACK);
                self->init_list[index] = 1;
            }
            btn->redraw();
        }
    }

    static void start_callback(Fl_Widget* w, void* data) {
        auto* self = (Life*)data;
        if (self->start_button->value()) {
            self->reset_button->deactivate();
            self->glider_button->deactivate();
            self->random_button->deactivate();
            while (self->start_button->value()) {
                self->simulate();
                Fl::check();
                Fl::wait(0.05); // Faster simulation
            }
        }
        else {
            self->reset_button->activate();
            self->glider_button->activate();
            self->random_button->activate();
        }
    }

    static void reset_callback(Fl_Widget* w, void* data) {
        auto* self = (Life*)data;
        self->init_list.assign(6400, 0);
        for (auto* btn : self->buttons) {
            btn->color(FL_WHITE);
            btn->redraw();
        }
        self->generation = 0;
        std::cout << "Reset simulation!" << std::endl;
    }

    static void glider_callback(Fl_Widget* w, void* data) {
        auto* self = (Life*)data;
        for (int i : self->glider) {
            self->init_list[i] = 1;
            self->buttons[i]->color(FL_BLACK);
            self->buttons[i]->redraw();
        }
        std::cout << "Glider pattern set!" << std::endl;
    }

    static void random_callback(Fl_Widget* w, void* data) {
        auto* self = (Life*)data;
        self->init_list.assign(6400, 0);
        for (auto* btn : self->buttons) {
            btn->color(FL_WHITE);
        }

        for (int i : self->not_border_list) {
            if (std::rand() % 2) {
                self->init_list[i] = 1;
                self->buttons[i]->color(FL_BLACK);
            }
        }

        for (auto* btn : self->buttons) {
            btn->redraw();
        }

        std::cout << "Random pattern generated!" << std::endl;
    }

    void simulate() {
        new_list.assign(6400, 0);
        for (int i : not_border_list) {
            int neighbors = init_list[i - 1] + init_list[i + 1] + init_list[i - 80] + init_list[i + 80] +
                init_list[i - 81] + init_list[i - 79] + init_list[i + 79] + init_list[i + 81];

            if (init_list[i] == 1) {
                new_list[i] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            }
            else {
                new_list[i] = (neighbors == 3) ? 1 : 0;
            }
        }

        for (int i = 0; i < 6400; ++i) {
            if (new_list[i] == 1) {
                buttons[i]->color(FL_BLACK);
            }
            else {
                buttons[i]->color(FL_WHITE);
            }
            buttons[i]->redraw();
        }

        init_list = new_list;
        ++generation;
        std::cout << "Generation: " << generation << std::endl;
    }

    void show_game() {
        show();
        Fl::run();
    }
};

int main() {
    Life game(1000, 800);
    game.show_game();
    return 0;
}