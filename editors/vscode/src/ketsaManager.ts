import * as fs from "fs";
import * as path from "path";
import * as child_process from "child_process";


export function findKetsa(): string | null {

    // PATH kontrol
    try {
        const result = child_process.execSync(
            "where ketsa",
            { encoding: "utf8" }
        ).trim();

        if (result) {
            return result.split("\n")[0];
        }

    } catch {}


    // Kullanıcı kurulumu
    const userInstall = path.join(
        process.env.USERPROFILE || "",
        ".ketsa",
        "bin",
        "ketsa.exe"
    );


    if (fs.existsSync(userInstall)) {
        return userInstall;
    }


    // Extension içine gömülü runtime
    const extensionKetsa = path.join(
        __dirname,
        "..",
        "runtime",
        "ketsa.exe"
    );


    if (fs.existsSync(extensionKetsa)) {
        return extensionKetsa;
    }


    return null;
}