#include <windows.h>
__declspec(dllexport) void run(void)
{
  DWORD i;
  for(i=1;i<=100;i++)
  {
    char buf[32];
    int pos = 0;
    long num = i;
    int neg = 0;
    if(num < 0) { neg = 1; num = -num; }
    if(num == 0) { buf[pos++] = '0'; }
    while(num) { buf[pos++] = '0' + (num % 10); num /= 10; }
    if(neg) buf[pos++] = '-';
    int start = 0, end = pos - 1;
    while(start < end) { char tmp = buf[start]; buf[start] = buf[end]; buf[end] = tmp; start++; end--; }
    buf[pos] = '\0';
    DWORD written = 0;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(h, buf, (DWORD)lstrlenA(buf), &written, 0);
    WriteConsoleA(h, "\r\n", 2, &written, 0);
  }
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