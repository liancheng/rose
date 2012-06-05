#ifndef __ROSE_DETAIL_SEXP_IO_H__
#define __ROSE_DETAIL_SEXP_IO_H__

#include "rose/context.h"
#include "rose/sexp.h"

void  r_write_string   (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_write_vector   (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_write_symbol   (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_write_pair     (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_write_null     (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_write_port     (rsexp     port,
                        rsexp     obj,
                        RContext* context);

void  r_display_string (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_display_vector (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_display_symbol (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_display_pair   (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_display_null   (rsexp     port,
                        rsexp     obj,
                        RContext* context);
void  r_display_port   (rsexp     port,
                        rsexp     obj,
                        RContext* context);

#endif  //  __ROSE_DETAIL_SEXP_IO_H__
