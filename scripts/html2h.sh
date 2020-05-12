#!/usr/bin/env node

// expected command: html2js.sh /path/to/html/file.hml /path/to/output.h PAGE_NAME
// this turns html files into a .h file

const fs = require('fs');

function main() {
    try {

        if (process.argv.length !== 5) {
            return console.log("Usage: html2h.sh /path/to/html/file.hml /path/to/output.h PAGE_NAME");
        }

        const htmlFile = process.argv[2];
        const hFile = process.argv[3];
        const htmlFileContents = fs.readFileSync(htmlFile).toString();
        if (htmlFileContents.length < 1) {
            return console.log("Error! Empty HTML File!");
        }
        if (fs.existsSync(hFile)) {
            // this is a race condition if anyone is wondering
            fs.unlinkSync(hFile);
        }
        fs.appendFileSync(hFile, "#include <pgmspace.h>\n\n");
        fs.appendFileSync(hFile, `const char ${process.argv[4]}[] PROGMEM = R"=====(\n`);
        fs.appendFileSync(hFile, htmlFileContents);
        fs.appendFileSync(hFile, "\n)=====\";\n");
    } catch (e) {
        console.log(`An error occured: ${e}`);
    }
}
main();
