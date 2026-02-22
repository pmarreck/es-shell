BEGIN {
	if (width == "" || width <= 0) {
		width = 84
	}
	if (height == "" || height <= 0) {
		height = 36
	}
	if (maxiter == "" || maxiter <= 0) {
		maxiter = 60
	}

	chars = " .:-=+*#%@"
	nchars = length(chars)

	for (y = 0; y < height; y++) {
		imag = (y / (height - 1)) * 2.0 - 1.0
		line = ""
		for (x = 0; x < width; x++) {
			real = (x / (width - 1)) * 3.5 - 2.5
			zr = 0.0
			zi = 0.0
			iter = 0

			while ((zr * zr + zi * zi) <= 4.0 && iter < maxiter) {
				tmp = zr * zr - zi * zi + real
				zi = 2.0 * zr * zi + imag
				zr = tmp
				iter++
			}

			idx = int((iter / maxiter) * (nchars - 1)) + 1
			if (idx > nchars) {
				idx = nchars
			}
			line = line substr(chars, idx, 1)
		}
		print line
	}
}
