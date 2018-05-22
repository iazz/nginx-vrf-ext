/* Copyright (C) 2018 by Green Communications
 *
 * ngx_vrf_ext NGINX module.
 *
 * This adds support for variable $request_vrf which contains the name
 * of the VRF from which the current request has arrived.  This works
 * only on Linux kernels supporting VRF.  For the default VRF, the
 * $request_vrf variable is empty.
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>

#include <assert.h>

/* Retrieve the value of the $request_vrf variable. */
static ngx_int_t
ngx_http_vrf_get_variable(ngx_http_request_t		*r,
			  ngx_http_variable_value_t	*v,
			  uintptr_t			data)
{
  u_char	*name;
  socklen_t	name_len = IFNAMSIZ;
  int		ret;

  assert(r->connection != NULL);

  name = ngx_pnalloc(r->pool, name_len);
  if (name == NULL) {
    ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
		  "Could not allocate memory for $request_vrf variable.");
    return NGX_ERROR;
  }

  ret = getsockopt(r->connection->fd, SOL_SOCKET, SO_BINDTODEVICE,
		   name, &name_len);
  if (ret == -1) {
    ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_socket_errno,
		  "Could not get interface to which connection socket"
		  " is bound.");
    return NGX_ERROR;
  }

  v->data         = name;
  v->len          = name_len;
  v->valid        = 1;
  v->no_cacheable = 0;
  v->not_found    = 0;

  ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
		"Getting $request_vrf value = \"%s\".", v->data);

  return NGX_OK;
}

/* List of supported variables. */
static ngx_http_variable_t	ngx_http_vrf_ext_vars[] = {
  { ngx_string("request_vrf"), NULL, ngx_http_vrf_get_variable, 0, 0, 0 },
  { ngx_null_string, NULL, NULL, 0, 0, 0 }
};

/* Add supported variables during preconfiguration. */
static ngx_int_t
ngx_http_vrf_ext_add_variable(ngx_conf_t *cf)
{
  ngx_http_variable_t	*var, *v;

  for (v = ngx_http_vrf_ext_vars; v->name.len; ++v) {
    var = ngx_http_add_variable(cf, &v->name, v->flags);

    if (var == NULL)
      return NGX_ERROR;

    var->get_handler = v->get_handler;
    var->data        = v->data;
  }

  return NGX_OK;
}

/* Module context */
static ngx_http_module_t
ngx_http_vrf_ext_module_ctx = {
  ngx_http_vrf_ext_add_variable,	/* Preconfiguration */
  NULL,					/* Postconfiguration */

  NULL,					/* Create main configuration */
  NULL,					/* Init main configuration */

  NULL,					/* Create server configuration */
  NULL,					/* Merge server configuration */

  NULL,					/* Create location configuration */
  NULL					/* Merge location configuration */
};

/* Module definition */
ngx_module_t
ngx_http_vrf_ext_module = {
  NGX_MODULE_V1,
  &ngx_http_vrf_ext_module_ctx,		/* Module context */
  NULL,					/* Module directives */
  NGX_HTTP_MODULE,			/* Module type */
  NULL,					/* Init master */
  NULL,					/* Init module */
  NULL,					/* Init process */
  NULL,					/* Init thread */
  NULL,					/* Exit thread */
  NULL,					/* Exit process */
  NULL,					/* Exit master */
  NGX_MODULE_V1_PADDING
};
