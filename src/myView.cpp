#include "myView.h"

float Plate::scale = 0.6f;
float Plate::height_cnt = 0.0f;
float Plate::width_max = 0.0f;
// status: 0 最初板材, 1 需要切割的板材, 2 板材余料
Plate::Plate(ImVec2 size_, int status_ = 0, int cnt_ = 1) : pos(ImVec2(0, 0)), size(size_), status(status_), cnt(cnt_) {}
ImVec2 Plate::get_pos() const { return pos; }
ImVec2 Plate::get_size() const { return size; }
int  Plate::get_status() const { return status; }
int  Plate::get_cnt() const { return cnt; }
void Plate::set_pos(ImVec2 pos_) { pos = pos_; }
void Plate::set_size(ImVec2 size_) { size = size_; }
void Plate::set_status(int status_) { status = status_; }
void Plate::set_cnt(int cnt_) { cnt = cnt_; }
bool Plate::check_size() const { return size.x > 0 && size.y > 0; }
void Plate::swap_width_height() { std::swap(size.x, size.y); }
void Plate::update_pos() { pos = ImVec2(0, height_cnt); height_cnt += size.y + 100; width_max = std::max(width_max, size.x); }
void Plate::add_cnt(int cnt_) { cnt += cnt_; }
void Plate::sub_cnt(int cnt_) { cnt = std::max(0, cnt - cnt_); }
bool Plate::operator<(const Plate& other) const { return size.x == other.get_size().x ? pos.y < other.get_size().y : size.x < other.get_size().x; }
bool Plate::operator>(const Plate& other) const { return size.x == other.get_size().x ? pos.y > other.get_size().y : size.x > other.get_size().x; }
bool Plate::operator==(const Plate& other) const { return size == other.get_size() && ((status == 2 && other.get_status() == 2) ? pos == other.get_pos() : true); }
ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return {a.x + b.x, a.y + b.y}; }
ImVec2 operator*(const ImVec2& a, const float& x)  { return {a.x * x, a.y * x}; };
ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return {a.x - b.x, a.y - b.y}; }
bool   operator==(const ImVec2& a, const ImVec2& b) { return a.x == b.x && a.y == b.y; }

void Plate::draw(const ImVec2& offset) {
    ImColor color = status == 1 ? ImColor(137, 207, 240) : ImColor(0, 255, 0);
    ImGui::GetWindowDrawList()->AddRect(pos * scale + offset, (pos + size) * scale + offset, color);
    std::string msg = std::to_string(int(size.x)) + " x " + std::to_string(int(size.y));

    ImGui::GetWindowDrawList()->AddText(pos * scale + offset, ImColor(255, 255, 255), msg.c_str());
}

// 模板化 add_plate 函数，便于接收less类型和greater类型
template <typename cmp>
void add_plate(const Plate& plate, std::multiset<Plate, cmp>& plate_set) {
    auto it = plate_set.begin();
    while (it != plate_set.end()) {
        if ((*it) == plate) {
            Plate& tmp = const_cast<Plate&> (*it);
            tmp.add_cnt(plate.get_cnt());
            return;
        }
        it++;
    }
    plate_set.insert(plate);
}

// bool值返回是否需要删除迭代器，true则cnt计数为0
template <typename cmp>
bool sub_plate(const Plate& plate, std::multiset<Plate, cmp>& plate_set) {
    auto it = plate_set.begin();
    while (it != plate_set.end()) {
        if ((*it) == plate) {
            Plate& tmp = const_cast<Plate&> (*it);
            tmp.sub_cnt(plate.get_cnt());
            return tmp.get_cnt() == 0;
        }
        it++;
    }
    return false;
}

// 初始化数据(测试)
void init() {
    plate_draw_list.clear();
    free_plates.clear();
    req_plates.clear();
    Plate::height_cnt = 0;
    input_w = input_h = 1.0f;
    input_cnt = 1;
    float a[9] = {1800, 1800, 1800, 1800, 1800, 1750, 1750, 1600, 1600};
    float b[9] = { 200,  200,  200,  190,  190,  200,  200,  180,  180};
    for (int i = 0; i < 9; i++) {
        ImVec2 size = {a[i], b[i]};
        add_plate(Plate(size, 1, 1), req_plates);
    }
    float c[5] = {2000, 2000, 3050, 3500, 2000};
    float d[5] = {1000, 1000, 1220, 1220, 1000};
    for (int i = 0; i < 5; i++) {
        ImVec2 size = {c[i], d[i]};
        add_plate(Plate(size, 0, 1), free_plates);
    }
}

// 遍历切割req_plates集合
void calc_plate() {
    auto req_it = req_plates.begin();
    while (req_it != req_plates.end()) {
        if (cut_plate(*req_it)) {
            // 删除一张板材
            Plate tmp = *req_it;
            tmp.set_cnt(1);
            if (sub_plate(tmp, req_plates))
                req_it = req_plates.erase(req_it);
        } else {
            req_it++;
        }
    }
    // 将余料加入渲染集合
    for (auto i : free_plates) 
        if (i.get_status() != 0)
            plate_draw_list.push_back(i);

}

