#ifndef FLEXER_H
#define FLEXER_H

#ifndef FLEXER_REALLOC
#include <stdlib.h>
#define FLEXER_REALLOC realloc
#endif  // FLEXER_REALLOC

#ifndef FLEXER_FREE
#include <stdlib.h>
#define FLEXER_FREE free
#endif  // FLEXER_FREE

#ifndef FLEXER_ASSERT
#include <assert.h>
#define FLEXER_ASSERT assert
#endif  // FLEXER_ASSERT

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#define FLEXER_UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

#ifndef FLEXER_DA_INIT_CAP
#define FLEXER_DA_INIT_CAP 64
#endif  // FLEXER_DA_INIT_CAP

// Ensure that the dynamic array has at minimum the required size, growing if not
#define flexer_da_reserve(da, required_size)                                              \
do {                                                                                      \
    if ((required_size) > (da)->capacity){                                                \
        if ((da)->capacity == 0){                                                         \
            (da)->capacity = FLEXER_DA_INIT_CAP;                                            \
        }                                                                                 \
        while ((required_size) > (da)->capacity){                                         \
            (da)->capacity *= 2;                                                          \
        }                                                                                 \
        (da)->items = FLEXER_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items)); \
        FLEXER_ASSERT((da)->items != NULL && "Failed to reserve enough space for DA");    \
    }                                                                                     \
} while (0)
// Free the memory allocated by the dynamic array
#define flexer_da_free(da) FLEXER_FREE((da).items)

// Append a new element onto the end of the dynamic array
#define flexer_da_push(da, elem)                \
do {                                            \
    flexer_da_reserve((da), (da)->count + 1);   \
    (da)->items[(da)->count++] = (elem);        \
} while(0)
// Remove and return the last element in the dynamic array 
#define flexer_da_pop(da) (da)->items[(FLEXER_ASSERT((da)->count > 0), --(da)->count)]

// Push a collection of elements onto the end of the dynamic array
#define flexer_da_push_many(da, elems, elems_count)                                     \
do {                                                                                    \
    flexer_da_reserve((da), (da)->count + (elems_count));                               \
    memcpy((da)->items + (da)->count, (elems), (elems_count) * sizeof(*(da)->items));   \
    (da)->count += elems_count;                                                         \
} while(0)

// Retrieve the first element from the dynamic array
#define flexer_da_first(da) (da)->items[(FLEXER_ASSERT((da)->count > 0), 0)]
// Retrieve the last element from the dynamic array
#define flexer_da_last(da) (da)->items[(FLEXER_ASSERT((da)->count > 0), (da)->count - 1)]

// Remove an element from the dynamic array at the specified index by swapping it with the last element and then removing it
#define flexer_da_swap_remove(da, elem_index)      \
do {                                               \
    size_t i = (elem_index);                       \
    FLEXER_ASSERT(i < (da)->count);                \
    (da)->items[i] = (da)->items[--(da)->count];   \
} while(0)

#define flexer_da_foreach(Type, var, da) for (Type *var = (da)->items; var < (da)->items + (da)->count; ++var)

typedef struct{
    const char *data;
    size_t count;
} Flexer_String_View;
#ifndef Flexer_SV_Fmt
#define Flexer_SV_Fmt "%.*s"
#endif  // Flexer_SV_Fmt
#ifndef Flexer_SV_Arg
#define Flexer_SV_Arg(sv) (int)(sv).count, (sv).data
#endif  // Flexer_SV_Arg

