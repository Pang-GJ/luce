-- define project
set_project("luce")
set_xmakever("2.7.0")
set_version("0.1.0", {build = "%Y%m%d%H%M"})

-- set common flags
set_warnings("all")
set_languages("cxx20")

-- add build mode
-- add_rules("mode.release", "mode.debug")
add_rules("mode.debug")

-- inclue subdirs
includes("luce", "tests", "example")

-- run script
target("check-lint")
  set_kind("phony")

  on_run(function (target)
    os.run("sh $(projectdir)/script/check_lint.sh")
  end)

target("check-tidy")
  set_kind("phony")

  on_run(function (target)
    os.run("xmake project -k compile_commands")
    --[[ os.run("sh $(projectdir)/script/check_tidy.sh") ]]
    os.run("python $(projectdir)/script/run_clang_tidy.py -j 12")
  end)
