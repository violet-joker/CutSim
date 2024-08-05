add_rules("mode.debug", "mode.release")

target("main")
    set_kind("binary")
    add_files("src/*.cpp", "build/imgui/*.cpp")
    add_links("GL", "glfw")