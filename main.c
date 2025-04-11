#include <windows.h>
 
typedef enum { VALUE_INT, VALUE_STRING } ValueType;
typedef struct value { ValueType type; long i; char *s; } value;
typedef struct Variable { char *name; value val; struct Variable *next; } Variable;
Variable *variables = 0;
typedef void (*lib_func)(void);
typedef struct Library { char *name; lib_func func; struct Library *next; } Library;
Library *libraries = 0;
typedef struct { char **lines; int count; } LineArray;
 
void *__malloc(SIZE_T size) { return HeapAlloc(GetProcessHeap(), 0, size); }
void __free(void *ptr) { HeapFree(GetProcessHeap(), 0, ptr); }
 
size_t __strlen(const char *s) { size_t n = 0; while(s && s[n]) n++; return n; }
int __strcmp(const char *s1, const char *s2) { while(*s1 && *s1==*s2) { s1++; s2++; } return *(unsigned char *)s1 - *(unsigned char *)s2; }
char *__strcpy(char *dest, const char *src) { char *d = dest; while((*d++ = *src++)); return dest; }
int __strncmp(const char *s1, const char *s2, size_t n) { while(n && *s1 && (*s1==*s2)) { s1++; s2++; n--; } if(n==0) return 0; return *(unsigned char *)s1 - *(unsigned char *)s2; }
char *__strcat(char *dest, const char *src) { char *p = dest + __strlen(dest); while(*src) { *p++ = *src++; } *p = '\0'; return dest; }
long __atoi(const char *s) { long res = 0; int sign = 1; if(*s=='-') { sign = -1; s++; } while(*s >= '0' && *s<='9') { res = res*10 + (*s - '0'); s++; } return sign * res; }
 
char *trim(char *s)
{
  char *end;
  if(!s) return s;
  while(*s==' ' || *s=='\t' || *s=='\n' || *s=='\r') s++;
  if(__strlen(s)==0) return s;
  end = s + __strlen(s) - 1;
  while(end>=s && (*end==' ' || *end=='\t' || *end=='\n' || *end=='\r')) { *end = '\0'; end--; }
  return s;
}
 
Variable *find_variable(const char *name)
{
  Variable *cur = variables;
  while(cur) { if(__strcmp(cur->name, name)==0) return cur; cur = cur->next; }
  return 0;
}
 
void set_variable(const char *name, value val)
{
  Variable *var = find_variable(name);
  if(var) { var->val = val; }
  else
  {
    Variable *nv = (Variable*)__malloc(sizeof(Variable));
    int i = 0;
    int len = (int)__strlen(name) + 1;
    nv->name = (char*)__malloc((SIZE_T)len);
    while(name[i]) { nv->name[i] = name[i]; i++; }
    nv->name[i] = '\0';
    nv->val = val;
    nv->next = variables;
    variables = nv;
  }
}
 
void add_library(const char *name, lib_func func)
{
  Library *lib = (Library*)__malloc(sizeof(Library));
  int i = 0;
  int len = (int)__strlen(name) + 1;
  lib->name = (char*)__malloc((SIZE_T)len);
  while(name[i]) { lib->name[i] = name[i]; i++; }
  lib->name[i] = '\0';
  lib->func = func;
  lib->next = libraries;
  libraries = lib;
}
 
lib_func get_library(const char *name)
{
  Library *cur = libraries;
  while(cur) { if(__strcmp(cur->name, name)==0) return cur->func; cur = cur->next; }
  return 0;
}
 
void print_string(const char *s)
{
  DWORD written = 0;
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  WriteConsoleA(h, s, (DWORD)__strlen(s), &written, 0);
}
 
void print_int(long i)
{
  char buf[32];
  int pos = 0, neg = 0, start, end;
  if(i < 0) { neg = 1; i = -i; }
  if(i==0) { buf[pos++] = '0'; }
  while(i) { buf[pos++] = (char)('0' + (i % 10)); i /= 10; }
  if(neg) buf[pos++] = '-';
  start = 0; end = pos - 1;
  while(start < end) { char tmp = buf[start]; buf[start] = buf[end]; buf[end] = tmp; start++; end--; }
  buf[pos] = '\0';
  print_string(buf);
}
 
void print_newline(void) { print_string("\r\n"); }
 
DWORD get_rand(void) { return GetTickCount(); }
void sleep_random(void) { DWORD t = (get_rand() % 3001) + 2000; Sleep(t); }
 
