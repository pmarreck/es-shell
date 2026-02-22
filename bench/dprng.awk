function dprng_next(    out) {
	# Park-Miller minimal standard LCG.
	# Range: 1..2147483646
	state = (state * 48271) % 2147483647
	out = state
	return out
}

function rand_char(    idx) {
	idx = (dprng_next() % length(alphabet)) + 1
	return substr(alphabet, idx, 1)
}

function emit_rows(count, width,    i, j, line) {
	for (i = 1; i <= count; i++) {
		line = sprintf("row-%06d|", i)
		for (j = 1; j <= width; j++) {
			line = line rand_char()
		}
		print line
	}
}

function emit_tokens(count, width,    i, j, token) {
	for (i = 1; i <= count; i++) {
		token = "w"
		for (j = 1; j <= width; j++) {
			token = token rand_char()
		}
		printf "%s%s", token, (i < count ? " " : "\n")
	}
}

function emit_numbers(count,    i, checksum) {
	checksum = 0
	for (i = 1; i <= count; i++) {
		checksum = checksum + dprng_next()
	}
	print checksum
}

BEGIN {
	mode = (mode == "" ? "rows" : mode)
	count = (count == "" ? 1000 : count)
	width = (width == "" ? 48 : width)
	seed = (seed == "" ? 424242 : seed)
	alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

	# Keep seed in the valid LCG range and avoid zero state.
	state = seed % 2147483647
	if (state <= 0) {
		state = state + 2147483646
	}

	if (mode == "rows") {
		emit_rows(count, width)
	} else if (mode == "tokens") {
		emit_tokens(count, width)
	} else if (mode == "numbers") {
		emit_numbers(count)
	} else {
		print "unknown dprng mode: " mode > "/dev/stderr"
		exit 2
	}
}