Flexer_String_View flexer_sv_from_parts(const char *data, size_t count);
Flexer_String_View flexer_sv_from_cstr(const char *data);
Flexer_String_View flexer_sv_trim_left(Flexer_String_View sv);
Flexer_String_View flexer_sv_trim_right(Flexer_String_View sv);
Flexer_String_View flexer_sv_trim(Flexer_String_View sv);
Flexer_String_View flexer_sv_chop_left(Flexer_String_View *sv, size_t n);
Flexer_String_View flexer_sv_chop_right(Flexer_String_View *sv, size_t n);
Flexer_String_View flexer_sv_chop_by_delim(Flexer_String_View *sv, char delim);
Flexer_String_View flexer_sv_chop_by_delim_right(Flexer_String_View *sv, char delim);
Flexer_String_View flexer_sv_chop_while_left(Flexer_String_View *sv, int (*pred)(int));
bool flexer_sv_eq(Flexer_String_View a, Flexer_String_View b);
bool flexer_sv_starts_with(Flexer_String_View sv, Flexer_String_View prefix);
bool flexer_sv_starts_with_cstr(Flexer_String_View sv, const char *cstr);
bool flexer_sv_ends_with(Flexer_String_View sv, Flexer_String_View suffix);
bool flexer_sv_ends_with_cstr(Flexer_String_View sv, const char *cstr);
bool flexer_sv_chop_prefix(Flexer_String_View *sv, Flexer_String_View prefix);
bool flexer_sv_chop_suffix(Flexer_String_View *sv, Flexer_String_View suffix);

typedef struct{
    union{
        char *items;
        char *chars;
    };
    size_t count;
    size_t capacity;
} Flexer_String_Builder;

#define flexer_sb_append_other(sb, other) flexer_da_push_many(sb, (other).items, (other).count)
#define flexer_sb_append_cstr(sb, cstr) \
do {                                    \
    const char *s = (cstr);             \
    size_t len = strlen(s);             \
    flexer_da_push_many(sb, s, len);    \
} while(0)
#define flexer_sb_append_null(sb) flexer_da_push(sb, 0)
#define flexer_sb_free(sb) FLEXER_FREE((sb).items)

#define flexer_sb_to_sv(sb) flexer_sv_from_parts((sb).items, (sb).count)



typedef enum{
    FLEXER_CHAR_DIGIT,
    FLEXER_CHAR_ALPHA,
    FLEXER_CHAR_DOT,
    FLEXER_CHAR_WHITESPACE,
    FLEXER_CHAR_OTHER,
    FLEXER_CHAR_QUOTES,
    FLEXER_CHAR_BACKSLASH,

    FLEXER_CHAR_CLASS_COUNT
} Flexer_Char_Class;

typedef enum{
    FLEXER_STATE_ERROR,         // Encountered an unlexible sequence
    FLEXER_STATE_START,         // Ready to begin a new token
    FLEXER_STATE_INTEGER,
    FLEXER_STATE_DECIMAL_POINT, // Inbetween for float literals
    FLEXER_STATE_FLOAT,
    FLEXER_STATE_IDENTIFIER,
    FLEXER_STATE_SYNTAX,
    FLEXER_STATE_STRING,
    FLEXER_STATE_ESCAPED,       // Encountered a backslash inside of a string literal
    FLEXER_STATE_STRING_END,    // Reached the end of a string literal
    
    FLEXER_STATE_COUNT
} Flexer_Lexer_State;

typedef struct{
    Flexer_String_View *items;
    size_t count;
    size_t capacity;
} Flexer_Strings;

typedef enum{
    FLEXER_EMPTY_TOKEN,
    FLEXER_IDENTIFIER_TOKEN,
    FLEXER_INTEGER_TOKEN,
    FLEXER_FLOAT_TOKEN,
    FLEXER_OPERATOR_TOKEN,
    FLEXER_KEYWORD_TOKEN,
    FLEXER_SYNTAX_TOKEN,
    FLEXER_STRING_TOKEN,
    FLEXER_EOF_TOKEN,

    FLEXER_TOKEN_COUNT
} Flexer_Token_Kind;

typedef struct{
    Flexer_Token_Kind kind;
    
    size_t line_number;
    size_t char_number;

    union {
        Flexer_String_View as_identifier;
        long long as_integer;
        double as_float;
        size_t as_operator_index;
        size_t as_keyword_index;
        char as_syntax_char;
        Flexer_String_View as_string;
    } data;
} Flexer_Token;
typedef struct{
    size_t line_number;
    size_t char_number;

    Flexer_Lexer_State state;

    size_t chars_consumed;
} Flexer_Token_Error;

