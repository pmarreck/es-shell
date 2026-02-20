/* match.c -- pattern matching routines ($Revision: 1.1.1.1 $) */

#include "es.h"

enum { RANGE_FAIL = -1, RANGE_ERROR = -2 };

/*
   From the ed(1) man pages (on ranges):

	The `-' is treated as an ordinary character if it occurs first
	(or first after an initial ^) or last in the string.

	The right square bracket does not terminate the enclosed string
	if it is the first character (after an initial `^', if any), in
	the bracketed string.

   rangematch() matches a single character against a class, and returns
   an integer offset to the end of the range on success, or -1 on
   failure.
*/

#define	ISQUOTED(q, n)	((q) == QUOTED || ((q) != UNQUOTED && (q)[n] == 'q'))
#define TAILQUOTE(q, n) ((q) == UNQUOTED ? UNQUOTED : ((q) + (n)))
#define QX(expr) ((q != QUOTED && q != UNQUOTED) ? (expr) : q)

/* rangematch -- match a character against a character class */
static int rangematch(const char *p, const char *q, char c) {
	const char *orig = p;
	Boolean neg;
	Boolean matched = FALSE;
	if (*p == '~' && !ISQUOTED(q, 0)) {
		p++, QX(q++);
	    	neg = TRUE;
	} else
		neg = FALSE;
	if (*p == ']' && !ISQUOTED(q, 0)) {
		p++, QX(q++);
		matched = (c == ']');
	}
	for (; *p != ']' || ISQUOTED(q, 0); p++, QX(q++)) {
		if (*p == '\0')
			return c == '[' ? 0 : RANGE_FAIL;	/* no right bracket */
		if (p[1] == '-' && !ISQUOTED(q, 1) && ((p[2] != ']' && p[2] != '\0') || ISQUOTED(q, 2))) {
			/* check for [..-..] but ignore [..-] */
			if (c >= *p && c <= p[2])
				matched = TRUE;
			p += 2;
			QX(q += 2);
		} else if (*p == c)
			matched = TRUE;
	}
	if (matched ^ neg)
		return p - orig + 1; /* skip the right-bracket */
	else
		return RANGE_FAIL;
}

typedef enum {
	mt_literal,
	mt_any,
	mt_star,
	mt_class,
} MatchTokenType;

typedef struct {
	MatchTokenType type;
	unsigned char literal;
	const char *classchars;
	const char *classquote;
	int classlen;
} MatchToken;

typedef struct {
	MatchToken *tokens;
	size_t count;
	size_t alloc;
	size_t nwild;
} CompiledPattern;

static Boolean israw(const char *q, size_t i) {
	return q == UNQUOTED || (q != QUOTED && q[i] == 'r');
}

static Boolean classisraw(const MatchToken *tok, int i) {
	return tok->classquote == UNQUOTED || tok->classquote[i] == 'r';
}

/* classspan -- return the span of a well-formed character class, or 1 */
static int classspan(const char *p, const char *q) {
	int i = 1;
	if (p[i] == '~' && israw(q, i))
		i++;
	if (p[i] == ']' && israw(q, i))
		i++;
	for (;; i++) {
		if (p[i] == '\0')
			return 1; /* malformed; treat '[' literally */
		if (p[i] == ']' && israw(q, i))
			return i + 1;
		if (
			p[i + 1] == '-'
			&& israw(q, i + 1)
			&& p[i + 2] != '\0'
			&& !(p[i + 2] == ']' && israw(q, i + 2))
		)
			i += 2;
	}
}

static void tokenpush(CompiledPattern *compiled, MatchToken token) {
	if (compiled->count == compiled->alloc) {
		size_t newalloc = compiled->alloc == 0 ? 8 : compiled->alloc * 2;
		if (compiled->tokens == NULL)
			compiled->tokens = ealloc(newalloc * sizeof *compiled->tokens);
		else
			compiled->tokens = erealloc(compiled->tokens, newalloc * sizeof *compiled->tokens);
		compiled->alloc = newalloc;
	}
	compiled->tokens[compiled->count++] = token;
}