char *__strchr(const char *s, int c)
{
  while(*s) { if(*s==c) return (char*)s; s++; }
  return 0;
}
 
char *__strstr(const char *haystack, const char *needle)
{
  int len = (int)__strlen(needle);
  if(len==0) return (char*)haystack;
  while(*haystack) { if(__strncmp(haystack, needle, (size_t)len)==0) return (char*)haystack; haystack++; }
  return 0;
}
 
char *__strrchr(const char *s, int c)
{
  char *res = 0;
  while(*s) { if(*s==c) res = (char*)s; s++; }
  return res;
}
 
typedef struct { int dummy; } Dummy;
value eval_simple(char *s)
{
  value result;
  int i, j;
  size_t len_temp;
  result.type = VALUE_INT;
  result.i = 0;
  s = trim(s);
  if(s[0]=='\"' && s[__strlen(s)-1]=='\"')
  {
    len_temp = __strlen(s);
    result.type = VALUE_STRING;
    result.s = (char*)__malloc(len_temp - 1);
    i = 1; j = 0;
    while(i < (int)len_temp - 1) { result.s[j++] = s[i++]; }
    result.s[j] = '\0';
    return result;
  }
  if((s[0]>='0' && s[0]<='9') || (s[0]=='-' && s[1]))
  {
    result.i = __atoi(s);
    return result;
  }
  { Variable *var = find_variable(s); if(var) return var->val; }
  return result;
}
 
value evaluate_expression(char *expr)
{
  char *cleaned;
  char *ops[] = {"<=", ">=", "==", "!=", "<", ">"};
  int num_ops = 6, i;
  cleaned = trim(expr);
  for(i = 0; i < num_ops; i++)
  {
    char *op = ops[i], *pos = 0;
    int j;
    for(j = 0; cleaned[j] != '\0'; j++)
    {
      int k = 0;
      while(op[k] && cleaned[j+k] && cleaned[j+k]==op[k]) k++;
      if(op[k]=='\0') { pos = cleaned+j; break; }
    }
    if(pos)
    {
      int op_len = (int)__strlen(op);
      char left[256], right[256];
      int j = 0;
      while(j < (int)(pos - cleaned) && j < 255) { left[j] = cleaned[j]; j++; }
      left[j] = '\0';
      { int k = 0; pos += op_len; while(*pos && k < 255) { right[k++] = *pos; pos++; } right[k] = '\0'; }
      { value lval = evaluate_expression(left), rval = evaluate_expression(right), result;
        result.type = VALUE_INT;
        if(__strcmp(op, "==")==0) result.i = (lval.i == rval.i);
        else if(__strcmp(op, "!=")==0) result.i = (lval.i != rval.i);
        else if(__strcmp(op, "<=")==0) result.i = (lval.i <= rval.i);
        else if(__strcmp(op, ">=")==0) result.i = (lval.i >= rval.i);
        else if(__strcmp(op, "<")==0) result.i = (lval.i < rval.i);
        else if(__strcmp(op, ">")==0) result.i = (lval.i > rval.i);
        return result;
      }
    }
  }
  {
    char token[256];
    int pos = 0, i_token = 0;
    value result;
    result.type = VALUE_INT;
    result.i = 0;
    while(cleaned[pos] != '\0' && i_token < 255)
    {
      if(cleaned[pos]==' ') break;
      token[i_token++] = cleaned[pos++];
    }
    token[i_token] = '\0';
    result = eval_simple(token);
    while(cleaned[pos] != '\0')
    {
      char op;
      char next[256];
      while(cleaned[pos]==' ') pos++;
      if(cleaned[pos]=='\0') break;
      op = cleaned[pos++];
      while(cleaned[pos]==' ') pos++;
      { int k = 0; while(cleaned[pos]!=' ' && cleaned[pos]!='\0' && k < 255) { next[k++] = cleaned[pos++]; } next[k] = '\0'; }
      { value next_val = eval_simple(next);
        if(op=='+') result.i += next_val.i;
        else if(op=='-') result.i -= next_val.i;
        else if(op=='*') result.i *= next_val.i;
        else if(op=='/' && next_val.i != 0) result.i /= next_val.i;
      }
    }
    return result;
  }
}
 
