#include "../build/imgui/imgui.h"
#include <string>
#include <vector>
#include <set>
#include <functional>

class Plate {
public:
    Plate() {}
    Plate(ImVec2 size_, int status_, int cnt);
    ImVec2 get_pos() const;
    ImVec2 get_size() const;
    int  get_status() const;
    int  get_cnt() const;
    void set_pos(ImVec2 pos_);
    void set_size(ImVec2 size_);
    void set_status(int status_);
    void set_cnt(int cnt_);
    bool check_size() const;
    void swap_width_height();
    void update_pos();
    void add_cnt(int cnt_);
    void sub_cnt(int cnt_);
    void draw(const ImVec2& offset);
    bool operator<(const Plate& other) const;
    bool operator>(const Plate& other) const;
    bool operator==(const Plate& other) const;
public:
    static float scale;
    static float height_cnt;
    static float width_max;
private:
    ImVec2 pos;
    ImVec2 size;
    int status;
    int cnt;
};


static std::multiset<Plate, std::less<Plate>> free_plates;
static std::multiset<Plate, std::greater<Plate>> req_plates;
static std::vector<Plate> plate_draw_list;
static float input_w, input_h;
static int input_cnt;

ImVec2 operator+(const ImVec2& a, const ImVec2& b);
ImVec2 operator*(const ImVec2& a, const float& x);
ImVec2 operator-(const ImVec2& a, const ImVec2& b);
bool  operator==(const ImVec2& a, const ImVec2& b);

void draw_plates();
template <typename cmp>
void add_plate(const Plate& plate, std::multiset<Plate, cmp>& plate_set);
template <typename cmp>
bool sub_plate(const Plate& plate, std::multiset<Plate, cmp>& plate_set);
void init();
void calc_plate();
bool cut_plate(Plate req);
void plate_view();
void operator_view();