static void freecompiled(CompiledPattern *compiled) {
	if (compiled->tokens != NULL)
		efree(compiled->tokens);
	compiled->tokens = NULL;
	compiled->count = 0;
	compiled->alloc = 0;
	compiled->nwild = 0;
}

static void compilepattern(CompiledPattern *compiled, const char *pattern, const char *quote) {
	size_t i = 0;
	memzero(compiled, sizeof *compiled);
	while (pattern[i] != '\0') {
		unsigned char c = (unsigned char) pattern[i];
		MatchToken token;
		if (israw(quote, i)) {
			switch (c) {
			case '?':
				token.type = mt_any;
				token.literal = '\0';
				token.classchars = NULL;
				token.classquote = NULL;
				token.classlen = 0;
				compiled->nwild++;
				tokenpush(compiled, token);
				i++;
				continue;
			case '*':
				token.type = mt_star;
				token.literal = '\0';
				token.classchars = NULL;
				token.classquote = NULL;
				token.classlen = 0;
				compiled->nwild++;
				tokenpush(compiled, token);
				i++;
				continue;
			case '[': {
				int span = classspan(pattern + i, quote == UNQUOTED ? UNQUOTED : quote + i);
				if (span > 1) {
					token.type = mt_class;
					token.literal = '\0';
					token.classchars = pattern + i + 1;
					token.classquote = quote == UNQUOTED ? UNQUOTED : quote + i + 1;
					token.classlen = span - 2;
					compiled->nwild++;
					tokenpush(compiled, token);
					i += span;
					continue;
				}
				break;
			}
			}
		}
		token.type = mt_literal;
		token.literal = c;
		token.classchars = NULL;
		token.classquote = NULL;
		token.classlen = 0;
		tokenpush(compiled, token);
		i++;
	}
}

/* tokenclassmatch -- match one class token against one character */
static Boolean tokenclassmatch(const MatchToken *tok, unsigned char c) {
	int i = 0;
	Boolean neg = FALSE;
	Boolean matched = FALSE;
	const char *p = tok->classchars;
	if (i < tok->classlen && p[i] == '~' && classisraw(tok, i)) {
		neg = TRUE;
		i++;
	}
	if (i < tok->classlen && p[i] == ']' && classisraw(tok, i)) {
		matched = (c == ']');
		i++;
	}
	while (i < tok->classlen) {
		if (
			i + 2 < tok->classlen
			&& p[i + 1] == '-'
			&& classisraw(tok, i + 1)
			&& !(p[i + 2] == ']' && classisraw(tok, i + 2))
		) {
			unsigned char lo = (unsigned char) p[i];
			unsigned char hi = (unsigned char) p[i + 2];
			if (lo <= c && c <= hi)
				matched = TRUE;
			i += 3;
		} else {
			if ((unsigned char) p[i] == c)
				matched = TRUE;
			i++;
		}
	}
	return matched ^ neg;
}

static Boolean tokenmatch(const MatchToken *tok, unsigned char c) {
	switch (tok->type) {
	case mt_literal:
		return tok->literal == c;
	case mt_any:
		return TRUE;
	case mt_class:
		return tokenclassmatch(tok, c);
	case mt_star:
		NOTREACHED;
	}
	NOTREACHED;
}

/* matchcompiled -- run one compiled pattern against one subject */
static Boolean matchcompiled(const char *subject, const CompiledPattern *compiled) {
	size_t si = 0, ti = 0;
	size_t backtrack_si = 0;
	long backtrack_ti = -1;
	while (subject[si] != '\0') {
		if (
			ti < compiled->count
			&& compiled->tokens[ti].type != mt_star
			&& tokenmatch(&compiled->tokens[ti], (unsigned char) subject[si])
		) {
			ti++;
			si++;
			continue;
		}
		if (ti < compiled->count && compiled->tokens[ti].type == mt_star) {
			backtrack_ti = (long) ti;
			backtrack_si = si;
			ti++;
			continue;
		}
		if (backtrack_ti >= 0) {
			ti = (size_t) backtrack_ti + 1;
			si = ++backtrack_si;
			continue;
		}
		return FALSE;
	}
	while (ti < compiled->count && compiled->tokens[ti].type == mt_star)
		ti++;
	return ti == compiled->count;
}

