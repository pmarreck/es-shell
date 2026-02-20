#!/usr/local/bin/es -p

# match-perf.es -- benchmark listmatch (~) and extractmatches (~~)
#
# Usage:
#   es bench/match-perf.es listmatch [iters]
#   es bench/match-perf.es extractmatches [iters]

fn usage {
	throw error 'usage: match-perf.es (listmatch|extractmatches) [iters]'
}

fn iterations count {
	result `{seq $count}
}

fn subjects {
	result `{awk 'BEGIN { for (i = 1; i <= 700; i++) printf "s%d\n", i }'}
}

fn run-listmatch iters {
	let (
		subs = <=subjects
	) {
		for (i = <={iterations $iters}) {
			if {!~ $subs (
				x1* x2* x3* x4* x5* x6* x7* x8* x9* x10*
				x11* x12* x13* x14* x15* x16* x17* x18* x19* x20*
				x21* x22* x23* x24* x25* x26* x27* x28* x29* x30*
				x31* x32* x33* x34* x35* x36* x37* x38* x39* x40*
				x41* x42* x43* x44* x45* x46* x47* x48* x49* x50*
				x51* x52* x53* x54* x55* x56* x57* x58* x59* x60*
				x61* x62* x63* x64* x65* x66* x67* x68* x69* x70*
				x71* x72* x73* x74* x75* x76* x77* x78* x79* x80*
				x81* x82* x83* x84* x85* x86* x87* x88* x89* x90*
				x91* x92* x93* x94* x95* x96* x97* x98* x99* x100*
				x101* x102* x103* x104* x105* x106* x107* x108* x109* x110*
				x111* x112* x113* x114* x115* x116* x117* x118* x119* x120*
				s70?
			)} {
				throw error listmatch benchmark invariant failed
			}
		}
	}
}

fn run-extractmatches iters {
	let (
		subs = <=subjects
		out = ()
	) {
		for (i = <={iterations $iters}) {
			out = <={~~ $subs (
				x1* x2* x3* x4* x5* x6* x7* x8* x9* x10*
				x11* x12* x13* x14* x15* x16* x17* x18* x19* x20*
				x21* x22* x23* x24* x25* x26* x27* x28* x29* x30*
				x31* x32* x33* x34* x35* x36* x37* x38* x39* x40*
				x41* x42* x43* x44* x45* x46* x47* x48* x49* x50*
				x51* x52* x53* x54* x55* x56* x57* x58* x59* x60*
				x61* x62* x63* x64* x65* x66* x67* x68* x69* x70*
				x71* x72* x73* x74* x75* x76* x77* x78* x79* x80*
				x81* x82* x83* x84* x85* x86* x87* x88* x89* x90*
				x91* x92* x93* x94* x95* x96* x97* x98* x99* x100*
				x101* x102* x103* x104* x105* x106* x107* x108* x109* x110*
				x111* x112* x113* x114* x115* x116* x117* x118* x119* x120*
				s70?
			)}
			if {~ $#out 0} {
				throw error extractmatches benchmark invariant failed
			}
		}
	}
}

if {~ $#* 0} {
	usage
}

case = $1
iters = 120
if {!~ $#* 1} {
	iters = $2
}

if {~ $case listmatch} {
	run-listmatch $iters
} {~ $case extractmatches} {
	run-extractmatches $iters
} {
	usage
}