typedef struct{
    bool is_ok;

    union{
        Flexer_Token as_token;
        Flexer_Token_Error as_error;
    };
} Flexer_Token_Result;

typedef struct{
    Flexer_Strings comment_openers;
    Flexer_Strings comment_closers;

    Flexer_Strings keywords;

    Flexer_Strings operators;
} Flexer_Lexer_Config;

typedef struct{
    Flexer_Lexer_Config *config;

    Flexer_String_View source;
    Flexer_String_View remaining;

    size_t line_index;
    size_t char_index;
} Flexer_Lexer;

static const Flexer_Lexer_State FLEXER_STATE_TRANSITIONS[FLEXER_STATE_COUNT][FLEXER_CHAR_CLASS_COUNT] = {
    [FLEXER_STATE_START][FLEXER_CHAR_DIGIT] = FLEXER_STATE_INTEGER,
    [FLEXER_STATE_START][FLEXER_CHAR_ALPHA] = FLEXER_STATE_IDENTIFIER,
    [FLEXER_STATE_START][FLEXER_CHAR_DOT] = FLEXER_STATE_SYNTAX,
    [FLEXER_STATE_START][FLEXER_CHAR_OTHER] = FLEXER_STATE_SYNTAX,
    [FLEXER_STATE_START][FLEXER_CHAR_QUOTES] = FLEXER_STATE_STRING,

    [FLEXER_STATE_INTEGER][FLEXER_CHAR_DIGIT] = FLEXER_STATE_INTEGER,
    [FLEXER_STATE_INTEGER][FLEXER_CHAR_DOT] = FLEXER_STATE_DECIMAL_POINT,

    [FLEXER_STATE_DECIMAL_POINT][FLEXER_CHAR_DIGIT] = FLEXER_STATE_FLOAT,

    [FLEXER_STATE_FLOAT][FLEXER_CHAR_DIGIT] = FLEXER_STATE_FLOAT,

    [FLEXER_STATE_IDENTIFIER][FLEXER_CHAR_DIGIT] = FLEXER_STATE_IDENTIFIER,
    [FLEXER_STATE_IDENTIFIER][FLEXER_CHAR_ALPHA] = FLEXER_STATE_IDENTIFIER,

    [FLEXER_STATE_SYNTAX][FLEXER_CHAR_OTHER] = FLEXER_STATE_SYNTAX,

    [FLEXER_STATE_STRING][FLEXER_CHAR_ALPHA] = FLEXER_STATE_STRING,
    [FLEXER_STATE_STRING][FLEXER_CHAR_DIGIT] = FLEXER_STATE_STRING,
    [FLEXER_STATE_STRING][FLEXER_CHAR_DOT] = FLEXER_STATE_STRING,
    [FLEXER_STATE_STRING][FLEXER_CHAR_OTHER] = FLEXER_STATE_STRING,
    [FLEXER_STATE_STRING][FLEXER_CHAR_WHITESPACE] = FLEXER_STATE_STRING,
    [FLEXER_STATE_STRING][FLEXER_CHAR_BACKSLASH] = FLEXER_STATE_ESCAPED,
    [FLEXER_STATE_STRING][FLEXER_CHAR_QUOTES] = FLEXER_STATE_STRING_END,

    [FLEXER_STATE_ESCAPED][FLEXER_CHAR_ALPHA] = FLEXER_STATE_STRING,
    [FLEXER_STATE_ESCAPED][FLEXER_CHAR_DIGIT] = FLEXER_STATE_STRING,
    [FLEXER_STATE_ESCAPED][FLEXER_CHAR_DOT] = FLEXER_STATE_STRING,
    [FLEXER_STATE_ESCAPED][FLEXER_CHAR_OTHER] = FLEXER_STATE_STRING,
    [FLEXER_STATE_ESCAPED][FLEXER_CHAR_WHITESPACE] = FLEXER_STATE_STRING,
    [FLEXER_STATE_ESCAPED][FLEXER_CHAR_BACKSLASH] = FLEXER_STATE_STRING,
    [FLEXER_STATE_ESCAPED][FLEXER_CHAR_QUOTES] = FLEXER_STATE_STRING,
};