static unsigned char *buildmatchtable(const char *subject, const CompiledPattern *compiled, size_t *widthp) {
	size_t m = compiled->count;
	size_t n = strlen(subject);
	size_t w = n + 1;
	size_t i, j;
	unsigned char *table = ealloc((m + 1) * w * sizeof *table);
	memzero(table, (m + 1) * w * sizeof *table);

#define TAB(ti, sj) table[(ti) * w + (sj)]

	TAB(m, n) = TRUE;
	for (i = m; i-- > 0;) {
		MatchToken *tok = &compiled->tokens[i];
		if (tok->type == mt_star) {
			TAB(i, n) = TAB(i + 1, n);
			for (j = n; j-- > 0;)
				TAB(i, j) = TAB(i + 1, j) || TAB(i, j + 1);
		} else {
			TAB(i, n) = FALSE;
			for (j = n; j-- > 0;)
				TAB(i, j) = tokenmatch(tok, (unsigned char) subject[j]) && TAB(i + 1, j + 1);
		}
	}

#undef TAB

	*widthp = w;
	return table;
}

/* match -- match a single pattern against a single string. */
extern Boolean match(const char *s, const char *p, const char *q) {
	struct { const char *s, *p, *q; } next;
	if (q == QUOTED)
		return streq(s, p);
	next.s = NULL;
	while (*s || *p) {
		if (*p) {
			if (q != UNQUOTED && *q != 'r')
				goto literal_char;
			switch (*p) {
			case '?':
				if (*s) {
					p++; s++; QX(q++);
					continue;
				}
				break;
			case '*':
				next.p = p++;
				next.q = QX(q++);
				next.s = *s ? s+1 : NULL;
				continue;
			case '[': {
				int r;
				if (!*s)
					return FALSE;
				r = 1 + rangematch(p+1, QX(q+1), *s);
				if (r > 0) {
					p += r; s++;
					q = QX(q + r);
					continue;
				}
				break;
			}
			default:
			literal_char:
				if (*s == *p) {
					p++; s++; QX(q++);
					continue;
				}
			}
		}
		if (next.s == NULL)
			return FALSE;
		s = next.s;
		p = next.p;
		q = next.q;
	}
	return TRUE;
}


/*
 * listmatch
 *
 *	Matches a list of words s against a list of patterns p.
 *	Returns true iff a pattern in p matches a word in s.
 *	() matches (), but otherwise null patterns match nothing.
 */

extern Boolean listmatch(List *subject, List *pattern, StrList *quote) {
	if (subject == NULL) {
		if (pattern == NULL)
			return TRUE;
		Ref(List *, p, pattern);
		Ref(StrList *, q, quote);
		for (; p != NULL; p = p->next, q = q->next) {
			/* one or more stars match null */
			char *pw = getstr(p->term), *qw;
			assert(q != NULL);
			qw = q->str;
			if (*pw != '\0' && qw != QUOTED) {
				int i;
				Boolean matched = TRUE;
				for (i = 0; pw[i] != '\0'; i++)
					if (pw[i] != '*'
					    || (qw != UNQUOTED && qw[i] != 'r')) {
						matched = FALSE;
						break;
					}
				if (matched) {
					RefPop2(q, p);
					return TRUE;
				}
			}
		}
		RefEnd2(q, p);
		return FALSE;
	}

	Ref(List *, s, subject);
	Ref(List *, p, pattern);
	Ref(StrList *, q, quote);

	for (; p != NULL; p = p->next, q = q->next) {
		CompiledPattern compiled;
		assert(q != NULL);
		assert(p->term != NULL);
		assert(q->str != NULL);
		Ref(char *, pw, getstr(p->term));
		Ref(char *, qw, q->str);
		Ref(List *, t, s);
		compilepattern(&compiled, pw, qw);
		for (; t != NULL; t = t->next) {
			char *tw = getstr(t->term);
			if (matchcompiled(tw, &compiled)) {
				freecompiled(&compiled);
				RefPop3(t, qw, pw);
				RefPop3(q, p, s);
				return TRUE;
			}
		}
		freecompiled(&compiled);
		RefEnd3(t, qw, pw);
	}
	RefEnd3(q, p, s);
	return FALSE;
}

