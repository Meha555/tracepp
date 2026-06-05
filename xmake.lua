set_project("tracepp")
set_languages("c++17")

add_rules("mode.debug", "mode.release", "plugin.compile_commands.autoupdate", { outputdir = "build" })

option("build-examples")
    set_default(false)
    set_showmenu(true)
    set_description("Build examples")

target("tracepp")
    set_kind("headeronly")
    add_includedirs("$(scriptdir)", { public = true })
    add_headerfiles("trace.hpp", "event.hpp")

if has_config("build-examples") then
    includes("examples")
end