static const Flexer_Token_Kind FLEXER_STATE_ACCEPTS[FLEXER_STATE_COUNT] = {
    [FLEXER_STATE_IDENTIFIER] = FLEXER_IDENTIFIER_TOKEN,
    [FLEXER_STATE_INTEGER] = FLEXER_INTEGER_TOKEN,
    [FLEXER_STATE_FLOAT] = FLEXER_FLOAT_TOKEN,
    [FLEXER_STATE_SYNTAX] = FLEXER_OPERATOR_TOKEN,
    [FLEXER_STATE_STRING_END] = FLEXER_STRING_TOKEN,
};

Flexer_Char_Class flexer_classify_char(unsigned char c);

void flexer_free_lexer_config(Flexer_Lexer_Config *config);

Flexer_Lexer flexer_create_lexer(Flexer_String_View source, Flexer_Lexer_Config *config);

void flexer_register_operator(Flexer_Lexer_Config *config, const char *operator);
void flexer_register_comment_signature(Flexer_Lexer_Config *config, const char *open, const char *close);
void flexer_register_keyword(Flexer_Lexer_Config *config, const char *keyword);

Flexer_Token_Result flexer_next_token(Flexer_Lexer *lexer);

#ifdef FLEXER_IMPL
Flexer_String_View flexer_sv_from_parts(const char *data, size_t count)
{
    Flexer_String_View sv;
    sv.data = data;
    sv.count = count;

    return sv;
}
Flexer_String_View flexer_sv_from_cstr(const char *data)
{
    return flexer_sv_from_parts(data, strlen(data));
}

Flexer_String_View flexer_sv_trim_left(Flexer_String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace((unsigned char)sv.data[i])) 
        i += 1;

    return flexer_sv_from_parts(sv.data + i, sv.count - i);
}
Flexer_String_View flexer_sv_trim_right(Flexer_String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace((unsigned char)sv.data[sv.count - i - 1]))
        i += 1;

    return flexer_sv_from_parts(sv.data, sv.count - i);
}

Flexer_String_View flexer_sv_trim(Flexer_String_View sv)
{
    return flexer_sv_trim_left(flexer_sv_trim_right(sv));
}

Flexer_String_View flexer_sv_chop_left(Flexer_String_View *sv, size_t n)
{
    if (n > sv->count)
        n = sv->count;

    Flexer_String_View result = flexer_sv_from_parts(sv->data, n);

    sv->data += n;
    sv->count -= n;

    return result;
}
Flexer_String_View flexer_sv_chop_right(Flexer_String_View *sv, size_t n)
{
    if (n > sv->count)
        n = sv->count;

    Flexer_String_View result = flexer_sv_from_parts(sv->data + sv->count - n, n);

    sv->count -= n;

    return result;
}

Flexer_String_View flexer_sv_chop_by_delim(Flexer_String_View *sv, char delim)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim)
        i += 1;

    Flexer_String_View result = flexer_sv_from_parts(sv->data, i);
    
    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data += i + 1;
    } else {
        sv->count -= i;
        sv->data += i;
    }

    return result;
}
Flexer_String_View flexer_sv_chop_by_delim_right(Flexer_String_View *sv, char delim)
{
    size_t i = sv->count;
    while (i > 0 && sv->data[i - 1] != delim)
        i -= 1;

    
    if (i == 0){
        Flexer_String_View result = *sv;
        sv->count = 0;
        return result;
    }

    Flexer_String_View result = flexer_sv_from_parts(sv->data + i, sv->count - i);
    sv->count = i - 1;

    return result;
}
Flexer_String_View flexer_sv_chop_while_left(Flexer_String_View *sv, int (*pred)(int))
{
    size_t i = 0;
    while (i < sv->count && pred(sv->data[i]))
        i++;
    
    return flexer_sv_chop_left(sv, i);
}