LineArray split_lines(char *buffer)
{
  LineArray la;
  int capacity;
  char *p;
  la.count = 0;
  la.lines = 0;
  capacity = 16;
  la.lines = (char**)__malloc(sizeof(char*) * capacity);
  p = buffer;
  while(*p)
  {
    char *line;
    line = p;
    while(*p && *p!='\n' && *p!='\r') p++;
    if(*p) { *p = '\0'; p++; if(*p=='\n' || *p=='\r') { p++; } }
    if(la.count >= capacity)
    {
      capacity = capacity * 2;
      la.lines = (char**)HeapReAlloc(GetProcessHeap(), 0, la.lines, sizeof(char*) * capacity);
    }
    la.lines[la.count++] = line;
  }
  return la;
}
 
void execute_block(LineArray la, int *index);
 
void execute_line(LineArray la, int *index)
{
  char buffer[256];
  int i = 0;
  char *line = la.lines[*index];
  while(line[i] && i < 255) { buffer[i] = line[i]; i++; }
  buffer[i] = '\0';
  {
    char *trimmed = trim(buffer);
    if(trimmed[0]=='\0') { (*index)++; return; }
    if(__strncmp(trimmed, "import", 6)==0)
    {
      char *p = trimmed + 6;
      while(*p && *p!='\"') p++;
      if(*p=='\"')
      {
        char libname[128];
        int j = 0;
        p++;
        while(*p && *p!='\"' && j < 127) { libname[j++] = *p; p++; }
        libname[j] = '\0';
        {
          char path[256];
          __strcpy(path, "libraries\\");
          __strcat(path, libname);
          __strcat(path, ".dll");
          { HINSTANCE hinst = LoadLibraryA(path);
            if(hinst) { lib_func func = (lib_func)GetProcAddress(hinst, "run"); if(func) add_library(libname, func); }
          }
        }
      }
      sleep_random();
      (*index)++;
      return;
    }
    if(__strncmp(trimmed, "for", 3)==0)
    {
      char *start_paren, *end_paren;
      char header[256], init[128], cond[128], iter[128];
      int i_idx, j_idx;
      start_paren = __strchr(trimmed, '(');
      end_paren = __strchr(trimmed, ')');
      if(start_paren && end_paren && end_paren > start_paren)
      {
        int len = (int)(end_paren - start_paren - 1);
        if(len > 255) len = 255;
        for(i_idx = 0; i_idx < len; i_idx++) { header[i_idx] = start_paren[i_idx+1]; }
        header[i_idx] = '\0';
        i_idx = 0; j_idx = 0;
        while(header[i_idx] && header[i_idx]!=';' && j_idx < 127) { init[j_idx++] = header[i_idx++]; }
        init[j_idx] = '\0';
        if(header[i_idx]==';') i_idx++;
        j_idx = 0;
        while(header[i_idx] && header[i_idx]!=';' && j_idx < 127) { cond[j_idx++] = header[i_idx++]; }
        cond[j_idx] = '\0';
        if(header[i_idx]==';') i_idx++;
        j_idx = 0;
        while(header[i_idx] && j_idx < 127) { iter[j_idx++] = header[i_idx++]; }
        iter[j_idx] = '\0';
        {
          char varname[128];
          int m = 0;
          while(init[m] && init[m]!=' ' && m < 127) { varname[m] = init[m]; m++; }
          varname[m] = '\0';
          { char *expr = init + m; while(*expr==' ') expr++; set_variable(varname, evaluate_expression(expr)); }
        }
        (*index)++;
        if(*index < la.count && __strchr(la.lines[*index], '{')) { (*index)++; }
        {
          int block_start = *index;
          for(;;)
          {
            value cond_val = evaluate_expression(cond);
            if(cond_val.i == 0) break;
            { int inner_index = block_start; execute_block(la, &inner_index); }
            {
              char varname[128];
              int m = 0;
              while(init[m] && init[m]!=' ' && m < 127) { varname[m] = init[m]; m++; }
              varname[m] = '\0';
              set_variable(varname, evaluate_expression(iter));
            }
            sleep_random();
          }
        }
        while(*index < la.count && !__strchr(la.lines[*index], '}')) { (*index)++; }
        if(*index < la.count) (*index)++;
        return;
      }
    }
    if(__strncmp(trimmed, "println", 7)==0)
    {
      char *start_paren, *end_paren;
      char content[256];
      int k;
      start_paren = __strchr(trimmed, '(');
      end_paren = __strrchr(trimmed, ')');
      if(start_paren && end_paren)
      {
        int len = (int)(end_paren - start_paren - 1);
        if(len > 255) len = 255;
        for(k = 0; k < len; k++) { content[k] = start_paren[k+1]; }
        content[k] = '\0';
        { value val = evaluate_expression(content);
          if(val.type == VALUE_STRING) { print_string(val.s); print_newline(); }
          else { print_int(val.i); print_newline(); }
        }
      }
      sleep_random();
      (*index)++;
      return;
    }
    if(__strncmp(trimmed, "print", 5)==0)
    {
      char *start_paren, *end_paren;
      char content[256];
      int k;
      start_paren = __strchr(trimmed, '(');
      end_paren = __strrchr(trimmed, ')');
      if(start_paren && end_paren)
      {
        int len = (int)(end_paren - start_paren - 1);
        if(len > 255) len = 255;
        for(k = 0; k < len; k++) { content[k] = start_paren[k+1]; }
        content[k] = '\0';
        { value val = evaluate_expression(content);
          if(val.type == VALUE_STRING) print_string(val.s); else print_int(val.i);
        }
      }
      sleep_random();
      (*index)++;
      return;
    }
    {
      char *eq = __strchr(trimmed, '=');
      if(eq)
      {
        char varname[128];
        int n = (int)(eq - trimmed);
        if(n > 127) n = 127;
        { int k; for(k = 0; k < n; k++) { varname[k] = trimmed[k]; } varname[k] = '\0'; }
        { char *expr = eq + 1; char *semi = __strchr(expr, ';'); if(semi) *semi = '\0';
          set_variable(trim(trimmed), evaluate_expression(expr));
        }
        sleep_random();
        (*index)++;
        return;
      }
    }
    {
      char *paren = __strstr(trimmed, "()");
      if(paren)
      {
        char funcname[128];
        int n = (int)(paren - trimmed);
        if(n > 127) n = 127;
        { int k; for(k = 0; k < n; k++) { funcname[k] = trimmed[k]; } funcname[k] = '\0'; }
        { lib_func func = get_library(trim(funcname)); if(func) func(); }
        sleep_random();
        (*index)++;
        return;
      }
    }
  }
  (*index)++;
}
 
