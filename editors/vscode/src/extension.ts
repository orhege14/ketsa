import * as vscode from "vscode";
import { findKetsa } from "./ketsaManager";
import { installKetsa } from "./installer";
import { spawn } from "child_process";


let outputChannel: vscode.OutputChannel;


export function activate(context: vscode.ExtensionContext) {

    outputChannel = vscode.window.createOutputChannel(
        "Ketsa Output"
    );


    let ketsaPath = findKetsa();


    if (!ketsaPath) {

        vscode.window.showWarningMessage(
            "Ketsa bulunamadı.",
            "Install Ketsa"
        ).then(async choice => {

            if (choice === "Install Ketsa") {
                await installKetsa();
            }

        });

    } else {

        vscode.window.showInformationMessage(
            "Ketsa bulundu ✅"
        );

    }



    const command = vscode.commands.registerCommand(
        "ketsa.run",
        () => {


            const editor =
                vscode.window.activeTextEditor;


            if (!editor) {

                vscode.window.showErrorMessage(
                    "Açık Ketsa dosyası yok!"
                );

                return;
            }



            const file =
                editor.document.fileName;



            const executable =
                ketsaPath ?? "ketsa";



            outputChannel.clear();

            outputChannel.show();

            outputChannel.appendLine(
                "Running Ketsa..."
            );

            outputChannel.appendLine(
                `File: ${file}\n`
            );



            const process =
                spawn(
                    executable,
                    [
                        "run",
                        file
                    ],
                    {
                        windowsHide: true
                    }
                );



            process.stdout.on(
                "data",
                (data) => {

                    outputChannel.append(
                        data.toString()
                    );

                }
            );



            process.stderr.on(
                "data",
                (data) => {

                    outputChannel.appendLine(
                        "\nERROR:"
                    );

                    outputChannel.append(
                        data.toString()
                    );

                }
            );



            process.on(
                "close",
                (code) => {

                    outputChannel.appendLine(
                        `\nProcess finished (${code})`
                    );

                }
            );


        }
    );



    context.subscriptions.push(command);

}



export function deactivate() {

    if (outputChannel) {
        outputChannel.dispose();
    }

}