bool flexer_sv_eq(Flexer_String_View a, Flexer_String_View b)
{
    if (a.count != b.count)
        return false;

    return memcmp(a.data, b.data, a.count) == 0;
}

bool flexer_sv_starts_with(Flexer_String_View sv, Flexer_String_View prefix)
{
    if (prefix.count > sv.count)
        return false;

    Flexer_String_View head = {
        .data = sv.data,
        .count = prefix.count,
    };

    return flexer_sv_eq(prefix, head);
}
bool flexer_sv_starts_with_cstr(Flexer_String_View sv, const char *cstr)
{
    return flexer_sv_starts_with(sv, flexer_sv_from_cstr(cstr));
}

bool flexer_sv_ends_with(Flexer_String_View sv, Flexer_String_View suffix)
{
    if (suffix.count > sv.count)
        return false;

    Flexer_String_View tail = {
        .data = sv.data + sv.count - suffix.count,
        .count = suffix.count,
    };

    return flexer_sv_eq(suffix, tail);
}
bool flexer_sv_ends_with_cstr(Flexer_String_View sv, const char *data)
{
    return flexer_sv_ends_with(sv, flexer_sv_from_cstr(data));
}

bool flexer_sv_chop_prefix(Flexer_String_View *sv, Flexer_String_View prefix)
{
    if (flexer_sv_starts_with(*sv, prefix)) {
        flexer_sv_chop_left(sv, prefix.count);
        return true;
    }

    return false;
}
bool flexer_sv_chop_suffix(Flexer_String_View *sv, Flexer_String_View suffix)
{
    if (flexer_sv_ends_with(*sv, suffix)) {
        flexer_sv_chop_right(sv, suffix.count);
        return true;
    }

    return false;
}

bool flexer_parse_ll(const char *str, size_t length, long long *out){
    if (length == 0){
        return false;
    }

    long long value = 0;

    for (size_t i = 0; i < length; i++){
        char c = str[i];

        if (c < '0' || c > '9'){
            return false;
        }
        
        int char_value = c - '0';

        if (value > (LLONG_MAX - char_value) / 10){
            return false;
        }

        // Previous conditions guard to ensure that we can safely add the new digit
        value = (value * 10) + char_value;
    }

    *out = value;
    return true;
}
bool flexer_parse_d(const char *str, size_t length, double *out){
    if (length == 0 || length == SIZE_MAX){
        return false;
    }

    // The standard library's string to number conversion functions require null terminated strings
    // Since we only have a view into a larger string, we need to create a temporary copy of the string elsewhere in memory
    // That way we can append our own null terminator and then 


    char *buffer = FLEXER_REALLOC(NULL, length + 1);
    if (buffer == NULL){
        // Failed to allocate buffer
        return false;
    }

    // Copy str data into the buffer
    memcpy(buffer, str, length);
    // Add null terminator
    buffer[length] = '\0';

    // Reset the global error number
    errno = 0;

    // Create the address used to signal where the `strtod` function stopped
    char *end;
    double value = strtod(buffer, &end);

    // Only succeeded if the entire string was used in the conversion and no meaningful error occurred
    bool successful = end == buffer + length && errno != ERANGE;

    // Conversion has been complete, free the allocated buffer
    FLEXER_FREE(buffer);

    if (!successful){
        return false;
    }
    *out = value;
    return true;
}

Flexer_Char_Class flexer_classify_char(unsigned char c){
    if (c >= '0' && c <= '9')
        return FLEXER_CHAR_DIGIT;

    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
        return FLEXER_CHAR_ALPHA;

    if (isspace(c))
        return FLEXER_CHAR_WHITESPACE;

    switch (c){
        case '.': return FLEXER_CHAR_DOT;
        case '"': return FLEXER_CHAR_QUOTES;
        case '\\': return FLEXER_CHAR_BACKSLASH;
        default: break;
    }

    return FLEXER_CHAR_OTHER;
}

