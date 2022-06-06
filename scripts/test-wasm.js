require("./amalgamate");

const path = require("path");

process.chdir(path.resolve(__dirname, ".."));

function getEmscriptenSdkPath() {
    if (process.env.EMSDK) {
        return process.env.EMSDK;
    }
    return path.join(process.env.HOME, "dev/emsdk");
}

const toolchainPath = path.join(getEmscriptenSdkPath(), "upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake");
const run = require("child_process").execSync;
run(`cmake -S . -B build-wasm -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=Profiling -DUNIT_LIBRARY_PROJECT=ON -DCMAKE_TOOLCHAIN_FILE=${toolchainPath}` +
    "\n",
    {
        stdio: "inherit"
    });
run("cmake --build build-wasm", {
    stdio: "inherit"
});
run("make test", {
    stdio: "inherit",
    cwd: path.resolve(__dirname, "../build-wasm"),
    env: {
        CTEST_OUTPUT_ON_FAILURE: "TRUE"
    }
});
