# tests/why_es_doc.es -- validate examples shown in WHY_ES.md

test 'list values and indexing' {
	let (xs = (alpha beta gamma)) {
		assert {~ $#xs 3} 'list count works'
		assert {~ $xs(2) beta} 'indexing is 1-based'
	}
}

test 'for can zip lists' {
	assert {~ `` () {
		for (a = (1 2 3); b = (x y z)) {
			echo $a:$b
		}
	} '1:x'\n'2:y'\n'3:z'\n} 'parallel list iteration'
}

test 'higher-order functions with closures' {
	fn make-adder n { result @ x { result `{expr $x + $n} } }
	let (add7 = <={make-adder 7}) {
		assert {~ <={$add7 35} 42} 'closure captures n'
	}
}

test 'functions as arguments' {
	fn map f list {
		let (out = ()) {
			for (x = $list)
				out = $out <={$f $x}
			result $out
		}
	}

	fn up x { result `{echo $x | tr a-z A-Z} }

	assert {~ <={map $fn-up (a bb ccc)} (A BB CCC)} 'map applies callable'
}

test 'match and extraction' {
	assert {~ <={match foo.c (
		*.c {result c-file};
		*.h {result h-file};
		* {result other}
	)} c-file} 'match expression dispatches by pattern'

	assert {~ <={~~ (src/main.c doc/es.1 README.md) *.[ch]} (src/main c)} \
		'~~ extracts wildcard captures'
}

test 'local is dynamic but let is lexical' {
	x = global
	fn show { result $x }

	assert {~ <={let (x = lexical) { show }} global} 'let does not dynamically rebind show'
	assert {~ <={local (x = dynamic) { show }} dynamic} 'local dynamically rebinds show'
	assert {~ <={show} global} 'outer binding remains unchanged'
}

test 'exceptions and cleanup' {
	assert {~ `` () {
		catch @ e {
			echo caught=$e
		} {
			unwind-protect {
				throw error fail
			} {
				echo cleanup-ran
			}
		}
	} 'cleanup-ran'\n'caught=error caught=fail'\n} 'cleanup always runs'
}

test 'source library-style script from examples' {
	. examples/number.es
	assert {~ <={number 99,000,454} (ninety-nine million four hundred fifty-four)} \
		'number.es can be sourced and called'
}
