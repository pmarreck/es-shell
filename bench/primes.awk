BEGIN {
	if (limit == "" || limit < 2) {
		limit = 20000
	}

	for (i = 2; i <= limit; i++) {
		prime[i] = 1
	}

	for (i = 2; i * i <= limit; i++) {
		if (prime[i]) {
			for (j = i * i; j <= limit; j += i) {
				prime[j] = 0
			}
		}
	}

	count = 0
	sum = 0
	for (i = 2; i <= limit; i++) {
		if (prime[i]) {
			count++
			sum += i
		}
	}

	print count, sum
}
