#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ERR_DIV_ZERO      0
#define ERR_OVERFLOW      1
#define ERR_PAREN_MISM    2
#define ERR_FUNC_INP_MISS 3
#define ERR_INV_OP        4

#define MAX_TOK_LEN 32

#define MAX_INP_LEN 256

#define MAX_OUT_LEN 256

#define MAX_TOKENS  64

#define MAX_ERR_STR 128

#define DIGIT(character) ((int)(character) - '0')

#define IS_ALPHA(c)((((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z')) ? 1 : 0)

typedef enum {
  add_s, sub_s, mul_s, exp_s, lpar_s, rpar_s, dig_s, div_s, dec_s, txt_s, comma_s, num_s,
  inval_s, eof_s, ceil_f, fmod_f, floor_f, fabs_f, atan_f, acos_f, asin_f, atan2_f, cos_f,
  sin_f, tan_f, cosh_f, sinh_f, tanh_f, sqrt_f, log_f, log10_f, log2_f, cbrt_f, trunc_f, inval_f
} sym_t;

typedef struct {
  sym_t symbol;
  char * txt;
} tok_t;

typedef double num_t;

tok_t * tokens[MAX_TOKENS] = {NULL};

int tokens_len;

int current_idx;

tok_t * next_tok = NULL;

tok_t eof_token;

char * error_string = NULL;

tok_t * error_token = NULL;


void error(char * error);

void do_exit();

void do_help();

void clear_toks();

char * trim(char * str);

num_t string_to_num(char * tok);

void num_to_string(num_t num, char * res);

sym_t symbol(char c);

sym_t get_func(tok_t * token);

int tokenize(char * str);

tok_t * get_next();

tok_t * peek_next();

int _parse_func_rest(num_t * result);

int _parse_func(num_t * result);

int _parse_numb(num_t * result);

int _parse_atom(num_t * result);

int _parse_factor(num_t * result);

int _parse_term(num_t * result);

int _parse_expression(num_t * result);

int parse(num_t * result);

int _accept_token(sym_t sym);

int process_request(char * req, char * res);

int perf_func(tok_t * func, num_t * op1, num_t * op2, num_t * res);


void error(char * error){
  printf("\n[ERR]: %s", error);
}

void do_help(){
  printf("\n***************************************************************");
  printf("\n               [---]: DBOS CALCULATOR :[---]");
  printf("\n***************************************************************");
  printf("\n[INF]: Type in any expression in the form of: (1 * 2) + 3 ... ");
  printf("\n[INF]: Operators: \'+\', \'-\', \'*\', \'/\', \'^ (power)\'");
  printf("\n[INF]: Functions[1]: ceil(x), fmod(x, y), floor(x), fabs(x)");
  printf("\n[INF]: Functions[2]: atan(x), atan2(x, y), acos(x), asin(x)");
  printf("\n[INF]: Functions[3]: cos(x), sin(x), tan(x), cosh(x), sinh(x)");
  printf("\n[INF]: Functions[4]: tanh(x), sqrt(x), log(x), log10(x)");
  printf("\n[INF]: Functions[5]: log2(x), cbrt(x), trunc(x)");
  printf("\n");
}

void do_exit(){
  printf("\n[EXT]: GOODBYE");
}

void clear_toks(){
  for(int i = 0; i < tokens_len; i++){
    free(tokens[i]->txt);
    free(tokens[i]);
    tokens[i] = NULL;
  }
}

char * trim(char * str){
  char * end;
  while(isspace(*str))
    str++;

  if(*str == '\0')
    return str;

  end = str + strlen(str) - 1;
  while(end > str && isspace(*end))
    end--;

  *(end + 1) = '\0';
  return str;
}

num_t string_to_num(char * tok){
  return (num_t) strtod( tok, NULL);
}

void num_to_string(num_t num, char * res){
  char * t = res;

  int ip = (int) num;

  double fp = (double) (num - ip);

  int i = itoa(ip, t);

  t[i] = '.';

  fp = fp * pow(10, 5);

  itoa((int) fabs(fp), t + i + 1);
}

sym_t symbol(char c){
  sym_t res;

  if(isdigit(c)){
    res = dig_s;

  } else if(IS_ALPHA(c)){
    res = txt_s;

  } else {
    switch(c){
      case '+': {res = add_s;   break;}
      case '-': {res = sub_s;   break;}
      case '*': {res = mul_s;   break;}
      case '/': {res = div_s;   break;}
      case '^': {res = exp_s;   break;}
      case '(': {res = lpar_s;  break;}
      case ')': {res = rpar_s;  break;}
      case '.': {res = dec_s;   break;}
      case ',': {res = comma_s; break;}
      default:  {
        res = inval_s;
        break;
      }
    }
  }

  return res;
}