void flexer_free_lexer_config(Flexer_Lexer_Config *config){
    flexer_da_free(config->comment_openers);
    flexer_da_free(config->comment_closers);
    flexer_da_free(config->keywords);
    flexer_da_free(config->operators);

    *config = (Flexer_Lexer_Config) {0};
}

Flexer_Lexer flexer_create_lexer(Flexer_String_View source, Flexer_Lexer_Config *config){
    Flexer_Lexer lexer = {
        .config = config,

        .source = source,
        .remaining = source,

        .line_index = 0,
        .char_index = 0,
    };

    return lexer;
}

void flexer_register_operator(Flexer_Lexer_Config *config, const char *operator){
    flexer_da_push(&config->operators, flexer_sv_from_cstr(operator));
}
void flexer_register_comment_signature(Flexer_Lexer_Config *config, const char *open, const char *close){
    flexer_da_push(&config->comment_openers, flexer_sv_from_cstr(open));
    flexer_da_push(&config->comment_closers, flexer_sv_from_cstr(close));
}
void flexer_register_keyword(Flexer_Lexer_Config *config, const char *keyword){
    flexer_da_push(&config->keywords, flexer_sv_from_cstr(keyword));
}

Flexer_Token_Result flexer_try_create_int_token(Flexer_Lexer *lexer, Flexer_Lexer_State state, size_t chars_consumed){
    Flexer_String_View token_string = flexer_sv_from_parts(lexer->remaining.data, chars_consumed);

    long long value;
    if (!flexer_parse_ll(token_string.data, token_string.count, &value)){
        // Parsing the value failed
        return (Flexer_Token_Result) {
            .is_ok = false,

            .as_error = {
                .char_number = lexer->char_index,
                .line_number = lexer->line_index,

                .chars_consumed = chars_consumed,
                .state = state,
            }
        };
    }

    // Previous condition gaurds against a failed parse
    return (Flexer_Token_Result) {
        .is_ok = true,

        .as_token = {
            .kind = FLEXER_INTEGER_TOKEN,
            
            .char_number = lexer->char_index,
            .line_number = lexer->line_index,

            .data.as_integer = value,
        }
    };
}
Flexer_Token_Result flexer_try_create_float_token(Flexer_Lexer *lexer, Flexer_Lexer_State state, size_t chars_consumed){
    Flexer_String_View token_string = flexer_sv_from_parts(lexer->remaining.data, chars_consumed);

    double value;
    if (!flexer_parse_d(token_string.data, token_string.count, &value)){
        // Parsing the value failed
        return (Flexer_Token_Result) {
            .is_ok = false,

            .as_error = {
                .char_number = lexer->char_index,
                .line_number = lexer->line_index,

                .chars_consumed = chars_consumed,
                .state = state,
            }
        };
    }

    // Previous condition gaurds against a failed parse
    return (Flexer_Token_Result) {
        .is_ok = true,

        .as_token = {
            .kind = FLEXER_FLOAT_TOKEN,
            
            .char_number = lexer->char_index,
            .line_number = lexer->line_index,

            .data.as_float = value,
        }
    };
}
Flexer_Token_Result flexer_create_string_token(Flexer_Lexer *lexer, size_t chars_consumed){
    Flexer_String_View token_string = flexer_sv_from_parts(lexer->remaining.data, chars_consumed);

    // Skip the open quotes
    token_string.data++;
    // Shrink count by 2 to remove the closing quotes and account for the skipped opener
    token_string.count -= 2;

    return (Flexer_Token_Result) {
        .is_ok = true,

        .as_token = {
            .kind = FLEXER_STRING_TOKEN,
            
            .char_number = lexer->char_index,
            .line_number = lexer->line_index,

            .data.as_string = token_string
        }
    };
}
Flexer_Token_Result flexer_create_ident_like_token(Flexer_Lexer *lexer, size_t chars_consumed){
    Flexer_String_View token_string = flexer_sv_from_parts(lexer->remaining.data, chars_consumed);

    flexer_da_foreach(Flexer_String_View, keyword, &lexer->config->keywords){
        if (flexer_sv_eq(token_string, *keyword)){
            // The identifier like token has matched a keyword and should be treated as such
            return (Flexer_Token_Result) {
                .is_ok = true,

                .as_token = {
                    .kind = FLEXER_KEYWORD_TOKEN,

                    .char_number = lexer->char_index,
                    .line_number = lexer->line_index,

                    .data.as_keyword_index = (keyword - lexer->config->keywords.items),
                }
            };
        }
    }

    // The loop's nested condition acts like a guard clause, meaning we only reach here if the token was not a keyword
    return (Flexer_Token_Result) {
        .is_ok = true,

        .as_token = {
            .kind = FLEXER_IDENTIFIER_TOKEN,

            .char_number = lexer->char_index,
            .line_number = lexer->line_index,

            .data.as_identifier = token_string,
        }
    };
}
Flexer_Token_Result flexer_create_syntax_like_token(Flexer_Lexer *lexer, size_t chars_consumed){
    Flexer_String_View token_string = flexer_sv_from_parts(lexer->remaining.data, chars_consumed);

    Flexer_String_View *longest_matching_prefix = NULL;
    flexer_da_foreach(Flexer_String_View, operator, &lexer->config->operators){
        // Relies on short circuit evaluation to precent a null dereference for the longest match's count
        bool longer_prefix = flexer_sv_starts_with(token_string, *operator) && (
            longest_matching_prefix == NULL
                ||
            operator->count > longest_matching_prefix->count
        );

        if (longer_prefix){
            longest_matching_prefix = operator;
        }
    }

    if (longest_matching_prefix != NULL){
        // Non-null means a match was found
        return (Flexer_Token_Result) {
            .is_ok = true,

            .as_token = {
                .kind = FLEXER_OPERATOR_TOKEN,

                .char_number = lexer->char_index,
                .line_number = lexer->line_index,

                .data.as_operator_index = longest_matching_prefix - lexer->config->operators.items,
            }
        };
    }

    // Previous conditional acts like a guard clause, meaning we only reach here if the token was not an operator
    // If there was no matching operator we can only interpret the token as a sequence of distinct syntax tokens
    char syntax_char = *lexer->remaining.data;
    return (Flexer_Token_Result) {
        .is_ok = true,
        
        .as_token = {
            .kind = FLEXER_SYNTAX_TOKEN,

            .char_number = lexer->char_index,
            .line_number = lexer->line_index,

            .data.as_syntax_char = syntax_char,
        }
    };
}
Flexer_Token_Result flexer_create_eof_token(Flexer_Lexer *lexer){
    return (Flexer_Token_Result) {
        .is_ok = true,
        
        .as_token = {
            .kind = FLEXER_EOF_TOKEN,
            
            .char_number = lexer->char_index,
            .line_number = lexer->line_index,
        }
    };
}

