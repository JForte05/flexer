# Flexer

Introducing **Flexer**. A C-based, configurable, language-independent lexer. Driven by a table-based finite-state machine, it is capable of converting source code into a stream of identifiable tokens. Flexer allows users to configure custom keywords, operators, and comment delimiters without having to modify the lexer itself.

## Features
- **Single-header library:** Easy to integrate into existing C projects.
- **Configurable syntax:** Register custom keywords, operators, and comment delimiters.
- **Token recognition:** Supports identifiers, integers, floating-point numbers, string literals, operators, and syntax characters.
- **Automatic filtering:** Skips whitespace and comments during tokenization.
- **Error reporting:** Provides information about failed tokenization, including the relevant position in the source.
- **Minimal allocations:** Uses string views to reference source text without unnecessary copying.

## Getting Started
Flexer requires no external dependencies beyond the C standard library.

To use Flexer, include `flexer.h` in your project and define `FLEXER_IMPL` in exactly one source file before including the header.

The basic workflow is:
1. Initialize a `Flexer_Lexer_Config` and register the desired syntax.
2. Create a lexer using `flexer_create_lexer()`, passing the source text and configuration.
3. Repeatedly call `flexer_next_token()` to retrieve tokens until encountering `FLEXER_EOF_TOKEN`.
4. Check each `Flexer_Token_Result` for success or failure.
5. Free the configuration when finished.

## Example
Here is a small example which tokenizes a small slice of C code and prints each token:

```c
#define FLEXER_IMPL
#include "flexer.h"

#define array_len(arr) (sizeof((arr)) / (sizeof(*(arr))))

const char *source_code = "int main() { int my_var = 10; printf(\"My var + 1 is %d\", my_var + 1); return 0; }";

const char *OPERATORS[] = {
    "+",
    "="
};
const char *KEYWORDS[] = {
    "int",
    "return"
};

int main(){
    // Initialize a config
    Flexer_Lexer_Config config = {0};

    // Register operators
    for (unsigned int i = 0; i < array_len(OPERATORS); i++){
        flexer_register_operator(&config, OPERATORS[i]);
    }
    // Register keywords
    for (unsigned int i = 0; i < array_len(KEYWORDS); i++){
        flexer_register_keyword(&config, KEYWORDS[i]);
    }

    // Create the lexer
    Flexer_Lexer lexer = flexer_create_lexer(flexer_sv_from_cstr(source_code), &config);

    Flexer_Token_Result result;
    do{
        // Repeatedly get the next token
        result = flexer_next_token(&lexer);

        // For simplicity exit if an error is encountered
        if (!result.is_ok){
            return 1;
        }

        Flexer_Token token = result.as_token;
        // Print token data
        switch (token.kind){
            case FLEXER_IDENTIFIER_TOKEN:{
                printf("IDENTIFIER(`"Flexer_SV_Fmt"`)\n", Flexer_SV_Arg(token.data.as_identifier));
                break;
            }
            case FLEXER_INTEGER_TOKEN:{
                printf("INTEGER(%lld)\n", token.data.as_integer);
                break;
            }
            case FLEXER_FLOAT_TOKEN:{
                printf("FLOAT(%f)\n", token.data.as_float);
                break;
            }
            case FLEXER_OPERATOR_TOKEN:{
                printf("OPERATOR(%s)\n", OPERATORS[token.data.as_operator_index]);
                break;
            }
            case FLEXER_KEYWORD_TOKEN:{
                printf("KEYWORD(%s)\n", KEYWORDS[token.data.as_keyword_index]);
                break;
            }
            case FLEXER_SYNTAX_TOKEN:{
                printf("SYNTAX(%c)\n", token.data.as_syntax_char);
                break;
            }
            case FLEXER_STRING_TOKEN:{
                printf("STRING(\""Flexer_SV_Fmt"\")\n", Flexer_SV_Arg(token.data.as_string));
                break;
            }
            default: break;
        }
        
        // Continue until an EOF token is encountered
    } while(result.as_token.kind != FLEXER_EOF_TOKEN);

    flexer_free_lexer_config(&config);
    return 0;
}
```

## How It Works
Flexer uses a finite-state machine to identify sequences of characters that form valid tokens. Characters are classified into categories, such as digits, letters, and whitespace, which are used to determine state transitions.

The lexer tracks the most recent accepting state, allowing it to recognize the longest valid token before returning a result. Registered keywords and operators are used to further classify recognized sequences.

## Additional Notes
- Token positions use zero-based line and character indices.
- String views reference the original source, which must remain valid while tokens are in use.
- String literal contents are returned as views without decoding escape sequences.
- Allocation and assertion behavior can be customized through the `FLEXER_REALLOC`, `FLEXER_FREE`, and `FLEXER_ASSERT` macros.