const std = @import("std");
const HashMap = std.AutoHashMap;
const ArrayList = std.ArrayList;

// const Command = union(enum) {
//     left: u8,
//     right: u8,
//     plus: u8,
//     minus: u8,
//     print,
//     input,
//     loop_start: u16,
//     loop_end: u16,
//     zero,
// };

const Cmd_Kind = enum(u4) {
    left,
    right,
    plus,
    minus,
    print,
    input,
    loop_start,
    loop_end,
    zero,
};

const Cmd = packed struct(u16) {
    k: Cmd_Kind,
    v: u12,
};

fn parse_input(src: []const u8, alloc: std.mem.Allocator) !ArrayList(Cmd) {
    var commands = ArrayList(Cmd).init(alloc);

    var loop_start_ips = ArrayList(u16).init(alloc);
    var i: usize = 0;
    while (i < src.len) : (i += 1) {
        var c = src[i];
        var cmd = @as(Cmd, switch (c) {
            '<', '>' => blk: {
                var cnt: i32 = 0;
                while (i < src.len and (src[i] == '<' or src[i] == '>')) : (i += 1) {
                    if (src[i] == '<') {
                        cnt -= 1;
                    } else {
                        cnt += 1;
                    }
                }
                i -= 1;
                if (cnt < 0) {
                    break :blk Cmd{ .k = .left, .v = @intCast(u12, -cnt) };
                }
                break :blk Cmd{ .k = .right, .v = @intCast(u12, cnt) };
            },
            '+', '-' => blk: {
                var cnt: i32 = 0;
                while (i < src.len and (src[i] == '-' or src[i] == '+')) : (i += 1) {
                    if (src[i] == '-') {
                        cnt -= 1;
                    } else {
                        cnt += 1;
                    }
                }
                i -= 1;
                if (cnt < 0) {
                    break :blk Cmd{ .k = .minus, .v = @intCast(u12, -cnt) };
                }
                break :blk Cmd{ .k = .plus, .v = @intCast(u12, cnt) };
            },
            '.' => .{ .k = .print, .v = undefined },
            ',' => .{ .k = .input, .v = undefined },
            '[' => .{ .k = .loop_start, .v = undefined },
            ']' => .{ .k = .loop_end, .v = undefined },
            else => continue,
        });

        const ip = commands.items.len;

        if (c == '[') {
            if (src[i + 1] == '-' and src[i + 2] == ']') {
                i += 2;
                try commands.append(.{ .k = .zero, .v = undefined });
                continue;
            }
            try loop_start_ips.append(@intCast(u16, ip));
        } else if (c == ']') {
            const start_ip = loop_start_ips.pop();
            commands.items[start_ip] = Cmd{ .k = .loop_start, .v = @intCast(u12, ip) };
            cmd = Cmd{ .k = .loop_end, .v = @intCast(u12, start_ip) };
        }

        try commands.append(cmd);
    }

    return commands;
}

fn run(commands: ArrayList(Cmd)) !void {
    var ip: usize = 0;
    var dp: usize = 0;
    var data = [_]u8{0} ** 3000;

    var stdout = std.io.getStdOut();
    const stdout_writer = stdout.writer();
    var buf_writer = std.io.BufferedWriter(256, @TypeOf(stdout_writer)){ .unbuffered_writer = stdout_writer };
    var buf_writer_writer = buf_writer.writer();

    while (ip != commands.items.len) {
        const cmd = commands.items[ip];
        switch (cmd.k) {
            .left => {
                const times = @intCast(usize, cmd.v);
                if (dp < times) {
                    return;
                }
                dp -= times;
            },
            .right => {
                dp += @intCast(usize, cmd.v);
            },
            .plus => {
                data[dp] = @addWithOverflow(data[dp], @intCast(u8, cmd.v))[0];
            },
            .minus => {
                data[dp] = @subWithOverflow(data[dp], @intCast(u8, cmd.v))[0];
            },
            .print => {
                buf_writer_writer.writeByte(data[dp]) catch {};
                if (data[dp] == '\n') {
                    try buf_writer.flush();
                }
            },
            .input => {},
            .loop_start => {
                if (data[dp] == 0) {
                    ip = cmd.v;
                }
            },
            .loop_end => {
                if (data[dp] != 0) {
                    ip = cmd.v;
                }
            },
            .zero => {
                data[dp] = 0;
            },
        }

        ip += 1;
    }
}

fn feval(src: []const u8, alloc: std.mem.Allocator) !void {
    const commands = try parse_input(src, alloc);
    try run(commands);
}

pub fn main() !void {
    var buffer: [102400]u8 = undefined;
    var fba = std.heap.FixedBufferAllocator.init(&buffer);
    const alloc = fba.allocator();
    const src = try std.fs.cwd().readFileAlloc(alloc, "./mandel.bf", 1024 * 1024);
    try feval(src, alloc);
}
