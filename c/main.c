#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    LEFT,
    RIGHT,
    PLUS,
    MINUS,
    PRINT,
    INPUT,
    LOOP_START,
    LOOP_END,
    ZERO
} cmd_kind_t;

typedef struct {
    uint16_t kind;
    uint16_t val;
} command_t;

typedef struct {
    command_t* items;
    uint16_t len;
    uint16_t cap;
} commands_t;

typedef struct {
    uint16_t* items;
    uint16_t cap;
    uint16_t len;
} int_vec;

typedef struct {
    commands_t commands;
    int_vec loop_end_to_start;
    int_vec loop_start_to_end;
} parsed_data_t;

commands_t create_commands() {
    commands_t ret = {
        .cap = 8, .len = 0, .items = malloc(sizeof(command_t) * 8)};
    memset(ret.items, 0, sizeof(command_t) * 8);
    return ret;
}

int_vec create_vec() {
    int_vec ret = {.len = 0, .cap = 8, .items = malloc(sizeof(uint16_t) * 8)};
    memset(ret.items, 0, sizeof(uint16_t) * 8);
    return ret;
}

void destroy_commands(commands_t* commands) {
    free(commands->items);
}

void resize_commands(commands_t* commands, uint16_t cap) {
    commands->items = realloc(commands->items, cap * sizeof(command_t));
    memset(commands->items + commands->cap, 0,
           sizeof(command_t) * (cap - commands->cap));
    commands->cap = cap;
}

void extend_vec(int_vec* vec, uint16_t size) {
    fflush(stdout);
    vec->items = realloc(vec->items, (vec->cap + size) * sizeof(uint16_t));
    memset(vec->items + vec->cap, 0, sizeof(uint16_t) * size);
    vec->cap += size;
}

void push_commands(commands_t* commands, command_t command) {
    if (commands->cap <= commands->len) {
        resize_commands(commands, commands->cap * 2);
    }
    commands->items[commands->len] = command;
    commands->len += 1;
}

command_t* pop_commands(commands_t* commands) {
    if (commands->len == 0) {
        return NULL;
    }
    commands->len -= 1;
    return &commands->items[commands->len];
}

void insert_vec(int_vec* vec, uint16_t key, uint16_t val) {
    if (vec->cap <= key) {
        extend_vec(vec, key);
    }
    vec->items[key] = val;
}

void push_vec(int_vec* vec, uint16_t val) {
    if (vec->cap <= vec->len) {
        extend_vec(vec, vec->cap);
    }
    vec->items[vec->len] = val;
    vec->len += 1;
}

uint16_t pop_vec(int_vec* vec) {
    if (vec->len == 0) {
        return 0;
    }
    vec->len -= 1;
    return vec->items[vec->len];
}

void destroy_vec(int_vec* vec) {
    free(vec->items);
}

parsed_data_t parse(char* src, size_t len) {
    commands_t commands = create_commands();
    int_vec loop_start_to_end = create_vec();
    int_vec loop_end_to_start = create_vec();

    int_vec loop_start_ips = create_vec();
    for (size_t i = 0; i < len; i += 1) {
        char c = src[i];
        command_t cmd = {0};

        switch (c) {
            case '<':
            case '>': {
                int16_t cnt = 0;
                for (; i < len && (src[i] == '<' || src[i] == '>'); i += 1) {
                    if (src[i] == '<') {
                        cnt -= 1;
                    } else {
                        cnt += 1;
                    }
                }
                i -= 1;
                if (cnt < 0) {
                    cmd.kind = LEFT;
                    cmd.val = -cnt;
                } else {
                    cmd.kind = RIGHT;
                    cmd.val = cnt;
                }
                break;
            }
            case '+':
            case '-': {
                int16_t cnt = 0;
                for (; i < len && (src[i] == '-' || src[i] == '+'); i += 1) {
                    if (src[i] == '-') {
                        cnt -= 1;
                    } else {
                        cnt += 1;
                    }
                }
                i -= 1;
                if (cnt < 0) {
                    cmd.kind = MINUS;
                    cmd.val = -cnt;
                } else {
                    cmd.kind = PLUS;
                    cmd.val = cnt;
                }
                break;
            }
            case '.': {
                cmd.kind = PRINT;
                break;
            }
            case ',': {
                cmd.kind = INPUT;
                break;
            }
            case '[': {
                cmd.kind = LOOP_START;
                break;
            }
            case ']': {
                cmd.kind = LOOP_END;
                break;
            }
            default:
                continue;
        }

        uint16_t ip = commands.len;

        if (c == '[') {
            if (src[i + 1] == '-' && src[i + 2] == ']') {
                i += 2;
                cmd.kind = ZERO;
                push_commands(&commands, cmd);
                continue;
            }
            push_vec(&loop_start_ips, ip);
        } else if (c == ']') {
            uint16_t start_ip = pop_vec(&loop_start_ips);
            insert_vec(&loop_start_to_end, start_ip, ip);
            insert_vec(&loop_end_to_start, ip, start_ip);
        }

        push_commands(&commands, cmd);
    }

    destroy_vec(&loop_start_ips);

    return (parsed_data_t){.commands = commands,
                           .loop_start_to_end = loop_start_to_end,
                           .loop_end_to_start = loop_end_to_start};
}

void run(parsed_data_t parsed_data) {
    uint16_t ip = 0;
    uint16_t dp = 0;
    int_vec data = create_vec();
    push_vec(&data, 0);

    while (ip < parsed_data.commands.len) {
        command_t cmd = parsed_data.commands.items[ip];
        switch (cmd.kind) {
            case LEFT: {
                uint16_t times = cmd.val;
                if (dp < times) {
                    return;
                }
                dp -= times;
                break;
            }
            case RIGHT: {
                dp += cmd.val;
                if (data.len <= dp) {
                    fflush(stdout);
                    int n = dp - data.len;
                    for (int i = 0; i <= n + 1; i += 1) {
                        push_vec(&data, 0);
                    }
                }
                break;
            }
            case PLUS: {
                data.items[dp] = (data.items[dp] + cmd.val) % 256;
                break;
            }
            case MINUS: {
                data.items[dp] -= cmd.val;
                if (data.items[dp] == data.items[dp] - cmd.val) {
                    data.items[dp] += 256;
                }
                break;
            }
            case PRINT: {
                putchar(data.items[dp]);
                break;
            }
            case LOOP_START: {
                if (data.items[dp] == 0) {
                    ip = parsed_data.loop_start_to_end.items[ip];
                }
                break;
            }
            case LOOP_END: {
                if (data.items[dp] != 0) {
                    ip = parsed_data.loop_end_to_start.items[ip];
                }
                break;
            }
            case ZERO: {
                data.items[dp] = 0;
            }
        }

        fflush(stdout);
        ip += 1;
    }

    destroy_vec(&data);
}

int main() {
    FILE* fptr = fopen("./test.fuck", "r");
    if (fptr == NULL) {
        printf("File not found");
        return 1;
    }

    fseek(fptr, 0, SEEK_END);
    size_t file_len = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    char* src = malloc(sizeof(char) * file_len);
    fread(src, 1, file_len, fptr);

    fclose(fptr);

    parsed_data_t parsed_data = parse(src, file_len);

    run(parsed_data);

    destroy_vec(&parsed_data.loop_start_to_end);
    destroy_vec(&parsed_data.loop_end_to_start);
    destroy_commands(&parsed_data.commands);

    free(src);
}