sym_t get_func(tok_t * token){
  sym_t ret;
  if(!strcmp("ceil", token->txt)){ret = ceil_f;}
  else if(!strcmp("fmod", token->txt)){ret = fmod_f;}
  else if(!strcmp("floor", token->txt)){ret = floor_f;}
  else if(!strcmp("fabs", token->txt)){ret = fabs_f;}
  else if(!strcmp("atan", token->txt)){ret = atan_f;}
  else if(!strcmp("acos", token->txt)){ret = acos_f;}
  else if(!strcmp("asin", token->txt)){ret = asin_f;}
  else if(!strcmp("atan2", token->txt)){ret = atan2_f;}
  else if(!strcmp("cos", token->txt)){ret = cos_f;}
  else if(!strcmp("sin", token->txt)){ret = sin_f;}
  else if(!strcmp("tan", token->txt)){ret = tan_f;}
  else if(!strcmp("cosh", token->txt)){ret = cosh_f;}
  else if(!strcmp("sinh", token->txt)){ret = sinh_f;}
  else if(!strcmp("tanh", token->txt)){ret = tanh_f;}
  else if(!strcmp("sqrt", token->txt)){ret = sqrt_f;}
  else if(!strcmp("log", token->txt)){ret = log_f;}
  else if(!strcmp("log10", token->txt)){ret = log10_f;}
  else if(!strcmp("log2", token->txt)){ret = log2_f;}
  else if(!strcmp("cbrt", token->txt)){ret = cbrt_f;}
  else if(!strcmp("trunc", token->txt)){ret = trunc_f;}
  else {
    ret = inval_f;
    error("Invalid function name!");
  }
  return ret;
}

int tokenize(char * str){
  tok_t * tmp = (tok_t *) malloc(sizeof(tok_t));

  char * _str = str;

  char ch = '\0';

  sym_t symb;

  while((ch = *_str++)){
    if(ch == ' '){
      continue;
    }
    if(symbol(ch) == inval_s){
      return -1;
    }

    tmp->txt = (char *) malloc(sizeof(char) * MAX_TOK_LEN);
    int len;
    switch( (symb = symbol(ch)) ){

      case dig_s: {
        if(tokens_len > 0 && tokens[tokens_len - 1]->symbol == txt_s){
          goto part_of_func;
        }
        tmp->txt[0] = ch;
        for(len = 1; *_str && (symbol(*_str) == dig_s || symbol(*_str) == dec_s) && len < MAX_TOK_LEN; len++){
          tmp->txt[len] = *_str++;
        }
        tmp->txt[len] = '\0';
        tmp->symbol = num_s;
        break;
      }

      case txt_s: {
part_of_func:
        tmp->txt[0] = ch;
        for(len = 1; *_str && ((symbol(*_str) == txt_s) || (symbol(*_str) == dig_s)) && len < MAX_TOK_LEN; len++){
          tmp->txt[len] = *_str++;
        }
        tmp->txt[len] = '\0';
        tmp->symbol = txt_s;
        if(get_func(tmp) == inval_f){
          return -1;
        }
        break;
      }

      default: {
        if(symbol(ch) == inval_s){
          return -1;
        }
        tmp->txt[0] = ch;
        tmp->txt[1] = '\0';
        tmp->symbol = symb;

      }
    }

    tokens[tokens_len] = tmp;
    tokens_len++;
    tmp = (tok_t *) malloc(sizeof(tok_t));
  }

  tokens[tokens_len] = &eof_token;
  tokens_len++;

  return 0;
}

int perf_op(tok_t * op, num_t * op1, num_t * op2, num_t * res){
  int ret = 0;

  if(op == NULL || res == NULL || op1 == NULL || op2 == NULL){ return -1; }

  switch(op->symbol){
    case add_s: {*res = (double)((*op1) + (*op2)); break;}
    case sub_s: {*res = (double)((*op1) - (*op2)); break;}
    case mul_s: {*res = (double)((*op1) * (*op2)); break;}
    case div_s: {
      if(*op2 == 0){ret = -1; error("Division by 0!"); break;}
      *res = (double)(*op1 / *op2); break;
    }
    case exp_s: {*res = (double) pow(*op1, *op2); break;}
    default: {
      ret = -1;
      break;
    }
  }
  return ret;
}

