function dprng_next(    out) {
	state = (state * 48271) % 2147483647
	out = state
	return out
}

function rand_char(    idx) {
	idx = (dprng_next() % length(alphabet)) + 1
	return substr(alphabet, idx, 1)
}

BEGIN {
	tokens = (tokens == "" ? 1000 : tokens)
	rounds = (rounds == "" ? 20 : rounds)
	width = (width == "" ? 7 : width)
	seed = (seed == "" ? 424242 : seed)
	alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

	state = seed % 2147483647
	if (state <= 0) {
		state = state + 2147483646
	}

	text = ""
	for (i = 1; i <= tokens; i++) {
		token = "w"
		for (j = 1; j <= width; j++) {
			token = token rand_char()
		}
		text = text token (i < tokens ? " " : "")
	}

	for (r = 1; r <= rounds; r++) {
		gsub(/ /, "__", text)
		gsub(/__/, "|", text)
		gsub(/\|/, " ", text)
	}

	n = split(text, parts, / /)
	if (n != tokens) {
		print "string benchmark invariant failed" > "/dev/stderr"
		exit 1
	}
}
