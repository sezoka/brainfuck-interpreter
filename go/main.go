package main

import (
	"fmt"
	"os"
)

const (
	LEFT       = 0
	RIGHT      = 1
	PLUS       = 2
	MINUS      = 3
	PRINT      = 4
	INPUT      = 5
	LOOP_START = 6
	LOOP_END   = 7
	ZERO       = 8
)

type Command struct {
	kind int
	val  int
}

type ParseResult struct {
	commands          []Command
	loop_end_to_start map[int]int
	loop_start_to_end map[int]int
}

func parse(src []byte) ParseResult {
	result := ParseResult{make([]Command, 0), map[int]int{}, map[int]int{}}

	loop_start_ips := make([]int, 0)
	i := 0

	for i < len(src) {
		c := src[i]
		cmd := Command{ZERO, 0}

		if c == '<' || c == '>' {
			cnt := 0
			for i < len(src) && (src[i] == '<' || src[i] == '>') {
				if src[i] == '<' {
					cnt -= 1
				} else {
					cnt += 1
				}
				i += 1
			}
			i -= 1
			if cnt < 0 {
				cmd.kind = LEFT
				cmd.val = -cnt
			} else {
				cmd.kind = RIGHT
				cmd.val = cnt
			}
		} else if c == '-' || c == '+' {
			cnt := 0
			for i < len(src) && (src[i] == '-' || src[i] == '+') {
				if src[i] == '-' {
					cnt -= 1
				} else {
					cnt += 1
				}
				i += 1
			}
			i -= 1
			if cnt < 0 {
				cmd.kind = MINUS
				cmd.val = -cnt
			} else {
				cmd.kind = PLUS
				cmd.val = cnt
			}
		} else if c == '.' {
			cmd.kind = PRINT
		} else if c == ',' {
			cmd.kind = INPUT
		} else if c == '[' {
			cmd.kind = LOOP_START
		} else if c == ']' {
			cmd.kind = LOOP_END
		} else {
			i += 1
			continue
		}

		i += 1
		ip := len(result.commands)

		if c == '[' {
			if src[i] == '-' && src[i+1] == ']' {
				i += 2
				cmd.kind = ZERO
				result.commands = append(result.commands, cmd)
				continue
			}
			loop_start_ips = append(loop_start_ips, ip)
		} else if c == ']' {
			start_ip := loop_start_ips[len(loop_start_ips)-1]
			loop_start_ips = loop_start_ips[:len(loop_start_ips)-1]
			result.loop_start_to_end[start_ip] = ip
			result.loop_end_to_start[ip] = start_ip
		}

		result.commands = append(result.commands, cmd)
	}

	return result

}

func run(parsed_data ParseResult) {
	ip := 0
	dp := 0
	data := make([]int, 0)
	data = append(data, 0)

	commands := parsed_data.commands
	loop_start_to_end := parsed_data.loop_start_to_end
	loop_end_to_start := parsed_data.loop_end_to_start

	for ip < len(commands) {
		cmd := commands[ip]
		switch cmd.kind {
		case LEFT:
			{
				times := cmd.val
				dp -= times
				if dp < 0 {
					return
				}
			}
		case RIGHT:
			{
				dp += cmd.val
				if len(data) <= dp {
					n := dp - len(data)
					for i := 0; i <= n+1; i += 1 {
						data = append(data, 0)
					}
				}
			}
		case PLUS:
			{
				rhs := cmd.val
				data[dp] = (data[dp] + rhs) % 256
			}
		case MINUS:
			{
				rhs := cmd.val
				data[dp] = data[dp] - rhs
				if data[dp] < 0 {
					data[dp] += 256
				}
			}
		case PRINT:
			{
				fmt.Printf("%c", data[dp])
			}
		case INPUT:
			{
			}
		case LOOP_START:
			{
				if data[dp] == 0 {
					ip = loop_start_to_end[ip]
				}
			}
		case LOOP_END:
			if data[dp] != 0 {
				ip = loop_end_to_start[ip]
			}
			break
		case ZERO:
			data[dp] = 0
			break
		}

		ip += 1
	}
}

func main() {
	content, error := os.ReadFile("./test.fuck")
	if error != nil {
		return
	}
	parse_result := parse(content)
    run(parse_result)
}