int perf_func(tok_t * func, num_t * op1, num_t * op2, num_t * res){
  int ret = 0;
  if(func == NULL || res == NULL || op1 == NULL){ return -1; }
  switch(func->symbol){
    case ceil_f:  {if(op2 != NULL){ ret = -1; break;} *res = ceil(*op1);        break;}
    case fmod_f:  {if(op2 == NULL){ ret = -1; break;} *res = fmod(*op1, *op2);  break;}
    case floor_f: {if(op2 != NULL){ ret = -1; break;} *res = floor(*op1);       break;}
    case fabs_f:  {if(op2 != NULL){ ret = -1; break;} *res = fabs(*op1);        break;}
    case atan_f:  {if(op2 != NULL){ ret = -1; break;} *res = atan(*op1);        break;}
    case atan2_f: {if(op2 == NULL){ ret = -1; break;} *res = atan2(*op1, *op2); break;}
    case acos_f:  {if(op2 != NULL){ ret = -1; break;} *res = acos(*op1);        break;}
    case asin_f:  {if(op2 != NULL){ ret = -1; break;} *res = asin(*op1);        break;}
    case cos_f:   {if(op2 != NULL){ ret = -1; break;} *res = cos(*op1);         break;}
    case sin_f:   {if(op2 != NULL){ ret = -1; break;} *res = sin(*op1);         break;}
    case tan_f:   {if(op2 != NULL){ ret = -1; break;} *res = tan(*op1);         break;}
    case cosh_f:  {if(op2 != NULL){ ret = -1; break;} *res = cosh(*op1);        break;}
    case sinh_f:  {if(op2 != NULL){ ret = -1; break;} *res = sinh(*op1);        break;}
    case tanh_f:  {if(op2 != NULL){ ret = -1; break;} *res = tanh(*op1);        break;}
    case sqrt_f:  {if(op2 != NULL){ ret = -1; break;} *res = sqrt(*op1);        break;}
    case log_f:   {if(op2 != NULL){ ret = -1; break;} *res = log(*op1);         break;}
    case log10_f: {if(op2 != NULL){ ret = -1; break;} *res = log10(*op1);       break;}
    case log2_f:  {if(op2 != NULL){ ret = -1; break;} *res = log2(*op1);        break;}
    case cbrt_f:  {if(op2 != NULL){ ret = -1; break;} *res = cbrt(*op1);        break;}
    case trunc_f: {if(op2 != NULL){ ret = -1; break;} *res = trunc(*op1);       break;}
    default: {
      error("Invalid function operand(s)!");
      ret = -1;
      break;
    }
  }
  return ret;
}

int _parse_func_rest(num_t * result){
  if(next_tok->symbol == rpar_s){
    return _accept_token(rpar_s);

  } else if(next_tok->symbol == comma_s){
    if(_accept_token(comma_s)){return -1;}
    if(_parse_atom(result)){return -1;}
    if(_accept_token(rpar_s)){return -1;}
    return 1;

  } else {
    return -1;

  }
}

int _parse_func(num_t * result){
  tok_t * func_tok = next_tok;
  func_tok->symbol = get_func(func_tok);

  if(_accept_token(func_tok->symbol)){return -1;}
  if(_accept_token(lpar_s)){return -1;}

  double op1;
  double op2;

  if(_parse_atom(&op1)){ return -1;}

  int st = _parse_func_rest(&op2);

  if(st == 1){
    st = perf_func(func_tok, &op1, &op2, result);
  } else if(st == 0){
    st = perf_func(func_tok, &op1, NULL, result);
  } else {
    return -1;
  }

  return st;
}

int _parse_numb(num_t * result){
  tok_t * tok = next_tok;
  if(_accept_token(num_s)){return -1;}
  *result = string_to_num(tok->txt);
  return 0;
}

int _parse_atom(num_t * result){
  if(next_tok->symbol == lpar_s){
    if(_accept_token(lpar_s)){return -1;}

    if(_parse_expression(result)){ return -1;}

    if(_accept_token(rpar_s)){return -1;}

    return 0;

  } else if(next_tok->symbol == num_s){
    if(_parse_numb(result)){ return -1;}

  } else if(next_tok->symbol == txt_s){
    if(_parse_func(result)){ return -1;}

  } else {
    return -1;
  }
}

