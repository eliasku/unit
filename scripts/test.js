require("./amalgamate");

const path = require("path");

process.chdir(path.resolve(__dirname, ".."));

const run = require("child_process").execSync;
run("cmake -S . -B build -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=Profiling -DUNIT_LIBRARY_PROJECT=ON -DCODE_COVERAGE=ON",
    {
        stdio: "inherit"
    });
run("cmake --build build", {
    stdio: "inherit"
});
run("make coverage", {
    stdio: "inherit",
    cwd: path.resolve(__dirname, "../build")
});