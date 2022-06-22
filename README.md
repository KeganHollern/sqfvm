# SQFVM
SQF Interpreter for Arma Reforger and Enfusion Engine. 

## Lexer
Converts SQF text into a sequence of tokens.

### Usage

```c#
SQFLexer lexer = new SQFLexer("hint 'this is a ''script''!';");
while(true) {
  SQFToken nextToken = lexer.Next();
  if(nextToken.TokenType() == ESQFTokenType.END_OF_SCRIPT)
    break;

  Print(nextToken.Stringify());
}
Print("lexing test complete");
```
outputs
```
SCRIPT       : Token: IDENTIFIER, 0, "hint"
SCRIPT       : Token: STRING, 5, "'this is a ''script''!'"
SCRIPT       : Token: SEMICOLON, 28, ";"
```

## Parser
Incomplete

## Interpreter
Incomplete


