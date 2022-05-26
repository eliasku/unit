const fs = require("fs");
const path = require("path");

const pkg = JSON.parse(fs.readFileSync("package.json", "utf-8"));
const vars = {
    VERSION: pkg.version,
    DESCRIPTION: pkg.description
};

const re = /^\s*#\s*include\s*"([^"]+)"/gm;
function inject(filepath) {
    let code = fs.readFileSync(filepath, "utf-8");
    code = code.replace(/\${([\w\d_]+)}/gm, (_, id) => vars[id] ?? _);
    return code.replace(re, (_, file) => inject(path.resolve(path.dirname(filepath), file)));
}

function amalgamate(input, output) {
    const code = inject(input);
    fs.writeFileSync(output, code, "utf-8");
}

amalgamate(
    path.resolve(__dirname, "../src/unit.h"),
    path.resolve(__dirname, "../include/unit.h")
);
