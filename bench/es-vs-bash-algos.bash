#!/usr/bin/env bash
set -euo pipefail

algo="${1:-}"

usage() {
	cat >&2 <<'EOF'
usage: es-vs-bash-algos.bash <algorithm> [args...]

algorithms:
  mandelbrot [width height maxiter]
  base64 [rows rounds seed]
  primes [limit]
  strings [tokens rounds seed]
  dprng [count seed]
EOF
	exit 2
}

run_mandelbrot() {
	local width="${1:-84}"
	local height="${2:-36}"
	local maxiter="${3:-60}"
	awk -v width="$width" -v height="$height" -v maxiter="$maxiter" -f bench/mandelbrot.awk | wc -c >/dev/null
}

run_base64_roundtrip() {
	local rows="${1:-2000}"
	local rounds="${2:-4}"
	local seed="${3:-424242}"
	local tmpdir src enc dec i

	tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/es-bm-b64.XXXXXX")"
	src="$tmpdir/src.txt"
	enc="$tmpdir/encoded.txt"
	dec="$tmpdir/decoded.txt"

	trap 'rm -rf "$tmpdir"' EXIT

	awk -v mode=rows -v count="$rows" -v width=48 -v seed="$seed" -f bench/dprng.awk >"$src"

	for ((i = 0; i < rounds; i++)); do
		openssl base64 -A -in "$src" >"$enc"
		openssl base64 -A -d -in "$enc" >"$dec"
		mv "$dec" "$src"
	done

	wc -c "$src" >/dev/null
	rm -rf "$tmpdir"
	trap - EXIT
}

run_primes() {
	local limit="${1:-60}"
	local n d prime count sum

	count=0
	sum=0
	for ((n = 2; n <= limit; n++)); do
		prime=1
		for ((d = 2; d * d <= n; d++)); do
			if ((n % d == 0)); then
				prime=0
				break
			fi
		done
		if ((prime)); then
			((count += 1))
			((sum += n))
		fi
	done

	if ((count == 0)); then
		echo "prime benchmark invariant failed" >&2
		exit 1
	fi
}

run_strings() {
	local tokens="${1:-1000}"
	local rounds="${2:-20}"
	local seed="${3:-424242}"
	local text i

	text="$(awk -v mode=tokens -v count="$tokens" -v width=7 -v seed="$seed" -f bench/dprng.awk)"
	for ((i = 0; i < rounds; i++)); do
		text="${text// /__}"
		text="${text//__/|}"
		text="${text//|/ }"
	done

	IFS=' ' read -r -a parts <<<"$text"
	[[ "${#parts[@]}" -eq "$tokens" ]] || {
		echo "string benchmark invariant failed" >&2
		exit 1
	}
}

run_dprng() {
	local count="${1:-80}"
	local seed="${2:-424242}"
	local state checksum i

	state=$((seed % 2147483647))
	if ((state <= 0)); then
		state=$((state + 2147483646))
	fi

	checksum=0
	for ((i = 0; i < count; i++)); do
		state=$(((state * 48271) % 2147483647))
		checksum=$((checksum + state))
	done

	if ((checksum <= 0)); then
		echo "dprng benchmark invariant failed" >&2
		exit 1
	fi
}

case "$algo" in
	mandelbrot)
		shift
		run_mandelbrot "$@"
		;;
	base64)
		shift
		run_base64_roundtrip "$@"
		;;
	primes)
		shift
		run_primes "$@"
		;;
	strings)
		shift
		run_strings "$@"
		;;
	dprng)
		shift
		run_dprng "$@"
		;;
	*)
		usage
		;;
esac