/*
 * extractsinglematch -- extract matching parts of a single subject and
 * a single compiled pattern, returning them backwards.
 */
static List *extractsinglematch(const char *subject, const CompiledPattern *compiled) {
	size_t i = 0, j = 0, m = compiled->count, n = strlen(subject), w;
	unsigned char *table;
	List *result = NULL;

	if (compiled->nwild == 0 || !matchcompiled(subject, compiled))
		return NULL;

	table = buildmatchtable(subject, compiled, &w);
#define TAB(ti, sj) table[(ti) * w + (sj)]

	assert(TAB(0, 0));
	while (i < m) {
		MatchToken *tok = &compiled->tokens[i];
		switch (tok->type) {
		case mt_star: {
			size_t start = j;
			while (!TAB(i + 1, j)) {
				assert(j < n);
				j++;
			}
			result = mklist(mkstr(gcndup(subject + start, j - start)), result);
			i++;
			break;
		}
		case mt_any:
		case mt_class:
			assert(j < n);
			result = mklist(mkstr(str("%c", subject[j])), result);
			i++;
			j++;
			break;
		case mt_literal:
			assert(j < n);
			assert(tokenmatch(tok, (unsigned char) subject[j]));
			i++;
			j++;
			break;
		default:
			NOTREACHED;
		}
	}

#undef TAB
	efree(table);
	return result;
}

/*
 * extractmatches
 *
 *	Compare subject and patterns like listmatch().  For all subjects
 *	that match a pattern, return the wildcarded portions of the
 *	subjects as the result.
 */

extern List *extractmatches(List *subjects, List *patterns, StrList *quotes) {
	List **prevp;
	List *subject;
	List *pattern;
	StrList *quote;
	struct Pattern {
		CompiledPattern compiled;
	} *compiled = NULL;
	size_t npatterns = 0, i = 0;
	Ref(List *, result, NULL);
	prevp = &result;

	gcdisable();

	for (pattern = patterns; pattern != NULL; pattern = pattern->next)
		npatterns++;
	if (npatterns > 0) {
		compiled = ealloc(npatterns * sizeof *compiled);
		memzero(compiled, npatterns * sizeof *compiled);
		for (pattern = patterns, quote = quotes, i = 0;
		     pattern != NULL;
		     pattern = pattern->next, quote = quote->next, i++) {
			assert(quote != NULL);
			compilepattern(&compiled[i].compiled, getstr(pattern->term), quote->str);
		}
	}

	for (subject = subjects; subject != NULL; subject = subject->next) {
		char *subj = getstr(subject->term);
		for (i = 0; i < npatterns; i++) {
			List *match;
			match = extractsinglematch(subj, &compiled[i].compiled);
			if (match != NULL) {
				/* match is returned backwards, so reverse it */
				match = reverse(match);
				for (*prevp = match; match != NULL; match = *prevp)
					prevp = &match->next;
				break;
			}
		}
	}

	for (i = 0; i < npatterns; i++)
		freecompiled(&compiled[i].compiled);
	if (compiled != NULL)
		efree(compiled);

	gcenable();
	RefReturn(result);
}
