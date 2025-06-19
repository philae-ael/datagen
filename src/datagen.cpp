#include "core/core.h"
#include <cctype>
#include <stdio.h>
#include <stdlib.h>

// Token types
enum TokenType {
  TOKEN_EOF,
  TOKEN_STRUCT,
  TOKEN_FLAGS,
  TOKEN_IDENTIFIER,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_COMMA,
  TOKEN_AT,
  TOKEN_GENERATES,
  TOKEN_ERROR
};

struct SourceLocation {
  u32 line;
  u32 column;
  usize offset;
};

struct Token {
  TokenType type;
  str8 value;
  SourceLocation location;
};

struct Lexer {
  str8 input;
  usize pos;
  u32 line;
  u32 column;
};

// Error reporting
struct ParseError {
  str8 message;
  SourceLocation location;
};

struct ErrorList {
  ParseError *errors;
  usize count;
  usize capacity;
};

struct FieldDecl {
  str8 type_name;
  str8 field_name;
  SourceLocation location;
};

struct StructDecl {
  str8 name;
  FieldDecl *fields;
  usize field_count;
  char **generators;
  usize generator_count;
  SourceLocation location;
};

struct FlagsDecl {
  char *name;
  char **members;
  usize member_count;
  char **generators;
  usize generator_count;
  SourceLocation location;
};

struct ParseResult {
  StructDecl *structs;
  usize struct_count;
  FlagsDecl *flags;
  usize flags_count;
  ErrorList errors;
};

struct Parser {
  Lexer *lexer;
  bool has_current;
  Token current;
  ErrorList *errors;
};

// Utility functions
void add_error(ErrorList *errors, str8 message, SourceLocation location) {
  if (errors->count >= errors->capacity) {
    errors->capacity = errors->capacity ? errors->capacity * 2 : 8;
    errors->errors = (ParseError *)realloc(
        errors->errors, errors->capacity * sizeof(ParseError));
  }

  errors->errors[errors->count].message = message;
  errors->errors[errors->count].location = location;
  errors->count++;
}

Lexer init_lexer(str8 input) { return Lexer{input, 0, 1, 1}; }

str8 peek(Lexer *lexer) {
  if (lexer->pos >= lexer->input.len)
    return {};
  return {lexer->input.data + lexer->pos, 1};
}

char advance(Lexer *lexer) {
  if (lexer->pos >= lexer->input.len)
    return '\0';
  char c = (u8)lexer->input.data[lexer->pos++];
  if (c == '\n') {
    lexer->line++;
    lexer->column = 1;
  } else {
    lexer->column++;
  }
  return c;
}

void skip_whitespace(Lexer *lexer) {
  str8 n;
  while ((n = peek(lexer), n.len > 0 && isspace(n.data[0]))) {
    advance(lexer);
  }
}

bool is_alpha(char c) { return isalpha(c) || c == '_'; }

bool is_alnum(char c) { return isalnum(c) || c == '_'; }

Token make_token(TokenType type, str8 value, SourceLocation location) {
  Token token;
  token.type = type;
  token.value = value;
  token.location = location;
  return token;
}

Token scan_identifier(Lexer *lexer) {
  SourceLocation location = {lexer->line, lexer->column, lexer->pos};
  usize start = lexer->pos;

  str8 n;
  while ((n = peek(lexer), n.len > 0 && is_alnum(n.data[0]))) {
    advance(lexer);
  }

  usize length = lexer->pos - start;
  str8 value = {lexer->input.data + start, length};

  TokenType type = TOKEN_IDENTIFIER;
  if (value.equal("struct"_u8))
    type = TOKEN_STRUCT;
  else if (value.equal("flags"_u8))
    type = TOKEN_FLAGS;
  else if (value.equal("generates"_u8))
    type = TOKEN_GENERATES;

  return make_token(type, value, location);
}

Token next_token(Lexer *lexer) {
  skip_whitespace(lexer);
  SourceLocation location = {lexer->line, lexer->column, lexer->pos};
  str8 c = peek(lexer);
  if (c.len == 0) {
    return make_token(TOKEN_EOF, {}, location);
  }

  switch (c.data[0]) {
  case '{':
    advance(lexer);
    return make_token(TOKEN_LBRACE, c, location);
  case '}':
    advance(lexer);
    return make_token(TOKEN_RBRACE, c, location);
  case '(':
    advance(lexer);
    return make_token(TOKEN_LPAREN, c, location);
  case ')':
    advance(lexer);
    return make_token(TOKEN_RPAREN, c, location);
  case ',':
    advance(lexer);
    return make_token(TOKEN_COMMA, c, location);
  case '@':
    advance(lexer);
    return make_token(TOKEN_AT, c, location);
  default:
    if (is_alpha(c.data[0])) {
      return scan_identifier(lexer);
    } else {
      advance(lexer);
      return make_token(TOKEN_ERROR, {}, location);
    }
  }
}

Parser init_parser(Lexer *lexer, ErrorList *errors) {
  return Parser{lexer, false, {}, errors};
}

Token current_token(Parser *parser) {
  if (parser->has_current) {
    return parser->current;
  }
  parser->current = next_token(parser->lexer);
  parser->has_current = true;
  return parser->current;
}
Token consume_token(Parser *parser) {
  current_token(parser);
  parser->has_current = false;
  return parser->current;
}

