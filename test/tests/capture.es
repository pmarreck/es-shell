# tests/capture.es -- dogfood tests for share/capture.es

. share/capture.es

test 'capture captures stdout stderr and rc' {
	local (out = ; err = ; rc = ) {
		capture sh -c 'echo out; echo err 1>&2; exit 7'
		assert {~ $out out} 'stdout captured'
		assert {~ $err err} 'stderr captured'
		assert {~ $rc 7} 'exit status captured'
	}
}

test 'capture succeeds for successful command' {
	local (out = ; err = ; rc = ) {
		capture sh -c 'echo fine; exit 0'
		assert {~ $out fine}
		assert {~ $err ()}
		assert {~ $rc 0}
	}
}

test 'capture --help prints usage text' {
	assert {~ `` \n {capture --help} 'usage: capture'*}
}

test 'capture without command fails clearly' {
	let ((status output) = <={$&backquote \n {
		capture >[2=1]
	}}) {
		assert {!~ $status 0} 'fails with non-zero status'
		assert {~ $^output *'no command given'*} 'error text mentions missing command'
	}
}
