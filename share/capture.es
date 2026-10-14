# capture.es -- capture stdout/stderr/exit status into dynamic vars.
#
# Usage:
#   local (out = ; err = ; rc = ) {
#       capture command args ...
#   }
#
# Dynamic (local-bound) variables are used intentionally so callers can choose
# scope while still getting assignment behavior similar to capture.bash.

fn capture {
	let (
		use-printable-binary = false
		args = $*
		lines = ()
		out-b64 = ()
		err-b64 = ()
		rc-value = ()
		helper-script = '
set -uo pipefail

use_printable_binary=0
if [[ "${1-}" == "--printable-binary" ]]; then
	use_printable_binary=1
	shift
fi

if [[ "${1-}" != "--" ]]; then
	printf "capture: helper usage error\n" >&2
	exit 2
fi
shift

if [[ $# -eq 0 ]]; then
	printf "capture: no command given\n" >&2
	exit 2
fi

filter=cat
old_pb_mute_stats="${PRINTABLE_BINARY_MUTE_STATS-}"
if (( use_printable_binary )); then
	if ! command -v printable_binary >/dev/null 2>&1; then
		printf "capture: command not found: printable_binary\n" >&2
		exit 127
	fi
	filter=printable_binary
	export PRINTABLE_BINARY_MUTE_STATS=true
fi

declare __out __err __rc
. <(
	{
		__err=$(
			{
				__out=$(
					set -o pipefail
					"$@" 2> >($filter >&3) | $filter
				)
				__rc=$?
				declare -p __out __rc >&4
			} 3>&1
		)
		declare -p __err >&4
	} 4>&1
)

if (( use_printable_binary )); then
	export PRINTABLE_BINARY_MUTE_STATS="$old_pb_mute_stats"
fi

printf "RC=%s\n" "$__rc"
printf "OUT=%s\n" "$(printf %s "$__out" | openssl base64 -A)"
printf "ERR=%s\n" "$(printf %s "$__err" | openssl base64 -A)"
'
	) {
		while {!~ $#args 0} {
			if {~ $args(1) -p --printable-binary} {
				use-printable-binary = true
				args = $args(2 ...)
			} {~ $args(1) -a --about} {
				echo 'capture: capture stdout, stderr, and exit status into dynamic vars: out, err, rc.'
				return 0
			} {~ $args(1) -h --help} {
				echo 'usage: capture [-p|--printable-binary] [--] command [args...]'
				echo
				echo 'Runs command and captures:'
				echo '    - stdout -> $out'
				echo '    - stderr -> $err'
				echo '    - exit status -> $rc'
				echo
				echo 'Notes:'
				echo '    - Bind out/err/rc with local (dynamic scope), e.g.:'
				echo '        local (out = ; err = ; rc = ) {'
				echo '            capture some-command ...'
				echo '        }'
				echo '    - This implementation keeps capture in-memory and does not use temp files.'
				return 0
			} {~ $args(1) --} {
				args = $args(2 ...)
				break
			} {
				break
			}
		}

		if {~ $#args 0} {
			echo >[1=2] 'capture: no command given'
			return 2
		}

		helper-args = -- $args
		if {$use-printable-binary} {
			helper-args = --printable-binary $helper-args
		}

		lines = `` \n {bash -lc $helper-script capture-helper $helper-args}
		if {!~ $bqstatus 0} {
			return $bqstatus
		}

		for (line = $lines) {
			if {~ $line OUT=*} {
				out-b64 = <={~~ $line OUT=*}
			} {~ $line ERR=*} {
				err-b64 = <={~~ $line ERR=*}
			} {~ $line RC=*} {
				rc-value = <={~~ $line RC=*}
			}
		}

		if {~ $out-b64 ()} {
			out =
		} {
			out = `` '' {echo $out-b64 | openssl base64 -A -d}
		}

		if {~ $err-b64 ()} {
			err =
		} {
			err = `` '' {echo $err-b64 | openssl base64 -A -d}
		}

		if {~ $rc-value ()} {
			rc = 0
		} {
			rc = $rc-value
		}
		return 0
	}
}