bool check(Parser *parser, TokenType type) {
  return current_token(parser).type == type;
}

bool match(Parser *parser, TokenType type) {
  if (check(parser, type)) {
    consume_token(parser);
    return true;
  }
  return false;
}

Token expect(Parser *parser, TokenType type, str8 error_msg) {
  if (check(parser, type)) {
    return consume_token(parser);
  }

  add_error(parser->errors, error_msg, current_token(parser).location);
  return make_token(TOKEN_ERROR, current_token(parser).value,
                    current_token(parser).location);
}

void synchronize_to_next_field(Parser *parser) {
  while (!check(parser, TOKEN_EOF) && !check(parser, TOKEN_COMMA) &&
         !check(parser, TOKEN_RBRACE)) {
    consume_token(parser);
  }

  if (match(parser, TOKEN_COMMA)) {
    // ready to parse next field
  }
}

struct FieldDeclRes {
  FieldDecl field;
  bool success;
};
FieldDeclRes parse_field(Parser *parser) {
  Token type_token = expect(parser, TOKEN_IDENTIFIER, "Expected type name"_u8);
  if (type_token.type == TOKEN_ERROR)
    return {{}, false};

  Token name_token = expect(parser, TOKEN_IDENTIFIER, "Expected field name"_u8);
  if (name_token.type == TOKEN_ERROR) {
    synchronize_to_next_field(parser);
    return {{}, false};
  }

  return {
      FieldDecl{
          .type_name = type_token.value,
          .field_name = name_token.value,
          .location = type_token.location,
      },
      true,
  };
}

struct StructDeclRes {
  StructDecl decl;
  bool success;
};
StructDeclRes parse_struct(Parser *parser) {
  Token struct_token = expect(parser, TOKEN_STRUCT, "Expected 'struct'"_u8);
  if (struct_token.type == TOKEN_ERROR)
    return {{}, false};

  Token name_token =
      expect(parser, TOKEN_IDENTIFIER, "Expected struct name"_u8);
  if (name_token.type == TOKEN_ERROR)
    return {{}, false};

  if (expect(parser, TOKEN_LBRACE, "Expected '{'"_u8).type == TOKEN_ERROR)
    return {{}, false};

  StructDecl decl = {name_token.value, NULL, 0, NULL, 0, struct_token.location};

  // Parse fields
  usize field_capacity = 0;
  while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
    FieldDeclRes field = parse_field(parser);
    if (field.success) {
      if (decl.field_count >= field_capacity) {
        field_capacity = field_capacity ? field_capacity * 2 : 4;
        decl.fields = (FieldDecl *)realloc(decl.fields,
                                           field_capacity * sizeof(FieldDecl));
      }
      decl.fields[decl.field_count++] = field.field;
    }

    if (!match(parser, TOKEN_COMMA)) {
      break;
    }
  }

  expect(parser, TOKEN_RBRACE, "Expected '}'"_u8);

  return {decl, true};
}

ParseResult parse_file(str8 input) {
  ParseResult result{};

  usize token_count;
  Lexer lexer = init_lexer(input);

  Parser parser = init_parser(&lexer, &result.errors);

  usize struct_capacity = 0;

  while (!check(&parser, TOKEN_EOF)) {
    if (check(&parser, TOKEN_STRUCT)) {
      StructDeclRes decl = parse_struct(&parser);
      if (decl.success) {
        if (result.struct_count >= struct_capacity) {
          struct_capacity = struct_capacity ? struct_capacity * 2 : 4;
          result.structs = (StructDecl *)realloc(
              result.structs, struct_capacity * sizeof(StructDecl));
        }
        result.structs[result.struct_count++] = decl.decl;
      }
    } else {
      add_error(&result.errors, "Expected declaration"_u8,
                current_token(&parser).location);
      consume_token(&parser);
    }
  }

  return result;
}

void print_errors(ErrorList *errors) {
  for (usize i = 0; i < errors->count; i++) {
    printf("Error at line %d, column %d: %.*s\n",
           errors->errors[i].location.line, errors->errors[i].location.column,
           (int)errors->errors[i].message.len, errors->errors[i].message.data);
  }
}

void print_parse_result(ParseResult *result) {
  printf("Parsed %zu structs:\n", result->struct_count);
  for (usize i = 0; i < result->struct_count; i++) {
    StructDecl *s = &result->structs[i];
    printf("  struct %.*s {\n", (int)s->name.len, s->name.data);
    for (usize j = 0; j < s->field_count; j++) {
      printf("    %.*s %.*s\n", (int)s->fields[j].type_name.len,
             s->fields[j].type_name.data, (int)s->fields[j].field_name.len,
             s->fields[j].field_name.data);
    }
    printf("  }\n");
  }
}

int main() {
  auto test_input = "struct Player {\n"
                    "    int health,\n"
                    "    float speed,\n"
                    "    char name\n"
                    "}\n"
                    "struct Enemy {\n"
                    "    int damage\n"
                    "} generates(to_string)\n"_u8;

  ParseResult result = parse_file(test_input);

  if (result.errors.count > 0) {
    printf("Parse errors:\n");
    print_errors(&result.errors);
    // return 1;
  }

  print_parse_result(&result);
  return 0;
}