void flexer_advance_lexer(Flexer_Lexer *lexer, size_t num_chars){
    FLEXER_ASSERT(num_chars <= lexer->remaining.count);

    for (size_t i = 0; i < num_chars; i++){
        char c = lexer->remaining.data[i];

        if (c == '\n'){
            lexer->line_index++;
            lexer->char_index = 0;
        } else { lexer->char_index++; }
    }

    (void)flexer_sv_chop_left(&lexer->remaining, num_chars);
}

Flexer_Token_Result flexer_next_token(Flexer_Lexer *lexer){
    size_t skipped_chars = 0;
    do{
        skipped_chars = 0; 
        while (skipped_chars < lexer->remaining.count && isspace(lexer->remaining.data[skipped_chars]))
            skipped_chars++;

        for (size_t i = 0; i < lexer->config->comment_openers.count; ++i){
            Flexer_String_View working_remainder = lexer->remaining;
            flexer_sv_chop_left(&working_remainder, skipped_chars);

            // Get the comment opener for this iteration
            Flexer_String_View comment_open = lexer->config->comment_openers.items[i];

            if (flexer_sv_chop_prefix(&working_remainder, comment_open)){
                // A comment was found
                skipped_chars += comment_open.count;

                // Get the matching closer for this comment signature
                Flexer_String_View comment_close = lexer->config->comment_closers.items[i];
                while(working_remainder.count >= comment_close.count){
                    if (flexer_sv_chop_prefix(&working_remainder, comment_close)){
                        // The closer has been found
                        skipped_chars += comment_close.count;
                        break;
                    }
                    
                    // Conditional's break statement guards, meaning we only reach here if the closer has not been found yet
                    skipped_chars++;
                    flexer_sv_chop_left(&working_remainder, 1);
                }

                // Because comments typically have whitespace in between them, we can skip checking the remaining comment
                // signatures on this iteration for marginally better average performance. This optimization provides very
                // little for most use cases of a lexer, as there are typically only a small number of comment signatures.
                break;
            }
        }

        // Using the unified advancement function does technically mean we are scanning the skipped characters range
        // twice. Once to determine how far to skip, and a second to update the internals indexes of the lexer.
        flexer_advance_lexer(lexer, skipped_chars);
    } while(skipped_chars > 0);

    if (lexer->remaining.count == 0){
        // Skipping comments and whitespace may leave us with no remaining characters, if so signal that we have reached the end
        return flexer_create_eof_token(lexer);
    }

    // Previous loop guarantees that we have skipped all comments and whitespace and now have tokenizable text
    
    size_t cursor = 0;
    Flexer_Lexer_State current_state = FLEXER_STATE_START;
    
    size_t num_accepted_chars = 0;
    Flexer_Lexer_State last_accepting_state = FLEXER_STATE_ERROR;

    while (cursor < lexer->remaining.count){
        unsigned char c = lexer->remaining.data[cursor];
        Flexer_Char_Class c_class = flexer_classify_char(c);

        Flexer_Lexer_State next_state = FLEXER_STATE_TRANSITIONS[current_state][c_class];

        if (next_state == FLEXER_STATE_ERROR){
            // We have reached the end of our current tokenizable sequence
            // This is the main way we exit our loop
            break;
        }

        cursor++;
        current_state = next_state;

        Flexer_Token_Kind next_kind = FLEXER_STATE_ACCEPTS[current_state];
        if (next_kind != FLEXER_EMPTY_TOKEN){
            // The current state is accepting, commit the consumed characters
            num_accepted_chars = cursor;
            last_accepting_state = current_state;
        }
    }

    // We have finished consuming characters
    Flexer_Token_Result output;
    switch (last_accepting_state)
    {
        case FLEXER_STATE_INTEGER:
            output = flexer_try_create_int_token(lexer, current_state, num_accepted_chars);
            break;
        case FLEXER_STATE_FLOAT:
            output = flexer_try_create_float_token(lexer, current_state, num_accepted_chars);
            break;
        case FLEXER_STATE_IDENTIFIER:
            output = flexer_create_ident_like_token(lexer, num_accepted_chars);
            break;
        case FLEXER_STATE_SYNTAX:
            output = flexer_create_syntax_like_token(lexer, num_accepted_chars);
            if (output.as_token.kind == FLEXER_SYNTAX_TOKEN){ 
                num_accepted_chars = 1; 
            }
            else { num_accepted_chars = lexer->config->operators.items[output.as_token.data.as_operator_index].count; }
            break;
        case FLEXER_STATE_STRING_END:
            output = flexer_create_string_token(lexer, num_accepted_chars);
            break;
        default:
            output = (Flexer_Token_Result) {
                .is_ok = false,

                .as_error = {
                    .char_number = lexer->char_index,
                    .line_number = lexer->line_index,

                    .chars_consumed = num_accepted_chars,
                    .state = current_state,
                }
            };
            break;
    }
    flexer_advance_lexer(lexer, num_accepted_chars);

    return output;
}
#endif  // FLEXER_IMPL

#endif  // FLEXER_H