int _parse_s_atom(num_t * result){
  if(next_tok->symbol == sub_s){
    if(_accept_token(sub_s)){return -1;}

    if(_parse_atom(result)){ return -1;}
    *result = 0 - *result;
    return 0;
  }

  if(_parse_atom(result)){ return -1;}

  return 0;
}

int _parse_factor(num_t * result){

  double op1;
  double op2;

  if(_parse_s_atom(&op1)){ return -1;}

  if(next_tok->symbol == exp_s){
    tok_t * tmp = next_tok;
    if(_accept_token(exp_s)){return -1;}
    if(_parse_s_atom(&op2)){ return -1;}
    if(perf_op(tmp, &op1, &op2, result)){return -1;}
    return 0;

  } else {
    *result = op1;
    return 0;

  }
}

int _parse_term(num_t * result){
  double op1;
  double op2;

  if(_parse_factor(&op1)){ return -1;}

  if(next_tok->symbol == mul_s){
    tok_t * tmp = next_tok;
    if(_accept_token(next_tok->symbol)){return -1;}
    if(_parse_factor(&op2)){ return -1;}
    if(perf_op(tmp, &op1, &op2, result)){return -1;}

  } else if(next_tok->symbol == div_s){
    tok_t * tmp = next_tok;
    if(_accept_token(next_tok->symbol)){return -1;}
    if(_parse_factor(&op2)){ return -1;}
    if(perf_op(tmp, &op1, &op2, result)){return -1;}

  } else {
    *result = op1;
  }

  return 0;
}

int _parse_expression(num_t * result){
  double op1;
  double op2;

  if(_parse_term(&op1)){ return -1;}

  if(next_tok->symbol == add_s){
    tok_t * tmp = next_tok;
    if(_accept_token(next_tok->symbol)){return -1;}
    if(_parse_term(&op2)){ return -1;}
    if(perf_op(tmp, &op1, &op2, result)){return -1;}

  } else if(next_tok->symbol == sub_s){
    tok_t * tmp = next_tok;
    if(_accept_token(next_tok->symbol)){return -1;}
    if(_parse_term(&op2)){ return -1;}
    if(perf_op(tmp, &op1, &op2, result)){return -1;}

  } else {
    *result = op1;
    return 0;
  }
}

int parse(num_t * result){
  int res = 0;

  next_tok = get_next();

  res = _parse_expression(result);
  if(res == -1){
    return -1;
  }

  res = _accept_token(eof_token.symbol);
  if(res == -1){
    return -1;
  }

  return res;
}

tok_t * get_next(){
  tok_t * tok = tokens[current_idx];
  current_idx++;
  return tok;
}

int _accept_token(sym_t sym){
  if(next_tok->symbol == sym){
    next_tok = get_next();
    return 0;
  } else {
    return -1;
  }
}

int process_request(char * req, char * res){

  num_t result = 0.0;

  int stat;

  clear_toks();

  tokens_len = 0;

  current_idx = 0;

  next_tok = NULL;

  req = trim(req);

  stat = tokenize(req);

  if(stat == -1){
    error("Couldn\'t tokenize the expression!");
    goto fail;
  }

  stat = parse(&result);

  if(stat == -1){
    error("Couldn\'t parse the expression!");
    goto fail;
  }

  num_to_string(result, res);

  return 0;

fail:
  return -1;
}


int main(int argc, char * argv[]){

  char * line = (char *) malloc(MAX_INP_LEN * sizeof(char));

  char * res = (char *) malloc(MAX_OUT_LEN * sizeof(char));

  error_string = (char *) malloc(MAX_ERR_STR * sizeof(char));

  eof_token.symbol = eof_s;
  eof_token.txt = "EOF\0";

  printf("\n[---]:> ");

  while((line = gets(line, MAX_INP_LEN)) != NULL){

    if(!strncmp(line, "exit", 4)){ break; }
    if(!strncmp(line, "help", 4)){ do_help();}
    else {
      if(process_request(line, res)){
        memset((void *) line, '\0', MAX_INP_LEN);
        memset((void *) res, '\0', MAX_OUT_LEN);

      } else {
        printf(" = %s", res);
        memset((void *) line, '\0', MAX_INP_LEN);
        memset((void *) res, '\0', MAX_OUT_LEN);

      }
    }

    printf("\n[---]:> ");
  }

  free(line);

  free(res);

  free(error_string);

  do_exit();

  return 0;

}
