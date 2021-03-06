/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/* {{{ includes
 */

#define ZEND_INCLUDE_FULL_WINDOWS_HEADERS

#include "php.h"
#include <stdio.h>
#include <fcntl.h>
#ifdef PHP_WIN32
#include <process.h>
#elif defined(NETWARE)
#include <sys/timeval.h>
#ifdef USE_WINSOCK
#include <novsock2.h>
#endif
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_SETLOCALE
#include <locale.h>
#endif
#include "zend.h"
#include "zend_extensions.h"
#include "php_ini.h"
#include "php_globals.h"
#include "php_main.h"
#include "fopen_wrappers.h"
#include "ext/standard/php_string.h"
#include "php_variables.h"
#include "Zend/zend_exceptions.h"

#if PHP_SIGCHILD
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "zend_compile.h"
#include "zend_execute.h"
#include "zend_extensions.h"
#include "zend_ini.h"

#include <folly/portability/SysTime.h>
#include <folly/portability/Unistd.h>

#include "hphp/util/text-util.h"

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/zend-printf.h"

PHPAPI void php_error_docref0(const char *docref TSRMLS_DC, int type, const char *format, ...)
{
  va_list args;

  va_start(args, format);

  std::string msg;
  const char* space;
  const char* class_name = get_active_class_name(&space TSRMLS_CC);
  HPHP::string_printf(msg, "%s%s%s(): ", class_name, space, get_active_function_name(TSRMLS_C));
  msg += format;

  auto mode = static_cast<HPHP::ErrorMode>(type);

  HPHP::raise_message(mode, msg.c_str(), args);
  va_end(args);
}

PHPAPI int php_write(void *buf, uint size TSRMLS_DC)
{
  always_assert(size < INT_MAX);
  HPHP::g_context->write((const char*)buf, size);
  return (int)size;
}

PHPAPI int php_printf(const char *format, ...)
{
  va_list args;
  int ret;
  char *buffer;
  int size;
  TSRMLS_FETCH();

  va_start(args, format);
  size = HPHP::vspprintf_ap(&buffer, 0, format, args);
  ret = php_write(buffer, size TSRMLS_CC);
  free(buffer);
  va_end(args);

  return ret;
}
