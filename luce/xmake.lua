add_requires("spdlog", {system = false, configs = {header_only = true, fmt_external = true}})
add_requireconfs("spdlog.fmt", {system = false, override = true, version = "9.1.0", configs = {header_only = true}})

target("luce")
    --if is_mode("mode.debug") then
        --[[ add_defines("LUCEDEBUG") ]]
        --[[ add_defines("USE_ORIGINAL") ]]
    --end
    set_kind("shared")
    add_files("$(projectdir)/luce/**.cpp")
    add_includedirs("$(projectdir)/", { public = true })
    add_packages("spdlog", "fmt", {public = true})
target_end()
