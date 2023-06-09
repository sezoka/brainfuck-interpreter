use std::collections::HashMap;

#[derive(Debug, Clone, Copy)]
enum Command {
    Left(u8),
    Right(u8),
    Plus(u8),
    Minus(u8),
    Print,
    Input,
    LoopStart,
    LoopEnd,
    Zero,
}

#[derive(Debug)]
struct ParseResult {
    commands: Vec<Command>,
    loop_start_to_end: HashMap<u16, u16>,
    loop_end_to_start: HashMap<u16, u16>,
}

fn parse_input(src: &str) -> Option<ParseResult> {
    let mut result = ParseResult {
        commands: Vec::with_capacity(src.len()),
        loop_start_to_end: HashMap::with_capacity(8),
        loop_end_to_start: HashMap::with_capacity(8),
    };

    let bytes = src.as_bytes();
    let mut loop_start_pos = Vec::with_capacity(8);
    let mut i = 0;
    while i < bytes.len() {
        let c = bytes[i];
        let cmd = match c {
            b'<' | b'>' => {
                let mut cnt: i32 = 0;
                while i < bytes.len() && (bytes[i] == b'<') || (bytes[i] == b'>') {
                    if bytes[i] == b'<' {
                        cnt -= 1;
                    } else {
                        cnt += 1;
                    }
                    i += 1;
                }
                i -= 1;
                if cnt < 0 {
                    Command::Left(-cnt as u8)
                } else {
                    Command::Right(cnt as u8)
                }
            }
            b'-' | b'+' => {
                let mut cnt: i32 = 0;
                while i < bytes.len() && (bytes[i] == b'-') || (bytes[i] == b'+') {
                    if bytes[i] == b'-' {
                        cnt -= 1;
                    } else {
                        cnt += 1;
                    }
                    i += 1;
                }
                i -= 1;
                if cnt < 0 {
                    Command::Minus(-cnt as u8)
                } else {
                    Command::Plus(cnt as u8)
                }
            }
            b'.' => Command::Print,
            b',' => Command::Input,
            b'[' => Command::LoopStart,
            b']' => Command::LoopEnd,
            _ => {
                i += 1;
                continue;
            }
        };

        i += 1;
        let ip = result.commands.len();

        if c == b'[' {
            if bytes[i] == b'-' && bytes[i + 1] == b']' {
                i += 2;
                result.commands.push(Command::Zero);
                continue;
            }
            loop_start_pos.push(ip);
        } else if c == b']' {
            let start_ip = loop_start_pos.pop()?;
            result.loop_start_to_end.insert(start_ip as u16, ip as u16);
            result.loop_end_to_start.insert(ip as u16, start_ip as u16);
        }

        result.commands.push(cmd);
    }
    Some(result)
}

fn run(parsed_data: ParseResult, input: &str) {
    let mut ip = 0;
    let mut dp = 0;
    let mut data = Vec::<u8>::with_capacity(8);
    data.push(0);

    let mut input_iter = input.bytes().into_iter();

    while ip != parsed_data.commands.len() {
        let cmd = parsed_data.commands[ip];
        match cmd {
            Command::Left(t) => {
                let times = t as usize;
                if dp < times {
                    return;
                }
                dp -= times;
            }
            Command::Right(t) => {
                dp += t as usize;
                if data.len() <= dp {
                    data.append(&mut vec![0; dp - data.len() + 1]);
                }
            }
            Command::Plus(rhs) => data[dp] = data[dp].wrapping_add(rhs),
            Command::Minus(rhs) => data[dp] = data[dp].wrapping_sub(rhs),
            Command::Print => {
                print!("{}", char::from(data[dp]));
            }
            Command::Input => match input_iter.next() {
                Some(c) => data[dp] = c,
                None => break,
            },
            Command::LoopStart if data[dp] == 0 => {
                ip = parsed_data.loop_start_to_end[&(ip as u16)] as usize
            }
            Command::LoopEnd if data[dp] != 0 => {
                ip = parsed_data.loop_end_to_start[&(ip as u16)] as usize
            }
            Command::Zero => {
                data[dp] = 0;
            }
            _ => {}
        }

        ip += 1;
    }
}

fn feval(src: &str, input: &str) -> Option<()> {
    return Some(run(parse_input(src)?, input));
}

fn main() {
    let source = include_str!("./mandel.bf");
    let input = "";
    feval(source, input);
}
