#include "myView.h"

float Plate::scale = 0.23f;
float Plate::height_cnt = 0.0f;
// status: 0 最初板材, 1 需要切割的板材, 2 板材余料
Plate::Plate(ImVec2 size_) : size(size_), status(1) {}
Plate::Plate(ImVec2 size_, int status_) : size(size_), status(status_) { pos = ImVec2(0, height_cnt), height_cnt += size.y + 100; }
ImVec2 Plate::get_pos() const { return pos; }
ImVec2 Plate::get_size() const { return size; }
int Plate::get_status() const { return status; }
void Plate::set_pos(ImVec2 pos_) { pos = pos_; }
void Plate::set_size(ImVec2 size_) { size = size_; }
void Plate::set_status(int status_) { status = status_; }
bool Plate::check_size() const { return size.x > 0 && size.y > 0; }
bool Plate::operator<(const Plate& other) const { return pos.x != other.get_pos().x ? pos.x < other.get_pos().x : pos.y < other.get_pos().y; }
bool Plate::operator>(const Plate& other) const { return pos.x != other.get_pos().x ? pos.x > other.get_pos().x : pos.y > other.get_pos().y; }
ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return {a.x + b.x, a.y + b.y}; }
ImVec2 operator*(const ImVec2& a, const float& x){ return {a.x * x, a.y * x}; };
ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return {a.x - b.x, a.y - b.y}; }

void Plate::draw(const ImVec2& offset) {
    ImColor color = status == 1 ? ImColor(137, 207, 240) : ImColor(0, 255, 0);
    ImGui::GetWindowDrawList()->AddRect(pos * scale + offset, (pos + size) * scale + offset, color);
    std::string msg = std::to_string(int(size.x)) + " x " + std::to_string(int(size.y));
    ImGui::GetWindowDrawList()->AddText(pos * scale + offset, ImColor(255, 255, 255), msg.c_str());
}

void Plate::swap_width_height() {
    std::swap(size.x, size.y);
}

// 初始化数据(测试)
void init() {
    plate_draw_list.clear();
    free_plates.clear();
    req_plates.clear();
    // free_plates.insert({{200, 100}, 0});
    float a[9] = {1800, 1800, 1800, 1800, 1800, 1750, 1750, 1600, 1600};
    float b[9] = { 200,  200,  200, 190, 190, 200, 200, 180, 180};
    for (int i = 0; i < 9; i++) {
        ImVec2 size = {a[i], b[i]};
        req_plates.insert(Plate{size});
    }
    float c[5] = {2000, 2000, 3050, 3500, 2000};
    float d[5] = {1000, 1000, 1220, 1220, 1000};
    for (int i = 0; i < 5; i++) {
        ImVec2 size = {c[i], d[i]};
        free_plates.insert(Plate{size, 0});
    }
}

// 计算切割情况
void calc_plate() {
    Iterator req_it = req_plates.begin();
    while (req_it != req_plates.end()) {
        if (cut_plate(*req_it)) {
            req_it = req_plates.erase(req_it);
        } else {
            req_it++;
        }
    }
    // 将余料加入渲染集合
    for (auto i : free_plates) 
        if (i.get_status() == 2)
            plate_draw_list.push_back(i);

}

// 裁剪板材
bool cut_plate(Plate req) {
    Iterator tmp_it = free_plates.end(); 
    ImVec2 sub;
    float Max = -1;
    // 遍历free板材，保留
    auto check([&] {
        Iterator free_it = free_plates.begin();
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

    // 将成功裁剪的板材加入渲染集合
    req.set_pos(tmp_it->get_pos());
    plate_draw_list.push_back(req);

    //  没有以剩余长度为指标，而是根据差值选择更优的切法，使得余料趋近正方形
    sub = tmp_it->get_size() - req.get_size();
    bool flag = sub.x != sub.y
                ? sub.x > sub.y
                : tmp_it->get_size().x > tmp_it->get_pos().y;
    float w = req.get_size().x;
    float h = req.get_size().y;
    Plate a, b;
    // 标记为余料，将余料添加到free集合里，使用过的删掉
    a = b = *tmp_it;
    a.set_status(2);
    b.set_status(2);
    free_plates.erase(tmp_it);

    if (flag) {
        // 横着切
        a.set_pos(a.get_pos() + ImVec2{0, h});
        a.set_size(a.get_size() - ImVec2{0, h});
        b.set_pos(b.get_pos() + ImVec2{w, 0});
        b.set_size(b.get_size() - ImVec2{w, a.get_size().y});
        if (a.check_size()) free_plates.insert(a);
        if (b.check_size()) free_plates.insert(b);
    } else {
        // 竖着切
        b.set_pos(b.get_pos() + ImVec2{w, 0});
        b.set_size(b.get_size() - ImVec2{w, 0});
        a.set_pos(a.get_pos() + ImVec2{0, h});
        a.set_size(a.get_size() - ImVec2{b.get_size().x, h});
        if (a.check_size()) free_plates.insert(a);
        if (b.check_size()) free_plates.insert(b);
    }
    return true;
}

// 遍历所有需要绘制的矩形
void draw_plates() {
    ImVec2 start_pos = ImGui::GetCursorScreenPos();
    for (auto &i : plate_draw_list) {
        i.draw(start_pos);
    }
}

// 板材显示窗口
void plate_view() {
    ImGui::SetNextWindowContentSize(ImVec2(300, 2000));
    ImGui::Begin("my window");
    draw_plates();
    ImGui::End();
}

// 操作窗口
void operator_view() {
    ImGui::Begin("Operator window");
    ImGui::Text("Plate::height_cnt = %.2f", Plate::height_cnt);
    ImGui::SliderFloat("float", &Plate::scale, 0.1f, 1.0f);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputFloat("width", &input_w); ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputFloat("height", &input_h);
    // 输入数据添加板材
    if (ImGui::Button("add to free_list")) {
        if (input_w && input_h) {
            free_plates.insert(Plate{ImVec2(input_w, input_h), 0});
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("add to req_list")) {
        if (input_w && input_h) {
            req_plates.insert(Plate{ImVec2(input_w, input_h)});
        }
    }

    if (ImGui::Button("calc palte")) {
        calc_plate();
    }
    ImGui::SameLine();
    if (ImGui::Button("init")) {
        init();
    }
    ImGui::BeginChild("req_window", ImVec2(200, 300), true);
    ImGui::Text("req_list");
    if (ImGui::BeginTable("req_list", 1)) {
        for (auto &i : req_plates) {
            char buf[20];
            sprintf(buf, "%.2f x %.2f", i.get_size().x, i.get_size().y);
            ImGui::TableNextColumn();
            ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("free_window", ImVec2(200, 300), true);
    ImGui::Text("free_list");
    if (ImGui::BeginTable("free_list", 1)) {
        for (auto &i : free_plates) {
            char buf[20];
            sprintf(buf, "%.2f x %.2f", i.get_size().x, i.get_size().y);
            ImGui::TableNextColumn();
            ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::End();
}