void execute_block(LineArray la, int *index)
{
  while(*index < la.count)
  {
    if(__strchr(la.lines[*index], '}')) { (*index)++; break; }
    execute_line(la, index);
  }
}
 
int WINAPI WinMainCRTStartup(void)
{
  LPSTR cmd;
  int argc;
  char *argv[16];
  char *p;
  cmd = GetCommandLineA();
  argc = 0;
  p = cmd;
  while(*p)
  {
    while(*p==' ' || *p=='\t') p++;
    if(!*p) break;
    if(*p=='\"')
    {
      p++;
      argv[argc++] = p;
      while(*p && *p!='\"') p++;
      if(*p) { *p = '\0'; p++; }
    }
    else
    {
      argv[argc++] = p;
      while(*p && *p!=' ' && *p!='\t') p++;
      if(*p) { *p = '\0'; p++; }
    }
    if(argc >= 16) break;
  }
  if(argc < 2)
  {
    print_string("Usage: badscript_interpreter <script file>\r\n");
    ExitProcess(1);
  }
  {
    HANDLE hFile;
    DWORD size, read;
    char *buffer;
    hFile = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(hFile == INVALID_HANDLE_VALUE)
    {
      print_string("Cannot open file.\r\n");
      ExitProcess(1);
    }
    size = GetFileSize(hFile, 0);
    buffer = (char*)__malloc(size+1);
    ReadFile(hFile, buffer, size, &read, 0);
    buffer[size] = '\0';
    CloseHandle(hFile);
    {
      LineArray la;
      int index;
      la = split_lines(buffer);
      index = 0;
      while(index < la.count) { execute_line(la, &index); }
    }
  }
  ExitProcess(0);
  return 0;
}
 
void __report_rangecheckfailure(void)
{
  ExitProcess(3);
}
 
void __GSHandlerCheck(void)
{
}
 
#ifdef _WIN64
unsigned __int64 __security_cookie = 0;
void __security_check_cookie(unsigned __int64 cookie) { (void)cookie; }
#else
unsigned int __security_cookie = 0;
void __security_check_cookie(unsigned int cookie) { (void)cookie; }
#endif
 
void *memcpy(void *dest, const void *src, size_t count)
{
  char *d = (char *)dest;
  const char *s = (const char *)src;
  while(count--) { *d++ = *s++; }
  return dest;
}
