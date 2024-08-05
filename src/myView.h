#include "../build/imgui/imgui.h"
#include <string>
#include <vector>
#include <set>
#include <functional>

class Plate {
public:
    Plate() {}
    Plate(ImVec2 size_);
    Plate(ImVec2 size_, int status_);
    ImVec2 get_pos() const;
    ImVec2 get_size() const;
    int get_status() const;
    void set_pos(ImVec2 pos_);
    void set_size(ImVec2 size_);
    void set_status(int status_);
    bool check_size() const;
    void swap_width_height();
    void update_pos();
    void draw(const ImVec2& offset);
    bool operator<(const Plate& other) const;
    bool operator>(const Plate& other) const;
public:
    static float scale;
    static float height_cnt;
    static float width_max;
private:
    ImVec2 pos;
    ImVec2 size;
    int status;
};


typedef std::multiset<Plate>::iterator Iterator;
static std::multiset<Plate, std::less<Plate>> free_plates;
static std::multiset<Plate, std::greater<Plate>> req_plates;
static std::vector<Plate> plate_draw_list;
static float input_w, input_h;

ImVec2 operator+(const ImVec2& a, const ImVec2& b);
ImVec2 operator*(const ImVec2& a, const float& x);
ImVec2 operator-(const ImVec2& a, const ImVec2& b);

void draw_plates();
void init();
void calc_plate();
bool cut_plate(Plate req);
void plate_view();
void operator_view();