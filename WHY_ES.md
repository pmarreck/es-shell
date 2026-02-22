# Why `es`?

`es` is what you get when a Unix shell grows up around functional ideas instead of around compatibility baggage.

If you like Bash/Zsh/Fish but want cleaner composition, first-class lists, real closures, and programmable semantics, `es` is a very different kind of shell.

## The Short Pitch

- Data model: lists first, strings second.
- Functions and lambdas are real values.
- Pattern matching (`match`, `~`, `~~`) is built in.
- Scope controls are explicit (`let` and `local`).
- Core behavior is implemented in shell code and can be redefined.

It still runs Unix programs naturally, but the language feels closer to a tiny Lisp-ish REPL than to “just another POSIX shell”.

## Why It Feels Different

1. Values are lists, not accidental whitespace-split strings.
2. You can pass callables around and build higher-order helpers directly in shell.
3. Matching and extraction are language features, not regex gymnastics.
4. Exceptions and cleanup are explicit (`catch`, `throw`, `unwind-protect`).
5. A lot of “shell semantics” are visible in `initial.es`, not hidden in C internals.

## Runnable Examples

All examples below are validated by `test/tests/why_es_doc.es`.

### 1) Lists are first-class

```es
xs = (alpha beta gamma)
echo n=$#xs second=$xs(2)
```

Expected:

```text
n=3 second=beta
```

### 2) `for` can zip multiple lists

```es
for (a = (1 2 3); b = (x y z)) {
	echo $a:$b
}
```

Expected:

```text
1:x
2:y
3:z
```

### 3) Closures in shell code

```es
fn make-adder n { result @ x { result `{expr $x + $n} } }
add7 = <={make-adder 7}
echo <={$add7 35}
```

Expected:

```text
42
```

### 4) Functions as arguments

```es
fn map f list {
	let (out = ()) {
		for (x = $list)
			out = $out <={$f $x}
		result $out
	}
}

fn up x { result `{echo $x | tr a-z A-Z} }
echo <={map $fn-up (a bb ccc)}
```

Expected:

```text
A BB CCC
```

### 5) Pattern matching and extraction

```es
echo <={match foo.c (
	*.c {result c-file};
	*.h {result h-file};
	* {result other}
)}

echo <={~~ (src/main.c doc/es.1 README.md) *.[ch]}
```

Expected:

```text
c-file
src/main c
```

### 6) `let` vs `local` in practice

```es
x = global
fn show { result $x }

echo let=<={let (x = lexical) { show }}
echo local=<={local (x = dynamic) { show }}
echo outer=<={show}
```

Expected:

```text
let=global
local=dynamic
outer=global
```

### 7) Exceptions and guaranteed cleanup

```es
catch @ e {
	echo caught=$e
} {
	unwind-protect {
		throw error fail
	} {
		echo cleanup-ran
	}
}
```

Expected:

```text
cleanup-ran
caught=error caught=fail
```

### 8) Scripts can be loaded like libraries

```es
. examples/number.es
echo <={number 99,000,454}
```

Expected:

```text
ninety-nine million four hundred fifty-four
```

## Practical Notes

- `es` excels at shell orchestration, pattern-heavy flows, and composable scripting.
- For tight numeric inner loops, delegate to specialized tools (`awk`, C, etc.) and use `es` as the composition layer.
- If you want to understand how deep customization goes, read `initial.es`.

## TL;DR

If other shells feel like imperative glue code with quoting hazards, `es` offers a cleaner model:
lists, closures, explicit control over semantics, and a language that is surprisingly hackable without being huge.
