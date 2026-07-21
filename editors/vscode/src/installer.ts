import * as vscode from "vscode";
import * as fs from "fs";
import * as path from "path";


export async function installKetsa() {

    const installDir = path.join(
        process.env.USERPROFILE || "",
        ".ketsa",
        "bin"
    );


    try {

        if (!fs.existsSync(installDir)) {
            fs.mkdirSync(installDir, {
                recursive: true
            });
        }


        vscode.window.showInformationMessage(
            "Ketsa kurulumu başlatılıyor..."
        );


        /*
            Buraya sonra:

            1. GitHub Release indirilecek
            2. Zip açılacak
            3. ketsa.exe buraya konulacak:

            C:\Users\User\.ketsa\bin\ketsa.exe

        */


        vscode.window.showInformationMessage(
            "Ketsa installer hazır ✅"
        );


    } catch (error) {

        vscode.window.showErrorMessage(
            "Ketsa kurulumu başarısız: " + error
        );

    }
}