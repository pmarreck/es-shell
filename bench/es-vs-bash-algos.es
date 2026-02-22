#!/usr/local/bin/es -p

fn usage {
	throw error 'usage: es-vs-bash-algos.es <algorithm> [args...]'
}

fn default value fallback {
	if {~ $value ()} {
		result $fallback
	} {
		result $value
	}
}

fn iadd a b { result `{expr $a + $b} }
fn imul a b { result `{expr $a \* $b} }
fn imod a b { result `{expr $a % $b} }
fn ieq a b { expr $a = $b >[1]/dev/null }
fn ile a b { expr $a '<=' $b >[1]/dev/null }

fn run-mandelbrot width height maxiter {
	awk -v width=$width -v height=$height -v maxiter=$maxiter -f bench/mandelbrot.awk | wc -c >[1]/dev/null
}

fn run-base64 rows rounds seed {
	tmpdir = `{mktemp -d /tmp/es-bm-b64.XXXXXX}
	src = $tmpdir^/src.txt
	enc = $tmpdir^/encoded.txt
	dec = $tmpdir^/decoded.txt

	unwind-protect {
		awk -v mode=rows -v count=$rows -v width=48 -v seed=$seed -f bench/dprng.awk >$src
		for (i = `{seq $rounds}) {
			openssl base64 -A -in $src >$enc
			openssl base64 -A -d -in $enc >$dec
			mv $dec $src
		}
		wc -c $src >[1]/dev/null
	} {
		rm -rf $tmpdir
	}
}

fn run-primes limit {
	n = 2
	count = 0
	sum = 0

	while {ile $n $limit} {
		d = 2
		is-prime = 1

		while {~ $is-prime 1 && ile <={imul $d $d} $n} {
			if {~ <={imod $n $d} 0} {
				is-prime = 0
			} {
				d = <={iadd $d 1}
			}
		}

		if {~ $is-prime 1} {
			count = <={iadd $count 1}
			sum = <={iadd $sum $n}
		}
		n = <={iadd $n 1}
	}

	if {ieq $count 0} {
		throw error prime benchmark invariant failed
	}
}

fn run-strings count rounds seed {
	tokens = `{awk -v mode=tokens -v count=$count -v width=7 -v seed=$seed -f bench/dprng.awk}

	for (i = `{seq $rounds}) {
		text = <={%flatten _ $tokens}
		tokens = <={%fsplit _ $text}
		text = <={%flatten : $tokens}
		tokens = <={%fsplit : $text}
	}

	if {!~ <={%count $tokens} $count} {
		throw error string benchmark invariant failed
	}
}

fn run-dprng count seed {
	state = <={imod $seed 2147483647}
	if {ile $state 0} {
		state = <={iadd $state 2147483646}
	}

	checksum = 0
	for (i = `{seq $count}) {
		state = <={imod <={imul $state 48271} 2147483647}
		checksum = <={iadd $checksum $state}
	}

	if {ile $checksum 0} {
		throw error dprng benchmark invariant failed
	}
}

if {~ $#* 0} {
	usage
}

algorithm = $1
match $algorithm (
	mandelbrot {
		width = <={default $2 84}
		height = <={default $3 36}
		maxiter = <={default $4 60}
		run-mandelbrot $width $height $maxiter
	}
	base64 {
		rows = <={default $2 2000}
		rounds = <={default $3 4}
		seed = <={default $4 424242}
		run-base64 $rows $rounds $seed
	}
	primes {
		limit = <={default $2 60}
		run-primes $limit
	}
	strings {
		count = <={default $2 1000}
		rounds = <={default $3 20}
		seed = <={default $4 424242}
		run-strings $count $rounds $seed
	}
	dprng {
		count = <={default $2 80}
		seed = <={default $3 424242}
		run-dprng $count $seed
	}
	* {
		usage
	}
)