// 裁剪板材
bool cut_plate(Plate req) {
    auto tmp_it = free_plates.end(); 
    ImVec2 sub;
    float Max = -1;
    auto check([&] {
        auto free_it = free_plates.begin();
        while (free_it != free_plates.end()) {
            sub = free_it->get_size() - req.get_size();
            // 余料从小到大排序，当前满足后续一定满足，找到第一个合适的即可退出
            if (std::min(sub.x, sub.y) >= 0) {
                tmp_it = free_it;
                Max = std::max(sub.x, sub.y);
                return;
            }
            free_it++;
        }
    });

    check(); 
    float Max_tmp = Max;
    req.swap_width_height();
    check();
    if (Max_tmp == Max)
        req.swap_width_height();

    // 没有满足的free板材
    if (tmp_it == free_plates.end())
        return false;

    // 找到了合适的板子，取1张从free_plates删除，后续用free_plate更新其他信息
    Plate free_plate = *tmp_it;
    free_plate.set_cnt(1);
    if (sub_plate(free_plate, free_plates))
        tmp_it = free_plates.erase(tmp_it);
    // 若用到了初始板材，更新坐标(height_cnt)，便于绘图
    if (free_plate.get_status() == 0) {
        free_plate.update_pos();
        free_plate.set_status(2);
    }
    // 将成功裁剪的板材加入渲染集合(req板材的坐标跟随选中的free_plate)
    req.set_pos(free_plate.get_pos());
    plate_draw_list.push_back(req);

    //  没有以剩余长度为指标，而是根据差值选择更优的切法，使得最大余料趋近正方形
    sub = free_plate.get_size() - req.get_size();
    bool flag = sub.x != sub.y
                ? sub.x > sub.y
                : tmp_it->get_size().x > tmp_it->get_pos().y;
    float w = req.get_size().x;
    float h = req.get_size().y;

    // 两块余料
    Plate a, b;
    a = b = free_plate;

    if (flag) {
        // 先横着切
        a.set_pos(a.get_pos() + ImVec2{0, h});
        a.set_size(a.get_size() - ImVec2{0, h});
        b.set_pos(b.get_pos() + ImVec2{w, 0});
        b.set_size(b.get_size() - ImVec2{w, a.get_size().y});
        if (a.check_size()) add_plate(a, free_plates);
        if (b.check_size()) add_plate(b, free_plates);
    } else {
        // 先竖着切
        b.set_pos(b.get_pos() + ImVec2{w, 0});
        b.set_size(b.get_size() - ImVec2{w, 0});
        a.set_pos(a.get_pos() + ImVec2{0, h});
        a.set_size(a.get_size() - ImVec2{b.get_size().x, h});
        if (a.check_size()) add_plate(a, free_plates);
        if (b.check_size()) add_plate(b, free_plates);
    }
    return true;
}

// 遍历所有需要绘制的矩形
void draw_plates() {
    ImVec2 start_pos = ImGui::GetCursorScreenPos();
    for (auto &i : plate_draw_list) {
        i.draw(start_pos + ImVec2(10, 10));
    }
}

// 板材显示窗口
void plate_view() {
    ImGui::SetNextWindowContentSize(ImVec2(std::max(300.0f, Plate::width_max * Plate::scale + 10), std::max(300.0f, Plate::height_cnt * Plate::scale)));
    ImGui::Begin("Plates window", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
    draw_plates();
    ImGui::End();
}

// 操作窗口
void operator_view() {
    ImGui::Begin("Operator window");
    ImGui::Text("Plate::height_cnt = %.2f", Plate::height_cnt);
    ImGui::SetNextItemWidth(250.0f); ImGui::SliderFloat("scale factor", &Plate::scale, 0.1f, 2.0f);
    ImGui::SetNextItemWidth(100.0f); ImGui::InputFloat("width", &input_w); ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f); ImGui::InputFloat("height", &input_h); ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f); ImGui::InputInt("cnt", &input_cnt);
    // 输入数据添加板材
    if (ImGui::Button("add to free_list")) {
        if (input_w && input_h) {
            add_plate(Plate({input_w, input_h}, 0, input_cnt), free_plates);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("add to req_list")) {
        if (input_w && input_h) {
            add_plate(Plate({input_w, input_h}, 1, input_cnt), req_plates);
        }
    }
    if (ImGui::Button("sub from free_list")) {
        if (input_w && input_h) {
            sub_plate(Plate({input_w, input_h}, 0, input_cnt), free_plates);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("sub from req_list")) {
        if (input_w && input_h) {
            sub_plate(Plate({input_w, input_h}, 1, input_cnt), req_plates);
        }
    }
    if (ImGui::Button("calc palte")) {
        calc_plate();
    }
    ImGui::SameLine();
    if (ImGui::Button("init")) {
        init();
    }
    // 显示板材尺寸列表
    ImGui::BeginChild("req_window", ImVec2(200, 300), true);
    ImGui::Text("req_list");
    if (ImGui::BeginTable("req_list", 1)) {
        auto it = req_plates.begin();
        while (it != req_plates.end()) {
            char buf[30];
            ImVec2 size = it->get_size();
            int cnt = it->get_cnt();
            sprintf(buf, "%.2f x %.2f, %d", size.x, size.y, cnt);
            ImGui::TableNextColumn();
            ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
            it++;
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("free_window", ImVec2(200, 300), true);
    ImGui::Text("free_list");
    if (ImGui::BeginTable("free_list", 1)) {
        auto it = free_plates.begin();
        while (it != free_plates.end()) {
            char buf[30];
            ImVec2 size = it->get_size();
            int cnt = it->get_cnt();
            sprintf(buf, "%.2f x %.2f, %d", size.x, size.y, cnt);
            ImGui::TableNextColumn();
            ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
            it++;
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::End();
}