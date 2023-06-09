import fs from "fs";

import std, { ArrayList } from "tstd";

enum CommandKind {
    Left,
    Right,
    Plus,
    Minus,
    Print,
    Input,
    LoopStart,
    LoopEnd,
    Zero,
}

type Command = [CommandKind, number];

function parse(src: string): std.ArrayList<Command> {
    const commands = std.ArrayList.init<Command>();

    const loop_start_ips: number[] = [];
    let i = 0;
    while (i < src.length) {
        const c = src[i];
        let cmd_kind = CommandKind.Zero;
        let cmd_times = 0;

        if (c === "<" || c === ">") {
            let cnt = 0;
            while (i < src.length && (src[i] === "<" || src[i] === ">")) {
                if (src[i] === "<") {
                    cnt -= 1;
                } else {
                    cnt += 1;
                }
                i += 1;
            }
            i -= 1;
            if (cnt < 0) {
                cmd_kind = CommandKind.Left;
                cmd_times = -cnt;
            } else {
                cmd_kind = CommandKind.Right;
                cmd_times = cnt;
            }
        } else if (c === "-" || c === "+") {
            let cnt = 0;
            while (i < src.length && (src[i] === "-" || src[i] === "+")) {
                if (src[i] === "-") {
                    cnt -= 1;
                } else {
                    cnt += 1;
                }
                i += 1;
            }
            i -= 1;
            if (cnt < 0) {
                cmd_kind = CommandKind.Minus;
                cmd_times = -cnt;
            } else {
                cmd_kind = CommandKind.Plus;
                cmd_times = cnt;
            }
        } else if (c === ".") {
            cmd_kind = CommandKind.Print;
        } else if (c === ",") {
            cmd_kind = CommandKind.Input;
        } else if (c === "[") {
            cmd_kind = CommandKind.LoopStart;
        } else if (c === "]") {
            cmd_kind = CommandKind.LoopEnd;
        } else {
            i += 1;
            continue;
        }

        i += 1;
        const ip = commands.len;

        if (c === "[") {
            if (src[i] === "-" && src[i + 1] === "]") {
                i += 2;
                cmd_kind = CommandKind.Zero;
                commands.append([cmd_kind, cmd_times]);
                continue;
            }
            loop_start_ips.push(ip);
        } else if (c === "]") {
            const start_ip = loop_start_ips.pop();
            if (start_ip === undefined) {
                std.debug.println("Unmatching brackets");
                process.exit(1);
            }
            const start_cmd = commands.getUnchecked(start_ip);
            start_cmd[1] = ip;
            cmd_times = start_ip;
        }

        commands.append([cmd_kind, cmd_times]);
    }

    return commands;
}


function run(commands: std.ArrayList<Command>): void {
    let ip = 0;
    let dp = 0;
    let data = ArrayList.initCapacity<number>(3000);
    data.fill(0);

    process.stdout.cork();

    while (ip < commands.len) {
        const [cmd_kind, cmd_times] = commands.getUnchecked(ip);
        const value = data.getUnchecked(dp);

        switch (cmd_kind) {
            case CommandKind.Left: {
                const times = cmd_times;
                dp -= times;
                if (dp < 0) {
                    return;
                }
                break;
            }
            case CommandKind.Right: {
                dp += cmd_times;
                break;
            }
            case CommandKind.Plus: {
                const rhs = cmd_times;
                data.insertUnchecked(dp, (value + rhs) % 256);
                break;
            }
            case CommandKind.Minus: {
                const rhs = cmd_times;
                let new_value = value - rhs;
                if (new_value < 0) {
                    new_value += 255;
                }
                data.insertUnchecked(dp, new_value);
                break;
            }
            case CommandKind.Print: {
                const char = String.fromCharCode(value);
                if (char === "\n") {
                    process.stdout.uncork();
                    process.stdout.cork();
                }
                process.stdout.write(String.fromCharCode(value));
                break;
            }
            case CommandKind.Input: {
                std.debug.println("Input is not implemented!");
                break;
            }
            case CommandKind.LoopStart: {
                if (value === 0) {
                    ip = cmd_times;
                }
                break;
            }
            case CommandKind.LoopEnd:
                if (value !== 0) {
                    ip = cmd_times;
                }
                break;
            case CommandKind.Zero:
                data.insertUnchecked(dp, 0);
                break;
        }

        ip += 1;
    }
}

async function runFile(path: string) {
    try {
        const src = fs.readFileSync(path).toString();
        const commands = parse(src);
        run(commands);
    } catch (err) {
        std.debug.println(`cannot read file "${path}", msg: ${err}`);
    }
}

async function main() {
    const args = process.argv;
    if (3 < args.length) {
        std.debug.println("Usage: bfuck [script]");
        process.exit(64);
    } else if (args.length == 3) {
        return await runFile(args[2]);
    }

    return process.exit(0);
}

main();
