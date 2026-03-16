/* norce extension for PHP */

#ifndef PHP_NORCE_H
# define PHP_NORCE_H

extern zend_module_entry norce_module_entry;
# define phpext_norce_ptr &norce_module_entry

# define PHP_NORCE_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_NORCE)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_NORCE_H */
